#include "Managers/Animation/CleavageState.hpp"
#include "Managers/Animation/AnimationManager.hpp"

#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Animation/Grab.hpp"
#include "Managers/Input/InputManager.hpp"
#include "Utils/InputConditions.hpp"

using namespace GTS;

/*
GTS events:
GTSBEH_Boobs_Enter
GTSBEH_Boobs_Exit
GTSBEH_Boobs_Crush_Light
GTSBEH_Boobs_Crush_Heavy
GTSBEH_Boobs_Vore
GTSBEH_Boobs_Absorb
GTSBEH_Boobs_Abort

TIny Events
GTSBEH_T_Boobs_Enter
GTSBEh_T_Boobs_Exit
GTSBEh_T_Boobs_Crush_Light
GTSBEh_T_Boobs_Crush_Heavy
GTSBEh_T_Boobs_Vore
GTSBEh_T_Boobs_Absorb 


IsInCleavageState(Actor* actor)
*/

namespace {

    const std::vector<std::string_view> BREAST_NODES_R = { // used for body rumble
        "R Breast01",
        "R Breast02",
		"R Breast03",
        "R Breast04",
	};

    const std::vector<std::string_view> BREAST_NODES_L = { // used for body rumble
        "L Breast00",
        "L Breast01",
        "L Breast02",
		"L Breast03",
        "L Breast04",
	};

    bool CanForceAction(Actor* giant, Actor* huggedActor, std::string pass_anim) {
        bool ForceCrush = Runtime::HasPerkTeam(giant, "GTSPerkHugMightyCuddles");
        float staminapercent = GetStaminaPercentage(giant);
        float stamina = GetAV(giant, ActorValue::kStamina);
        if (ForceCrush && staminapercent >= 0.50f) {
            AnimationManager::StartAnim(pass_anim, giant);
            DamageAV(giant, ActorValue::kStamina, stamina * 1.10f);
            return true;
        }
        return false;
    }

    float GetMasteryReduction(Actor* giant) {
        float hp_reduction = 0.0f;
        if (Runtime::HasPerk(giant, "GTSPerkBreastsMastery2")) {
            float level = GetGtsSkillLevel(giant) - 60.0f;
            hp_reduction = std::clamp(level * 0.015f, 0.0f, 0.6f);
        }

        return hp_reduction;
    }

    void AttemptBreastActionOnTiny(const std::string& pass_anim) {
        Actor* player = GetPlayerOrControlled();
        if (IsInCleavageState(player)) {
            auto tiny = Grab::GetHeldActor(player);
            if (tiny) {
                AnimationManager::StartAnim(pass_anim, tiny);
            }
        }
    }

    bool AttemptBreastAction(const std::string& pass_anim, CooldownSource Source, std::string cooldown_msg, const std::string& perk) {
        Actor* player = GetPlayerOrControlled();
        if (IsInCleavageState(player)) {
            auto tiny = Grab::GetHeldActor(player);
            if (tiny) {
                bool OnCooldown = IsActionOnCooldown(player, Source);
                if (!OnCooldown) {
                    if (Runtime::HasPerkTeam(player, perk)) {
                        float HpThreshold = (GetHugCrushThreshold(player, tiny, true) * 0.125f) + GetMasteryReduction(player);
                        float health = GetHealthPercentage(tiny);
                        if (health <= HpThreshold) {
                            AnimationManager::StartAnim(pass_anim, player);
                            return true;
                        } else if (HasSMT(player)) {
                            DamageAV(player, ActorValue::kStamina, 60);
                            AnimationManager::StartAnim(pass_anim, player);
                            AddSMTPenalty(player, 10.0f);
                            return true;
                        } else {
                            if (CanForceAction(player, tiny, pass_anim)) {
                                return true;
                            }
                            std::string message = std::format("{} is too healthy for {}", tiny->GetDisplayFullName(), cooldown_msg);
                            shake_camera(player, 0.45f, 0.30f);
                            NotifyWithSound(player, message);

                            Notify("Health: {:.0f}%; Requirement: {:.0f}%", health * 100.0f, HpThreshold * 100.0f);
                            return false;
                        }
                    }
                } else {
                    std::string message = std::format("{} is on a cooldown: {:.1f} sec", cooldown_msg, GetRemainingCooldown(player, Source));
                    shake_camera(player, 0.45f, 0.30f);
                    NotifyWithSound(player, message);
                    return false;
                }
            }
        } 

        return false;
    }

    bool PassAnimation(const std::string& animation, bool check_cleavage) {
        Actor* player = GetPlayerOrControlled();
        if (player) {
            bool BetweenCleavage = IsInCleavageState(player);
            if (BetweenCleavage || !check_cleavage) {
                AnimationManager::StartAnim(animation, player);
                return true;
            }
        }
        return false;
    }

    void CleavageEnterEvent(const ManagedInputEvent& data) {
        Actor* giant = GetPlayerOrControlled();
        Utils_UpdateHighHeelBlend(giant, false);
        PassAnimation("Cleavage_EnterState", false);
        AttemptBreastActionOnTiny("Cleavage_EnterState_Tiny");

        if (giant->formID == 0x14 && Runtime::HasPerkTeam(giant, "GTSPerkBreastsIntro") && Grab::GetHeldActor(giant)) {
            auto Camera = PlayerCamera::GetSingleton();
            bool Sheathed = Camera->isWeapSheathed;
            if (!Sheathed) {
                std::string message = std::format("You need to sheathe weapon/magic first");
                shake_camera(giant, 0.45f, 0.30f);
                NotifyWithSound(giant, message);
            }
        }
    }
    void CleavageExitEvent(const ManagedInputEvent& data) {
        PassAnimation("Cleavage_ExitState", true);
    }
    void CleavageLightAttackEvent(const ManagedInputEvent& data) {
        if (PassAnimation("Cleavage_LightAttack", true)) {
            AttemptBreastActionOnTiny("Cleavage_LightAttack_Tiny");
        }
    }
    void CleavageHeavyAttackEvent(const ManagedInputEvent& data) {
        if (PassAnimation("Cleavage_HeavyAttack", true)) {
            AttemptBreastActionOnTiny("Cleavage_HeavyAttack_Tiny");
        }
    }
    void CleavageSuffocateEvent(const ManagedInputEvent& data) {
        if (AttemptBreastAction("Cleavage_Suffocate", CooldownSource::Action_Breasts_Suffocate, "Suffocation", "GTSPerkBreastsSuffocation")) {
            AttemptBreastActionOnTiny("Cleavage_Suffocate_Tiny");
        }
    }
    void CleavageAbsorbEvent(const ManagedInputEvent& data) {
        if (AttemptBreastAction("Cleavage_Absorb", CooldownSource::Action_Breasts_Absorb, "Absorption", "GTSPerkBreastsAbsorb")) {
            AttemptBreastActionOnTiny("Cleavage_Absorb_Tiny");
        }
    }
    void CleavageVoreEvent(const ManagedInputEvent& data) {
        if (AttemptBreastAction("Cleavage_Vore", CooldownSource::Action_Breasts_Vore, "Vore", "GTSPerkBreastsVore")) {
            AttemptBreastActionOnTiny("Cleavage_Vore_Tiny");
        }
    }

    void CleavageDOTEvent(const ManagedInputEvent& data) {
        PassAnimation("Cleavage_DOT_Start", true);
    }
}

namespace GTS
{
    void Animation_Cleavage::LaunchCooldownFor(Actor* giant, CooldownSource Source) {
        std::string name = std::format("CDWatcher_{}_{}", giant->formID, Time::WorldTimeElapsed());
        ActorHandle gianthandle = giant->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

            if (!IsInCleavageState(giantref)) {
                return false;
            }

            ApplyActionCooldown(giant, Source);

			return true;
		});
    }

	void Animation_Cleavage::RegisterEvents() {
        InputManager::RegisterInputEvent("CleavageEnter", CleavageEnterEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageExit", CleavageExitEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageLightAttack", CleavageLightAttackEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageHeavyAttack", CleavageHeavyAttackEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageSuffocate", CleavageSuffocateEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageAbsorb", CleavageAbsorbEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageVore", CleavageVoreEvent, CleavageCondition);
        InputManager::RegisterInputEvent("CleavageDOT", CleavageDOTEvent, CleavageDOTCondition);
	}

	void Animation_Cleavage::RegisterTriggers() {
        
        AnimationManager::RegisterTrigger("Cleavage_EnterState_Tiny", "Cleavage", "GTSBEH_T_Boobs_Enter");
        AnimationManager::RegisterTrigger("Cleavage_EnterState", "Cleavage", "GTSBEH_Boobs_Enter");

        AnimationManager::RegisterTrigger("Cleavage_ExitState", "Cleavage", "GTSBEH_Boobs_Exit");

        AnimationManager::RegisterTrigger("Cleavage_LightAttack_Tiny", "Cleavage", "GTSBEH_T_Boobs_Crush_Light");
		AnimationManager::RegisterTrigger("Cleavage_LightAttack", "Cleavage", "GTSBEH_Boobs_Crush_Light");

        AnimationManager::RegisterTrigger("Cleavage_HeavyAttack_Tiny", "Cleavage", "GTSBEH_T_Boobs_Crush_Heavy");
        AnimationManager::RegisterTrigger("Cleavage_HeavyAttack", "Cleavage", "GTSBEH_Boobs_Crush_Heavy");

        AnimationManager::RegisterTrigger("Cleavage_SuffocateStop", "Cleavage", "GTSBEH_Boobs_SufoStop");

        AnimationManager::RegisterTrigger("Cleavage_Suffocate_Tiny", "Cleavage", "GTSBEH_T_Boobs_SufoStart");
        AnimationManager::RegisterTrigger("Cleavage_Suffocate", "Cleavage", "GTSBEH_Boobs_SufoStart");

        AnimationManager::RegisterTrigger("Cleavage_Absorb_Tiny", "Cleavage", "GTSBEH_T_Boobs_Absorb");
        AnimationManager::RegisterTrigger("Cleavage_Absorb", "Cleavage", "GTSBEH_Boobs_Absorb");

        AnimationManager::RegisterTrigger("Cleavage_Abort", "Cleavage", "GTSBEH_Boobs_Abort");

        AnimationManager::RegisterTrigger("Cleavage_Vore_Tiny", "Cleavage", "GTSBEH_T_Boobs_Vore");
        AnimationManager::RegisterTrigger("Cleavage_Vore", "Cleavage", "GTSBEH_Boobs_Vore");


	}
}