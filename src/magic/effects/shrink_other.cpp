#include "magic/effects/shrink_other.hpp"
#include "magic/effects/common.hpp"
#include "ActionSettings.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string ShrinkOther::GetName() {
		return "ShrinkOther";
	}

	void ShrinkOther::OnUpdate() {
		const float BASE_POWER = 0.00180f;
		const float CRUSH_BONUS = 0.00180f;
		const float GROWTH_AMOUNT_BONUS = 1.4f;
		const float DUAL_CAST_BONUS = 2.0f;
		const float SCALE_FACTOR = 0.5f;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float power = BASE_POWER;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("ShrinkAllyAdept")) {
			power *= 1.33f;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkAllyExpert")) {
			power *= 1.75f;
		}

		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		if (Runtime::GetFloat("CrushGrowthRate") >= GROWTH_AMOUNT_BONUS) {
			power += CRUSH_BONUS;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}

		if (target_scale > Minimum_Actor_Scale) {
			if (!IsHostile(target, caster)) {
				ShrinkActor(target, power*0.10f, 0);
			}
		} else {
			set_target_scale(target, Minimum_Actor_Scale);
		}
	}
}
