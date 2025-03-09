#pragma once

namespace GTS {

    enum class Difficulty : std::int32_t {
		Novice = 0,
		Apprentice = 1,
		Adept = 2,
		Expert = 3,
		Master = 4,
		Legendary = 5
	};

    float GetSettingValue(const char* setting);
    float GetDifficultyMultiplier(Actor* attacker, Actor* receiver);
}