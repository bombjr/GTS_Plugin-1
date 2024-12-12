#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Stomp_Under_Butt.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/audio/footstep.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void DoImpactRumble(Actor* giant, std::string_view node, std::string_view name, bool hh, float magnitude) {
		float shake_power = Rumble_Stomp_Under_Strong;
		float smt = HasSMT(giant) ? 1.5f : 1.0f;
        if (hh) {
		    smt *= GetHighHeelsBonusDamage(giant, true);
        }
		Rumbling::Once(name, giant, shake_power * smt * magnitude, 0.0f, node, 1.25f);
	}

	void UnderStomp_RecoveryStomp(Actor* giant, bool right, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float damage = 1.0f;
		float SMT = 1.0f;

		if (HasSMT(giant)) {
			damage = 1.25f;
			SMT = 1.75f; // Larger Dust
		}

		DoDamageEffect(giant, Damage_Walk_Defaut * damage * perk, Radius_Stomp_Strong, 8, 0.30f, Event, 1.0f, Source, false);
		DoDustExplosion(giant, 1.0f * (SMT), Event, Node);
        DoImpactRumble(giant, Node, rumble, true, 0.8f);
        
        DoFootstepSound(giant, SMT, Event, Node);

		FootStepManager::PlayVanillaFootstepSounds(giant, right);
	}

    void UnderStomp_FootTipDamage(Actor* giant, bool right, FootEvent Event, DamageSource Source, std::string_view Node) {
		float perk = GetPerkBonus_Basics(giant);
		float SMT = 1.0f;
		float damage = 1.0f;
		if (HasSMT(giant)) {
			damage = 1.25f;
			SMT = 1.75f; // Larger Dust
		}
		std::string taskname = std::format("StrongUnderStomp_{}", giant->formID);
		ActorHandle giantHandle = giant->CreateRefHandle();

		double Start = Time::WorldTimeElapsed();
		
		TaskManager::RunFor(taskname, 1.0f, [=](auto& update){ // Needed because anim has wrong timing
			if (!giantHandle) {
				return false;
			}

			double Finish = Time::WorldTimeElapsed();
			auto giantref = giantHandle.get().get();

			if (Finish - Start >= 0.0) { 
				DoDamageEffect(giantref, Damage_Stomp_Under_LegLand * damage * perk, Radius_Stomp_Strong, 8, 0.20f, Event, 1.0f, Source, false);
				DoDustExplosion(giantref, 1.0f * (SMT), Event, Node);
				DoImpactRumble(giantref, Node, "FootTip", true, 1.0f);
			
				DoFootstepSound(giantref, SMT, Event, Node);

				LaunchTask(giantref, 0.85f * perk, 1.40f, Event);

				FootStepManager::PlayVanillaFootstepSounds(giantref, right);
				return false;
			}
			return true;
		});
	}

    void UnderStomp_LegImpacts(Actor* giant) {
        for (auto LeftOrRight:{true, false}) {
            ApplyThighDamage(giant, LeftOrRight, false, Radius_ThighCrush_Spread_In, Damage_Stomp_Under_LegLand, 0.15f, 1.0f, 8, DamageSource::ThighCrushed);
        }

        UnderStomp_FootTipDamage(giant, false, FootEvent::Left, DamageSource::CrushedLeft, LNode);
        UnderStomp_FootTipDamage(giant, true, FootEvent::Right, DamageSource::CrushedRight, RNode);
    }

    void UnderStomp_DoButtDamage(Actor* giant) {
		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.0f;
		float smt = 1.0f;

		if (HasSMT(giant)) {
			dust = 1.25f;
			smt = 1.5f;
		}
		std::string taskname = std::format("UnderButtCrushAttack_{}", giant->formID);
		ActorHandle giantHandle = giant->CreateRefHandle();
		double Start = Time::WorldTimeElapsed();

		DrainStamina(giant,"StaminaDrain_UnderButtCrush", "DestructionBasics", false, 18.0f);
		
		TaskManager::RunFor(taskname, 1.0f, [=](auto& update){ // Needed because anim has wrong timing
			if (!giantHandle) {
				return false;
			}

			double Finish = Time::WorldTimeElapsed();
			auto giantref = giantHandle.get().get();

			if (Finish - Start > 0.04) { 
				auto ThighL = find_node(giantref, "NPC L Thigh [LThg]");
				auto ThighR = find_node(giantref, "NPC R Thigh [RThg]");
				auto ButtR = find_node(giantref, "NPC R Butt");
				auto ButtL = find_node(giantref, "NPC L Butt");

				float damage = 1.15f;

				float shake_power = Rumble_ButtCrush_UnderStomp_ButtImpact/2 * dust * damage;

				if (ButtR && ButtL) {
					if (ThighL && ThighR) {
						DoDamageAtPoint(giantref, Radius_UnderStomp_Butt_Impact, Damage_ButtCrush_Under_ButtImpact * damage, ThighL, 8, 0.35f, 0.925f, DamageSource::Booty);
						DoDamageAtPoint(giantref, Radius_UnderStomp_Butt_Impact, Damage_ButtCrush_Under_ButtImpact * damage, ThighR, 8, 0.35f, 0.925f, DamageSource::Booty);
						DoDustExplosion(giantref, 1.45f * dust * damage, FootEvent::Butt, "NPC R Butt");
						DoDustExplosion(giantref, 1.45f * dust * damage, FootEvent::Butt, "NPC L Butt");
						DoLaunch(giantref, 1.65f * perk, 5.0f, FootEvent::Butt);
						DoFootstepSound(giantref, 1.05f, FootEvent::Right, RNode);
						
						Rumbling::Once("Butt_L", giantref, shake_power * smt, 0.075f, "NPC R Butt", 0.0f);
						Rumbling::Once("Butt_R", giantref, shake_power * smt, 0.075f, "NPC L Butt", 0.0f);
					}
				} else {
					if (!ButtR) {
						Notify("Error: Missing Butt Nodes"); // Will help people to troubleshoot it. Not everyone has 3BB/XPMS32 body.
						Notify("Error: effects not inflicted");
						Notify("install 3BBB/XP32 Skeleton");
					}
					if (!ThighL) {
						Notify("Error: Missing Thigh Nodes");
						Notify("Error: effects not inflicted");
						Notify("install 3BBB/XP32 Skeleton");
					}
				}
				return false;
			}
			return true;
		});
	}

	void GTS_UnderStomp_Butt_DisableHH(AnimationEventData& data) {
		data.stage = 2;
		data.HHspeed = 3.25f;
		data.disableHH = true;
		
	}
	void GTS_UnderStomp_Butt_EnableHH(AnimationEventData& data) {
		data.disableHH = false;
	}

	void GTS_UnderStomp_Butt_RecFootstepL(AnimationEventData& data) {
		UnderStomp_RecoveryStomp(&data.giant, false, FootEvent::Left, DamageSource::CrushedLeft, LNode, "RecFSL");
	}

	void GTS_UnderStomp_Butt_RecFootstepR(AnimationEventData& data) {
		UnderStomp_RecoveryStomp(&data.giant, true, FootEvent::Right, DamageSource::CrushedRight, RNode, "RecFSR");
	}

    void GTS_UnderStomp_ButtCamOn(AnimationEventData& data) {
		DrainStamina(&data.giant,"StaminaDrain_UnderButtCrush", "DestructionBasics", true, 18.0f); // 13.5 * 5.1
		ManageCamera(&data.giant, true, CameraTracking::Butt);
	}
    void GTS_UnderStomp_ButtCamOff(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::Butt);
	}

    void GTS_UnderStomp_ButtImpact(AnimationEventData& data) {
        UnderStomp_DoButtDamage(&data.giant);
    }
    void GTS_UnderStomp_ButtLegImpact(AnimationEventData& data) {
		UnderStomp_LegImpacts(&data.giant);
    }
}
namespace Gts {
    void AnimationUnderStompButt::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_UnderStomp_ButtCamOn", "UnderStompButt", GTS_UnderStomp_ButtCamOn);
		AnimationManager::RegisterEvent("GTS_UnderStomp_ButtCamOff", "UnderStompButt", GTS_UnderStomp_ButtCamOff);

		AnimationManager::RegisterEvent("GTS_UnderStomp_Butt_DisableHH", "UnderStompButt", GTS_UnderStomp_Butt_DisableHH);
		AnimationManager::RegisterEvent("GTS_UnderStomp_Butt_EnableHH", "UnderStompButt", GTS_UnderStomp_Butt_EnableHH);

		AnimationManager::RegisterEvent("GTS_UnderStomp_Butt_RecFootstepL", "UnderStompButt", GTS_UnderStomp_Butt_RecFootstepL);
		AnimationManager::RegisterEvent("GTS_UnderStomp_Butt_RecFootstepR", "UnderStompButt", GTS_UnderStomp_Butt_RecFootstepR);

        AnimationManager::RegisterEvent("GTS_UnderStomp_ButtImpact", "UnderStompButt", GTS_UnderStomp_ButtImpact);
		AnimationManager::RegisterEvent("GTS_UnderStomp_ButtLegImpact", "UnderStompButt", GTS_UnderStomp_ButtLegImpact);
	}
}