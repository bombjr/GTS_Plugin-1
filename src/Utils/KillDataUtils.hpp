#pragma once

namespace GTS {

    enum class SizeKillType {
		kTotalKills = 0,

		kShrunkToNothing = 1,
		kOtherSources = 2,

		kBreastAbsorbed = 3,
		kBreastCrushed = 4,

		kHugCrushed = 5,
		kGrabCrushed = 6,
		kButtCrushed = 7,

		kThighCrushed = 8,
		kThighSuffocated = 9,
		kThighSandwiched = 10,
		kThighGrinded = 11,

		kFingerCrushed = 12,

		kErasedFromExistence = 13,

		kAbsorbed = 14,
		kCrushed = 15,
		kKicked = 16,
		kEaten = 17,

		kGrinded = 18,
	};

    void IncrementKillCount(Actor* giant, SizeKillType Type);
    int GetKillCount(Actor* giant, SizeKillType Type);
}