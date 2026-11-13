// Vulkan backend (Vulkan 1.0 core + VK_KHR_swapchain).
//
// Kept deliberately straightforward for now: one render pass onto the
// swapchain, host-visible buffers, two frames in flight. Streaming uploads,
// a memory allocator and off-screen passes come later with the real renderer.
//
// Textures live in device-local images uploaded through a staging buffer and a
// one-shot command buffer. Every draw sees kMaxTextureSlots combined image
// samplers in descriptor set 0 (bindings 0..3); a descriptor set is written from
// the currently bound textures whenever they change, from per-frame pools that
// are reset once the frame's fence has signalled.
//
// The DrawUniforms block (binding 4) is a dynamic uniform buffer: each frame in
// flight owns host-visible chunks that SetDrawUniforms fills front to back, and
// draws select their slice with a dynamic offset.
#include "../shader_blob.h"
#include "rr/render/render_device.h"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace rr::render {
namespace {

constexpr std::uint32_t kFramesInFlight = 2;

bool Check(VkResult result, const char* what) {
    if (result == VK_SUCCESS) return true;
    std::fprintf(stderr, "Vulkan: %s failed (VkResult %d)\n", what, static_cast<int>(result));
    return false;
}

VkFormat ToVk(VertexFormat format) {
    switch (format) {
    case VertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case VertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
    case VertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case VertexFormat::UByte4Norm: return VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

bool HasExtension(const std::vector<VkExtensionProperties>& list, const char* name) {
    return std::any_of(list.begin(), list.end(),
                       [&](const VkExtensionProperties& e) { return std::strcmp(e.extensionName, name) == 0; });
}

struct VkBufferResource {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct VkTextureResource {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    TextureDesc desc;
};

VkFormat ToVk(TextureFormat format) {
    switch (format) {
    case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::RGBA8_sRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::ASTC_4x4: return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    case TextureFormat::ASTC_4x4_sRGB: return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    case TextureFormat::ASTC_5x5: return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    case TextureFormat::ASTC_5x5_sRGB: return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    case TextureFormat::ASTC_6x6: return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    case TextureFormat::ASTC_6x6_sRGB: return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    case TextureFormat::ASTC_8x8: return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    case TextureFormat::ASTC_8x8_sRGB: return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    case TextureFormat::ASTC_10x10: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    case TextureFormat::ASTC_10x10_sRGB: return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    case TextureFormat::ASTC_12x12: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    case TextureFormat::ASTC_12x12_sRGB: return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
    }
    return VK_FORMAT_R8G8B8A8_UNORM;
}

VkSamplerAddressMode ToVk(WrapMode wrap) {
    switch (wrap) {
    case WrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case WrapMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case WrapMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

struct VkPipelineResource {
    VkPipeline pipeline = VK_NULL_HANDLE;
    PipelineDesc desc; // kept so the pipeline can be rebuilt if the render pass changes
};

// Host-visible memory for DrawUniforms contents, filled front to back.
struct UniformChunk {
    VkBufferResource buffer;
    std::uint8_t* mapped = nullptr;
};

constexpr std::size_t kUniformChunkSize = 1u << 20;
constexpr std::uint32_t kDrawUniformsBinding = kMaxTextureSlots;

struct FrameResources {
    VkCommandBuffer commands = VK_NULL_HANDLE;
    std::vector<UniformChunk> uniformChunks; // grown on demand, rewound per frame
    std::size_t activeChunk = 0;
    std::size_t chunkOffset = 0;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;
    std::vector<VkDescriptorPool> descriptorPools; // grown on demand, reset per frame
    std::size_t activePool = 0;
};

constexpr std::uint32_t kSetsPerPool = 256;

class VulkanDevice final : public RenderDevice {
public:
    explicit VulkanDevice(platform::Platform& platform) : platform_(platform) {}

    ~VulkanDevice() override {
        if (device_) vkDeviceWaitIdle(device_);
        for (auto& [id, b] : buffers_) DestroyBufferResource(b);
        for (auto& [id, t] : textures_) DestroyTextureResource(t);
        DestroyTextureResource(whiteTexture_);
        for (auto& [desc, sampler] : samplers_) vkDestroySampler(device_, sampler, nullptr);
        for (auto& [id, p] : pipelines_) vkDestroyPipeline(device_, p.pipeline, nullptr);
        DestroySwapchain();
        if (renderPass_) vkDestroyRenderPass(device_, renderPass_, nullptr);
        if (pipelineLayout_) vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
        if (textureSetLayout_) vkDestroyDescriptorSetLayout(device_, textureSetLayout_, nullptr);
        for (auto& f : frames_) {
            for (UniformChunk& chunk : f.uniformChunks) {
                if (chunk.mapped) vkUnmapMemory(device_, chunk.buffer.memory);
                DestroyBufferResource(chunk.buffer);
            }
            for (VkDescriptorPool pool : f.descriptorPools) vkDestroyDescriptorPool(device_, pool, nullptr);
            if (f.imageAvailable) vkDestroySemaphore(device_, f.imageAvailable, nullptr);
            if (f.inFlight) vkDestroyFence(device_, f.inFlight, nullptr);
        }
        if (commandPool_) vkDestroyCommandPool(device_, commandPool_, nullptr);
        if (device_) vkDestroyDevice(device_, nullptr);
        if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
        if (debugMessenger_) {
            auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroy) destroy(instance_, debugMessenger_, nullptr);
        }
        if (instance_) vkDestroyInstance(instance_, nullptr);
    }

    bool Initialise() {
        const platform::VulkanSurfaceProvider* provider = platform_.GetVulkanSurfaceProvider();
        if (!provider) {
            std::fprintf(stderr, "Vulkan: the window was not created for Vulkan\n");
            return false;
        }
        return CreateInstance(*provider) && CreateSurface(*provider) && PickPhysicalDevice() &&
               CreateLogicalDevice() && CreateCommandResources() && CreatePipelineLayout() &&
               CreateSwapchain() && CreateWhiteTexture();
    }

    GraphicsApi Api() const override { return GraphicsApi::Vulkan; }
    std::string DeviceName() const override { return deviceName_; }

    // Resources ------------------------------------------------------------

    BufferHandle CreateBuffer(BufferUsage usage, const void* data, std::size_t size) override {
        VkBufferResource resource;
        const VkBufferUsageFlags flags = usage == BufferUsage::Index
                                             ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                             : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (!CreateHostBuffer(size, flags, resource)) return {};
        void* mapped = nullptr;
        if (!Check(vkMapMemory(device_, resource.memory, 0, size, 0, &mapped), "vkMapMemory")) {
            DestroyBufferResource(resource);
            return {};
        }
        std::memcpy(mapped, data, size);
        vkUnmapMemory(device_, resource.memory);

        const BufferHandle handle{nextId_++};
        buffers_.emplace(handle.id, resource);
        return handle;
    }

    void DestroyBuffer(BufferHandle handle) override {
        auto it = buffers_.find(handle.id);
        if (it == buffers_.end()) return;
        // TODO: defer destruction until the GPU is done with the frame instead of stalling.
        vkDeviceWaitIdle(device_);
        DestroyBufferResource(it->second);
        buffers_.erase(it);
    }

    PipelineHandle CreatePipeline(const PipelineDesc& desc) override {
        VkPipelineResource resource;
        resource.desc = desc;
        if (!BuildPipeline(resource)) return {};
        const PipelineHandle handle{nextId_++};
        pipelines_.emplace(handle.id, std::move(resource));
        return handle;
    }

    void DestroyPipeline(PipelineHandle handle) override {
        auto it = pipelines_.find(handle.id);
        if (it == pipelines_.end()) return;
        vkDeviceWaitIdle(device_);
        vkDestroyPipeline(device_, it->second.pipeline, nullptr);
        pipelines_.erase(it);
    }

    bool SupportsTextureFormat(TextureFormat format) const override {
        if (!DescribeTextureFormat(format).compressed) return true;
        if (!astcEnabled_) return false;
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice_, ToVk(format), &props);
        constexpr VkFormatFeatureFlags needed =
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        return (props.optimalTilingFeatures & needed) == needed;
    }

    TextureHandle CreateTexture(const TextureDesc& desc, std::span<const TextureLevelData> levels) override {
        if (!SupportsTextureFormat(desc.format) || desc.width <= 0 || desc.height <= 0 || desc.mipCount == 0 ||
            levels.size() < desc.mipCount) {
            std::fprintf(stderr, "Vulkan: unsupported texture (%s %dx%d)\n", ToString(desc.format), desc.width,
                         desc.height);
            return {};
        }
        VkTextureResource texture;
        if (!CreateTextureResource(desc, levels, texture)) return {};
        const TextureHandle handle{nextId_++};
        textures_.emplace(handle.id, texture);
        return handle;
    }

    void DestroyTexture(TextureHandle handle) override {
        auto it = textures_.find(handle.id);
        if (it == textures_.end()) return;
        vkDeviceWaitIdle(device_);
        for (std::uint32_t slot = 0; slot < kMaxTextureSlots; ++slot) {
            if (boundViews_[slot] == it->second.view) {
                boundViews_[slot] = whiteTexture_.view;
                descriptorsDirty_ = true;
            }
        }
        DestroyTextureResource(it->second);
        textures_.erase(it);
    }

    std::optional<Image> ReadTexture(TextureHandle handle, std::uint32_t mipLevel) override {
        auto it = textures_.find(handle.id);
        if (it == textures_.end() || mipLevel >= it->second.desc.mipCount) return std::nullopt;
        return ReadTextureLevel(it->second, mipLevel);
    }

    // Frame ----------------------------------------------------------------

    bool BeginFrame() override {
        int width = 0, height = 0;
        platform_.GetDrawableSize(width, height);
        if (width == 0 || height == 0) return false;
        if (swapchainDirty_ || static_cast<std::uint32_t>(width) != extent_.width ||
            static_cast<std::uint32_t>(height) != extent_.height) {
            if (!RecreateSwapchain()) return false;
        }

        FrameResources& frame = frames_[frameIndex_];
        vkWaitForFences(device_, 1, &frame.inFlight, VK_TRUE, UINT64_MAX);

        VkResult acquired = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                                  frame.imageAvailable, VK_NULL_HANDLE, &imageIndex_);
        if (acquired == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchainDirty_ = true;
            return false;
        }
        if (acquired != VK_SUCCESS && acquired != VK_SUBOPTIMAL_KHR) {
            Check(acquired, "vkAcquireNextImageKHR");
            return false;
        }

        vkResetFences(device_, 1, &frame.inFlight);
        for (VkDescriptorPool pool : frame.descriptorPools) vkResetDescriptorPool(device_, pool, 0);
        frame.activePool = 0;
        frame.activeChunk = 0;
        frame.chunkOffset = 0;
        uniformOffset_ = 0;
        descriptorsDirty_ = true; // a new command buffer has no descriptor sets bound
        vkResetCommandBuffer(frame.commands, 0);
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(frame.commands, &begin);
        cmd_ = frame.commands;
        return true;
    }

    void BeginRenderPass(const ClearValues& clear) override {
        std::array<VkClearValue, 2> values{};
        std::memcpy(values[0].color.float32, clear.color, sizeof(clear.color));
        values[1].depthStencil = {clear.depth, 0};

        VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        info.renderPass = renderPass_;
        info.framebuffer = framebuffers_[imageIndex_];
        info.renderArea.extent = extent_;
        info.clearValueCount = static_cast<std::uint32_t>(values.size());
        info.pClearValues = values.data();
        vkCmdBeginRenderPass(cmd_, &info, VK_SUBPASS_CONTENTS_INLINE);

        const VkViewport viewport{0.0f, 0.0f, static_cast<float>(extent_.width),
                                  static_cast<float>(extent_.height), 0.0f, 1.0f};
        const VkRect2D scissor{{0, 0}, extent_};
        vkCmdSetViewport(cmd_, 0, 1, &viewport);
        vkCmdSetScissor(cmd_, 0, 1, &scissor);
    }

    void BindPipeline(PipelineHandle handle) override {
        auto it = pipelines_.find(handle.id);
        pipelineBound_ = it != pipelines_.end();
        if (pipelineBound_) vkCmdBindPipeline(cmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, it->second.pipeline);
    }

    void BindVertexBuffer(BufferHandle handle) override {
        auto it = buffers_.find(handle.id);
        if (it == buffers_.end()) return;
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd_, 0, 1, &it->second.buffer, &offset);
    }

    void BindIndexBuffer(BufferHandle handle, IndexType type) override {
        auto it = buffers_.find(handle.id);
        if (it == buffers_.end()) return;
        vkCmdBindIndexBuffer(cmd_, it->second.buffer, 0,
                             type == IndexType::UInt32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
    }

    void SetDrawConstants(const void* data, std::size_t size) override {
        size = std::min(size, kMaxDrawConstantsSize);
        vkCmdPushConstants(cmd_, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0, static_cast<std::uint32_t>(size), data);
    }

    void SetDrawUniforms(const void* data, std::size_t size) override {
        size = std::min(size, kMaxDrawUniformsSize);
        FrameResources& frame = frames_[frameIndex_];
        const std::size_t alignment = std::max<std::size_t>(uniformAlignment_, 16);
        std::size_t offset = (frame.chunkOffset + alignment - 1) / alignment * alignment;
        // The descriptor always covers kMaxDrawUniformsSize bytes from the offset.
        if (offset + kMaxDrawUniformsSize > kUniformChunkSize) {
            ++frame.activeChunk;
            offset = 0;
            descriptorsDirty_ = true; // the set must point at the new chunk
        }
        if (!EnsureUniformChunk(frame)) return;
        std::memcpy(frame.uniformChunks[frame.activeChunk].mapped + offset, data, size);
        frame.chunkOffset = offset + size;
        uniformOffset_ = static_cast<std::uint32_t>(offset);
        uniformsDirty_ = true;
    }

    void BindTexture(std::uint32_t slot, TextureHandle handle, const SamplerDesc& sampler) override {
        if (slot >= kMaxTextureSlots) return;
        auto it = textures_.find(handle.id);
        boundViews_[slot] = it != textures_.end() ? it->second.view : whiteTexture_.view;
        boundSamplers_[slot] = Sampler(sampler);
        descriptorsDirty_ = true;
    }

    void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex) override {
        if (pipelineBound_ && FlushDescriptors()) vkCmdDraw(cmd_, vertexCount, 1, firstVertex, 0);
    }

    void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex,
                     std::int32_t vertexOffset) override {
        if (pipelineBound_ && FlushDescriptors()) vkCmdDrawIndexed(cmd_, indexCount, 1, firstIndex, vertexOffset, 0);
    }

    void EndRenderPass() override { vkCmdEndRenderPass(cmd_); }

    void EndFrame() override {
        FrameResources& frame = frames_[frameIndex_];
        const bool capturing = captureRequested_ && canCapture_;
        VkBufferResource readback;
        if (capturing) {
            captureRequested_ = false;
            if (!RecordCapture(readback)) readback = {};
        }
        vkEndCommandBuffer(cmd_);

        const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSemaphore renderFinished = renderFinished_[imageIndex_];
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &frame.imageAvailable;
        submit.pWaitDstStageMask = &waitStage;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd_;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderFinished;
        Check(vkQueueSubmit(queue_, 1, &submit, frame.inFlight), "vkQueueSubmit");

        if (readback.buffer) {
            vkWaitForFences(device_, 1, &frame.inFlight, VK_TRUE, UINT64_MAX);
            capture_ = ReadCapture(readback);
            DestroyBufferResource(readback);
        }

        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &renderFinished;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain_;
        present.pImageIndices = &imageIndex_;
        const VkResult presented = vkQueuePresentKHR(queue_, &present);
        if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR) {
            swapchainDirty_ = true;
        } else {
            Check(presented, "vkQueuePresentKHR");
        }

        frameIndex_ = (frameIndex_ + 1) % kFramesInFlight;
        cmd_ = VK_NULL_HANDLE;
    }

    void GetRenderSize(int& width, int& height) const override {
        width = static_cast<int>(extent_.width);
        height = static_cast<int>(extent_.height);
    }

    void RequestCapture() override {
        if (!canCapture_) {
            std::fprintf(stderr, "Vulkan: this swapchain can't be read back; capture skipped\n");
        }
        captureRequested_ = true;
    }

    std::optional<Image> TakeCapture() override {
        auto image = std::move(capture_);
        capture_.reset();
        return image;
    }

private:
    // Setup ----------------------------------------------------------------

    bool CreateInstance(const platform::VulkanSurfaceProvider& provider) {
        std::vector<const char*> extensions = provider.RequiredInstanceExtensions();

        std::uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> available(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

        // Validation is opt-in so release users never pay for it.
        std::vector<const char*> layers;
        const char* env = std::getenv("RR_VULKAN_VALIDATION");
        if (env && env[0] == '1') {
            std::uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> layerProps(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data());
            for (const auto& l : layerProps) {
                if (std::strcmp(l.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                    layers.push_back("VK_LAYER_KHRONOS_validation");
                }
            }
            if (layers.empty()) {
                std::fprintf(stderr, "Vulkan: validation requested but the layer isn't installed\n");
            } else if (HasExtension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                wantDebugMessenger_ = true;
            }
        }

        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app.pApplicationName = "Ranger Rumble";
        app.pEngineName = "rr";
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        info.pApplicationInfo = &app;
        info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
        info.ppEnabledLayerNames = layers.data();
        if (!Check(vkCreateInstance(&info, nullptr, &instance_), "vkCreateInstance")) return false;

        if (wantDebugMessenger_) CreateDebugMessenger();
        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                        VkDebugUtilsMessageTypeFlagsEXT,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* data,
                                                        void*) {
        std::fprintf(stderr, "Vulkan validation: %s\n", data->pMessage);
        return VK_FALSE;
    }

    void CreateDebugMessenger() {
        auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));
        if (!create) return;
        VkDebugUtilsMessengerCreateInfoEXT info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        info.pfnUserCallback = &DebugCallback;
        create(instance_, &info, nullptr, &debugMessenger_);
    }

    bool CreateSurface(const platform::VulkanSurfaceProvider& provider) {
        std::uint64_t surface = 0;
        if (!provider.CreateSurface(instance_, &surface)) return false;
        surface_ = (VkSurfaceKHR)(surface);
        return true;
    }

    bool PickPhysicalDevice() {
        std::uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance_, &count, nullptr);
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance_, &count, devices.data());

        int bestScore = -1;
        for (VkPhysicalDevice candidate : devices) {
            std::uint32_t extCount = 0;
            vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extCount, nullptr);
            std::vector<VkExtensionProperties> exts(extCount);
            vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extCount, exts.data());
            if (!HasExtension(exts, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;

            std::uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, families.data());

            for (std::uint32_t i = 0; i < familyCount; ++i) {
                VkBool32 canPresent = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface_, &canPresent);
                if (!(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) || !canPresent) continue;

                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(candidate, &props);
                const int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU     ? 3
                                  : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2
                                                                                               : 1;
                if (score > bestScore) {
                    bestScore = score;
                    uniformAlignment_ = static_cast<std::size_t>(props.limits.minUniformBufferOffsetAlignment);
                    physicalDevice_ = candidate;
                    queueFamily_ = i;
                    deviceName_ = std::string(props.deviceName) + " (Vulkan " +
                                  std::to_string(VK_VERSION_MAJOR(props.apiVersion)) + "." +
                                  std::to_string(VK_VERSION_MINOR(props.apiVersion)) + ")";
                }
                break;
            }
        }
        if (!physicalDevice_) {
            std::fprintf(stderr, "Vulkan: no GPU can render and present to this window\n");
            return false;
        }
        vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
        return true;
    }

    bool CreateLogicalDevice() {
        const float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueInfo.queueFamilyIndex = queueFamily_;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        // Compressed mobile formats are optional; enable ASTC where the GPU has it.
        VkPhysicalDeviceFeatures supported{};
        vkGetPhysicalDeviceFeatures(physicalDevice_, &supported);
        VkPhysicalDeviceFeatures enabled{};
        enabled.textureCompressionASTC_LDR = supported.textureCompressionASTC_LDR;
        astcEnabled_ = supported.textureCompressionASTC_LDR == VK_TRUE;

        const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        info.queueCreateInfoCount = 1;
        info.pQueueCreateInfos = &queueInfo;
        info.enabledExtensionCount = 1;
        info.ppEnabledExtensionNames = extensions;
        info.pEnabledFeatures = &enabled;
        if (!Check(vkCreateDevice(physicalDevice_, &info, nullptr, &device_), "vkCreateDevice")) {
            return false;
        }
        vkGetDeviceQueue(device_, queueFamily_, 0, &queue_);
        return true;
    }

    bool CreateCommandResources() {
        VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool.queueFamilyIndex = queueFamily_;
        if (!Check(vkCreateCommandPool(device_, &pool, nullptr, &commandPool_), "vkCreateCommandPool")) {
            return false;
        }

        for (FrameResources& frame : frames_) {
            VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            alloc.commandPool = commandPool_;
            alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;
            VkSemaphoreCreateInfo sem{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fence.flags = VK_FENCE_CREATE_SIGNALED_BIT; // first wait must not block
            if (!Check(vkAllocateCommandBuffers(device_, &alloc, &frame.commands), "vkAllocateCommandBuffers") ||
                !Check(vkCreateSemaphore(device_, &sem, nullptr, &frame.imageAvailable), "vkCreateSemaphore") ||
                !Check(vkCreateFence(device_, &fence, nullptr, &frame.inFlight), "vkCreateFence")) {
                return false;
            }
        }
        return true;
    }

    bool CreatePipelineLayout() {
        std::array<VkDescriptorSetLayoutBinding, kMaxTextureSlots + 1> bindings{};
        for (std::uint32_t i = 0; i < kMaxTextureSlots; ++i) {
            bindings[i].binding = i;
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        bindings[kDrawUniformsBinding].binding = kDrawUniformsBinding;
        bindings[kDrawUniformsBinding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[kDrawUniformsBinding].descriptorCount = 1;
        bindings[kDrawUniformsBinding].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo setInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        setInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
        setInfo.pBindings = bindings.data();
        if (!Check(vkCreateDescriptorSetLayout(device_, &setInfo, nullptr, &textureSetLayout_),
                   "vkCreateDescriptorSetLayout")) {
            return false;
        }

        VkPushConstantRange range{};
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        range.size = static_cast<std::uint32_t>(kMaxDrawConstantsSize);
        VkPipelineLayoutCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        info.setLayoutCount = 1;
        info.pSetLayouts = &textureSetLayout_;
        info.pushConstantRangeCount = 1;
        info.pPushConstantRanges = &range;
        return Check(vkCreatePipelineLayout(device_, &info, nullptr, &pipelineLayout_),
                     "vkCreatePipelineLayout");
    }

    // Swapchain ------------------------------------------------------------

    bool CreateSwapchain() {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &caps);

        int width = 0, height = 0;
        platform_.GetDrawableSize(width, height);
        if (caps.currentExtent.width != UINT32_MAX) {
            extent_ = caps.currentExtent;
        } else {
            extent_.width = std::clamp(static_cast<std::uint32_t>(width), caps.minImageExtent.width,
                                       caps.maxImageExtent.width);
            extent_.height = std::clamp(static_cast<std::uint32_t>(height), caps.minImageExtent.height,
                                        caps.maxImageExtent.height);
        }
        if (extent_.width == 0 || extent_.height == 0) return false;

        // Pick a plain 8-bit UNORM format so output matches the OpenGL backend.
        std::uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats.data());
        surfaceFormat_ = formats.front();
        for (const auto& f : formats) {
            if ((f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) &&
                f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat_ = f;
                break;
            }
        }

        std::uint32_t imageCount = std::max(caps.minImageCount + 1, 2u);
        if (caps.maxImageCount != 0) imageCount = std::min(imageCount, caps.maxImageCount);

        canCapture_ = (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0 &&
                      (surfaceFormat_.format == VK_FORMAT_B8G8R8A8_UNORM ||
                       surfaceFormat_.format == VK_FORMAT_R8G8B8A8_UNORM);

        VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        info.surface = surface_;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat_.format;
        info.imageColorSpace = surfaceFormat_.colorSpace;
        info.imageExtent = extent_;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                          (canCapture_ ? VkImageUsageFlags{VK_IMAGE_USAGE_TRANSFER_SRC_BIT} : VkImageUsageFlags{0});
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = VK_PRESENT_MODE_FIFO_KHR; // always available; vsync
        info.clipped = VK_TRUE;
        if (!Check(vkCreateSwapchainKHR(device_, &info, nullptr, &swapchain_), "vkCreateSwapchainKHR")) {
            return false;
        }

        vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
        swapchainImages_.resize(imageCount);
        vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());

        const bool firstTime = renderPass_ == VK_NULL_HANDLE;
        if (firstTime && !CreateRenderPass()) return false;
        if (!CreateDepthBuffer()) return false;

        for (VkImage image : swapchainImages_) {
            VkImageView view = CreateView(image, surfaceFormat_.format, VK_IMAGE_ASPECT_COLOR_BIT);
            if (!view) return false;
            swapchainViews_.push_back(view);

            const VkImageView attachments[] = {view, depthView_};
            VkFramebufferCreateInfo fb{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            fb.renderPass = renderPass_;
            fb.attachmentCount = 2;
            fb.pAttachments = attachments;
            fb.width = extent_.width;
            fb.height = extent_.height;
            fb.layers = 1;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            if (!Check(vkCreateFramebuffer(device_, &fb, nullptr, &framebuffer), "vkCreateFramebuffer")) {
                return false;
            }
            framebuffers_.push_back(framebuffer);

            VkSemaphoreCreateInfo sem{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkSemaphore finished = VK_NULL_HANDLE;
            if (!Check(vkCreateSemaphore(device_, &sem, nullptr, &finished), "vkCreateSemaphore")) {
                return false;
            }
            renderFinished_.push_back(finished);
        }
        swapchainDirty_ = false;
        return true;
    }

    void DestroySwapchain() {
        if (!device_) return;
        for (auto fb : framebuffers_) vkDestroyFramebuffer(device_, fb, nullptr);
        for (auto view : swapchainViews_) vkDestroyImageView(device_, view, nullptr);
        for (auto sem : renderFinished_) vkDestroySemaphore(device_, sem, nullptr);
        framebuffers_.clear();
        swapchainViews_.clear();
        renderFinished_.clear();
        swapchainImages_.clear();
        if (depthView_) vkDestroyImageView(device_, depthView_, nullptr);
        if (depthImage_) vkDestroyImage(device_, depthImage_, nullptr);
        if (depthMemory_) vkFreeMemory(device_, depthMemory_, nullptr);
        depthView_ = VK_NULL_HANDLE;
        depthImage_ = VK_NULL_HANDLE;
        depthMemory_ = VK_NULL_HANDLE;
        if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    bool RecreateSwapchain() {
        vkDeviceWaitIdle(device_);
        DestroySwapchain();
        return CreateSwapchain();
    }

    bool CreateRenderPass() {
        // Depth format: D32 is near-universal; fall back to D24S8.
        for (VkFormat candidate : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT}) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice_, candidate, &props);
            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                depthFormat_ = candidate;
                break;
            }
        }

        std::array<VkAttachmentDescription, 2> attachments{};
        attachments[0].format = surfaceFormat_.format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments[1].format = depthFormat_;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        subpass.pDepthStencilAttachment = &depthRef;

        // Make the first use of the attachments wait for the image to be acquired
        // and for the previous frame's depth writes.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        return Check(vkCreateRenderPass(device_, &info, nullptr, &renderPass_), "vkCreateRenderPass");
    }

    bool CreateDepthBuffer() {
        VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = depthFormat_;
        info.extent = {extent_.width, extent_.height, 1};
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (!Check(vkCreateImage(device_, &info, nullptr, &depthImage_), "vkCreateImage(depth)")) return false;

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(device_, depthImage_, &req);
        if (!Allocate(req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthMemory_)) return false;
        vkBindImageMemory(device_, depthImage_, depthMemory_, 0);
        depthView_ = CreateView(depthImage_, depthFormat_, VK_IMAGE_ASPECT_DEPTH_BIT);
        return depthView_ != VK_NULL_HANDLE;
    }

    VkImageView CreateView(VkImage image, VkFormat format, VkImageAspectFlags aspect) {
        VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        info.image = image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = format;
        info.subresourceRange = {aspect, 0, 1, 0, 1};
        VkImageView view = VK_NULL_HANDLE;
        Check(vkCreateImageView(device_, &info, nullptr, &view), "vkCreateImageView");
        return view;
    }

    // Pipelines ------------------------------------------------------------

    VkShaderModule CreateModule(const shaders::ShaderBlob& blob) {
        VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        info.codeSize = blob.spirvSize;
        // SPIR-V must be passed as 32-bit words; copy to guarantee alignment.
        std::vector<std::uint32_t> words((blob.spirvSize + 3) / 4);
        std::memcpy(words.data(), blob.spirv, blob.spirvSize);
        info.pCode = words.data();
        VkShaderModule module = VK_NULL_HANDLE;
        Check(vkCreateShaderModule(device_, &info, nullptr, &module), "vkCreateShaderModule");
        return module;
    }

    bool BuildPipeline(VkPipelineResource& resource) {
        const PipelineDesc& desc = resource.desc;
        const auto* program = shaders::FindProgram(desc.shaderProgram.c_str());
        if (!program || !program->vertex->spirv || !program->fragment->spirv) {
            std::fprintf(stderr, "Vulkan: no SPIR-V for shader program '%s'\n", desc.shaderProgram.c_str());
            return false;
        }
        VkShaderModule vs = CreateModule(*program->vertex);
        VkShaderModule fs = CreateModule(*program->fragment);
        if (!vs || !fs) {
            if (vs) vkDestroyShaderModule(device_, vs, nullptr);
            if (fs) vkDestroyShaderModule(device_, fs, nullptr);
            return false;
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> stages{};
        stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vs;
        stages[0].pName = "main";
        stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fs;
        stages[1].pName = "main";

        VkVertexInputBindingDescription binding{0, desc.vertexLayout.stride, VK_VERTEX_INPUT_RATE_VERTEX};
        std::vector<VkVertexInputAttributeDescription> attributes;
        for (const VertexAttribute& a : desc.vertexLayout.attributes) {
            attributes.push_back({a.location, 0, ToVk(a.format), a.offset});
        }
        VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &binding;
        vertexInput.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
        vertexInput.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        assembly.topology = desc.topology == PrimitiveTopology::LineList ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST
                                                                         : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewport.viewportCount = 1;
        viewport.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.lineWidth = 1.0f;
        raster.cullMode = desc.cull == CullMode::None    ? VK_CULL_MODE_NONE
                          : desc.cull == CullMode::Back ? VK_CULL_MODE_BACK_BIT
                                                        : VK_CULL_MODE_FRONT_BIT;
        // Counter-clockwise (in the engine's Y-up convention) is front on both
        // APIs. rr_ToClip's Y flip and Vulkan's Y-down framebuffer cancel out,
        // so the same winding rule applies here as in OpenGL.
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depth.depthTestEnable = desc.depthTest ? VK_TRUE : VK_FALSE;
        depth.depthWriteEnable = desc.depthWrite ? VK_TRUE : VK_FALSE;
        depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (desc.blend != BlendMode::Opaque) {
            // Same factors for colour and alpha, as glBlendFunc sets them.
            VkBlendFactor src = VK_BLEND_FACTOR_SRC_ALPHA;
            VkBlendFactor dst = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            if (desc.blend == BlendMode::Additive) dst = VK_BLEND_FACTOR_ONE;
            if (desc.blend == BlendMode::Premultiplied) src = VK_BLEND_FACTOR_ONE;
            blendAttachment.blendEnable = VK_TRUE;
            blendAttachment.srcColorBlendFactor = src;
            blendAttachment.dstColorBlendFactor = dst;
            blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            blendAttachment.srcAlphaBlendFactor = src;
            blendAttachment.dstAlphaBlendFactor = dst;
            blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        }
        VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        blend.attachmentCount = 1;
        blend.pAttachments = &blendAttachment;

        const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = dynamicStates;

        VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        info.stageCount = static_cast<std::uint32_t>(stages.size());
        info.pStages = stages.data();
        info.pVertexInputState = &vertexInput;
        info.pInputAssemblyState = &assembly;
        info.pViewportState = &viewport;
        info.pRasterizationState = &raster;
        info.pMultisampleState = &multisample;
        info.pDepthStencilState = &depth;
        info.pColorBlendState = &blend;
        info.pDynamicState = &dynamic;
        info.layout = pipelineLayout_;
        info.renderPass = renderPass_;

        const bool ok = Check(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &info, nullptr,
                                                        &resource.pipeline),
                              "vkCreateGraphicsPipelines");
        vkDestroyShaderModule(device_, vs, nullptr);
        vkDestroyShaderModule(device_, fs, nullptr);
        return ok;
    }

    // Memory ---------------------------------------------------------------

    bool Allocate(const VkMemoryRequirements& req, VkMemoryPropertyFlags wanted, VkDeviceMemory& out) {
        for (std::uint32_t i = 0; i < memoryProperties_.memoryTypeCount; ++i) {
            const bool allowed = (req.memoryTypeBits & (1u << i)) != 0;
            const bool matches = (memoryProperties_.memoryTypes[i].propertyFlags & wanted) == wanted;
            if (allowed && matches) {
                VkMemoryAllocateInfo info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                info.allocationSize = req.size;
                info.memoryTypeIndex = i;
                return Check(vkAllocateMemory(device_, &info, nullptr, &out), "vkAllocateMemory");
            }
        }
        std::fprintf(stderr, "Vulkan: no suitable memory type\n");
        return false;
    }

    bool CreateHostBuffer(std::size_t size, VkBufferUsageFlags usage, VkBufferResource& out) {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size = size;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (!Check(vkCreateBuffer(device_, &info, nullptr, &out.buffer), "vkCreateBuffer")) return false;
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(device_, out.buffer, &req);
        if (!Allocate(req, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      out.memory)) {
            DestroyBufferResource(out);
            return false;
        }
        vkBindBufferMemory(device_, out.buffer, out.memory, 0);
        return true;
    }

    void DestroyBufferResource(VkBufferResource& resource) {
        if (resource.buffer) vkDestroyBuffer(device_, resource.buffer, nullptr);
        if (resource.memory) vkFreeMemory(device_, resource.memory, nullptr);
        resource = {};
    }

    // Textures -------------------------------------------------------------

    VkCommandBuffer BeginOneShot() {
        VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc.commandPool = commandPool_;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 1;
        VkCommandBuffer commands = VK_NULL_HANDLE;
        if (!Check(vkAllocateCommandBuffers(device_, &alloc, &commands), "vkAllocateCommandBuffers")) {
            return VK_NULL_HANDLE;
        }
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commands, &begin);
        return commands;
    }

    // Submits and waits; uploads are rare (load time), so stalling is acceptable.
    bool EndOneShot(VkCommandBuffer commands) {
        vkEndCommandBuffer(commands);
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &commands;
        const bool ok = Check(vkQueueSubmit(queue_, 1, &submit, VK_NULL_HANDLE), "vkQueueSubmit(upload)") &&
                        Check(vkQueueWaitIdle(queue_), "vkQueueWaitIdle");
        vkFreeCommandBuffers(device_, commandPool_, 1, &commands);
        return ok;
    }

    static void ImageBarrier(VkCommandBuffer commands, VkImage image, std::uint32_t baseMip, std::uint32_t mipCount,
                             VkImageLayout from, VkImageLayout to, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                             VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcAccessMask = srcAccess;
        barrier.dstAccessMask = dstAccess;
        barrier.oldLayout = from;
        barrier.newLayout = to;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, baseMip, mipCount, 0, 1};
        vkCmdPipelineBarrier(commands, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    bool CreateTextureResource(const TextureDesc& desc, std::span<const TextureLevelData> levels,
                               VkTextureResource& out) {
        // Lay the levels out in one staging buffer.
        std::vector<VkDeviceSize> offsets;
        VkDeviceSize total = 0;
        for (std::uint32_t level = 0; level < desc.mipCount; ++level) {
            const int w = std::max(1, desc.width >> level);
            const int h = std::max(1, desc.height >> level);
            if (!levels[level].data || levels[level].size != TextureLevelBytes(desc.format, w, h)) {
                std::fprintf(stderr, "Vulkan: texture level %u has the wrong size\n", level);
                return false;
            }
            total = (total + 15) & ~VkDeviceSize{15}; // copy offsets must be texel-block aligned
            offsets.push_back(total);
            total += levels[level].size;
        }
        VkBufferResource staging;
        if (!CreateHostBuffer(static_cast<std::size_t>(total), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging)) return false;
        void* mapped = nullptr;
        if (!Check(vkMapMemory(device_, staging.memory, 0, total, 0, &mapped), "vkMapMemory")) {
            DestroyBufferResource(staging);
            return false;
        }
        for (std::uint32_t level = 0; level < desc.mipCount; ++level) {
            std::memcpy(static_cast<std::uint8_t*>(mapped) + offsets[level], levels[level].data, levels[level].size);
        }
        vkUnmapMemory(device_, staging.memory);

        out.desc = desc;
        VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = ToVk(desc.format);
        info.extent = {static_cast<std::uint32_t>(desc.width), static_cast<std::uint32_t>(desc.height), 1};
        info.mipLevels = desc.mipCount;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        bool ok = Check(vkCreateImage(device_, &info, nullptr, &out.image), "vkCreateImage(texture)");
        if (ok) {
            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device_, out.image, &req);
            ok = Allocate(req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, out.memory) &&
                 Check(vkBindImageMemory(device_, out.image, out.memory, 0), "vkBindImageMemory");
        }
        if (ok) {
            VkCommandBuffer commands = BeginOneShot();
            ok = commands != VK_NULL_HANDLE;
            if (ok) {
                ImageBarrier(commands, out.image, 0, desc.mipCount, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
                std::vector<VkBufferImageCopy> regions;
                for (std::uint32_t level = 0; level < desc.mipCount; ++level) {
                    VkBufferImageCopy region{};
                    region.bufferOffset = offsets[level];
                    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1};
                    region.imageExtent = {static_cast<std::uint32_t>(std::max(1, desc.width >> level)),
                                          static_cast<std::uint32_t>(std::max(1, desc.height >> level)), 1};
                    regions.push_back(region);
                }
                vkCmdCopyBufferToImage(commands, staging.buffer, out.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       static_cast<std::uint32_t>(regions.size()), regions.data());
                ImageBarrier(commands, out.image, 0, desc.mipCount, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                ok = EndOneShot(commands);
            }
        }
        DestroyBufferResource(staging);
        if (ok) {
            VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view.image = out.image;
            view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view.format = info.format;
            view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, desc.mipCount, 0, 1};
            ok = Check(vkCreateImageView(device_, &view, nullptr, &out.view), "vkCreateImageView(texture)");
        }
        if (!ok) DestroyTextureResource(out);
        return ok;
    }

    void DestroyTextureResource(VkTextureResource& texture) {
        if (texture.view) vkDestroyImageView(device_, texture.view, nullptr);
        if (texture.image) vkDestroyImage(device_, texture.image, nullptr);
        if (texture.memory) vkFreeMemory(device_, texture.memory, nullptr);
        texture = {};
    }

    bool CreateWhiteTexture() {
        const std::uint8_t white[4] = {255, 255, 255, 255};
        TextureDesc desc;
        desc.width = desc.height = 1;
        const TextureLevelData level{white, sizeof(white)};
        if (!CreateTextureResource(desc, std::span(&level, 1), whiteTexture_)) return false;
        const VkSampler sampler = Sampler(SamplerDesc{});
        for (std::uint32_t slot = 0; slot < kMaxTextureSlots; ++slot) {
            boundViews_[slot] = whiteTexture_.view;
            boundSamplers_[slot] = sampler;
        }
        return sampler != VK_NULL_HANDLE;
    }

    VkSampler Sampler(const SamplerDesc& desc) {
        for (const auto& [d, sampler] : samplers_) {
            if (d == desc) return sampler;
        }
        VkSamplerCreateInfo info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        info.magFilter = info.minFilter = desc.filter == FilterMode::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        info.mipmapMode = desc.mipFilter == FilterMode::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                                                               : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        info.addressModeU = ToVk(desc.wrapU);
        info.addressModeV = ToVk(desc.wrapV);
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod = 0.0f;
        info.maxLod = desc.useMips ? VK_LOD_CLAMP_NONE : 0.0f;
        VkSampler sampler = VK_NULL_HANDLE;
        if (!Check(vkCreateSampler(device_, &info, nullptr, &sampler), "vkCreateSampler")) return VK_NULL_HANDLE;
        samplers_.emplace_back(desc, sampler);
        return sampler;
    }

    VkDescriptorSet AllocateTextureSet() {
        FrameResources& frame = frames_[frameIndex_];
        for (;;) {
            const bool freshPool = frame.activePool == frame.descriptorPools.size();
            if (freshPool) {
                const VkDescriptorPoolSize sizes[] = {
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kSetsPerPool * kMaxTextureSlots},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kSetsPerPool},
                };
                VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
                info.maxSets = kSetsPerPool;
                info.poolSizeCount = 2;
                info.pPoolSizes = sizes;
                VkDescriptorPool pool = VK_NULL_HANDLE;
                if (!Check(vkCreateDescriptorPool(device_, &info, nullptr, &pool), "vkCreateDescriptorPool")) {
                    return VK_NULL_HANDLE;
                }
                frame.descriptorPools.push_back(pool);
            }
            VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            alloc.descriptorPool = frame.descriptorPools[frame.activePool];
            alloc.descriptorSetCount = 1;
            alloc.pSetLayouts = &textureSetLayout_;
            VkDescriptorSet set = VK_NULL_HANDLE;
            const VkResult result = vkAllocateDescriptorSets(device_, &alloc, &set);
            if (result == VK_SUCCESS) return set;
            if (freshPool) { // an empty pool can't be "full": something else is wrong
                Check(result, "vkAllocateDescriptorSets");
                return VK_NULL_HANDLE;
            }
            // Full (Vulkan 1.0 drivers may report this with any error code); next pool.
            ++frame.activePool;
        }
    }

    // Makes sure frame.uniformChunks[frame.activeChunk] exists and is mapped.
    bool EnsureUniformChunk(FrameResources& frame) {
        while (frame.uniformChunks.size() <= frame.activeChunk) {
            UniformChunk chunk;
            if (!CreateHostBuffer(kUniformChunkSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, chunk.buffer)) return false;
            void* mapped = nullptr;
            if (!Check(vkMapMemory(device_, chunk.buffer.memory, 0, kUniformChunkSize, 0, &mapped), "vkMapMemory")) {
                DestroyBufferResource(chunk.buffer);
                return false;
            }
            chunk.mapped = static_cast<std::uint8_t*>(mapped);
            frame.uniformChunks.push_back(chunk);
        }
        return true;
    }

    // Writes a descriptor set for the bound textures and the current uniform chunk
    // if they changed, and (re)binds it with the current DrawUniforms offset.
    bool FlushDescriptors() {
        if (!descriptorsDirty_ && !uniformsDirty_) return true;
        FrameResources& frame = frames_[frameIndex_];
        if (descriptorsDirty_) {
            if (!EnsureUniformChunk(frame)) return false;
            const VkDescriptorSet set = AllocateTextureSet();
            if (!set) return false;
            std::array<VkDescriptorImageInfo, kMaxTextureSlots> images{};
            std::array<VkWriteDescriptorSet, kMaxTextureSlots + 1> writes{};
            for (std::uint32_t i = 0; i < kMaxTextureSlots; ++i) {
                images[i] = {boundSamplers_[i], boundViews_[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
                writes[i] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writes[i].dstSet = set;
                writes[i].dstBinding = i;
                writes[i].descriptorCount = 1;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writes[i].pImageInfo = &images[i];
            }
            const VkDescriptorBufferInfo uniforms{frame.uniformChunks[frame.activeChunk].buffer.buffer, 0,
                                                  kMaxDrawUniformsSize};
            VkWriteDescriptorSet& w = writes[kDrawUniformsBinding];
            w = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            w.dstSet = set;
            w.dstBinding = kDrawUniformsBinding;
            w.descriptorCount = 1;
            w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            w.pBufferInfo = &uniforms;
            vkUpdateDescriptorSets(device_, static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr);
            currentSet_ = set;
        }
        vkCmdBindDescriptorSets(cmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &currentSet_, 1,
                                &uniformOffset_);
        descriptorsDirty_ = false;
        uniformsDirty_ = false;
        return true;
    }

    // Copies one level into host memory as RGBA8. Compressed levels are first
    // blitted (decoded by the GPU) into an RGBA8 image where the format allows it.
    std::optional<Image> ReadTextureLevel(const VkTextureResource& texture, std::uint32_t level) {
        const auto width = static_cast<std::uint32_t>(std::max(1, texture.desc.width >> level));
        const auto height = static_cast<std::uint32_t>(std::max(1, texture.desc.height >> level));
        const bool compressed = DescribeTextureFormat(texture.desc.format).compressed;
        const bool srgb = DescribeTextureFormat(texture.desc.format).srgb;
        if (compressed) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice_, ToVk(texture.desc.format), &props);
            if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) return std::nullopt;
        }

        VkBufferResource readback;
        const VkDeviceSize size = VkDeviceSize{width} * height * 4;
        if (!CreateHostBuffer(static_cast<std::size_t>(size), VK_BUFFER_USAGE_TRANSFER_DST_BIT, readback)) {
            return std::nullopt;
        }
        VkTextureResource decoded; // RGBA8 copy of a compressed level
        if (compressed) {
            TextureDesc desc;
            desc.width = static_cast<int>(width);
            desc.height = static_cast<int>(height);
            desc.format = srgb ? TextureFormat::RGBA8_sRGB : TextureFormat::RGBA8;
            std::vector<std::uint8_t> zeros(static_cast<std::size_t>(size));
            const TextureLevelData data{zeros.data(), zeros.size()};
            if (!CreateTextureResource(desc, std::span(&data, 1), decoded)) {
                DestroyBufferResource(readback);
                return std::nullopt;
            }
        }

        VkCommandBuffer commands = BeginOneShot();
        if (!commands) {
            DestroyTextureResource(decoded);
            DestroyBufferResource(readback);
            return std::nullopt;
        }
        constexpr VkImageLayout kRead = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageBarrier(commands, texture.image, level, 1, kRead, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                     VK_PIPELINE_STAGE_TRANSFER_BIT);
        VkImage source = texture.image;
        std::uint32_t sourceLevel = level;
        if (compressed) {
            ImageBarrier(commands, decoded.image, 0, 1, kRead, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
            VkImageBlit blit{};
            blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1};
            blit.srcOffsets[1] = {static_cast<std::int32_t>(width), static_cast<std::int32_t>(height), 1};
            blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            blit.dstOffsets[1] = blit.srcOffsets[1];
            vkCmdBlitImage(commands, texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, decoded.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
            ImageBarrier(commands, decoded.image, 0, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
            source = decoded.image;
            sourceLevel = 0;
        }
        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, sourceLevel, 0, 1};
        region.imageExtent = {width, height, 1};
        vkCmdCopyImageToBuffer(commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.buffer, 1, &region);
        ImageBarrier(commands, texture.image, level, 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, kRead,
                     VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        VkBufferMemoryBarrier hostRead{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
        hostRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        hostRead.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        hostRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostRead.buffer = readback.buffer;
        hostRead.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1,
                             &hostRead, 0, nullptr);
        const bool ok = EndOneShot(commands);

        std::optional<Image> image;
        void* mapped = nullptr;
        if (ok && Check(vkMapMemory(device_, readback.memory, 0, VK_WHOLE_SIZE, 0, &mapped), "vkMapMemory")) {
            image.emplace();
            image->width = static_cast<int>(width);
            image->height = static_cast<int>(height);
            image->rgba.resize(static_cast<std::size_t>(size));
            std::memcpy(image->rgba.data(), mapped, image->rgba.size());
            vkUnmapMemory(device_, readback.memory);
        }
        DestroyTextureResource(decoded);
        DestroyBufferResource(readback);
        return image;
    }

    // Capture --------------------------------------------------------------

    // Records a copy of the just-rendered swapchain image into a host buffer.
    bool RecordCapture(VkBufferResource& readback) {
        const VkDeviceSize size = VkDeviceSize{extent_.width} * extent_.height * 4;
        if (!CreateHostBuffer(static_cast<std::size_t>(size), VK_BUFFER_USAGE_TRANSFER_DST_BIT, readback)) {
            return false;
        }
        VkImage image = swapchainImages_[imageIndex_];

        VkImageMemoryBarrier toTransfer{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        toTransfer.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        toTransfer.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toTransfer.image = image;
        toTransfer.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCmdPipelineBarrier(cmd_, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toTransfer);

        VkBufferImageCopy region{};
        region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.imageExtent = {extent_.width, extent_.height, 1};
        vkCmdCopyImageToBuffer(cmd_, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.buffer, 1, &region);

        VkImageMemoryBarrier toPresent = toTransfer;
        toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        toPresent.dstAccessMask = 0;
        toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vkCmdPipelineBarrier(cmd_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &toPresent);

        VkBufferMemoryBarrier hostRead{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
        hostRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        hostRead.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
        hostRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        hostRead.buffer = readback.buffer;
        hostRead.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(cmd_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
                             nullptr, 1, &hostRead, 0, nullptr);
        return true;
    }

    std::optional<Image> ReadCapture(const VkBufferResource& readback) {
        Image image;
        image.width = static_cast<int>(extent_.width);
        image.height = static_cast<int>(extent_.height);
        image.rgba.resize(std::size_t{extent_.width} * extent_.height * 4);
        void* mapped = nullptr;
        if (!Check(vkMapMemory(device_, readback.memory, 0, VK_WHOLE_SIZE, 0, &mapped), "vkMapMemory")) {
            return std::nullopt;
        }
        std::memcpy(image.rgba.data(), mapped, image.rgba.size());
        vkUnmapMemory(device_, readback.memory);
        if (surfaceFormat_.format == VK_FORMAT_B8G8R8A8_UNORM) {
            for (std::size_t i = 0; i < image.rgba.size(); i += 4) std::swap(image.rgba[i], image.rgba[i + 2]);
        }
        return image;
    }

    platform::Platform& platform_;
    std::string deviceName_;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    bool wantDebugMessenger_ = false;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memoryProperties_{};
    std::uint32_t queueFamily_ = 0;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout textureSetLayout_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    bool astcEnabled_ = false;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormat_{};
    VkExtent2D extent_{};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainViews_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<VkSemaphore> renderFinished_; // one per swapchain image
    VkFormat depthFormat_ = VK_FORMAT_D32_SFLOAT;
    VkImage depthImage_ = VK_NULL_HANDLE;
    VkDeviceMemory depthMemory_ = VK_NULL_HANDLE;
    VkImageView depthView_ = VK_NULL_HANDLE;
    bool swapchainDirty_ = false;
    bool canCapture_ = false;

    std::array<FrameResources, kFramesInFlight> frames_{};
    std::uint32_t frameIndex_ = 0;
    std::uint32_t imageIndex_ = 0;
    VkCommandBuffer cmd_ = VK_NULL_HANDLE;
    bool pipelineBound_ = false;

    std::unordered_map<std::uint32_t, VkBufferResource> buffers_;
    std::unordered_map<std::uint32_t, VkPipelineResource> pipelines_;
    std::unordered_map<std::uint32_t, VkTextureResource> textures_;
    std::vector<std::pair<SamplerDesc, VkSampler>> samplers_;
    VkTextureResource whiteTexture_;
    std::array<VkImageView, kMaxTextureSlots> boundViews_{};
    std::array<VkSampler, kMaxTextureSlots> boundSamplers_{};
    bool descriptorsDirty_ = true;
    VkDescriptorSet currentSet_ = VK_NULL_HANDLE;
    std::size_t uniformAlignment_ = 256;
    std::uint32_t uniformOffset_ = 0;
    bool uniformsDirty_ = true;
    std::uint32_t nextId_ = 1;

    bool captureRequested_ = false;
    std::optional<Image> capture_;
};

} // namespace

std::unique_ptr<RenderDevice> CreateVulkanDevice(platform::Platform& platform) {
    auto device = std::make_unique<VulkanDevice>(platform);
    if (!device->Initialise()) {
        return nullptr;
    }
    return device;
}

} // namespace rr::render
