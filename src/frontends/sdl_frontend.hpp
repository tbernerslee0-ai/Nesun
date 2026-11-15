#pragma once

#include <SDL.h>
#include "nesun/bus.hpp"
#include "nesun/cheats/cheat_manager.hpp" // From previous step
#include <string>

namespace Nesun::Frontend {

class SDLFrontend {
public:
    SDLFrontend(Bus& bus, int window_scale = 3);
    ~SDLFrontend();

    bool initialize(const std::string& window_title);
    void run();
    void shutdown();

private:
    Bus& bus_;
    int window_scale_;
    bool running_ = false;
    bool show_cheat_menu_ = true; // Toggle with F1

    // SDL Subsystems
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* framebuffer_texture_ = nullptr;
    SDL_AudioDeviceID audio_device_ = 0;

    // Cheat Manager instance
    Cheats::CheatManager cheat_manager_;

    // ImGui / Input Helpers
    void handle_input();
    void update_emulation();
    void render_frame();
    void render_cheat_window(); // ImGui UI code
    void push_audio_samples();
};

} // namespace Nesun::Frontend
