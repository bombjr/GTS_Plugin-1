#pragma once

#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Damage/TinyCalamity.hpp"

namespace GTS {

    inline bool IsGtsBusy_ForControls(Actor* actor) {
		bool GTSBusy = false;
		actor->GetGraphVariableBool("GTS_Busy", GTSBusy);
        // Have to use this because Hand Swipes make original bool return false

		return GTSBusy;
	}

	inline bool AllowToPerformSneak(RE::IDEvent* id) {
		bool allow = true;
		if (id) {
			auto player = PlayerCharacter::GetSingleton();
			if (player) {
				auto as_str = id->userEvent;
				if (as_str == "Sneak" && IsProning(player)) {
					if (player->IsSneaking()) {
						allow = false;
						AnimationManager::StartAnim("SBO_ProneOff", player);
					}
				} else if (as_str == "Activate") {
					if (TinyCalamity_WrathfulCalamity(player)) {
						allow = false;
					}
				}
			}
		}
		return allow;
	}

	inline bool CanMove() {
        auto player = PlayerCharacter::GetSingleton();
        if (!player) {
            return true;
        }
		Actor* Controlled = GetPlayerOrControlled();
		if (Controlled->formID != 0x14) {
			if (IsThighSandwiching(Controlled)) { // Disallow player movement if we have control over other actor and actor does thigh sandwich
				return false;
			} if (IsBetweenBreasts(Controlled)) {
				return false;
			}
		}
		if (IsFreeCameraEnabled()) {
			return true;
		}
		if (!AnimationsInstalled(player)) { // Don't mess with movement if user didn't install anims correctly
			return true;
		}
		if (IsTransitioning(player)) { // Disallow to move during transition
			return false;
		}
		if (IsGrabAttacking(player)) { // Allow movement for Grab Attacking
			return true;
		}
        return !IsGtsBusy_ForControls(player); // Else return GTS Busy
    }
}

namespace Hooks {

	template <class T>
	class Hook_Controls: public T { // HUGE Credits to Vermunds (SkyrimSoulsRE source code)!
		
	public:
		using CanProcess_t = decltype(&T::CanProcess);
		static inline REL::Relocation<CanProcess_t> _CanProcess;  

		bool HookMovement(RE::InputEvent* a_event);

		static void Hook(REL::Relocation<std::uintptr_t> a_vtbl);
	};

	template <class T>
	inline bool Hook_Controls<T>::HookMovement(RE::InputEvent* a_event) {
		if (!CanMove()) {
			return false;
		}

		auto id = a_event->AsIDEvent();
		if (!AllowToPerformSneak(id)) {
			return false;
		}

		return _CanProcess(this, a_event);
	}

	template <class T>
	inline void Hook_Controls<T>::Hook(REL::Relocation<std::uintptr_t> a_vtbl) {
		_CanProcess = a_vtbl.write_vfunc(0x1, &HookMovement);
	}
}
