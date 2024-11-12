#pragma once
// Module that handles AttributeAdjustment
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

    enum class CooldownSource {
        Damage_Launch,
        Damage_Hand,
        Damage_Thigh,
        Push_Basic,
        Action_ButtCrush,
        Action_HealthGate,
        Action_ScareOther,
        Action_AbsorbOther,
        Action_Breasts_Absorb,
        Action_Breasts_Suffocate,
        Action_Breasts_Vore,
        Action_Hugs,
        Emotion_Laugh,
        Emotion_Moan,
        Misc_RevertSound,
        Misc_GrowthSound,
        Misc_BeingHit,
        Misc_AiGrowth,
        Misc_ShrinkOutburst,
        Misc_ShrinkOutburst_Forced,
        Misc_ShrinkParticle,
        Misc_ShrinkParticle_Animation,
        Misc_ShrinkParticle_Gaze,
        Misc_TinyCalamityRage,
        Footstep_Right,
        Footstep_Left,
    };

    struct CooldownData {
        float lastPushTime = -1.0e8f;
        float lastHandDamageTime = -1.0e8f;
        float lastLaunchTime = -1.0e8f;
        float lastHealthGateTime = -1.0e8f;
        float lastThighDamageTime = -1.0e8f;
        float lastButtCrushTime = -1.0e8f;
        float lastScareTime = -1.0e8f;
        float lastHugTime = -1.0e8f;

        float lastAbsorbTime = -1.0e8f;

        float lastBreastAbsorbTime = -1.0e8f;
        float lastBreastSuffocateTime = -1.0e8f;
        float lastBreastVoreTime = -1.0e8f;
        
        float lastLaughTime = -1.0e8f;
        float lastMoanTime = -1.0e8f;

        float lastRevertTime = -1.0e8f;
        float lastSoundGrowthTime = -1.0e8f;
        float lastHitTime = -1.0e8f;
        float lastGrowthTime = -1.0e8f;
        float lastOutburstTime = -1.0e8f;
        float lastForceOutburstTime = -1.0e8f;
        float lastShrinkParticleTime = -1.0e8f;
        float lastAnimShrinkParticleTime = -1.0e8f;
        float lastGazeShrinkParticleTime = -1.0e8f;
        float lastTinyCalamityTime = -1.0e8f;

        float lastFootstepTime_R = -1.0e8f;
        float lastFootstepTime_L = -1.0e8f;
    };

    void ApplyActionCooldown(Actor* giant, CooldownSource source);
    float GetRemainingCooldown(Actor* giant, CooldownSource source);
    bool IsActionOnCooldown(Actor* giant, CooldownSource source);

    class CooldownManager : public Gts::EventListener {
		public:
			[[nodiscard]] static CooldownManager& GetSingleton() noexcept;
			virtual std::string DebugName() override;

			virtual void Reset() override;

			CooldownData& GetCooldownData(Actor* actor);

        private: 
			std::map<Actor*, CooldownData> CooldownData;
    };
}