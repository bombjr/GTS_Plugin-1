#include "Managers/Cameras/FP/Normal.hpp"

namespace GTS {

	float FirstPerson::GetScaleOverride(bool IsCrawling) {
		return GetProneAdjustment();
	}
}
