#include "magic/effects/Potions/ShrinkPotion.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"


namespace Gts {
	std::string ShrinkPotion::GetName() {
		return "ShrinkPotion";
	}

	void ShrinkPotion::OnStart() {
		auto caster = GetCaster();
		auto player = PlayerCharacter::GetSingleton();
		if (!caster || !player) {
			return;
		}

		Rumbling::Once("ShrinkPotion", caster, 2.0f, 0.05f);

		Potion_Penalty(caster);

		float Volume = std::clamp(get_visual_scale(caster)/8.0f, 0.15f, 2.0f);
		Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0f, "NPC Pelvis [Pelv]");
		log::info("Growth Potion start actor: {}", caster->GetDisplayFullName());
	}

	void ShrinkPotion::OnUpdate() {
		const float BASE_POWER = 0.000480f;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = std::clamp(caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100.0f + 1.0f, 1.0f, 2.0f);
		Rumbling::Once("ShrinkPotion", caster, 0.4f, 0.05f);

		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		if (get_target_scale(caster) > 0.12f) {
			ShrinkActor(caster, Power, 0.0f);
		} else {
			set_target_scale(caster, 0.12f);
		}
	}

	void ShrinkPotion::OnFinish() {
	}
}
