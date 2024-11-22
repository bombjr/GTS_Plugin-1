#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Stomp_Under.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/cameras/camutil.hpp"
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

    void DoImpactRumble(Actor* giant, std::string_view node, std::string_view name) {
		float shake_power = Rumble_Stomp_Under_Light;
		float smt = HasSMT(giant) ? 1.5f : 1.0f;
		smt *= GetHighHeelsBonusDamage(giant, true);
		Rumbling::Once(name, giant, shake_power * smt, 0.0f, node, 1.25f);
	}

    void UnderStomp_DoEverything(Actor* giant, float animSpeed, bool right, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		float SMT = 1.0f;
		float damage = 1.0f;
		if (HasSMT(giant)) {
			SMT = 1.75f; // Larger Dust
			damage = 1.25f;
		}
        DoDamageEffect(giant, Damage_Stomp_Under_Light * damage * perk, Radius_Stomp_Strong, 8, 0.30f, Event, 1.0f, Source);
        DoImpactRumble(giant, Node, rumble);
        DoDustExplosion(giant, 1.0f * (SMT), Event, Node);

        DrainStamina(giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 3.4f);

        DoFootstepSound(giant, SMT, Event, Node);

        LaunchTask(giant, 0.825f * perk, 2.10f, Event);

        FootStepManager::PlayVanillaFootstepSounds(giant, right);

	}

    void GTS_UnderStomp_CamOnR(AnimationEventData& data) {
        DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8f);
        ManageCamera(&data.giant, true, CameraTracking::R_Foot);
    }

    void GTS_UnderStomp_CamOnL(AnimationEventData& data) {
        DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8f);
        ManageCamera(&data.giant, true, CameraTracking::L_Foot);
    }

    void GTS_UnderStomp_CamOffR(AnimationEventData& data) {ManageCamera(&data.giant, false, CameraTracking::R_Foot);}
    void GTS_UnderStomp_CamOffL(AnimationEventData& data) {ManageCamera(&data.giant, false, CameraTracking::L_Foot);}

    void GTS_UnderStomp_ImpactR(AnimationEventData& data) {
		float SavedSpeed = data.animSpeed;
		UnderStomp_DoEverything(&data.giant, SavedSpeed, true, FootEvent::Right, DamageSource::CrushedRight, RNode, "HeavyStompR");
	}
	void GTS_UnderStomp_ImpactL(AnimationEventData& data) {
		float SavedSpeed = data.animSpeed;
		UnderStomp_DoEverything(&data.giant, SavedSpeed, false, FootEvent::Left, DamageSource::CrushedLeft, LNode, "HeavyStompL");
	}

}
namespace Gts {

    static inline float Remap(float x, float in_min, float in_max, float out_min, float out_max) {
        return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
    }

    bool AnimationUnderStomp::ShouldStompUnder(Actor* giant) {
        if (giant->formID == 0x14 && IsFreeCameraEnabled()) {
            return false;
        }
        //Range is between -1 (looking down) and 1 (looking up)
        //abs makes it become 1 -> 0 -> 1 for down -> middle -> up
        const float absPitch = abs(GetCameraRotation().GetVectorY().z);
        //Remap our starting range
        const float InvLookDownStartAngle = 0.925f; //Starting value of remap. Defines start angle for how down we are looking
        const float InvLookdownIntensity = std::clamp(Remap(absPitch, 1.0f, InvLookDownStartAngle, 0.0f, 1.0f), 0.0f, 1.0f);
        log::info("LookDownIntensity: {}", InvLookdownIntensity);

        bool allow = absPitch > InvLookDownStartAngle;
        // Allow to stomp when looking from above or below
        log::info("ShouldStompUnder: {}, Pitch: {}", allow, absPitch);
        if (allow) {
            float beh = std::clamp(InvLookdownIntensity * 1.35f, 0.0f, 1.0f);
            giant->SetGraphVariableFloat("GTS_StompBlend", beh);
            // Blend between "close" and "far" under-stomps
        }
        return allow;
    }

    void AnimationUnderStomp::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_UnderStomp_CamOnR", "UnderStomp", GTS_UnderStomp_CamOnR);
		AnimationManager::RegisterEvent("GTS_UnderStomp_CamOnL", "UnderStomp", GTS_UnderStomp_CamOnL);

        AnimationManager::RegisterEvent("GTS_UnderStomp_CamOffR", "UnderStomp", GTS_UnderStomp_CamOffR);
		AnimationManager::RegisterEvent("GTS_UnderStomp_CamOffL", "UnderStomp", GTS_UnderStomp_CamOffL);
        

		AnimationManager::RegisterEvent("GTS_UnderStomp_ImpactR", "UnderStomp", GTS_UnderStomp_ImpactR);
		AnimationManager::RegisterEvent("GTS_UnderStomp_ImpactL", "UnderStomp", GTS_UnderStomp_ImpactL);
	}

	void AnimationUnderStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("UnderStompRight", "Stomp", "GTSBeh_UnderStomp_StartR");
		AnimationManager::RegisterTrigger("UnderStompLeft", "Stomp", "GTSBeh_UnderStomp_StartL");
	}
}