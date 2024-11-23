#include "data/plugin.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	bool Plugin::Enabled() {
		return Plugin::GetSingleton().enabled.load();
	}

	bool Plugin::InGame() {
		auto ui = RE::UI::GetSingleton();
		if (!ui) {
			return false;
		}
		if (ui->IsMenuOpen(MainMenu::MENU_NAME)) {
			return false;
		}

		return Plugin::GetSingleton().ingame.load();
	}
	void Plugin::SetInGame(bool value) {
		Plugin::GetSingleton().ingame.store(value);
	}

	bool Plugin::Ready() {
		if (Plugin::InGame()) {
			// We are not loading or in the mainmenu
			auto player_char = RE::PlayerCharacter::GetSingleton();
			if (player_char) {
				if (player_char->Is3DLoaded()) {
					// Player is loaded
					return true;
				}
			}
		}
		return false;
	}

	bool Plugin::Live() {
		if (Plugin::Ready()) {
			auto ui = RE::UI::GetSingleton();
			if (!ui->GameIsPaused() || ui->GetMenu("RaceSex Menu")) {
				// Not paused
				return true;
			}
		}
		return false;
	}

	bool Plugin::AnyMenuOpen() {

		//There's no better way to do this :(
		//Static const means the list doesnt get recreated Each call. So no perf impact here.
		static const std::vector<std::string_view> Menus = {
			RE::CraftingMenu::MENU_NAME,
			RE::BarterMenu::MENU_NAME,
			RE::BookMenu::MENU_NAME,
			RE::ContainerMenu::MENU_NAME,
			RE::FavoritesMenu::MENU_NAME,
			RE::GiftMenu::MENU_NAME,
			RE::InventoryMenu::MENU_NAME,
			RE::JournalMenu::MENU_NAME,
			RE::LevelUpMenu::MENU_NAME,
			RE::LockpickingMenu::MENU_NAME,
			RE::MagicMenu::MENU_NAME,
			RE::MapMenu::MENU_NAME,
			RE::MessageBoxMenu::MENU_NAME,
			RE::RaceSexMenu::MENU_NAME,
			RE::SleepWaitMenu::MENU_NAME,
			RE::StatsMenu::MENU_NAME,
			RE::TrainingMenu::MENU_NAME,
			RE::TutorialMenu::MENU_NAME,
			RE::TweenMenu::MENU_NAME
		};

		if (Plugin::Ready()) {
			//Looping Through the list Is O(n) (almost) no perf impact here.
			auto ui = RE::UI::GetSingleton();
			for (const auto& Menu : Menus) {
				if (ui->IsMenuOpen(Menu)) {
					//log::debug("Menu is open: {}", Menu);
					return true;
				}
			}
		}
		return false;
	}


	bool Plugin::OnMainThread() {
		return Plugin::GetSingleton().onmainthread.load();
	}
	void Plugin::SetOnMainThread(bool value) {
		Plugin::GetSingleton().onmainthread.store(value);
	}

	Plugin& Plugin::GetSingleton() {
		static Plugin instance;
		return instance;
	}
}