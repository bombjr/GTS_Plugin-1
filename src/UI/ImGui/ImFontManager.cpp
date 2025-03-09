#include "UI/ImGui/ImFontManager.hpp"
#include "UI/DearImGui/imgui_impl_dx11.h"

namespace GTS {

    void ImFontManager::Init() {
        //Initalzie font data, add any new font defines here.
        //Fontname, Path to ttf font file, font scale
        AddFont(new FontData("sidebar", _Jost_Regular, 34.0f));
        AddFont(new FontData("title", _Jost_Medium, 56.0f));
        AddFont(new FontData("footer", _Jost_Medium, 30.0f));
        AddFont(new FontData("text", _OpenSans_Regular, 20.0f));
        AddFont(new FontData("errortext", _OpenSans_Regular, 22.0f));
        AddFont(new FontData("subscript", _OpenSans_Regular, 16.0f));
        AddFont(new FontData("widgetbody", _Jost_Regular, 21.0f));
        AddFont(new FontData("widgettitle", _Jost_Light, 34.0f));

        BuildFontsInt();

        for (auto& value : Fonts | views::values) {
            if (value->font == nullptr) {
                logger::critical("Could not load font: {}", value->name);
                const std::string msg = fmt::format("Could not load ImGui Font {}.\nThe game will now close.", value->name);
                ReportAndExit(msg);
            }
        }
    }

    void ImFontManager::AddFont(FontData* a_font) {
        if (a_font) {
            Fonts[a_font->name] = a_font;
        }
    }

    void ImFontManager::BuildFontsInt() const {
        ImGuiIO& io = ImGui::GetIO();
        ImFontAtlas* fontAtlas = io.Fonts;
        fontAtlas->Clear();
        fontAtlas->AddFontDefault();
        for (const auto& value : Fonts | views::values) {
            value->font = fontAtlas->AddFontFromFileTTF(value->path.c_str(), value->size * Settings.fScale, value->conf);
        }
    }

    // Utility function to get a font with fallback
    ImFont* ImFontManager::GetFont(const std::string& fontName) {
        auto F = ImFontManager::GetSingleton();
        auto it = F.Fonts.find(fontName);
        if (it != F.Fonts.end()) {
            return it->second->font;  // Font found
        }
        return ImGui::GetIO().Fonts->Fonts[0];    // Fallback to default font
    }

    void ImFontManager::ProcessActions() {
        if (!ActionQueue.empty()) {

            auto [type, value] = ActionQueue.top();

            switch (type) {

                case AQueueType::kRasterizerScale: {
                    ChangeRasterizerScaleImpl(value);
                    break;
                }

                case AQueueType::kRebuildAtlas: {
                    RebuildFontAtlasImpl();
                    break;
                }

                default: {
                    logger::warn("ImFontmanager: Unimplemented Action!");
                    break;
                }
            }
            ActionQueue.pop();
        }
    }

    void ImFontManager::ChangeRasterizerScaleImpl(float a_scale) {
        for (const auto& value : Fonts | views::values) {
            if (value->font != nullptr) {
                value->conf->RasterizerDensity = a_scale;
            }
        }
        RebuildFontAtlasImpl();
    }

    void ImFontManager::RebuildFontAtlasImpl() const {

        BuildFontsInt();
        ImGui::GetIO().Fonts->ClearTexData();
        ImGui::GetIO().Fonts->Build();

        //Causes a complete reinit of the imgui context
        //Idealy we'd only want to invalidate the font related stuff
        //But that appears to not be possible
        //As long as you dont spam this it's fine.
        //Even if you did it doesn't cause a memleak. It just lags for ~100ms on each call
        ImGui_ImplDX11_InvalidateDeviceObjects();
    }
}
