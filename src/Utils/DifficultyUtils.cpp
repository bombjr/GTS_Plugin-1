#include "Utils/DifficultyUtils.hpp"

using namespace GTS;

namespace {

    const std::vector<const char*> Difficulty_ByPC{
        "fDiffMultHPByPCVE",
        "fDiffMultHPByPCE",
        "fDiffMultHPByPCN",
        "fDiffMultHPByPCH",
        "fDiffMultHPByPCVH",
        "fDiffMultHPByPCL"
    };

    const std::vector<const char*> Difficulty_ToPC{
        "fDiffMultHPToPCVE",
        "fDiffMultHPToPCE",
        "fDiffMultHPToPCN",
        "fDiffMultHPToPCH",
        "fDiffMultHPToPCVH",
        "fDiffMultHPToPCL",
    };
}

namespace GTS {

    float GetSettingValue(const char* setting) {
		float modifier = 1.0f;
		auto GameSetting = GameSettingCollection::GetSingleton();
		if (GameSetting) {
			modifier = GameSetting->GetSetting(setting)->GetFloat();
		}
		//log::info("Difficulty Modifier: {}", modifier);
		return modifier;
	}

	float GetDifficultyMultiplier(Actor* attacker, Actor* receiver) { // Credits to Doodlum for this method
		if (attacker && (attacker->IsPlayerRef() || IsTeammate(attacker))) {
			auto currentdiff = PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
            //log::info("Current By PC Difficulty: {}", Difficulty_ByPC[currentdiff]);
			return GetSettingValue(Difficulty_ByPC[currentdiff]);
		}
    	else if (receiver && (receiver->IsPlayerRef() || IsTeammate(attacker))) {
			auto currentdiff = PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
            //log::info("Current To PC Difficulty: {}", Difficulty_ToPC[currentdiff]);
			return GetSettingValue(Difficulty_ToPC[currentdiff]);
		}
		return 1.0f;
	}
}