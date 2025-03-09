#pragma once

#include "UI/DearImGui/imgui.h"
#include "Config/Config.hpp"

namespace GTS {

    class ImFontManager {
        private:

        //Consts
        const std::string _ext = ".ttf";
        const std::string _basePath = R"(Data\SKSE\Plugins\GTSPlugin\Fonts\)";

        //Jost mimicks futura <- Skyrim Default UI Font
        //OpenSans mimicks Arial <- Skyrim Default UI Font for the console

        const std::string _Jost_Light = _basePath + R"(Jost\Jost-Light)" + _ext;
        const std::string _Jost_Regular = _basePath + R"(Jost\Jost-Regular)" + _ext;
        const std::string _Jost_Medium = _basePath + R"(Jost\Jost-Medium)" + _ext;
        const std::string _OpenSans_Regular = _basePath + R"(OpenSans\OpenSans-Regular)" + _ext;

        const SettingsUI& Settings = Config::GetUI();

        //Structs
        typedef struct FontData {
            std::string name;
            std::string path;
            float size;
            ImFontConfig* conf = new ImFontConfig();
            ImFont* font;
            FontData(const std::string& name, const std::string& path, float size) : name(name), path(path), size(size) {
                //8 is probably overkill...
                //By directly chaning the font scale this hack is no longer needed.
                conf->OversampleH = 4;
                conf->OversampleV = 3;
            }
        } FontData;

        enum class AQueueType {
            kRasterizerScale,
            kRebuildAtlas
        };

        //Lists
        std::stack<std::pair<AQueueType, float>> ActionQueue;
        std::unordered_map<std::string, FontData*> Fonts;

        //Funcs
        void ChangeRasterizerScaleImpl(float a_scale);
        void RebuildFontAtlasImpl() const;
        void BuildFontsInt() const;

        public:
        ~ImFontManager() = default;

        [[nodiscard]] static inline ImFontManager& GetSingleton() {
            static ImFontManager instance;
            return instance;
        }

        inline void PushAction(AQueueType a_type, const float a_value){
            ActionQueue.emplace(a_type, a_value);
        }

        //Queuable Actions
        inline void ChangeRasterizerScale(const float a_scale) {
            PushAction(AQueueType::kRasterizerScale,a_scale);
        }

        inline void RebuildFonts() {
            PushAction(AQueueType::kRebuildAtlas,1.0);
        }

        void Init();
        void AddFont(FontData* a_font);
        [[nodiscard]] static ImFont* GetFont(const std::string& fontName);
        void ProcessActions();

    };
}