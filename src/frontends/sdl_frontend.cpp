#include "frontends/sdl_frontend.hpp"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>

namespace Nesun::Frontend {

// ... (constructor, shutdown, initialize setup) ...

bool SDLFrontend::initialize(const std::string& window_title) {
    // ... [Standard SDL window & renderer init from earlier] ...

    // Setup Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Style
    ImGui::StyleColorsDark();

    // Init ImGui SDL2 / SDL_Renderer bindings
    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);

    running_ = true;
    return true;
}

void SDLFrontend::handle_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Forward SDL events to ImGui first
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running_ = false;
            }
            // Toggle Cheat Menu with F1 key
            else if (event.key.keysym.sym == SDLK_F1) {
                show_cheat_menu_ = !show_cheat_menu_;
            }
        }
    }
}

void SDLFrontend::render_frame() {
    PPU& ppu = bus_.get_ppu();
    
    // 1. Update PPU Framebuffer Texture
    SDL_UpdateTexture(
        framebuffer_texture_,
        nullptr,
        ppu.get_framebuffer(),
        PPU::SCREEN_WIDTH * sizeof(uint32_t)
    );

    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, framebuffer_texture_, nullptr, nullptr);

    // 2. Start Dear ImGui Frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 3. Render Cheat Overlay Window if active
    if (show_cheat_menu_) {
        render_cheat_window();
    }

    // 4. Render ImGui Draw Data over SDL Window
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);

    SDL_RenderPresent(renderer_);
}

void SDLFrontend::render_cheat_window() {
    auto& cheat_manager = bus_.get_cheat_manager();

    ImGui::Begin("Nesun // Cheat & Game Genie Manager", &show_cheat_menu_, ImGuiWindowFlags_AlwaysAutoResize);

    static char code_buf[16] = "";
    static char name_buf[32] = "";
    static bool invalid_code_error = false;

    ImGui::Text("Enter Game Genie Code (6 or 8 chars):");
    ImGui::InputText("Code", code_buf, IM_ARRAYSIZE(code_buf), ImGuiInputTextFlags_CharsUppercase);
    ImGui::InputText("Description", name_buf, IM_ARRAYSIZE(name_buf));

    if (ImGui::Button("Add Cheat")) {
        std::string code(code_buf);
        std::string name(name_buf);
        if (name.empty()) name = code;

        if (!code.empty()) {
            if (cheat_manager.add_cheat(name, code)) {
                code_buf[0] = '\0';
                name_buf[0] = '\0';
                invalid_code_error = false;
            } else {
                invalid_code_error = true;
            }
        }
    }

    if (invalid_code_error) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid Game Genie code!");
    }

    ImGui::Separator();
    ImGui::Text("Active Cheats:");

    std::string to_remove;
    for (auto& [name, cheat] : cheat_manager.get_cheats()) {
        ImGui::Checkbox(name.c_str(), &cheat.enabled);
        ImGui::SameLine();
        ImGui::TextDisabled("[$%04X -> $%02X]", cheat.address, cheat.patch_value);
        ImGui::SameLine();
        if (ImGui::SmallButton(("Remove##" + name).c_str())) {
            to_remove = name;
        }
    }

    if (!to_remove.empty()) {
        cheat_manager.remove_cheat(to_remove);
    }

    ImGui::End();
}

    ImGui::Separator();
    ImGui::Text("Active Cheats:");

    // Display list of active cheats with toggle checkboxes
    // (Assuming CheatManager exposes a method to iterate or access active codes)
    /*
    auto& cheats = cheat_manager_.get_cheats();
    for (auto& [name, cheat] : cheats) {
        bool enabled = cheat.enabled;
        if (ImGui::Checkbox(name.c_str(), &enabled)) {
            cheat.enabled = enabled;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(("Remove##" + name).c_str())) {
            cheat_manager_.remove_cheat(name);
            break;
        }
    }
    */

    ImGui::End();
}

void SDLFrontend::shutdown() {
    // Cleanup ImGui
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // ... [Standard SDL cleanup] ...
}

} // namespace Nesun::Frontend
