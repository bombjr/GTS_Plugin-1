#pragma once

#include "Managers/Animation/Utils/CooldownManager.hpp"







namespace GTS
{

	class Animation_Cleavage {
		public:
			static void AttemptBreastActionOnTiny(const std::string& pass_anim, Actor* giant = PlayerCharacter::GetSingleton());
			static void LaunchCooldownFor(Actor* giant, CooldownSource Source);
			static void RegisterEvents();
			static void RegisterTriggers();
	};
}
