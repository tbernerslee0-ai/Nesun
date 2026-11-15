#pragma once

#include <SDL.h>
#include "nesun/bus.hpp"
#include <string>

namespace Nesun::Frontend {

class SDLFrontend {
public:
    SDLFrontend(Bus& bus, int window_scale = 3);
    ~SDLFrontend();

    // Lifecycle
    bool initialize(const std::string& window_title);
    void run();
    void shutdown();

    // Runtime Configuration
    void set_scale(int scale) { window_scale_ = scale; }

private:
    Bus& bus_;
    int window_scale_;
    bool running_ = false;

    // SDL Subsystem Handles
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* framebuffer_texture_ = nullptr;
    SDL_AudioDeviceID audio_device_ = 0;

    // Internal loop helpers
    void handle_input();
    void update_emulation();
    void render_frame();
    void push_audio_samples();
};

} // namespace Nesun::Frontend
