#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/perks/ShrinkingGaze.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

namespace {
	float GetSMTBonus(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->SMT_Bonus_Duration;
		}
		return 0.0f;
	}

	float GetSMTPenalty(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->SMT_Penalty_Duration;
		}
		return 0.0f;
	}

	void NullifySMTDuration(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->SMT_Bonus_Duration = 0.0f;
			transient->SMT_Penalty_Duration = 0.0f;
		}
	}
}

namespace Gts {

	std::string TinyCalamity::GetName() {
		return "TinyCalamity";
	}

	void TinyCalamity::OnStart() {
		//std::string message = std::format("While Tiny Calamity is active, your size-related actions are massively empowered, but your max scale is limited. You can perform all size-related actions (Vore, Grab, Hug Crush, etc) while being same size, but performing them wastes some of Tiny Calamity's duration.");
		//TutorialMessage(message, "Calamity");
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		
		
		Runtime::PlaySoundAtNode("TinyCalamitySound", caster, 1.0f, 1.0f, "NPC COM [COM ]");
		auto node = find_node(caster, "NPC Root [Root]");
		StartShrinkingGaze(caster);

		if (node) {
			NiPoint3 position = node->world.translate;
			float scale = get_visual_scale(caster);
			TinyCalamityExplosion(caster, 84);

			SpawnParticle(caster, 6.00f, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), position, scale * 3.0f, 7, nullptr); // Spawn
			Rumbling::For("TinyCalamity", caster, 4.0f, 0.14f, "NPC COM [COM ]", 0.10f, 0.0f);
		}
	}

	void TinyCalamity::OnUpdate() {
		const float BASE_POWER = 0.00035f;
		const float DUAL_CAST_BONUS = 2.0f;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		static Timer warningtimer = Timer(3.0f);
		float CasterScale = get_target_scale(caster);
		float bonus = GetSMTBonus(caster);
		float penalty = GetSMTPenalty(caster);

		float naturalscale = get_natural_scale(caster);

		if (bonus > 0.5f) {
			GetActiveEffect()->duration += bonus;

			NullifySMTDuration(caster);
		}
		if (penalty > 0.5f) {
			GetActiveEffect()->duration -= penalty;
			NullifySMTDuration(caster);
		}
		if (CasterScale < 1.0f) {// Disallow to be smaller than 1.5f to avoid weird interactions with others
			set_target_scale(caster, 1.0f);
		} else if (CasterScale > 1.5f) {
			update_target_scale(caster, -0.0300f, SizeEffectType::kNeutral);
			if (warningtimer.ShouldRun() && caster->formID == 0x14) {
				Notify("Im getting too big, it becomes hard to handle such power.");
			}
		} // <- Disallow having it when scale is > natural scale * 1.50f
	}

	void TinyCalamity::OnFinish() {
		auto caster = GetCaster();
		if (caster) {
			float CasterScale = get_target_scale(caster);
			float naturalscale = get_natural_scale(caster, true);
			if (CasterScale < naturalscale) {
				set_target_scale(caster, naturalscale);
			}
		}
	}
}
