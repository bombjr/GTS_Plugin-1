#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/shrink_cloak.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"

namespace {
	/// Currently this magic is unused
}


namespace Gts {
	std::string ShrinkCloak::GetName() {
		return "ShrinkCloak";
	}

	ShrinkCloak::ShrinkCloak(ActiveEffect* effect) : Magic(effect) {
		this->power = GetActiveEffect()->magnitude;
	}

	void ShrinkCloak::OnStart() {
	}

	void ShrinkCloak::OnUpdate() {
        auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		if (caster != target) {
			log::info("Applying Shrink Cloak to {}", target->GetDisplayFullName());
			if (ShrinkToNothing(caster, target, true, this->time_mult)) { // STN when size difference is met and when time ticks allow to
			} else {
				if (get_target_scale(target) >= SHRINK_TO_NOTHING_SCALE / GetSizeFromBoundingBox(target)) {
					TransferSize(caster, target, IsDualCasting(), this->power * 0.0166f, 0.0f, false, ShrinkSource::Magic);
				}
			}
		}
	}

	void ShrinkCloak::OnFinish() {
	}
}
