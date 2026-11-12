// Desktop platform backend built on SDL2: window, events, keyboard, mouse,
// gamepad, and the OpenGL / Vulkan surface hooks.
#include "../common/common.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <cmath>
#include <cstdio>

namespace rr::platform {
namespace {

Key TranslateKey(SDL_Scancode sc) {
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        return static_cast<Key>(static_cast<int>(Key::A) + (sc - SDL_SCANCODE_A));
    }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
        return static_cast<Key>(static_cast<int>(Key::Num1) + (sc - SDL_SCANCODE_1));
    }
    if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F12) {
        return static_cast<Key>(static_cast<int>(Key::F1) + (sc - SDL_SCANCODE_F1));
    }
    switch (sc) {
    case SDL_SCANCODE_0: return Key::Num0;
    case SDL_SCANCODE_ESCAPE: return Key::Escape;
    case SDL_SCANCODE_RETURN: return Key::Enter;
    case SDL_SCANCODE_SPACE: return Key::Space;
    case SDL_SCANCODE_TAB: return Key::Tab;
    case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
    case SDL_SCANCODE_LEFT: return Key::Left;
    case SDL_SCANCODE_RIGHT: return Key::Right;
    case SDL_SCANCODE_UP: return Key::Up;
    case SDL_SCANCODE_DOWN: return Key::Down;
    case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
    case SDL_SCANCODE_RSHIFT: return Key::RightShift;
    case SDL_SCANCODE_LCTRL: return Key::LeftCtrl;
    case SDL_SCANCODE_RCTRL: return Key::RightCtrl;
    case SDL_SCANCODE_LALT: return Key::LeftAlt;
    case SDL_SCANCODE_RALT: return Key::RightAlt;
    default: return Key::Unknown;
    }
}

int TranslateMouseButton(Uint8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT: return static_cast<int>(MouseButton::Left);
    case SDL_BUTTON_RIGHT: return static_cast<int>(MouseButton::Right);
    case SDL_BUTTON_MIDDLE: return static_cast<int>(MouseButton::Middle);
    default: return -1;
    }
}

constexpr SDL_GameControllerButton kPadButtons[] = {
    SDL_CONTROLLER_BUTTON_A,          SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,          SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,  SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_BACK,       SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_DPAD_UP,    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,  SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
};
static_assert(std::size(kPadButtons) == static_cast<std::size_t>(GamepadButton::Count));

constexpr SDL_GameControllerAxis kPadAxes[] = {
    SDL_CONTROLLER_AXIS_LEFTX,       SDL_CONTROLLER_AXIS_LEFTY,
    SDL_CONTROLLER_AXIS_RIGHTX,      SDL_CONTROLLER_AXIS_RIGHTY,
    SDL_CONTROLLER_AXIS_TRIGGERLEFT, SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
};
static_assert(std::size(kPadAxes) == static_cast<std::size_t>(GamepadAxis::Count));

class SdlOpenGLContext final : public OpenGLContext {
public:
    explicit SdlOpenGLContext(SDL_Window* window) : window_(window) {}

    void* GetProcAddress(const char* name) const override { return SDL_GL_GetProcAddress(name); }
    void SwapBuffers() override { SDL_GL_SwapWindow(window_); }

private:
    SDL_Window* window_;
};

class SdlVulkanSurfaceProvider final : public VulkanSurfaceProvider {
public:
    explicit SdlVulkanSurfaceProvider(SDL_Window* window) : window_(window) {}

    std::vector<const char*> RequiredInstanceExtensions() const override {
        unsigned int count = 0;
        SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr);
        std::vector<const char*> names(count);
        SDL_Vulkan_GetInstanceExtensions(window_, &count, names.data());
        return names;
    }

    bool CreateSurface(void* instance, std::uint64_t* outSurface) const override {
        VkSurfaceKHR surface{};
        if (!SDL_Vulkan_CreateSurface(window_, static_cast<VkInstance>(instance), &surface)) {
            std::fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n", SDL_GetError());
            return false;
        }
        static_assert(sizeof(surface) <= sizeof(std::uint64_t));
        *outSurface = (std::uint64_t)(surface);
        return true;
    }

private:
    SDL_Window* window_;
};

class SdlPlatform final : public Platform {
public:
    explicit SdlPlatform(const PlatformConfig& config)
        : files_(config.contentRoot, config.userDataRoot), frameLimit_(config.frameLimit) {}

    ~SdlPlatform() override {
        if (gamepad_) SDL_GameControllerClose(gamepad_);
        if (glContext_) SDL_GL_DeleteContext(glContext_);
        if (window_) SDL_DestroyWindow(window_);
        if (initialised_) SDL_Quit();
    }

    bool Initialise() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
            std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            return false;
        }
        initialised_ = true;
        return true;
    }

    bool CreateMainWindow(const WindowDesc& desc) override {
        Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI;
        if (desc.resizable) flags |= SDL_WINDOW_RESIZABLE;
        if (desc.fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

        if (desc.api == GraphicsApi::OpenGL) {
            flags |= SDL_WINDOW_OPENGL;
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        } else if (desc.api == GraphicsApi::Vulkan) {
            flags |= SDL_WINDOW_VULKAN;
        }

        window_ = SDL_CreateWindow(desc.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, desc.width, desc.height, flags);
        if (!window_) {
            std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
            return false;
        }

        if (desc.api == GraphicsApi::OpenGL) {
            glContext_ = SDL_GL_CreateContext(window_);
            if (!glContext_) {
                std::fprintf(stderr, "OpenGL 4.1 context creation failed: %s\n", SDL_GetError());
                return false;
            }
            SDL_GL_SetSwapInterval(desc.vsync ? 1 : 0);
            glInterface_ = std::make_unique<SdlOpenGLContext>(window_);
        } else if (desc.api == GraphicsApi::Vulkan) {
            vulkanInterface_ = std::make_unique<SdlVulkanSurfaceProvider>(window_);
        }
        return true;
    }

    bool PumpEvents() override {
        if (frameLimit_ != 0 && framesPumped_++ >= frameLimit_) {
            return false;
        }

        for (auto& k : input_.keys) k.ClearEdges();
        for (auto& b : input_.mouseButtons) b.ClearEdges();
        input_.mouseWheel = 0.0f;

        bool keepRunning = true;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                keepRunning = false;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (!e.key.repeat) {
                    const Key key = TranslateKey(e.key.keysym.scancode);
                    if (key != Key::Unknown) {
                        input_.keys[static_cast<std::size_t>(key)].Set(e.type == SDL_KEYDOWN);
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                input_.mouseX = static_cast<float>(e.motion.x);
                input_.mouseY = static_cast<float>(e.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (const int b = TranslateMouseButton(e.button.button); b >= 0) {
                    input_.mouseButtons[static_cast<std::size_t>(b)].Set(e.type ==
                                                                         SDL_MOUSEBUTTONDOWN);
                }
                break;
            case SDL_MOUSEWHEEL:
                input_.mouseWheel += e.wheel.preciseY;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (!gamepad_) gamepad_ = SDL_GameControllerOpen(e.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (gamepad_ && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_)) ==
                                    e.cdevice.which) {
                    SDL_GameControllerClose(gamepad_);
                    gamepad_ = nullptr;
                }
                break;
            default:
                break;
            }
        }

        RefreshGamepad();
        return keepRunning;
    }

    void GetDrawableSize(int& width, int& height) const override {
        width = height = 0;
        if (!window_ || (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)) {
            return;
        }
        if (glContext_) {
            SDL_GL_GetDrawableSize(window_, &width, &height);
        } else if (vulkanInterface_) {
            SDL_Vulkan_GetDrawableSize(window_, &width, &height);
        } else {
            SDL_GetWindowSize(window_, &width, &height);
        }
    }

    const InputState& GetInput() const override { return input_; }
    const Clock& GetClock() const override { return clock_; }
    const FileSystem& GetFileSystem() const override { return files_; }
    OpenGLContext* GetOpenGLContext() override { return glInterface_.get(); }
    const VulkanSurfaceProvider* GetVulkanSurfaceProvider() const override {
        return vulkanInterface_.get();
    }

private:
    void RefreshGamepad() {
        input_.gamepadConnected = gamepad_ != nullptr;
        for (std::size_t i = 0; i < input_.gamepadButtons.size(); ++i) {
            const bool down = gamepad_ && SDL_GameControllerGetButton(gamepad_, kPadButtons[i]);
            input_.gamepadButtons[i].Set(down);
        }
        for (std::size_t i = 0; i < input_.gamepadAxes.size(); ++i) {
            const float raw =
                gamepad_ ? static_cast<float>(SDL_GameControllerGetAxis(gamepad_, kPadAxes[i])) : 0.f;
            input_.gamepadAxes[i] = std::fmax(-1.0f, raw / 32767.0f);
        }
    }

    detail::SteadyClock clock_;
    detail::DirectoryFileSystem files_;
    InputState input_;
    std::uint64_t frameLimit_;
    std::uint64_t framesPumped_ = 0;

    bool initialised_ = false;
    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    SDL_GameController* gamepad_ = nullptr;
    std::unique_ptr<SdlOpenGLContext> glInterface_;
    std::unique_ptr<SdlVulkanSurfaceProvider> vulkanInterface_;
};

} // namespace

std::unique_ptr<Platform> CreateDesktopPlatform(const PlatformConfig& config) {
    auto platform = std::make_unique<SdlPlatform>(config);
    if (!platform->Initialise()) {
        return nullptr;
    }
    return platform;
}

} // namespace rr::platform
