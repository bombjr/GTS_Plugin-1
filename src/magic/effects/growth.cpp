#include "magic/effects/growth.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string Growth::GetName() {
		return "Growth";
	}

	void Growth::OnUpdate() {
		const float BASE_POWER = 0.00150f;
		const float DUAL_CAST_BONUS = 2.0f;


		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float GtsSkillLevel = GetGtsSkillLevel(caster);

		float SkillMult = 1.0f + (GtsSkillLevel * 0.01f);
		float HpRegen = GetMaxAV(caster, ActorValue::kHealth) * 0.00005f;

		float bonus = 1.0f;

		if (Runtime::HasMagicEffect(caster,"EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25f + 0.75f;
		}

		float power = BASE_POWER * SkillMult;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("GrowthSpellAdept")) {
			power *= 1.33f;
		} else if (base_spell == Runtime::GetMagicEffect("GrowthSpellExpert")) {
			power *= 1.75f;
			caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}


		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}

		Grow(caster, 0.0f, power * bonus);


		float mult = 0.15f;
		if (caster->IsInCombat()) {
			mult = 1.0f;
		}

		AdvanceSkill(caster, ActorValue::kAlteration, power * bonus, 1.0f); // Gain vanilla Alteration xp
	}
}
