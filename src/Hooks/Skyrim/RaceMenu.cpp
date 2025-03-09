#include "Hooks/Skyrim/RaceMenu.hpp"

using namespace GTS;

namespace Hooks {

    void Hook_RaceMenu::Hook(Trampoline& trampoline) {
        static FunctionHook<void(Actor* actor, TESRace* a_race, bool a_player)>SwitchRaceHook(
            REL::RelocationID(36901, 37925),
            [](Actor* actor, TESRace* a_race, bool a_player) {
                if (actor) {
                    if (actor->formID == 0x14) { // Updates natural scale of Player when changing races
                        log::info("SwitchRace hooked!");
                        RefreshInitialScales(actor);
                    }
                }
                    
                return SwitchRaceHook(actor, a_race, a_player);
            }
        );
    }
}