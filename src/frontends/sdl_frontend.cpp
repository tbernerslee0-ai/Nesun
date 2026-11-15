#include "frontends/sdl_frontend.hpp"
#include "nesun/subor/keyboard.hpp"
#include <iostream>
#include <chrono>
#include <thread>

namespace Nesun::Frontend {

SDLFrontend::SDLFrontend(Bus& bus, int window_scale)
    : bus_(bus), window_scale_(window_scale) {}

SDLFrontend::~SDLFrontend() {
    shutdown();
}

bool SDLFrontend::initialize(const std::string& window_title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    window_ = SDL_CreateWindow(
        window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        PPU::SCREEN_WIDTH * window_scale_,
        PPU::SCREEN_HEIGHT * window_scale_,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(
        window_, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        return false;
    }

    framebuffer_texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        PPU::SCREEN_WIDTH,
        PPU::SCREEN_HEIGHT
    );

    // Initialize SDL Audio (44.1kHz, 16-bit stereo/mono queue)
    SDL_AudioSpec desired_spec{}, obtained_spec{};
    desired_spec.freq = 44100;
    desired_spec.format = AUDIO_F32SYS;
    desired_spec.channels = 1;
    desired_spec.samples = 1024;
    desired_spec.callback = nullptr; // Using audio queueing

    audio_device_ = SDL_OpenAudioDevice(nullptr, 0, &desired_spec, &obtained_spec, 0);
    if (audio_device_ == 0) {
        std::cout << "Warning: Failed to open SDL Audio device: " << SDL_GetError() << "\n";
    } else {
        SDL_PauseAudioDevice(audio_device_, 0); // Unpause audio
    }

    running_ = true;
    return true;
}

void SDLFrontend::run() {
    const double target_frame_time_ms = 1000.0 / 60.09; // NTSC frame rate

    while (running_) {
        auto frame_start = std::chrono::high_resolution_clock::now();

        handle_input();
        update_emulation();
        render_frame();
        push_audio_samples();

        // Frame timing regulation
        auto frame_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = frame_end - frame_start;

        if (elapsed.count() < target_frame_time_ms) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(target_frame_time_ms - elapsed.count()))
            );
        }
    }
}

void SDLFrontend::handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool pressed = (event.type == SDL_KEYDOWN);
            if (pressed && event.key.keysym.sym == SDLK_ESCAPE) {
                running_ = false;
                break;
            }

            // Route key events to active mapper / Subor keyboard matrix if applicable
            Mapper* mapper = bus_.get_mapper();
            if (mapper) {
                // Example hook for Subor keyboard routing
                // mapper->handle_sdl_key(event.key.keysym.scancode, pressed);
            }
        }
    }
}

void SDLFrontend::update_emulation() {
    // Run CPU/PPU/APU cycles until a complete frame is rendered
    PPU& ppu = bus_.get_ppu();
    ppu.clear_frame_complete();

    while (!ppu.is_frame_complete()) {
        bus_.get_cpu().step();
        ppu.step();
        bus_.get_apu().step();
    }
}

void SDLFrontend::render_frame() {
    PPU& ppu = bus_.get_ppu();
    
    // Update streaming texture with PPU framebuffer pixels
    SDL_UpdateTexture(
        framebuffer_texture_,
        nullptr,
        ppu.get_framebuffer(),
        PPU::SCREEN_WIDTH * sizeof(uint32_t)
    );

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, framebuffer_texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

void SDLFrontend::push_audio_samples() {
    if (audio_device_ == 0) return;

    APU& apu = bus_.get_apu();
    if (apu.is_sample_ready()) {
        float sample = apu.get_mixed_sample();
        SDL_QueueAudio(audio_device_, &sample, sizeof(float));
        apu.clear_sample_ready();
    }
}

void SDLFrontend::shutdown() {
    if (audio_device_ != 0) {
        SDL_CloseAudioDevice(audio_device_);
        audio_device_ = 0;
    }
    if (framebuffer_texture_) {
        SDL_DestroyTexture(framebuffer_texture_);
        framebuffer_texture_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

} // namespace Nesun::Frontend
