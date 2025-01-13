#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/CleavageEvents.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/animation/CleavageState.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "utils/DifficultyUtils.hpp"
#include "managers/CrushManager.hpp"
#include "managers/perks/PerkHandler.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "utils/looting.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
    void Absorb_GrowInSize(Actor* giant, Actor* tiny, float multiplier) {
        if (Runtime::HasPerkTeam(giant, "HugCrush_Greed")) {
			multiplier *= 1.15f;
		}
        float grow_value = 0.08f * multiplier * 0.845f;
        float original = Vore::ReadOriginalScale(tiny) + 0.875f; // Compensate 
        update_target_scale(giant, grow_value * original, SizeEffectType::kGrow);
    }

    void CancelAnimation(Actor* giant) {
        auto tiny = Grab::GetHeldActor(giant);
        giant->SetGraphVariableBool("GTS_OverrideZ", false);
        if (tiny) {
            KillActor(giant, tiny);
            PerkHandler::UpdatePerkValues(giant, PerkUpdate::Perk_LifeForceAbsorption);
            DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75f);
            tiny->SetGraphVariableBool("GTSBEH_T_InStorage", false);
            SetBetweenBreasts(tiny, false);
            SetBeingEaten(tiny, false);
            SetBeingHeld(tiny, false);
        }

        std::string name = std::format("GrabAttach_{}", giant->formID);
        TaskManager::Cancel(name);

        Grab::DetachActorTask(giant);
		Grab::Release(giant);
    }

    void RecoverAttributes(Actor* giant, ActorValue Attribute, float percentage) {
        float Percent = GetMaxAV(giant, Attribute);
        float value = Percent * percentage;

        if (Runtime::HasPerk(giant, "Breasts_Predominance")) {
            value *= 1.5f;
        }

        DamageAV(giant, Attribute, -value);
    }

    void ShrinkTinyWithCleavage(Actor* giant, float scale_limit, float shrink_for, float stamina_damage, bool hearts, bool damage_stamina) {
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            if (get_target_scale(tiny) > scale_limit && shrink_for < 1.0f) {
                DamageAV(giant, ActorValue::kHealth, -get_target_scale(tiny) * 10); // Heal GTS
                set_target_scale(tiny, get_target_scale(tiny) * shrink_for);
            } else {
                set_target_scale(tiny, scale_limit);
            }
            
            if (damage_stamina) {
                DamageAV(tiny, ActorValue::kStamina, stamina_damage);
            }
            if (hearts) {
                SpawnHearts(giant, tiny, 35, 0.5f, false);
            }
        }
    }
    void SuffocateTinyFor(Actor* giant, Actor* tiny, float DamageMult, float Shrink, float StaminaDrain) {
        float TotalHP = GetMaxAV(tiny, ActorValue::kHealth);
        float CurrentHP = GetAV(tiny, ActorValue::kHealth);
        float Threshold = 1.0f;
        
        float damage = TotalHP * DamageMult; // 100% hp by default
        
        if (CurrentHP > Threshold) {
            if (DamageMult > 0 && CurrentHP - damage > Threshold) {
                DamageAV(tiny, ActorValue::kHealth, damage);
            } else {
                if (DamageMult > 0) {
                    SetAV(tiny, ActorValue::kHealth, 1.0f);
                }
            }
            DamageAV(tiny, ActorValue::kStamina, StaminaDrain);
        }
    }

    void Task_RunSuffocateTask(Actor* giant, Actor* tiny) {
        std::string name = std::format("SuffoTask_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();

        float starting_hppercentage = GetHealthPercentage(tiny);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			if (!tinyref) {
				return false; // end task in that case
			}
            float damage = 0.0f;//0.0015f * starting_hppercentage * TimeScale();
            SuffocateTinyFor(giantref, tinyref, damage, 0.998f, 0.0035f * TimeScale());

            if (tinyref->IsDead() || GetAV(tinyref, ActorValue::kHealth) <= 0) {
                return false;
            }
			// All good try another frame
			return true;
		});
    }

    void PrintBreastAbsorbed(Actor* giant, Actor* tiny) {
        int random = RandomInt(0, 4);
        if (random == 0) {
            Cprint("Breasts of {} lovingly ate {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
        } else if (random == 1) {
            Cprint("{} suddenly disappeared between the breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
        } else if (random == 2) {
            Cprint("Mountains of {} greedily absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
        } else if (random == 3) {
            Cprint("{} became one with the breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
        } else {
            Cprint("{} was gently devoured by the milkers of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
        }
    }

    ///=================================================================== Functions
    void Deal_breast_damage(Actor* giant, float damage_mult) {
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            if (!IsTeammate(tiny)) {
                Attacked(tiny, giant); // force combat
            }

            float bonus = 1.0f;
            auto& sizemanager = SizeManager::GetSingleton();

			float tiny_scale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);
			float gts_scale = get_visual_scale(giant) * GetSizeFromBoundingBox(giant);

			float sizeDiff = gts_scale/tiny_scale;
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, SizeAttribute::Normal), 1.0f, 999999.0f);
			float additionaldamage = 1.0f + sizemanager.GetSizeVulnerability(tiny);
			float damage = (Damage_Breast_Squish * damage_mult) * power * additionaldamage * additionaldamage * sizeDiff;
			float experience = std::clamp(damage/1600, 0.0f, 0.06f);

			if (HasSMT(giant)) {
				bonus = 1.65f;
			}

            if (CanDoDamage(giant, tiny, false)) {
                if (Runtime::HasPerkTeam(giant, "GrowingPressure")) {
                    auto& sizemanager = SizeManager::GetSingleton();
                    sizemanager.ModSizeVulnerability(tiny, damage * 0.0010f);
                }

                TinyCalamity_ShrinkActor(giant, tiny, damage * 0.20f * GetDamageSetting());

                SizeHitEffects::GetSingleton().PerformInjuryDebuff(giant, tiny, damage * 0.15f, 6);
                if (!IsTeammate(tiny) || IsHostile(giant, tiny)) {
                    InflictSizeDamage(giant, tiny, damage);
                    DamageAV(tiny, ActorValue::kStamina, damage * 0.25f);
                } else {
                    tiny->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage * 5);
                }

                DamageAV(giant, ActorValue::kHealth, -damage * 0.33f);
            }
			
			Rumbling::Once("GrabAttack", tiny, Rumble_Grab_Hand_Attack * bonus * damage_mult, 0.05f, "NPC Root [Root]", 0.0f);
            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, 1.0f, 1.0f, "NPC Root [Root]");

            Utils_CrushTask(giant, tiny, bonus, false, false, DamageSource::BreastImpact, QuestStage::Crushing);
            ModSizeExperience(giant, experience);
        }
    }

    ///===================================================================

    ///=================================================================== Camera

    void GTS_BS_CamOn(const AnimationEventData& data) {ManageCamera(&data.giant, true, CameraTracking::Breasts_02);}
    void GTS_BS_CamOff(const AnimationEventData& data) {ManageCamera(&data.giant, false, CameraTracking::Breasts_02);}

    ///===================================================================

    ///=================================================================== Attacks

    void GTS_BS_SpringUp(const AnimationEventData& data) {
        ManageCamera(&data.giant, true, CameraTracking::ObjectB);
    }

    void GTS_BS_BoobsRelease(const AnimationEventData& data) {
    }

    void GTS_BS_DamageTiny_L(const AnimationEventData& data) {
        DamageAV(&data.giant, ActorValue::kStamina, 30 * GetWasteMult(&data.giant));

        Rumbling::Once("HandImpact_L", &data.giant, 0.3f, 0.0f, "L Breast02", 0.0f);
        Rumbling::Once("HandImpact_R", &data.giant, 0.3f, 0.0f, "R Breast02", 0.0f);

        Deal_breast_damage(&data.giant, 1.0f);
    }
    void GTS_BS_DamageTiny_H(const AnimationEventData& data) {
        DamageAV(&data.giant, ActorValue::kStamina, 45 * GetWasteMult(&data.giant));

        Rumbling::Once("HandImpactH_L", &data.giant, 0.55f, 0.0f, "L Breast02", 0.0f);
        Rumbling::Once("HandImpactH_R", &data.giant, 0.55f, 0.0f, "R Breast02", 0.0f);

        Deal_breast_damage(&data.giant, 2.25f);
    }

    void GTS_BS_Shake(const AnimationEventData& data) {
        ShrinkTinyWithCleavage(&data.giant, 0.035f, 0.980f, 25.0f, true, true);

        Rumbling::Once("BreastShake_L", &data.giant, 0.3f, 0.0f, "L Breast02", 0.0f);
        Rumbling::Once("BreastShake_R", &data.giant, 0.3f, 0.0f, "R Breast02", 0.0f);

        if (!IsActionOnCooldown(&data.giant, CooldownSource::Emotion_Laugh)) {
            Task_FacialEmotionTask_Smile(&data.giant, 6.0f, "ShakeSmile");
            ApplyActionCooldown(&data.giant, CooldownSource::Emotion_Laugh);
        }
    }

    ///===================================================================

    ///=================================================================== Vore

    void GTS_BS_Smile(const AnimationEventData& data) {
    }
    void GTS_BS_OpenMouth(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		if (tiny) {
			SetBeingEaten(tiny, true);
			Vore::GetSingleton().ShrinkOverTime(giant, tiny, 0.1f);
		}
		Task_FacialEmotionTask_OpenMouth(giant, 0.66f, "PrepareVore");
    }
    void GTS_BS_CloseMouth(const AnimationEventData& data) {
    }
    void GTS_BS_PrepareEat(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        ManageCamera(&data.giant, true, CameraTracking::ObjectB);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
        
		if (tiny) {
			VoreData.AddTiny(tiny);
		}
    }
    void GTS_BS_Swallow(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (tiny) {
            Runtime::PlaySoundAtNode("VoreSwallow", &data.giant, 1.0f, 1.0f, "NPC Head [Head]"); // Play sound
			for (auto& tiny: VoreData.GetVories()) {
				if (!AllowDevourment()) {
					VoreData.Swallow();
					if (IsCrawling(&data.giant)) {
						tiny->SetAlpha(0.0f); // Hide Actor
					}
				} else {
					CallDevourment(&data.giant, tiny);
				}
			}
		}

        RecoverAttributes(&data.giant, ActorValue::kHealth, 0.07f);
        RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.07f);
        RecoverAttributes(&data.giant, ActorValue::kStamina, 0.07f);
    }
    void GTS_BS_KillAll(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(&data.giant);
		if (tiny) {
			SetBeingEaten(tiny, false);
			auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
			for (auto& tiny: VoreData.GetVories()) {
				VoreData.KillAll();
			}
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);

			SetBeingHeld(tiny, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Vore);
    }

    ///=================================================================== Absorb

    void GTS_BS_AbsorbStart(const AnimationEventData& data) {
        Task_FacialEmotionTask_Smile(&data.giant, 3.2f, "AbsorbStart");
        Task_ApplyAbsorbCooldown(&data.giant);

        auto tiny = Grab::GetHeldActor(&data.giant);
		if (tiny) {
            Vore::RecordOriginalScale(tiny);
        }
    }

    void GTS_BS_AbsorbPulse(const AnimationEventData& data) {
        auto giant = &data.giant;
		auto tiny = Grab::GetHeldActor(&data.giant);

        float growth = 0.0650f;

		if (tiny) {
            Rumbling::Once("AbsorbPulse_R", giant, 0.45f, 0.0f, "L Breast02", 0.0f);
            Rumbling::Once("AbsorbPulse_L", giant, 0.45f, 0.0f, "R Breast02", 0.0f);

            float volume = std::clamp(0.12f * get_visual_scale(giant), 0.12f, 1.0f);

            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, volume, 1.0f, "NPC Root [Root]");

            bool Blocked = IsActionOnCooldown(giant, CooldownSource::Emotion_Laugh);
            if (!Blocked) {
                ApplyActionCooldown(giant, CooldownSource::Emotion_Laugh);
                Task_FacialEmotionTask_Smile(giant, 0.9f, "AbsorbSmile", 0.12f);
                PlayLaughSound(giant, 0.6f, 1);
            }

            ShrinkTinyWithCleavage(giant, 0.010f, 0.66f, 45.0f, true, true);
            RecoverAttributes(giant, ActorValue::kHealth, 0.025f);

            Absorb_GrowInSize(giant, tiny, growth);
        }
    }

    void GTS_BS_FinishAbsorb(const AnimationEventData& data) {
        Actor* giant = &data.giant;
        Task_FacialEmotionTask_Moan(giant, 1.1f, "AbsorbMoan");
        auto tiny = Grab::GetHeldActor(giant);
        float growth = 1.05f;

		if (tiny) {
            SpawnHearts(giant, tiny, 35, 0.75f, false);
            PlayMoanSound(giant, 0.8f);
            Attacked(tiny, giant);

            AdvanceQuestProgression(giant, tiny, QuestStage::HugSteal, 1.0f, false);

            Rumbling::Once("AbsorbTiny_R", giant, 0.8f, 0.05f, "L Breast02", 0.0f);
            Rumbling::Once("AbsorbTiny_L", giant, 0.8f, 0.05f, "R Breast02", 0.0f);

            DamageAV(giant, ActorValue::kHealth, -30); // Heal GTS
            Absorb_GrowInSize(giant, tiny, growth);
            PrintBreastAbsorbed(giant, tiny);

            AdjustSizeReserve(giant, 0.0285f);
			AdjustSizeLimit(0.0095f, giant);
			AdjustMassLimit(0.0095f, giant);

            DecreaseShoutCooldown(giant);
            KillActor(giant, tiny, true);

            if (tiny->formID != 0x14) {
                Disintegrate(tiny);
                SendDeathEvent(giant, tiny);
            } else {
                DamageAV(tiny, ActorValue::kHealth, 999999);
                tiny->KillImpl(giant, 1, true, true);
            }
            
            std::string taskname = std::format("MergeWithTiny_{}_{}", giant->formID, tiny->formID);
            ActorHandle giantHandle = giant->CreateRefHandle();
            ActorHandle tinyHandle = tiny->CreateRefHandle();
            TaskManager::RunOnce(taskname, [=](auto& update){
                if (!tinyHandle) {
                    return;
                }
                if (!giantHandle) {
                    return;
                }

                auto giantref = giantHandle.get().get();
                auto tinyref = tinyHandle.get().get();

                PerkHandler::UpdatePerkValues(giantref, PerkUpdate::Perk_LifeForceAbsorption);

                TransferInventory(tinyref, giantref, get_visual_scale(tinyref) * GetSizeFromBoundingBox(tinyref), false, true, DamageSource::Vored, true);
                // Actor Reset is done inside TransferInventory:StartResetTask!
            });
            
            RecoverAttributes(giant, ActorValue::kHealth, 0.05f);
            ModSizeExperience(giant, 0.235f);
        }
    }
    void GTS_BS_GrowBoobs(const AnimationEventData& data) {
        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Absorb);
    }

    ///===================================================================

    ///=================================================================== Suffocate

    void GTS_BS_SufoStart(const AnimationEventData& data) { 
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            SuffocateTinyFor(&data.giant, tiny, 0.10f, 0.85f, 25.0f);
            Task_RunSuffocateTask(&data.giant, tiny);
            SpawnHearts(&data.giant, tiny, 35, 0.35f, false);
        }
        Task_FacialEmotionTask_Smile(&data.giant, 1.8f, "SufoStart", RandomFloat(0.0f, 0.75f));

        int rng = RandomInt(0, 3);
        if (rng >= 2) {
            PlayLaughSound(&data.giant, 1.0f, 1);
        }
    }
    void GTS_BS_SufoStop(const AnimationEventData& data) {}

    void GTS_BS_SufoKill(const AnimationEventData& data) {
        Animation_Cleavage::LaunchCooldownFor(&data.giant, CooldownSource::Action_Breasts_Suffocate);
        ModSizeExperience(&data.giant, 0.235f);
        CancelAnimation(&data.giant);
    }
    void GTS_BS_SufoPress(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.035f);
            SuffocateTinyFor(&data.giant, tiny, 0.35f, 0.85f, 25.0f);
            SpawnHearts(&data.giant, tiny, 35, 0.50f, false);
        }
        Task_FacialEmotionTask_Smile(&data.giant, 1.2f, "SufoPress");
    }
    
    void GTS_BS_PullTiny(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            Task_FacialEmotionTask_Smile(&data.giant, 1.6f, "SufoPullOut");
            Rumbling::Once("PullOut_R", &data.giant, 0.75f, 0.0f, "L Breast02", 0.0f);
            Rumbling::Once("PullOut_L", &data.giant, 0.75f, 0.0f, "R Breast02", 0.0f);

            Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectL);

            ManageCamera(&data.giant, true, CameraTracking::ObjectB);
            SpawnHearts(&data.giant, tiny, 35, 0.50f, false);

            PlayLaughSound(&data.giant, 1.0f, 2);
        }
    }

    void GTS_BS_HandsLand(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            RecoverAttributes(&data.giant, ActorValue::kMagicka, 0.045f);
            SuffocateTinyFor(&data.giant, tiny, 0.20f, 0.75f, 20.0f);

            Rumbling::Once("HandLand_R", &data.giant, 0.45f, 0.0f, "L Breast02", 0.0f);
            Rumbling::Once("HandLand_L", &data.giant, 0.45f, 0.0f, "R Breast02", 0.0f);

            float volume = std::clamp(0.12f * get_visual_scale(&data.giant), 0.12f, 1.0f);
            Runtime::PlaySoundAtNode("ThighSandwichImpact", tiny, volume, 1.0f, "NPC Root [Root]");
        }
    }

    void GTS_BS_OverrideZ_ON(const AnimationEventData& data) { 
        data.giant.SetGraphVariableBool("GTS_OverrideZ", true);
    }
    void GTS_BS_OverrideZ_OFF(const AnimationEventData& data) { 
        data.giant.SetGraphVariableBool("GTS_OverrideZ", false);
    }

    ///===================================================================

    ///=================================================================== Utils

    void GTS_BS_SwitchToObjectB(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectB);}
    void GTS_BS_SwitchToCleavage(const AnimationEventData& data) {Attachment_SetTargetNode(&data.giant, AttachToNode::None);}

    void GTS_BS_Poke(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            Task_FacialEmotionTask_Smile(&data.giant, 0.7f, "SufoPoke", RandomFloat(0.0f, 0.90f));
            SpawnHearts(&data.giant, tiny, 35, 0.4f, false);
        }
    }
    void GTS_BS_Pat(const AnimationEventData& data) {
        auto tiny = Grab::GetHeldActor(&data.giant);
        if (tiny) {
            SpawnHearts(&data.giant, tiny, 35, 0.4f, false);

            if (IsHostile(&data.giant, tiny)) {
				AnimationManager::StartAnim("Breasts_Idle_Unwilling", tiny);
			} else {
				AnimationManager::StartAnim("Breasts_Idle_Willing", tiny);
			}
        }
    }

    void GTSBEH_Boobs_StartTransition(const AnimationEventData& data) {
        Actor* giant = &data.giant;
        Actor* tiny = Grab::GetHeldActor(giant);
        if (tiny) {
            tiny->SetGraphVariableBool("GTSBEH_T_InStorage", false);
            SetBetweenBreasts(tiny, false);
            SetBeingEaten(tiny, false);
            SetBeingHeld(tiny, false);
        }

        DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75f);
        giant->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
        giant->SetGraphVariableInt("GTS_Grab_State", 0);
        giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
        Grab::DetachActorTask(giant);
        Grab::Release(giant);
    }

    ///===================================================================
}

namespace Gts
{
	void Animation_CleavageEvents::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_BS_CamOn", "Cleavage", GTS_BS_CamOn);
        AnimationManager::RegisterEvent("GTS_BS_CamOff", "Cleavage", GTS_BS_CamOff);

        AnimationManager::RegisterEvent("GTS_BS_SpringUp", "Cleavage", GTS_BS_SpringUp);
        AnimationManager::RegisterEvent("GTS_BS_BoobsRelease", "Cleavage", GTS_BS_BoobsRelease);
        AnimationManager::RegisterEvent("GTS_BS_DamageTiny_L", "Cleavage", GTS_BS_DamageTiny_L);
        AnimationManager::RegisterEvent("GTS_BS_DamageTiny_H", "Cleavage", GTS_BS_DamageTiny_H);
        AnimationManager::RegisterEvent("GTS_BS_Shake", "Cleavage", GTS_BS_Shake);

        AnimationManager::RegisterEvent("GTS_BS_Smile", "Cleavage", GTS_BS_Smile);
        AnimationManager::RegisterEvent("GTS_BS_OpenMouth", "Cleavage", GTS_BS_OpenMouth);
        AnimationManager::RegisterEvent("GTS_BS_CloseMouth", "Cleavage", GTS_BS_CloseMouth);

        AnimationManager::RegisterEvent("GTS_BS_PrepareEat", "Cleavage", GTS_BS_PrepareEat);
        AnimationManager::RegisterEvent("GTS_BS_Swallow", "Cleavage", GTS_BS_Swallow);
        AnimationManager::RegisterEvent("GTS_BS_KillAll", "Cleavage", GTS_BS_KillAll);

        AnimationManager::RegisterEvent("GTS_BS_AbsorbStart", "Cleavage", GTS_BS_AbsorbStart);
        AnimationManager::RegisterEvent("GTS_BS_AbsorbPulse", "Cleavage", GTS_BS_AbsorbPulse);
        AnimationManager::RegisterEvent("GTS_BS_FinishAbsorb", "Cleavage", GTS_BS_FinishAbsorb);
        AnimationManager::RegisterEvent("GTS_BS_GrowBoobs", "Cleavage", GTS_BS_GrowBoobs);

        AnimationManager::RegisterEvent("GTS_BS_SufoStart", "Cleavage", GTS_BS_SufoStart);
        AnimationManager::RegisterEvent("GTS_BS_SufoStop", "Cleavage", GTS_BS_SufoStop);
        AnimationManager::RegisterEvent("GTS_BS_SufoKill", "Cleavage", GTS_BS_SufoKill);
        AnimationManager::RegisterEvent("GTS_BS_SufoPress", "Cleavage", GTS_BS_SufoPress);
        AnimationManager::RegisterEvent("GTS_BS_PullTiny", "Cleavage", GTS_BS_PullTiny);
        AnimationManager::RegisterEvent("GTS_BS_OverrideZ_ON", "Cleavage", GTS_BS_OverrideZ_ON);
        AnimationManager::RegisterEvent("GTS_BS_OverrideZ_OFF", "Cleavage", GTS_BS_OverrideZ_OFF);

        AnimationManager::RegisterEvent("GTS_BS_SwitchToObjectB", "Cleavage", GTS_BS_SwitchToObjectB);
        AnimationManager::RegisterEvent("GTS_BS_SwitchToCleavage", "Cleavage", GTS_BS_SwitchToCleavage);

        AnimationManager::RegisterEvent("GTS_BS_Shake", "Cleavage", GTS_BS_Shake);
        AnimationManager::RegisterEvent("GTS_BS_HandsLand", "Cleavage", GTS_BS_HandsLand);
        AnimationManager::RegisterEvent("GTS_BS_Poke", "Cleavage", GTS_BS_Poke);
        AnimationManager::RegisterEvent("GTS_BS_Pat", "Cleavage", GTS_BS_Pat);

        AnimationManager::RegisterEvent("GTSBEH_Boobs_StartTransition", "Cleavage", GTSBEH_Boobs_StartTransition);
	}
} 
 
 
 