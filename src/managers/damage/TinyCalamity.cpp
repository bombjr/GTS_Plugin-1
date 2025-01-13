#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/TinyCalamity_Shrink.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/perks/PerkHandler.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/MovementForce.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "utils/looting.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"


#include "profiler.hpp"

#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void ScareEnemies(Actor* giant)  {
		int FearChance = RandomInt(0, 2);
		if (FearChance <= 0) {
			Runtime::CastSpell(giant, giant, "GtsVoreFearSpell");
		}
	}

    void PlayGoreEffects(Actor* giant, Actor* tiny) {
        if (!IsLiving(tiny)) {
            SpawnDustParticle(tiny, giant, "NPC Root [Root]", 3.0f);
        } else {
            if (!LessGore()) {
                auto root = find_node(tiny, "NPC Root [Root]");
                if (root) {
                    float currentSize = get_visual_scale(tiny);
                    SpawnParticle(tiny, 0.60f, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25f, 7, root);
                    SpawnParticle(tiny, 0.60f, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25f, 7, root);
                    SpawnParticle(tiny, 0.60f, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25f, 7, root);
                    SpawnParticle(tiny, 0.60f, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25f, 7, root);
                    SpawnParticle(tiny, 1.20f, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 12.5f, 7, root);
                }
            }
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::CreateExplosion(tiny, get_visual_scale(tiny) * 0.5f, "BloodExplosion");
        }
    }

    void RefreshDuration(Actor* giant) {
        if (Runtime::HasPerk(giant, "NoSpeedLoss")) {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.75f); // Reduce speed after crush
        } else {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.35f); // Reduce more speed after crush
        }
    }

    bool Collision_AllowTinyCalamityCrush(Actor* giant, Actor* tiny) {
        if (IsEssential(giant, tiny)) {
            return false;
        }
        float giantHp = GetAV(giant, ActorValue::kHealth);
		float tinyHp = GetAV(tiny, ActorValue::kHealth);

        float Multiplier = (get_visual_scale(giant) + 0.5f) / get_visual_scale(tiny);

        if (giantHp >= ((tinyHp / Multiplier) * 1.25f)) {
            return true;
        } else {
            return false;
        }
    }

    void FullSpeed_ApplyEffect(Actor* giant, float speed) {
        auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
            bool& CanApplyEffect = transient->SMT_ReachedFullSpeed;
            if (speed < 1.0f) {
                CanApplyEffect = true;
            } else if (speed >= 1.0f && CanApplyEffect) {
                CanApplyEffect = false;
                //Runtime::PlaySoundAtNode("TinyCalamity_ReachedSpeed", giant, 1.0f, 1.0f, "NPC COM [COM ]");
            } 
        }
    }
}

namespace Gts {
    bool TinyCalamity_WrathfulCalamity(Actor* giant) {
        bool perform = false;
        if (Runtime::HasPerkTeam(giant, "WrathfulCalamity") && HasSMT(giant) && !giant->IsSneaking()) {

            float threshold = 0.25f;
            float level_bonus = std::clamp(GetGtsSkillLevel(giant) - 70.0f, 0.0f, 0.30f);
            threshold += level_bonus;

            float duration = 0.35f;

            std::vector<Actor*> preys = Vore::GetSingleton().GetVoreTargetsInFront(giant, 1);
            bool OnCooldown = IsActionOnCooldown(giant, CooldownSource::Misc_TinyCalamityRage);
            for (auto tiny: preys) {
                if (tiny) {
                    if (IsHuman(tiny)) {
                        float health = GetHealthPercentage(tiny);

                        float gts_hp = GetMaxAV(giant, ActorValue::kHealth);
                        float tiny_hp = GetMaxAV(tiny, ActorValue::kHealth);

                        float difference = std::clamp(gts_hp / tiny_hp, 0.5f, 2.0f);

                        threshold *= difference;

                        if (health <= threshold && !OnCooldown) {
                            if (IsBeingHeld(giant, tiny) || IsRagdolled(tiny)) {
                                return false;
                            }
                            ApplyActionCooldown(giant, CooldownSource::Misc_TinyCalamityRage);
                            Animation_TinyCalamity::AddToData(giant, tiny, 1.0f);

                            AnimationManager::StartAnim("InstaKill_Start_Tiny", tiny);
                            AnimationManager::StartAnim("InstaKill_Start_GTS", giant);

                            DisarmActor(giant, false);
                            DisarmActor(tiny, false);
                            FaceOpposite(giant, tiny);
                            
                            perform = true;
                        } else {
                            if (giant->formID == 0x14) {
                                if (!OnCooldown) {
                                    std::string message = std::format("{} is too healthy for Wrathful Calamity", tiny->GetDisplayFullName());
                                    Notify("Health: {:.0f}%; Requirement: {:.0f}%", health * 100.0f, threshold * 100.0f);
                                    shake_camera(giant, 0.45f, 0.30f);
                                    NotifyWithSound(giant, message);
                                } else {
                                    std::string message = std::format("{} is on a cooldown: {:.1f} sec", "Wrathful Calamity", GetRemainingCooldown(giant, CooldownSource::Misc_TinyCalamityRage));
                                    shake_camera(giant, 0.45f, 0.30f);
                                    NotifyWithSound(giant, message);
                                }
                            }
                        }
                    }
                } 
            }  
        }
        return perform;
    }

    void TinyCalamity_ShrinkActor(Actor* giant, Actor* tiny, float shrink) {
        auto profiler = Profilers::Profile("Calamity: Shrink");
        if (HasSMT(giant)) {
            bool HasPerk = Runtime::HasPerk(giant, "LifeSteal");
            float limit = Minimum_Actor_Scale;
            if (HasPerk) {
				DamageAV(giant, ActorValue::kHealth, -shrink * 1.25f);
                shrink *= 1.25f;
			}

            float target_scale = get_target_scale(tiny);

            if (target_scale > limit/GetSizeFromBoundingBox(tiny)) {
                if ((target_scale - shrink*0.0045f) <= limit || target_scale <= limit) {
                    set_target_scale(tiny, limit);
                    return;
                }
                ShrinkActor(tiny, shrink * 0.0045f, 0.0f);
            } else { // cap it just in case
                set_target_scale(tiny, limit);
            }
        }
    }

    void TinyCalamity_SeekForShrink(Actor* giant, Actor* tiny, float damage, float maxFootDistance, DamageSource Cause, bool Right, bool ApplyCooldown, bool ignore_rotation) {
        std::vector<NiPoint3> CoordsToCheck = GetFootCoordinates(giant, Right, ignore_rotation);
        int nodeCollisions = 0;
        auto model = tiny->GetCurrent3D();
        if (model) {
            for (auto &point : CoordsToCheck) {
                bool StopDamageLookup = false;
                if (!StopDamageLookup) {
                    VisitNodes(model, [&nodeCollisions, point, maxFootDistance, &StopDamageLookup](NiAVObject& a_obj) {
                        float distance = (point - a_obj.world.translate).Length() - Collision_Distance_Override;
                        if (distance <= maxFootDistance) {
                            StopDamageLookup = true;
                            nodeCollisions += 1;
                            return false;
                        }
                        return true;
                    });
                }
            }
            if (nodeCollisions > 0) {
                auto& CollisionDamage = CollisionDamage::GetSingleton();
                if (ApplyCooldown) { // Needed to fix Thigh Crush stuff
                    auto& sizemanager = SizeManager::GetSingleton();
                    bool OnCooldown = IsActionOnCooldown(tiny, CooldownSource::Damage_Thigh);
                    if (!OnCooldown) {
                        Utils_PushCheck(giant, tiny, Get_Bone_Movement_Speed(giant, Cause)); // pass original un-altered force
                        CollisionDamage.DoSizeDamage(giant, tiny, damage, 0.0f, 10, 0, Cause, false);
                        ApplyActionCooldown(giant, CooldownSource::Damage_Thigh);
                    }
                } else {
                    Utils_PushCheck(giant, tiny, Get_Bone_Movement_Speed(giant, Cause)); // pass original un-altered force
                    CollisionDamage.DoSizeDamage(giant, tiny, damage, 0.0f, 10, 0, Cause, false);
                }
            }
        }
    }

    void TinyCalamity_ExplodeActor(Actor* giant, Actor* tiny) {
        ModSizeExperience_Crush(giant, tiny, true);

        if (!tiny->IsDead()) {
            KillActor(giant, tiny);
        }

        PerkHandler::UpdatePerkValues(giant, PerkUpdate::Perk_LifeForceAbsorption);

        ActorHandle giantHandle = giant->CreateRefHandle();
        ActorHandle tinyHandle = tiny->CreateRefHandle();

        CrushBonuses(giant, tiny);                             // common.hpp
        PlayGoreEffects(tiny, giant);    
        MoveItems(giantHandle, tinyHandle, tiny->formID, DamageSource::Collision);

        Attacked(tiny, giant);
        
        PrintDeathSource(giant, tiny, DamageSource::Collision);

        float OldScale;
        giant->GetGraphVariableFloat("GiantessScale", OldScale); // save old scale
        giant->SetGraphVariableFloat("GiantessScale", 1.0f); // Needed to allow Stagger to play, else it won't work

        int Random = RandomInt(1, 4);
		if (Random >= 4 && !IsActionOnCooldown(giant, CooldownSource::Emotion_Moan_Crush) && Runtime::HasPerk(giant, "GrowthDesirePerk")) {
			PlayMoanSound(giant, 1.0f);
		}

        shake_camera(giant, 1.7f, 0.8f);
        StaggerActor(giant, 0.5f);
        RefreshDuration(giant);

        Runtime::PlaySound("DefaultCrush", giant, 1.0f, 1.0f);

        if (tiny->formID != 0x14) {
            Disintegrate(tiny); // Set critical stage 4 on actors
        } else if (tiny->formID == 0x14) {
            TriggerScreenBlood(50);
            tiny->SetAlpha(0.0f); // Player can't be disintegrated, so we make player Invisible
        }
        
        Runtime::PlaySoundAtNode("TinyCalamity_Crush", giant, 1.0f, 1.0f, "NPC COM [COM ]");
        giant->SetGraphVariableFloat("GiantessScale", OldScale);
        DecreaseShoutCooldown(giant);

        ScareEnemies(giant);
    }

    void TinyCalamity_StaggerActor(Actor* giant, Actor* tiny, float giantHp) { // when we can't crush the target
        float OldScale; 
        giant->GetGraphVariableFloat("GiantessScale", OldScale); // record old slace
        giant->SetGraphVariableFloat("GiantessScale", 1.0f); // Needed to allow Stagger to play, else it won't work

        PushForward(giant, tiny, 800);
        AddSMTDuration(giant, 2.5f);
        StaggerActor(giant, 0.5f); // play stagger on the player

        Attacked(tiny, giant);

        DamageAV(tiny, ActorValue::kHealth, giantHp * 0.75f);
        DamageAV(giant, ActorValue::kHealth, giantHp * 0.25f);

        float hpcalc = (giantHp * 0.75f)/800.0f;
        float xp = std::clamp(hpcalc, 0.0f, 0.12f);
        update_target_scale(tiny, -0.06f, SizeEffectType::kShrink);
        ModSizeExperience(giant, xp);

        Runtime::PlaySoundAtNode("TinyCalamity_Impact", giant, 1.0f, 1.0f, "NPC COM [COM ]");
        shake_camera_at_node(giant, "NPC COM [COM ]", 16.0f, 1.0f);
        
        if (IsEssential(giant, tiny)) {
            Notify("{} is essential", tiny->GetDisplayFullName());
        } else {
            Notify("{} is too tough to be crushed", tiny->GetDisplayFullName());
        }

        giant->SetGraphVariableFloat("GiantessScale", OldScale);
        RefreshDuration(giant);
    }

    void TinyCalamity_SeekActors(Actor* giant) {
        auto profiler = Profilers::Profile("Calamity: SeekActor");
        if (giant->formID == 0x14) {
            if (giant->AsActorState()->IsSprinting() && HasSMT(giant)) {
                auto node = find_node(giant, "NPC Pelvis [Pelv]");
                if (!node) {
                    return;
                }
                NiPoint3 NodePosition = node->world.translate;

                float giantScale = get_visual_scale(giant);

                const float BASE_DISTANCE = 48.0f;
                float CheckDistance = BASE_DISTANCE*giantScale;

                if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
                    DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 100, {0.0f, 1.0f, 1.0f, 1.0f});
                }

                NiPoint3 giantLocation = giant->GetPosition();
                for (auto otherActor: find_actors()) {
                    if (otherActor != giant) {
                        NiPoint3 actorLocation = otherActor->GetPosition();
                        if ((actorLocation - giantLocation).Length() < BASE_DISTANCE*giantScale*3) {
                            int nodeCollisions = 0;
                            float force = 0.0f;

                            auto model = otherActor->GetCurrent3D();

                            if (model) {
                                VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
                                    float distance = (NodePosition - a_obj.world.translate).Length();
                                    if (distance < CheckDistance) {
                                        nodeCollisions += 1;
                                        force = 1.0f - distance / CheckDistance;
                                        return false;
                                    }
                                    return true;
                                });
                            }
                            if (nodeCollisions > 0) {
                                TinyCalamity_CrushCheck(giant, otherActor);
                            }
                        }
                    }
                }
            }
        }
    }

    void TinyCalamity_CrushCheck(Actor* giant, Actor* tiny) {
		auto profiler = Profilers::Profile("tinyCalamity: CrushCheck");
		if (giant == tiny) {
			return;
		}
        if (IsBeingHeld(giant, tiny)) { // Don't explode the ones in our hand
            return;
        }
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(giant)) {
			if (persistent.GetData(giant)->smt_run_speed >= 1.0f) {
                float giantHp = GetAV(giant, ActorValue::kHealth);

				if (giantHp <= 0) {
					return; // just in case, to avoid CTD
				}

				if (Collision_AllowTinyCalamityCrush(giant, tiny)) {
                    StartCombat(tiny, giant);
                    TinyCalamity_ExplodeActor(giant, tiny);
				} else {
                    StartCombat(tiny, giant);
                    TinyCalamity_StaggerActor(giant, tiny, giantHp);
				}
			}
		}
	}

    void TinyCalamity_BonusSpeed(Actor* giant) { // Manages SMT bonus speed
		// Andy's TODO: Calc on demand rather than poll

		auto Attributes = Persistent::GetSingleton().GetData(giant);
		float Gigantism = 1.0f + (Ench_Aspect_GetPower(giant) * 0.25f);

        float speed = 1.0f; 
        float decay = 1.0f;
        float cap = 1.0f;

		float& currentspeed = Attributes->smt_run_speed;
		if (giant->AsActorState()->IsSprinting() && HasSMT(giant)) { // SMT Active and sprinting
			if (Runtime::HasPerk(giant, "NoSpeedLoss")) {
				speed = 1.25f;
                decay = 1.5f;
				cap = 1.10f;
			}

			currentspeed += 0.004400f * speed * Gigantism * 10; // increase MS

			if (currentspeed > cap) {
				currentspeed = cap;
			}

            FullSpeed_ApplyEffect(giant, currentspeed);
		} else { // else decay bonus speed over time
			if (currentspeed > 0.0f) {
				currentspeed -= (0.045000f * 10) / decay;
			} else if (currentspeed <= 0.0f) {
				currentspeed = 0.0f;
			} 
		}
    }
}