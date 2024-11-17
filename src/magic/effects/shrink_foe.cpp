#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/shrink_foe.hpp"
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
	void ResetMovementSlowdown(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->MovementSlowdown = 1.0f;
		}
	}
	void SetMovementSlowdown(Actor* giant, Actor* tiny) { 
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			float slow = 0.75f;
			if (Runtime::HasPerkTeam(giant, "FastShrink")) {
				slow += 0.05f;
			}
			if (Runtime::HasPerkTeam(giant, "LethalShrink")) {
				slow += 0.05f;
			}
			transient->MovementSlowdown = slow;
		}
	}
}


namespace Gts {
	std::string ShrinkFoe::GetName() {
		return "ShrinkFoe";
	}

	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) : Magic(effect) {
		const float SHRINK_POWER = 2.15f; // Power = Shrink Power
		const float SHRINK_EFFIC = 0.16f; // Efficiency = size steal efficiency.
		const float SHRINK_AOE_POWER = 2.40f;
		const float SHRINK_AOE_EFFIC = 0.18f;
		const float SHRINK_AOE_MASTER_POWER = 2.70f;
		const float SHRINK_AOE_MASTER_EFFIC = 0.20f;
		const float SHRINK_BOLT_POWER = 6.0f;
		const float SHRINK_BOLT_EFFIC = 0.06f;
		const float SHRINK_STORM_POWER = 12.0f;
		const float SHRINK_STORM_EFFIC = 0.03f;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("ShrinkEnemy")) {
			this->power = SHRINK_POWER;
			this->efficiency = SHRINK_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOE")) {
			this->power = SHRINK_AOE_POWER;
			this->efficiency = SHRINK_AOE_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOEMast")) {
			// ShrinkEnemyAOEMast
			this->power = SHRINK_AOE_MASTER_POWER;
			this->efficiency = SHRINK_AOE_MASTER_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkBolt")) {
			// ShrinkBolt
			this->power = SHRINK_BOLT_POWER;
			this->efficiency = SHRINK_BOLT_EFFIC;
			this->time_mult = 0.2f;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkStorm")) {
			// ShrinkBolt
			this->power = SHRINK_STORM_POWER;
			this->efficiency = SHRINK_STORM_EFFIC;
			this->time_mult = 0.2f;
		}
	}

	void ShrinkFoe::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		if (!IsEssential_WithIcons(caster, target)) {
			float sizediff = GetSizeDifference(caster, target, SizeType::VisualScale, true, false);
			if (this->power >= 18.00f && sizediff > 4.0f) {
				StaggerActor(caster, target, 100.0f);
			}
			SetMovementSlowdown(caster, target);
		}
	}

	void ShrinkFoe::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		if (caster == target) {
			return;
		}

		if (IsEssential(caster, target)) {
			return; // Disallow shrinking Essentials
		}

		auto& Persist = Persistent::GetSingleton();
		float SizeDifference = 1.0f;
		float bonus = 1.0f;
		float balancemodebonus = 1.0f;
		float shrink = this->power * 3.2f;
		float gainpower = this->efficiency;
		auto actor_data = Persist.GetData(target);
		
		if (this->power >= 18.00f) {
			if (actor_data) {
				actor_data->half_life = 0.25f; // Faster shrink, less smooth.
			}
			SizeDifference = 1.0f / std::clamp((get_visual_scale(target) * GetSizeFromBoundingBox(target)), 0.25f, 1.0f);
		} else if (this->power >= 10.0f) {
			if (actor_data) {
				actor_data->half_life = 0.50f; // Faster shrink, less smooth.
			}
			SizeDifference = 1.0f / std::clamp((get_visual_scale(target) * GetSizeFromBoundingBox(target)), 0.50f, 1.0f);
		} else {
			if (actor_data) {
				actor_data->half_life = 1.0f;
			}
		}

		if (target->IsDead()) {
			bonus = 2.5f;
			gainpower *= 0.20f;
		}

		if (caster->formID == 0x14 && SizeManager::GetSingleton().BalancedMode() == 2.0f) { // This is checked only if Balance Mode is enabled.
			balancemodebonus = 0.5f;
		}

		float HealthPercent = std::clamp(GetHealthPercentage(target), 0.25f, 1.0f);
		shrink /= HealthPercent;

		

		Attacked(target, caster); // make it work like a hostile spell

		ChanceToScare(caster, target, 5.0f, 1200, true);

		if (ShrinkToNothing(caster, target, true, this->time_mult)) { // STN when size difference is met and when time ticks allow to

		} else {
			if (get_target_scale(target) >= SHRINK_TO_NOTHING_SCALE / GetSizeFromBoundingBox(target)) {
				TransferSize(caster, target, IsDualCasting(), shrink * SizeDifference * bonus, gainpower * balancemodebonus, false, ShrinkSource::Magic);
			}
		}
	}

	void ShrinkFoe::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();

		if (!caster) {
			return;
		}
		if (!target) {
			return;
		}
		if (!IsEssential(caster, target)) {
			Task_TrackSizeTask(caster, target, "Spell", true, this->time_mult);
			ResetMovementSlowdown(target);
		}
	}
}
