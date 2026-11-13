// OpenGL 4.1 core backend.
#include "gl_api.h"

#include "../shader_blob.h"
#include "rr/render/render_device.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>

namespace rr::render {
namespace {

using namespace gl;

struct GlBuffer {
    GLuint name = 0;
    BufferUsage usage = BufferUsage::Vertex;
};

struct GlTexture {
    GLuint name = 0;
    TextureDesc desc;
};

// Internal format for glTexImage2D / glCompressedTexImage2D.
GLenum InternalFormat(TextureFormat format) {
    switch (format) {
    case TextureFormat::RGBA8: return GL_RGBA8;
    case TextureFormat::RGBA8_sRGB: return GL_SRGB8_ALPHA8;
    case TextureFormat::ASTC_4x4: return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
    case TextureFormat::ASTC_4x4_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
    case TextureFormat::ASTC_5x5: return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
    case TextureFormat::ASTC_5x5_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
    case TextureFormat::ASTC_6x6: return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
    case TextureFormat::ASTC_6x6_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
    case TextureFormat::ASTC_8x8: return GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
    case TextureFormat::ASTC_8x8_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
    case TextureFormat::ASTC_10x10: return GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
    case TextureFormat::ASTC_10x10_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
    case TextureFormat::ASTC_12x12: return GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
    case TextureFormat::ASTC_12x12_sRGB: return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
    }
    return GL_RGBA8;
}

GLint ToGl(WrapMode wrap) {
    switch (wrap) {
    case WrapMode::Repeat: return static_cast<GLint>(GL_REPEAT);
    case WrapMode::ClampToEdge: return static_cast<GLint>(GL_CLAMP_TO_EDGE);
    case WrapMode::MirroredRepeat: return static_cast<GLint>(GL_MIRRORED_REPEAT);
    }
    return static_cast<GLint>(GL_REPEAT);
}

GLint MinFilter(const SamplerDesc& s) {
    const bool linear = s.filter == FilterMode::Linear;
    if (!s.useMips) return static_cast<GLint>(linear ? GL_LINEAR : GL_NEAREST);
    if (s.mipFilter == FilterMode::Linear) {
        return static_cast<GLint>(linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    }
    return static_cast<GLint>(linear ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
}

struct GlPipeline {
    GLuint program = 0;
    PipelineDesc desc;
};

struct AttributeFormat {
    GLint components;
    GLenum type;
    GLboolean normalized;
};

AttributeFormat ToGl(VertexFormat format) {
    switch (format) {
    case VertexFormat::Float2: return {2, GL_FLOAT, GL_FALSE};
    case VertexFormat::Float3: return {3, GL_FLOAT, GL_FALSE};
    case VertexFormat::Float4: return {4, GL_FLOAT, GL_FALSE};
    case VertexFormat::UByte4Norm: return {4, GL_UNSIGNED_BYTE, GL_TRUE};
    }
    return {4, GL_FLOAT, GL_FALSE};
}

// The draw-constant block is always bound at this uniform buffer slot, the
// larger DrawUniforms block at the next one.
constexpr GLuint kDrawConstantsBinding = 0;
constexpr GLuint kDrawUniformsBinding = 1;

class OpenGLDevice final : public RenderDevice {
public:
    OpenGLDevice(platform::Platform& platform, platform::OpenGLContext& context)
        : platform_(platform), context_(context) {}

    ~OpenGLDevice() override {
        for (auto& [id, t] : textures_) gl_.glDeleteTextures(1, &t.name);
        for (auto& [desc, sampler] : samplers_) gl_.glDeleteSamplers(1, &sampler);
        if (whiteTexture_) gl_.glDeleteTextures(1, &whiteTexture_);
        for (auto& [id, b] : buffers_) gl_.glDeleteBuffers(1, &b.name);
        for (auto& [id, p] : pipelines_) gl_.glDeleteProgram(p.program);
        if (constantsBuffer_) gl_.glDeleteBuffers(1, &constantsBuffer_);
        if (uniformsBuffer_) gl_.glDeleteBuffers(1, &uniformsBuffer_);
        if (vertexArray_) gl_.glDeleteVertexArrays(1, &vertexArray_);
    }

    bool Initialise() {
        if (const char* missing = gl_.Load([this](const char* n) { return context_.GetProcAddress(n); })) {
            std::fprintf(stderr, "OpenGL: missing entry point %s (needs OpenGL 4.1)\n", missing);
            return false;
        }
        deviceName_ = std::string(reinterpret_cast<const char*>(gl_.glGetString(GL_RENDERER))) +
                      " (OpenGL " +
                      reinterpret_cast<const char*>(gl_.glGetString(GL_VERSION)) + ")";

        // Core profile requires a bound vertex array object; we use one and
        // re-specify attributes when a pipeline/buffer pair changes.
        gl_.glGenVertexArrays(1, &vertexArray_);
        gl_.glBindVertexArray(vertexArray_);

        gl_.glGenBuffers(1, &constantsBuffer_);
        gl_.glBindBuffer(GL_UNIFORM_BUFFER, constantsBuffer_);
        gl_.glBufferData(GL_UNIFORM_BUFFER, kMaxDrawConstantsSize, nullptr, GL_DYNAMIC_DRAW);
        gl_.glBindBufferBase(GL_UNIFORM_BUFFER, kDrawConstantsBinding, constantsBuffer_);

        gl_.glGenBuffers(1, &uniformsBuffer_);
        gl_.glBindBuffer(GL_UNIFORM_BUFFER, uniformsBuffer_);
        gl_.glBufferData(GL_UNIFORM_BUFFER, kMaxDrawUniformsSize, nullptr, GL_DYNAMIC_DRAW);
        gl_.glBindBufferBase(GL_UNIFORM_BUFFER, kDrawUniformsBinding, uniformsBuffer_);

        gl_.glFrontFace(GL_CCW);

        GLint extensionCount = 0;
        gl_.glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
        for (GLint i = 0; i < extensionCount; ++i) {
            const auto* name = reinterpret_cast<const char*>(gl_.glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
            if (name && std::string(name) == "GL_KHR_texture_compression_astc_ldr") hasAstc_ = true;
        }

        // Slots without a texture sample this opaque white texel.
        const std::uint8_t white[4] = {255, 255, 255, 255};
        gl_.glGenTextures(1, &whiteTexture_);
        gl_.glBindTexture(GL_TEXTURE_2D, whiteTexture_);
        gl_.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        gl_.glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA8), 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        for (std::uint32_t slot = 0; slot < kMaxTextureSlots; ++slot) {
            boundTextures_[slot] = whiteTexture_;
            gl_.glActiveTexture(GL_TEXTURE0 + slot);
            gl_.glBindTexture(GL_TEXTURE_2D, whiteTexture_);
            gl_.glBindSampler(slot, Sampler(SamplerDesc{}));
        }
        gl_.glActiveTexture(GL_TEXTURE0);
        return true;
    }

    GraphicsApi Api() const override { return GraphicsApi::OpenGL; }
    std::string DeviceName() const override { return deviceName_; }

    BufferHandle CreateBuffer(BufferUsage usage, const void* data, std::size_t size) override {
        GlBuffer buffer;
        buffer.usage = usage;
        gl_.glGenBuffers(1, &buffer.name);
        const GLenum target = usage == BufferUsage::Index ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        gl_.glBindBuffer(target, buffer.name);
        gl_.glBufferData(target, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
        const BufferHandle handle{nextId_++};
        buffers_.emplace(handle.id, buffer);
        return handle;
    }

    void DestroyBuffer(BufferHandle handle) override {
        if (auto it = buffers_.find(handle.id); it != buffers_.end()) {
            gl_.glDeleteBuffers(1, &it->second.name);
            buffers_.erase(it);
        }
    }

    PipelineHandle CreatePipeline(const PipelineDesc& desc) override {
        const auto* program = shaders::FindProgram(desc.shaderProgram.c_str());
        if (!program) {
            std::fprintf(stderr, "OpenGL: unknown shader program '%s'\n", desc.shaderProgram.c_str());
            return {};
        }
        const GLuint vs = CompileShader(GL_VERTEX_SHADER, program->vertex->glsl, desc.shaderProgram);
        const GLuint fs = CompileShader(GL_FRAGMENT_SHADER, program->fragment->glsl, desc.shaderProgram);
        if (!vs || !fs) {
            if (vs) gl_.glDeleteShader(vs);
            if (fs) gl_.glDeleteShader(fs);
            return {};
        }

        GlPipeline pipeline;
        pipeline.desc = desc;
        pipeline.program = gl_.glCreateProgram();
        gl_.glAttachShader(pipeline.program, vs);
        gl_.glAttachShader(pipeline.program, fs);
        gl_.glLinkProgram(pipeline.program);
        gl_.glDeleteShader(vs);
        gl_.glDeleteShader(fs);

        GLint linked = GL_FALSE;
        gl_.glGetProgramiv(pipeline.program, GL_LINK_STATUS, &linked);
        if (!linked) {
            std::fprintf(stderr, "OpenGL: linking '%s' failed:\n%s\n", desc.shaderProgram.c_str(),
                         ProgramLog(pipeline.program).c_str());
            gl_.glDeleteProgram(pipeline.program);
            return {};
        }

        const GLuint block = gl_.glGetUniformBlockIndex(pipeline.program, "DrawConstants");
        if (block != GL_INVALID_INDEX) {
            gl_.glUniformBlockBinding(pipeline.program, block, kDrawConstantsBinding);
        }
        const GLuint uniforms = gl_.glGetUniformBlockIndex(pipeline.program, "DrawUniforms");
        if (uniforms != GL_INVALID_INDEX) {
            gl_.glUniformBlockBinding(pipeline.program, uniforms, kDrawUniformsBinding);
        }
        // Texture slot N is texture unit N (uTextureN in the shader).
        gl_.glUseProgram(pipeline.program);
        for (std::uint32_t slot = 0; slot < kMaxTextureSlots; ++slot) {
            const std::string uniform = "uTexture" + std::to_string(slot);
            const GLint location = gl_.glGetUniformLocation(pipeline.program, uniform.c_str());
            if (location >= 0) gl_.glUniform1i(location, static_cast<GLint>(slot));
        }
        gl_.glUseProgram(current_ ? current_->program : 0);

        const PipelineHandle handle{nextId_++};
        pipelines_.emplace(handle.id, std::move(pipeline));
        return handle;
    }

    void DestroyPipeline(PipelineHandle handle) override {
        if (auto it = pipelines_.find(handle.id); it != pipelines_.end()) {
            gl_.glDeleteProgram(it->second.program);
            pipelines_.erase(it);
        }
    }

    bool SupportsTextureFormat(TextureFormat format) const override {
        return !DescribeTextureFormat(format).compressed || hasAstc_;
    }

    TextureHandle CreateTexture(const TextureDesc& desc, std::span<const TextureLevelData> levels) override {
        if (!SupportsTextureFormat(desc.format) || desc.width <= 0 || desc.height <= 0 || desc.mipCount == 0 ||
            levels.size() < desc.mipCount) {
            std::fprintf(stderr, "OpenGL: unsupported texture (%s %dx%d)\n", ToString(desc.format), desc.width,
                         desc.height);
            return {};
        }
        for (std::uint32_t level = 0; level < desc.mipCount; ++level) {
            const int w = std::max(1, desc.width >> level);
            const int h = std::max(1, desc.height >> level);
            if (levels[level].size != TextureLevelBytes(desc.format, w, h) || !levels[level].data) {
                std::fprintf(stderr, "OpenGL: texture level %u has the wrong size\n", level);
                return {};
            }
        }
        GlTexture texture;
        texture.desc = desc;
        gl_.glGenTextures(1, &texture.name);
        gl_.glActiveTexture(GL_TEXTURE0);
        gl_.glBindTexture(GL_TEXTURE_2D, texture.name);
        gl_.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(desc.mipCount - 1));
        const GLenum internal = InternalFormat(desc.format);
        const bool compressed = DescribeTextureFormat(desc.format).compressed;
        for (std::uint32_t level = 0; level < desc.mipCount; ++level) {
            const int w = std::max(1, desc.width >> level);
            const int h = std::max(1, desc.height >> level);
            if (compressed) {
                gl_.glCompressedTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), internal, w, h, 0,
                                           static_cast<GLsizei>(levels[level].size), levels[level].data);
            } else {
                gl_.glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), static_cast<GLint>(internal), w, h, 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, levels[level].data);
            }
        }
        gl_.glBindTexture(GL_TEXTURE_2D, boundTextures_[0]);
        if (const GLenum error = gl_.glGetError(); error != GL_NO_ERROR) {
            std::fprintf(stderr, "OpenGL: creating a %s texture failed (error 0x%x)\n", ToString(desc.format), error);
            gl_.glDeleteTextures(1, &texture.name);
            return {};
        }
        const TextureHandle handle{nextId_++};
        textures_.emplace(handle.id, texture);
        return handle;
    }

    void DestroyTexture(TextureHandle handle) override {
        auto it = textures_.find(handle.id);
        if (it == textures_.end()) return;
        for (std::uint32_t slot = 0; slot < kMaxTextureSlots; ++slot) {
            if (boundTextures_[slot] == it->second.name) {
                boundTextures_[slot] = whiteTexture_;
                gl_.glActiveTexture(GL_TEXTURE0 + slot);
                gl_.glBindTexture(GL_TEXTURE_2D, whiteTexture_);
            }
        }
        gl_.glActiveTexture(GL_TEXTURE0);
        gl_.glDeleteTextures(1, &it->second.name);
        textures_.erase(it);
    }

    std::optional<Image> ReadTexture(TextureHandle handle, std::uint32_t mipLevel) override {
        auto it = textures_.find(handle.id);
        if (it == textures_.end() || mipLevel >= it->second.desc.mipCount) return std::nullopt;
        Image image;
        image.width = std::max(1, it->second.desc.width >> mipLevel);
        image.height = std::max(1, it->second.desc.height >> mipLevel);
        image.rgba.resize(static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height) * 4);
        gl_.glActiveTexture(GL_TEXTURE0);
        gl_.glBindTexture(GL_TEXTURE_2D, it->second.name);
        gl_.glPixelStorei(GL_PACK_ALIGNMENT, 1);
        gl_.glGetTexImage(GL_TEXTURE_2D, static_cast<GLint>(mipLevel), GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
        gl_.glBindTexture(GL_TEXTURE_2D, boundTextures_[0]);
        if (gl_.glGetError() != GL_NO_ERROR) return std::nullopt;
        return image;
    }

    bool BeginFrame() override {
        platform_.GetDrawableSize(width_, height_);
        return width_ > 0 && height_ > 0;
    }

    void BeginRenderPass(const ClearValues& clear) override {
        gl_.glViewport(0, 0, width_, height_);
        // Depth writes must be enabled for the depth clear to take effect.
        gl_.glDepthMask(GL_TRUE);
        gl_.glClearColor(clear.color[0], clear.color[1], clear.color[2], clear.color[3]);
        gl_.glClearDepthf(clear.depth);
        gl_.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void BindPipeline(PipelineHandle handle) override {
        auto it = pipelines_.find(handle.id);
        if (it == pipelines_.end()) {
            current_ = nullptr;
            return;
        }
        current_ = &it->second;
        const PipelineDesc& d = current_->desc;
        gl_.glUseProgram(current_->program);

        if (d.depthTest) {
            gl_.glEnable(GL_DEPTH_TEST);
            gl_.glDepthFunc(GL_LEQUAL);
        } else {
            gl_.glDisable(GL_DEPTH_TEST);
        }
        gl_.glDepthMask(d.depthWrite ? GL_TRUE : GL_FALSE);

        if (d.cull == CullMode::None) {
            gl_.glDisable(GL_CULL_FACE);
        } else {
            gl_.glEnable(GL_CULL_FACE);
            gl_.glCullFace(d.cull == CullMode::Back ? GL_BACK : GL_FRONT);
        }

        switch (d.blend) {
        case BlendMode::Opaque: gl_.glDisable(GL_BLEND); break;
        case BlendMode::AlphaBlend:
            gl_.glEnable(GL_BLEND);
            gl_.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            gl_.glEnable(GL_BLEND);
            gl_.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::Premultiplied:
            gl_.glEnable(GL_BLEND);
            gl_.glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
        attributesDirty_ = true;
    }

    void BindVertexBuffer(BufferHandle handle) override {
        auto it = buffers_.find(handle.id);
        vertexBuffer_ = it != buffers_.end() ? it->second.name : 0;
        attributesDirty_ = true;
    }

    void BindIndexBuffer(BufferHandle handle, IndexType type) override {
        auto it = buffers_.find(handle.id);
        gl_.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it != buffers_.end() ? it->second.name : 0);
        indexType_ = type;
    }

    void SetDrawConstants(const void* data, std::size_t size) override {
        if (size > kMaxDrawConstantsSize) size = kMaxDrawConstantsSize;
        gl_.glBindBuffer(GL_UNIFORM_BUFFER, constantsBuffer_);
        gl_.glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }

    void SetDrawUniforms(const void* data, std::size_t size) override {
        if (size > kMaxDrawUniformsSize) size = kMaxDrawUniformsSize;
        // The driver keeps the old contents for draws already issued.
        gl_.glBindBuffer(GL_UNIFORM_BUFFER, uniformsBuffer_);
        gl_.glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
    }

    void BindTexture(std::uint32_t slot, TextureHandle handle, const SamplerDesc& sampler) override {
        if (slot >= kMaxTextureSlots) return;
        auto it = textures_.find(handle.id);
        const GLuint name = it != textures_.end() ? it->second.name : whiteTexture_;
        gl_.glActiveTexture(GL_TEXTURE0 + slot);
        gl_.glBindTexture(GL_TEXTURE_2D, name);
        gl_.glBindSampler(slot, Sampler(sampler));
        gl_.glActiveTexture(GL_TEXTURE0);
        boundTextures_[slot] = name;
    }

    void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override {
        if (!PrepareDraw()) return;
        gl_.glDrawArrays(Topology(), static_cast<GLint>(firstVertex),
                         static_cast<GLsizei>(vertexCount));
    }

    void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex,
                     std::int32_t vertexOffset) override {
        if (!PrepareDraw()) return;
        const bool wide = indexType_ == IndexType::UInt32;
        const std::size_t byteOffset = firstIndex * (wide ? 4u : 2u);
        gl_.glDrawElementsBaseVertex(Topology(), static_cast<GLsizei>(indexCount),
                                     wide ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
                                     reinterpret_cast<const void*>(byteOffset), vertexOffset);
    }

    void EndRenderPass() override {}

    void EndFrame() override {
        if (captureRequested_) {
            captureRequested_ = false;
            capture_ = ReadFramebuffer();
        }
        context_.SwapBuffers();
    }

    void GetRenderSize(int& width, int& height) const override {
        width = width_;
        height = height_;
    }

    void RequestCapture() override { captureRequested_ = true; }

    std::optional<Image> TakeCapture() override {
        auto image = std::move(capture_);
        capture_.reset();
        return image;
    }

private:
    GLuint CompileShader(GLenum stage, const char* body, const std::string& programName) {
        const char* sources[] = {"#version 410 core\n", body};
        const GLuint shader = gl_.glCreateShader(stage);
        gl_.glShaderSource(shader, 2, sources, nullptr);
        gl_.glCompileShader(shader);
        GLint ok = GL_FALSE;
        gl_.glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint length = 0;
            gl_.glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
            gl_.glGetShaderInfoLog(shader, length, nullptr, log.data());
            std::fprintf(stderr, "OpenGL: compiling %s shader of '%s' failed:\n%s\n",
                         stage == GL_VERTEX_SHADER ? "vertex" : "fragment", programName.c_str(),
                         log.c_str());
            gl_.glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    std::string ProgramLog(GLuint program) {
        GLint length = 0;
        gl_.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
        gl_.glGetProgramInfoLog(program, length, nullptr, log.data());
        return log;
    }

    // Sampler objects are shared between all draws with the same settings.
    GLuint Sampler(const SamplerDesc& desc) {
        for (const auto& [d, sampler] : samplers_) {
            if (d == desc) return sampler;
        }
        GLuint sampler = 0;
        gl_.glGenSamplers(1, &sampler);
        gl_.glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, MinFilter(desc));
        gl_.glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER,
                                static_cast<GLint>(desc.filter == FilterMode::Linear ? GL_LINEAR : GL_NEAREST));
        gl_.glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, ToGl(desc.wrapU));
        gl_.glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, ToGl(desc.wrapV));
        samplers_.emplace_back(desc, sampler);
        return sampler;
    }

    GLenum Topology() const {
        return current_->desc.topology == PrimitiveTopology::LineList ? GL_LINES : GL_TRIANGLES;
    }

    // Applies the current pipeline's vertex layout to the bound vertex buffer.
    bool PrepareDraw() {
        if (!current_ || !vertexBuffer_) return false;
        if (attributesDirty_) {
            for (GLuint loc : enabledAttributes_) gl_.glDisableVertexAttribArray(loc);
            enabledAttributes_.clear();

            const VertexLayout& layout = current_->desc.vertexLayout;
            gl_.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
            for (const VertexAttribute& a : layout.attributes) {
                const AttributeFormat f = ToGl(a.format);
                gl_.glEnableVertexAttribArray(a.location);
                gl_.glVertexAttribPointer(a.location, f.components, f.type, f.normalized,
                                          static_cast<GLsizei>(layout.stride),
                                          reinterpret_cast<const void*>(std::size_t{a.offset}));
                enabledAttributes_.push_back(a.location);
            }
            attributesDirty_ = false;
        }
        return true;
    }

    Image ReadFramebuffer() {
        Image image;
        image.width = width_;
        image.height = height_;
        const auto rowBytes = static_cast<std::size_t>(width_) * 4;
        std::vector<std::uint8_t> bottomUp(rowBytes * static_cast<std::size_t>(height_));
        gl_.glPixelStorei(GL_PACK_ALIGNMENT, 1);
        gl_.glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, bottomUp.data());
        // OpenGL returns rows bottom-up; the capture convention is top-down.
        image.rgba.resize(bottomUp.size());
        for (int y = 0; y < height_; ++y) {
            const auto src = static_cast<std::size_t>(height_ - 1 - y) * rowBytes;
            std::copy_n(bottomUp.begin() + static_cast<std::ptrdiff_t>(src), rowBytes,
                        image.rgba.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(y) * rowBytes));
        }
        return image;
    }

    platform::Platform& platform_;
    platform::OpenGLContext& context_;
    gl::Api gl_;
    std::string deviceName_;

    std::unordered_map<std::uint32_t, GlBuffer> buffers_;
    std::unordered_map<std::uint32_t, GlPipeline> pipelines_;
    std::unordered_map<std::uint32_t, GlTexture> textures_;
    std::vector<std::pair<SamplerDesc, GLuint>> samplers_;
    std::array<GLuint, kMaxTextureSlots> boundTextures_{};
    GLuint whiteTexture_ = 0;
    bool hasAstc_ = false;
    std::uint32_t nextId_ = 1;

    GLuint vertexArray_ = 0;
    GLuint constantsBuffer_ = 0;
    GLuint uniformsBuffer_ = 0;

    int width_ = 0;
    int height_ = 0;
    const GlPipeline* current_ = nullptr;
    GLuint vertexBuffer_ = 0;
    IndexType indexType_ = IndexType::UInt16;
    bool attributesDirty_ = true;
    std::vector<GLuint> enabledAttributes_;

    bool captureRequested_ = false;
    std::optional<Image> capture_;
};

} // namespace

std::unique_ptr<RenderDevice> CreateOpenGLDevice(platform::Platform& platform) {
    platform::OpenGLContext* context = platform.GetOpenGLContext();
    if (!context) {
        std::fprintf(stderr, "OpenGL: the window was not created with an OpenGL context\n");
        return nullptr;
    }
    auto device = std::make_unique<OpenGLDevice>(platform, *context);
    if (!device->Initialise()) {
        return nullptr;
    }
    return device;
}

} // namespace rr::render
