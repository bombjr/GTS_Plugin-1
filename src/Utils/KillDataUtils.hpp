#pragma once

namespace GTS {

    enum class SizeKillType {
		kTotalKills = 0,

		kShrunkToNothing = 1,
		kOtherSources = 2,

		kBreastAbsorbed = 3,
		kBreastCrushed = 4,
		kBreastSuffocated = 5,

		kHugCrushed = 6,
		kGrabCrushed = 7,
		kButtCrushed = 8,

		kThighCrushed = 9,
		kThighSuffocated = 10,
		kThighSandwiched = 11,
		kThighGrinded = 12,

		kFingerCrushed = 13,

		kErasedFromExistence = 14,

		kAbsorbed = 15,
		kCrushed = 16,
		kKicked = 17,
		kEaten = 18,

		kGrinded = 19,
	};

    void IncrementKillCount(Actor* giant, SizeKillType Type);
    uint32_t GetKillCount(Actor* giant, SizeKillType Type);
}