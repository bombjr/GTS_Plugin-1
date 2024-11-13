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
				mult = 0.85f;
			}
			SpawnCustomParticle(caster, ParticleType::Green, NiPoint3(), "NPC Root [Root]", scale * mult);
		}
	}

	void SlowGrow::OnUpdate() {
		const float BASE_POWER = 0.000025f; // Default growth over time.
		const float DUAL_CAST_BONUS = 2.25f;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto GtsSkillLevel = GetGtsSkillLevel(caster);

		float SkillBonus = 1.0f + (GtsSkillLevel * 0.01f); // Calculate bonus power. At the Alteration/Siz Mastery of 100 it becomes 200%.
		float power = BASE_POWER * SkillBonus;

		if (this->timer.ShouldRun()) {
			float Volume = std::clamp(get_visual_scale(caster)/8.0f, 0.20f, 1.0f);
			Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0f,  "NPC Pelvis [Pelv]");
		}
		if (Runtime::GetFloat("AllowMoanSounds") == 1.0f && this->MoanTimer.ShouldRun() && IsFemale(caster)) {
			float MoanVolume = std::clamp(get_visual_scale(caster)/8.0f, 0.25f, 1.0f);
			Task_FacialEmotionTask_Moan(caster, 1.4f, "SlowGrow");
			PlayMoanSound(caster, MoanVolume);
			//log::info("Attempting to play Moan Sound for: {}", caster->GetDisplayFullName());
		}
		float bonus = 1.0f;
		if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25f + 0.75f;
		}

		if (this->IsDual) {
			power *= DUAL_CAST_BONUS;
		}

		Grow(caster, 0.0f, power * bonus);
		Rumbling::Once("SlowGrow", caster, Rumble_Growth_SlowGrowth_Loop, 0.05f);
		//log::info("Slowly Growing, actor: {}", caster->GetDisplayFullName());
	}

	void SlowGrow::OnFinish() {
	}
}
