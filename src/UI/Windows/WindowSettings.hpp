#pragma once

#include "UI/ImGUI/ImWindow.hpp"
#include "UI/ImGUI/ImCategoryContainer.hpp"
#include "UI/ImGUI/ImFontManager.hpp"
#include "UI/ImGUI/ImStyleManager.hpp"

#include "Config/Config.hpp"
#include "Config/Keybinds.hpp"

namespace GTS {

    class WindowSettings : public ImWindow {

        public:
        
        WindowSettings();

        void Draw() override;

        inline bool ShouldDraw() override {
            return this->Show;
        }

        inline float GetAlphaMult() override {
            return sUI.fAlpha;
        }

        inline float GetBGAlphaMult() override {
            return sUI.fBGAlphaMult;
        }

        inline void SetDisabled(bool a_Disabled) {
            Disabled = a_Disabled;
        }

        void AsyncSave() {

            if (SaveLoadBusy.load()) {
                return;
            }

            //TODO Use std::async instead of spawning a new thread. So we can get a return value.
            SaveLoadBusy.store(true);
            std::thread(&WindowSettings::SaveImpl, this).detach();
        }

        void AsyncLoad() {

            if (SaveLoadBusy.load()) {
                return;
            }

            //TODO Use std::async instead of spawning a new thread. So we can get a return value.
            SaveLoadBusy.store(true);
            std::thread(&WindowSettings::LoadImpl, this).detach();
        }


        private:
        void LoadImpl();
        void SaveImpl();

        void SetErrorAndUnlock(const std::string& error) {
            ErrorString = error;
            SaveLoadBusy.store(false);
        }

        static constexpr const char* kLoadSettingsError = "Could Not Load Settings! Check GTSPlugin.log for more info";
        static constexpr const char* kLoadInputError = "Could Not Load Input Settings! Check GTSPlugin.log for more info";
        static constexpr const char* kSaveSettingsError = "Could Not Save Settings! Check GTSPlugin.log for more info.";
        static constexpr const char* kSaveInputError = "Could Not Save Input Settings! Check GTSPlugin.log for more info.";


        std::atomic<bool> SaveLoadBusy = false;

        std::string ErrorString;
        ImCategoryManager& CatMgr = ImCategoryManager::GetSingleton();
        ImFontManager& FontMgr = ImFontManager::GetSingleton();
        ImStyleManager& StyleMgr = ImStyleManager::GetSingleton();

        Config& Settings = Config::GetSingleton();
        Keybinds& KeyMgr = Keybinds::GetSingleton();
        const SettingsHidden& sHidden = Config::GetHidden();
        const WindowConfSettings& sUI= Config::GetUI().SettingsWindow;
        bool Disabled = false;
    };
}