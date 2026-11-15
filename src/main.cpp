#include <iostream>
#include <chrono>
#include <thread>
#include <SDL.h>

// Include Nesun core headers (adjust paths based on your folder layout)
// #include "nesun/bus.hpp"
// #include "nesun/cpu.hpp"
// #include "nesun/ppu.hpp"

const int NES_SCREEN_WIDTH = 256;
const int NES_SCREEN_HEIGHT = 240;
const int WINDOW_SCALE = 3;

int main(int argc, char* argv[]) {
    // 1. Initialize SDL2 Video and Audio subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "Failed to initialize SDL2: " << SDL_GetError() << "\n";
        return 1;
    }

    // 2. Create the emulator window with a retro pixel-art scale
    SDL_Window* window = SDL_CreateWindow(
        "Nesun - Famiclone Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        NES_SCREEN_WIDTH * WINDOW_SCALE,
        NES_SCREEN_HEIGHT * WINDOW_SCALE,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // 3. Create a hardware-accelerated renderer with VSync enabled
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a streaming texture to hold PPU framebuffers (supports VTxx extended resolutions later)
    SDL_Texture* frame_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        NES_SCREEN_WIDTH,
        NES_SCREEN_HEIGHT
    );

    // 4. Emulation & Timing Initialization
    bool running = true;
    SDL_Event event;

    // NES runs at approximately 1.789773 MHz (~60.09 frames per second NTSC)
    const double target_frame_time_ms = 1000.0 / 60.09;

    std::cout << "Nesun initialized successfully. Starting main loop...\n";

    // 5. Main Emulation Loop
    while (running) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        // Handle window events and inputs (keyboard / Subor keyboard mapping)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // TODO: Step the emulation core here
        // e.g., bus.clock_frame(); or CPU/PPU step synchronization loop

        // Render placeholder background (e.g., retro blue screen)
        SDL_SetRenderDrawColor(renderer, 15, 15, 35, 255);
        SDL_RenderClear(renderer);

        // Render the PPU texture
        SDL_RenderCopy(renderer, frame_texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // Frame rate regulation to maintain accurate Famiclone speed
        auto frame_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = frame_end - frame_start;
        
        if (elapsed.count() < target_frame_time_ms) {
            std::chrono::duration<double, std::milli> sleep_time = target_frame_time_ms - elapsed.count();
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time.count())));
        }
    }

    // 6. Cleanup Resources
    SDL_DestroyTexture(frame_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
