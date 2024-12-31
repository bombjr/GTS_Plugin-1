#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/slow_grow.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"

namespace {
	bool PerformMoanAndParticle(Actor* caster) {
		if (caster && IsFemale(caster) && !IsActionOnCooldown(caster, CooldownSource::Emotion_Moan)) {
			for (auto Foot: {"NPC L Foot [Lft ]", "NPC R Foot [Rft ]"}) {
				auto FootNode = find_node(caster, Foot);
				if (FootNode) {
					SpawnCustomParticle(caster, ParticleType::Green, FootNode->world.translate, Foot, 0.75f);
				}
			}

			if (Runtime::GetFloat("AllowMoanSounds") == 1.0f) {
				Task_FacialEmotionTask_Moan(caster, 1.0f + RandomFloat(0.0f, 0.25f), "SlowGrow");
				float MoanVolume = std::clamp(get_visual_scale(caster)/8.0f, 0.25f, 1.0f);
				PlayMoanSound(caster, MoanVolume);
			}
			ApplyActionCooldown(caster, CooldownSource::Emotion_Moan);
			return true;
		}
		return false;
	}
}

namespace Gts {
	std::string SlowGrow::GetName() {
		return "SlowGrow";
	}

	SlowGrow::SlowGrow(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("SlowGrowth")) {
			this->IsDual = false;
		} if (base_spell == Runtime::GetMagicEffect("SlowGrowthDual")) {
			this->IsDual = true;
		}
	}

	void SlowGrow::OnStart() {
		Actor* caster = GetCaster();
		if (caster) {
			float scale = get_visual_scale(caster);
			float mult = 0.40f;
			if (this->IsDual) {
				Rumbling::For("SlowGrow", caster, Rumble_Growth_SlowGrowth_Start, 0.10f, "NPC COM [COM ]", 0.35f, 0.0f);
				mult *= 1.5f;
			}
			SpawnCustomParticle(caster, ParticleType::Green, NiPoint3(), "NPC COM [COM ]", scale * mult * 1.75f);
		}
	}

	void SlowGrow::OnUpdate() {
		const float BASE_POWER = 0.000025f; // Default growth over time.
		const float DUAL_CAST_BONUS = 2.25f;
		auto caster = GetCaster();

		if (caster) {
			auto GtsSkillLevel = GetGtsSkillLevel(caster);

			float SkillBonus = 1.0f + (GtsSkillLevel * 0.01f); // Calculate bonus power. At the Alteration/Size Mastery of 100 it becomes 200%.
			float power = BASE_POWER * SkillBonus;
			float bonus = 1.0f;

			if (this->timer.ShouldRun()) {
				float Volume = std::clamp(get_visual_scale(caster)/8.0f, 0.20f, 1.0f);
				Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0f,  "NPC Pelvis [Pelv]");
			}
			
			if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
				bonus = get_visual_scale(caster) * 0.25f + 0.75f;
			}

			if (this->IsDual) {
				power *= DUAL_CAST_BONUS;
			}

			PerformMoanAndParticle(caster) ? power *= 320.0f : power *= 0.625f;
			// Mini growth spurts, else weaker growth over time

			Rumbling::Once("SlowGrow", caster, Rumble_Growth_SlowGrowth_Loop, 0.05f);
			Grow(caster, 0.0f, power * bonus);
			
			//log::info("Slowly Growing, actor: {}", caster->GetDisplayFullName());
		}
	}

	void SlowGrow::OnFinish() {
	}
}
