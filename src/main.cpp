#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <memory>
#include <vector>
#include "analyzer.h"
#include "portable-file-dialogs.h"
#include "texture_loader.h"
#include <GL/gl.h>
#include "embedded_resources.h"
void SetupSekiroStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Arrotondamenti morbidi per un look più moderno ma in tema
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.WindowRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f; // Rimuovo i bordi netti per un look più pulito
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    style.ItemSpacing = ImVec2(12, 10);
    style.FramePadding = ImVec2(10, 6);
    style.WindowPadding = ImVec2(12, 12);

    ImVec4* colors = style.Colors;
    
    // Testo color Pergamena/Crema sporco (come le descrizioni degli oggetti)
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.88f, 0.84f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.55f, 0.53f, 0.48f, 1.00f);
    
    // Sfondi base grigio-marrone "pietra scheggiata" (Taupe) con trasparenza
    colors[ImGuiCol_WindowBg]               = ImVec4(0.18f, 0.17f, 0.15f, 0.85f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.14f, 0.13f, 0.11f, 0.85f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.11f, 0.09f, 0.95f);
    
    // Bordi e Divisori rigidi e scuri
    colors[ImGuiCol_Border]                 = ImVec4(0.26f, 0.24f, 0.20f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.35f, 0.35f, 0.35f, 0.50f); // Resa un po' più visibile
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.79f, 0.64f, 0.33f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.79f, 0.64f, 0.33f, 1.00f);
    
    // Riquadri incavati (campi di testo, fondi delle barre progressione)
    colors[ImGuiCol_FrameBg]                = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.15f, 0.14f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.18f, 0.15f, 1.00f);
    
    // Colore di accento principale: L'Arancione Ruggine della "Pennellata" di Sekiro
    ImVec4 sekiro_orange = ImVec4(0.79f, 0.64f, 0.33f, 1.00f);
    
    colors[ImGuiCol_TitleBg]                = ImVec4(0.12f, 0.11f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.12f, 0.11f, 0.09f, 1.00f);
    
    colors[ImGuiCol_Tab]                    = ImVec4(0.12f, 0.11f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.65f, 0.35f, 0.15f, 1.00f);
    colors[ImGuiCol_TabActive]              = sekiro_orange;
    
    // Intestazioni (Menu a tendina)
    colors[ImGuiCol_Header]                 = ImVec4(0.79f, 0.64f, 0.33f, 0.35f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.79f, 0.64f, 0.33f, 0.60f);
    colors[ImGuiCol_HeaderActive]           = sekiro_orange;
    
    // Pulsanti (come "Load Save" / "Clona Slot")
    colors[ImGuiCol_Button]                 = ImVec4(0.22f, 0.20f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = sekiro_orange;
    
    colors[ImGuiCol_CheckMark]              = sekiro_orange;
    colors[ImGuiCol_SliderGrab]             = sekiro_orange;
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.95f, 0.50f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = sekiro_orange;
    
    // Tabelle
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.12f, 0.11f, 0.09f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.15f, 0.14f, 0.12f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.18f, 0.17f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.26f, 0.24f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.20f, 0.18f, 0.15f, 1.00f);
}
int main(int argc, char* argv[]) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Sekiro: Shadows Read Twice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1100, 750, window_flags);

    // Set Window Icon
    int req_format = 4; // STBI_rgb_alpha
    int i_width, i_height, orig_format;
    unsigned char* data = stbi_load_from_memory(SEKIRO_LOGO_PNG, SEKIRO_LOGO_PNG_SIZE, &i_width, &i_height, &orig_format, req_format);
    if (data != NULL) {
        Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000; gmask = 0x00ff0000; bmask = 0x0000ff00; amask = 0x000000ff;
#else
        rmask = 0x000000ff; gmask = 0x0000ff00; bmask = 0x00ff0000; amask = 0xff000000;
#endif
        SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)data, i_width, i_height, 32, 4 * i_width, rmask, gmask, bmask, amask);
        if (surf) {
            SDL_SetWindowIcon(window, surf);
            SDL_FreeSurface(surf);
        }
        stbi_image_free(data);
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load Premium Fonts
    ImFontConfig font_cfg;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 2;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)MONTSERRAT_SEMIBOLD_OTF, MONTSERRAT_SEMIBOLD_OTF_SIZE, 18.0f, &font_cfg);
    ImFont* font_bold = io.Fonts->AddFontFromMemoryTTF((void*)MONTSERRAT_BOLD_TTF, MONTSERRAT_BOLD_TTF_SIZE, 20.0f, &font_cfg);
    ImFont* font_title_large = io.Fonts->AddFontFromMemoryTTF((void*)SERPENTINE_TTF, SERPENTINE_TTF_SIZE, 48.0f, &font_cfg);
    ImFont* font_title_small = io.Fonts->AddFontFromMemoryTTF((void*)SERPENTINE_TTF, SERPENTINE_TTF_SIZE, 18.0f, &font_cfg);


    SetupSekiroStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLuint bg_texture = 0;
    int bg_w = 0, bg_h = 0;
    LoadTextureFromMemory(BG_JPG, BG_JPG_SIZE, &bg_texture, &bg_w, &bg_h);

    std::string target_file = "";
    // Using embedded dict now
    std::vector<SaveSlot> slots;
    int selected_slot_idx = 0;
    std::shared_ptr<pfd::open_file> open_file_dialog;


    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) done = true;
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Disegna lo sfondo globale mantenendo l'aspect ratio (modalità 'cover')
        if (bg_texture && bg_w > 0 && bg_h > 0) {
            float window_aspect = io.DisplaySize.x / io.DisplaySize.y;
            float image_aspect = (float)bg_w / (float)bg_h;
            
            ImVec2 uv_min = ImVec2(0, 0);
            ImVec2 uv_max = ImVec2(1, 1);
            
            if (window_aspect > image_aspect) {
                float scale = io.DisplaySize.x / bg_w;
                float scaled_h = bg_h * scale;
                float crop = (scaled_h - io.DisplaySize.y) / 2.0f;
                float crop_uv = crop / scaled_h;
                uv_min.y = crop_uv;
                uv_max.y = 1.0f - crop_uv;
            } else {
                float scale = io.DisplaySize.y / bg_h;
                float scaled_w = bg_w * scale;
                float crop = (scaled_w - io.DisplaySize.x) / 2.0f;
                float crop_uv = crop / scaled_w;
                uv_min.x = crop_uv;
                uv_max.x = 1.0f - crop_uv;
            }

            ImGui::GetBackgroundDrawList()->AddImage(
                (void*)(intptr_t)bg_texture, 
                ImVec2(0, 0), io.DisplaySize, 
                uv_min, uv_max, 
                IM_COL32(255, 255, 255, 150)
            );
        }

        // Main Window covering the whole screen
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGuiWindowFlags window_flags_main = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Main UI", nullptr, window_flags_main);
        ImGui::PopStyleVar();

        // Split Layout: Sidebar and Main Content
        if (ImGui::BeginTable("MainLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Sidebar", ImGuiTableColumnFlags_WidthFixed, 320.0f);
            ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            // Lasciamo 4 pixel liberi sulla destra in modo che il cursore del mouse 
            // riesca a intercettare il divisore della colonna (lo splitter) per il ridimensionamento
            ImGui::BeginChild("Sidebar", ImVec2(-4.0f, 0), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoTitleBar);
            
            ImGui::Spacing(); ImGui::Spacing();
            float windowWidth = ImGui::GetWindowSize().x;
            
            ImGui::PushFont(font_title_large);
            float textWidth1 = ImGui::CalcTextSize("SEKIRO").x;
            ImGui::SetCursorPosX((windowWidth - textWidth1) * 0.5f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "SEKIRO");
            ImGui::PopFont();
            
            ImGui::PushFont(font_title_small);
            float textWidth2 = ImGui::CalcTextSize("SHADOWS READ TWICE").x;
            ImGui::SetCursorPosX((windowWidth - textWidth2) * 0.5f);
            ImGui::TextColored(ImVec4(0.79f, 0.64f, 0.33f, 1.0f), "SHADOWS READ TWICE");
            ImGui::PopFont();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing(); ImGui::Spacing();
            
            if (ImGui::Button("Load Save", ImVec2(-1, 30))) {
                open_file_dialog = std::make_shared<pfd::open_file>("Select Sekiro Save", ".", std::vector<std::string>{ "Sekiro Saves", "*.sl2 *.bak", "All Files", "*" });
            }
            
            // Handle dialog asynchronously
            if (open_file_dialog && open_file_dialog->ready(0)) {
                auto sel = open_file_dialog->result();
                if (!sel.empty()) {
                    target_file = sel[0];
                    SekiroAnalyzer analyzer(target_file, ITEM_IDS_TXT, ITEM_IDS_TXT_SIZE);
                    slots = analyzer.parse_bnd4();
                    selected_slot_idx = 0;
                }
                open_file_dialog.reset();
            }
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();


            if (slots.empty()) {
                ImGui::Text("No valid save found.");
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "SELECT SLOT:");
                for (size_t i = 0; i < slots.size(); i++) {
                    std::string label = "Slot " + std::to_string(slots[i].slot_num);
                    if (ImGui::Selectable(label.c_str(), selected_slot_idx == (int)i)) {
                        selected_slot_idx = (int)i;
                    }
                }
                ImGui::Spacing();
                if (ImGui::Button("Clone Slot...", ImVec2(-1, 30))) {
                    ImGui::OpenPopup("CloneMenu");
                }
                
                if (ImGui::BeginPopupModal("CloneMenu", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Clone Slot %d to another slot.", slots[selected_slot_idx].slot_num);
                    ImGui::Text("A new separate file will be created for safety!");
                    ImGui::Separator();
                    
                    static int target_clone_slot = 1;
                    ImGui::Combo("Target Slot", &target_clone_slot, "Slot 1\0Slot 2\0Slot 3\0Slot 4\0Slot 5\0Slot 6\0Slot 7\0Slot 8\0Slot 9\0Slot 10\0");
                    
                    ImGui::Spacing();
                    if (ImGui::Button("Execute Clone", ImVec2(150, 0))) {
                        std::string new_file;
                        SekiroAnalyzer temp_analyzer(target_file, ITEM_IDS_TXT, ITEM_IDS_TXT_SIZE);
                        if (temp_analyzer.clone_slot(slots[selected_slot_idx].slot_num, target_clone_slot + 1, new_file)) {
                            pfd::message("Success", "Cloning completed!\nNew file saved as:\n" + new_file, pfd::choice::ok, pfd::icon::info).result();
                        } else {
                            pfd::message("Error", "Failed to clone slot.", pfd::choice::ok, pfd::icon::error).result();
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                // Stats for selected slot
                const auto& slot = slots[selected_slot_idx];
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "STATISTICS:");
                
                int m = slot.playtime_seconds / 60;
                int s = slot.playtime_seconds % 60;
                int h = m / 60; m = m % 60;
                
                ImGui::Text("Time: %02d:%02d:%02d", h, m, s);
                ImGui::Text("Sen: %d", slot.sen);
                ImGui::Text("XP: %d", slot.xp);
                ImGui::Text("Vitality: %d/20", slot.vitalita);
                ImGui::Text("Attack Power: %d", slot.attack_power);
                ImGui::TextWrapped("Diff: %s", slot.diff_str.c_str());
            }
            
            float avail = ImGui::GetContentRegionAvail().y;
            if (avail > 40.0f) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + avail - 40.0f);
            } else {
                ImGui::Spacing(); ImGui::Spacing();
            }
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Developed by melloXIII");
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
            
            ImGui::TableSetColumnIndex(1);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            ImGui::BeginChild("Content", ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding);
        if (!slots.empty()) {
            const auto& slot = slots[selected_slot_idx];

            if (ImGui::BeginTabBar("Tabs")) {
                if (ImGui::BeginTabItem("Progression")) {
                    ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.79f, 0.64f, 0.33f, 1.0f), "MAIN COMPLETION");
                    ImGui::PopFont();
                    ImGui::Spacing();
                    
                    int total_boss = 14;
                    int defeated = total_boss - slot.missing_bosses.size();
                    ImGui::Text("Main Bosses");
                    ImGui::ProgressBar((float)defeated / total_boss, ImVec2(-1.0f, 18.0f), (std::to_string(defeated) + "/" + std::to_string(total_boss)).c_str());
                    
                    int total_beads = 40;
                    int found_beads = total_beads - slot.missing_beads.size();
                    ImGui::Spacing();
                    ImGui::Text("Prayer Beads (Mini-Bosses / Secrets)");
                    ImGui::ProgressBar((float)found_beads / total_beads, ImVec2(-1.0f, 18.0f), (std::to_string(found_beads) + "/" + std::to_string(total_beads)).c_str());
                    
                    int total_gourds = 9;
                    int found_gourds = total_gourds - slot.missing_gourds.size();
                    ImGui::Spacing();
                    ImGui::Text("Gourd Seeds");
                    ImGui::ProgressBar((float)found_gourds / total_gourds, ImVec2(-1.0f, 18.0f), (std::to_string(found_gourds) + "/" + std::to_string(total_gourds)).c_str());

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                    if (ImGui::BeginTable("SplitLists", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame)) {
                        ImGui::TableNextRow();
                        
                        // --- COMPLETED LIST ---
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushFont(font_bold);
                        ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "COMPLETED");
                        ImGui::PopFont();
                        ImGui::Spacing();
                        
                        if (ImGui::BeginChild("CompletedList", ImVec2(-4.0f, 0), true)) {
                            if (!slot.defeated_bosses.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ MAIN BOSSES ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.defeated_bosses) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.found_beads.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ PRAYER BEADS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.found_beads) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.found_gourds.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ GOURD SEEDS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.found_gourds) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }

                            if (!slot.found_skills.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ ESOTERIC TEXTS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.found_skills) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.found_upgrades.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ PROSTHETIC UPGRADES ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.found_upgrades) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.defeated_headless.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.55f, 0.65f, 0.55f, 1.0f), "[ HEADLESS / SHICHIMEN ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.defeated_headless) ImGui::BulletText("%s", m.c_str());
                            }
                        }
                        ImGui::EndChild();
                        
                        // --- MISSING LIST ---
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushFont(font_bold);
                        ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "MISSING");
                        ImGui::PopFont();
                        ImGui::Spacing();
                        
                        if (ImGui::BeginChild("MissingList", ImVec2(0, 0), true)) {
                            if (!slot.missing_bosses.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ MAIN BOSSES ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_bosses) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.missing_beads.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ PRAYER BEADS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_beads) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.missing_gourds.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ GOURD SEEDS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_gourds) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }

                            if (!slot.missing_skills.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ ESOTERIC TEXTS ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_skills) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.missing_upgrades.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ PROSTHETIC UPGRADES ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_upgrades) ImGui::BulletText("%s", m.c_str());
                                ImGui::Spacing();
                            }
                            if (!slot.missing_headless.empty()) {
                                ImGui::PushFont(font_bold);
                                ImGui::TextColored(ImVec4(0.75f, 0.45f, 0.40f, 1.0f), "[ HEADLESS / SHICHIMEN ]");
                                ImGui::PopFont();
                                for (const auto& m : slot.missing_headless) ImGui::BulletText("%s", m.c_str());
                            }
                        }
                        ImGui::EndChild();
                        
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Inventory")) {
                    ImGui::TextColored(ImVec4(0.79f, 0.64f, 0.33f, 1.0f), "DETAILED INVENTORY");
                    
                    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
                    
                    ImGui::PushFont(font_bold);
                    ImGui::TextColored(ImVec4(0.79f, 0.64f, 0.33f, 1.0f), "VERIFIED INVENTORY");
                    ImGui::PopFont();
                    ImGui::Spacing();
                    
                    static ImGuiTableFlags premium_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
                    
                    // Group items by category
                    std::map<std::string, std::vector<std::pair<uint32_t, std::string>>> cats;
                    for (const auto& item : slot.inventory_raw) {
                        std::string name = "Unknown Item";
                        auto it = slot.inventory_names.find(item.first);
                        if (it != slot.inventory_names.end()) name = it->second;
                        
                        std::string lower = name;
                        for (auto& c : lower) c = tolower(c);
                        
                        std::string cat = "9. Misc";
                        if (lower.find("prayer bead") != std::string::npos || lower.find("gourd") != std::string::npos || lower.find("seed") != std::string::npos) cat = "1. Upgrades";
                        else if (lower.find("memory") != std::string::npos || lower.find("remnant") != std::string::npos) cat = "2. Boss Memories";
                        else if (lower.find("spiritfall") != std::string::npos || lower.find("sugar") != std::string::npos || lower.find("balloon") != std::string::npos) cat = "3. Consumables (Sugars & Balloons)";
                        else if (lower.find("scrap") != std::string::npos || lower.find("magnetite") != std::string::npos || lower.find("lapis") != std::string::npos || lower.find("wax") != std::string::npos || lower.find("powder") != std::string::npos || lower.find("mercury") != std::string::npos || lower.find("resin") != std::string::npos) cat = "4. Upgrade Materials";
                        else if (lower.find("coin") != std::string::npos || lower.find("purse") != std::string::npos) cat = "5. Coin Purses";
                        else if (lower.find("text") != std::string::npos || lower.find("esoteric") != std::string::npos || lower.find("art") != std::string::npos || lower.find("skill") != std::string::npos) cat = "6. Texts and Skills";
                        else if (lower.find("prosthetic") != std::string::npos || lower.find("shuriken") != std::string::npos || lower.find("axe") != std::string::npos || lower.find("firecracker") != std::string::npos || lower.find("spark") != std::string::npos || lower.find("flame vent") != std::string::npos || lower.find("umbrella") != std::string::npos || lower.find("sabimaru") != std::string::npos || lower.find("spear") != std::string::npos || lower.find("divine abduction") != std::string::npos || lower.find("finger") != std::string::npos || lower.find("whistle") != std::string::npos || lower.find("mist raven") != std::string::npos || lower.find("feather") != std::string::npos || lower.find("echo") != std::string::npos || lower.find("malcontent") != std::string::npos || lower.find("vortex") != std::string::npos || lower.find("sacred flame") != std::string::npos || lower.find("kunai") != std::string::npos || lower.find("gouging top") != std::string::npos || lower.find("sen throw") != std::string::npos || lower.find("phantom") != std::string::npos) cat = "7. Prosthetic Tools & Upgrades";
                        else if (lower.find("pellet") != std::string::npos || lower.find("grass") != std::string::npos || lower.find("bite down") != std::string::npos || lower.find("tooth") != std::string::npos || lower.find("droplet") != std::string::npos || lower.find("medicine") != std::string::npos || lower.find("rice") != std::string::npos || lower.find("persimmon") != std::string::npos) cat = "8. Healing & Resurrection";
                        
                        cats[cat].push_back({item.second, name});
                    }

                    for (const auto& cat_pair : cats) {
                        if (ImGui::CollapsingHeader(cat_pair.first.substr(3).c_str())) {
                            if (ImGui::BeginTable(("InvTable_" + cat_pair.first).c_str(), 2, premium_flags)) {
                                ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                                ImGui::TableSetupColumn("Item Name", ImGuiTableColumnFlags_WidthStretch);
                                
                                for (const auto& item : cat_pair.second) {
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " x %d ", item.first);
                                    ImGui::TableSetColumnIndex(1);
                                    ImGui::Text("%s", item.second.c_str());
                                }
                                ImGui::EndTable();
                            }
                            ImGui::Spacing();
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild(); // Content
        ImGui::PopStyleVar();
        ImGui::EndTable(); // MainLayout
        } // if BeginTable

        ImGui::End(); // Main UI

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.16f, 0.15f, 0.13f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
