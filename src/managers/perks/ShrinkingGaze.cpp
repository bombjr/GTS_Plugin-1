
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/perks/ShrinkingGaze.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
    void ShrinkByOverTime(Actor* tiny, float over_time, float by) {
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		std::string name = std::format("ShrinkToSize_{}", tiny->formID);

		if (tiny->IsDead()) {
			by *= 1.75f;
		}
		float bb = std::clamp(GetSizeFromBoundingBox(tiny), 0.6f, 100.f);
		float Start = static_cast<float>(Time::WorldTimeElapsed());

		TaskManager::Run(name, [=](auto& progressData) {
			Actor* actor = tinyHandle.get().get();
			if (!actor) {
				return false;
			}

			float Scale = get_visual_scale(actor);
			float End = static_cast<float>(Time::WorldTimeElapsed());

			if (End - Start < over_time) {
				override_actor_scale(actor, -by * (0.0166f/bb) * TimeScale(), SizeEffectType::kNeutral);
				if (get_target_scale(actor) < SHRINK_TO_NOTHING_SCALE / bb) {
					set_target_scale(actor, SHRINK_TO_NOTHING_SCALE / bb);
					return false;
				}
				return true;
			} else {
				return false;
			}
		});
	}

	void MoanOr(Actor* giant) {
		int MoanRNG = RandomInt(0, 3);
		if (MoanRNG >= 2 && IsActionOnCooldown(giant, CooldownSource::Emotion_Moan)) {
			ApplyActionCooldown(giant, CooldownSource::Emotion_Moan);
			Task_FacialEmotionTask_Moan(giant, 2.0f, "CalamityMoan");
			PlayMoanSound(giant, 1.0f);
		}
	}

	void ShrinkTheTargetOr(Actor* giant, Actor* otherActor, float stare_threshold, float tiny_size, float difference, TempActorData* data) {
		SpawnParticle(otherActor, 6.00f, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), otherActor->GetPosition(), tiny_size * 4.5f, 7, nullptr); 
		SpawnCustomParticle(otherActor, ParticleType::Red, otherActor->GetPosition(), "NPC Root [Root]", tiny_size);

		Runtime::PlaySoundAtNode("Magic_ProtectTinies", otherActor, 0.5f, 0.5f, "NPC Root [Root]");

		InflictSizeDamage(giant, otherActor, GetAV(otherActor, ActorValue::kHealth) * 0.015f);
		AddSMTDuration(giant, stare_threshold * 0.75f, false);

		float shrink_bonus = std::clamp(tiny_size, 0.20f, 1.0f);
		ShrinkByOverTime(otherActor, stare_threshold * 0.1f, tiny_size * (0.45f / shrink_bonus));

		if (GetSizeDifference(giant, otherActor, SizeType::VisualScale, false, false) >= 0.92f) {
			StaggerActor_Directional(giant, difference, otherActor);
		}

		if (tiny_size <= 0.50f) {
			ForceFlee(giant, otherActor, 3.0f, true);
		}
		DamageAV(giant, ActorValue::kMagicka, 75 * tiny_size * Perk_GetCostReduction(giant));

		data->Shrink_Ticks_Calamity = 0.0f;
	}

	void PerformShrinkOnActor(Actor* giant) {
		CrosshairPickData* data = CrosshairPickData::GetSingleton();
		if (data) {
			float maxDistance = 24.0f;
			float checkDistance = 280.0f;
		
			NiPoint3 NodePosition = data->collisionPoint;

			if (NodePosition.Length() > 0.0f) {

				if (IsDebugEnabled() && (giant->formID == 0x14)) {
					DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), maxDistance);
				}

				if (NodePosition.Length() > 0) {
					NiPoint3 giantLocation = giant->GetPosition();
					for (auto otherActor: find_actors()) {
						if (otherActor != giant && IsHostile(giant, otherActor)) {
							auto data = Transient::GetSingleton().GetActorData(otherActor);
							if (data) {
								NiPoint3 actorLocation = otherActor->GetPosition();
								if ((actorLocation-giantLocation).Length() <= checkDistance) {
									int nodeCollisions = 0;

									float bb = std::clamp(GetSizeFromBoundingBox(otherActor), 1.0f, 100.0f);
									auto model = otherActor->GetCurrent3D();
									
									if (model) {
										VisitNodes(model, [&nodeCollisions, &data, NodePosition, maxDistance, bb](NiAVObject& a_obj) {
											float distance = (NodePosition - a_obj.world.translate).Length() - Collision_Distance_Override * 20 * bb;
				
											if (distance <= maxDistance) {
												data->Shrink_Ticks_Calamity += 0.0166f * TimeScale();
												if (data->MovementSlowdown > 0.33f) {
													data->MovementSlowdown -= 0.0032f * TimeScale();
												}
												nodeCollisions += 1;
												return false;
											} else {
												if (data->Shrink_Ticks_Calamity > 0) {
													data->Shrink_Ticks_Calamity -= 0.0166f * 0.20f * TimeScale();
												}
												data->MovementSlowdown = 1.0f; // Reset it
											}
											return true;
										});
									}
									if (nodeCollisions > 0) {
										float difference = GetSizeDifference(giant, otherActor, SizeType::VisualScale, false, false);
										float stare_threshold_s = std::clamp(3.25f * get_visual_scale(otherActor), 0.25f, 6.0f);
										float tiny_size = get_visual_scale(otherActor);

										if (data->Shrink_Ticks_Calamity >= stare_threshold_s) {
											Laugh_Chance(giant, 1.25f, "CalamityShrink");
											if (!ShrinkToNothing(giant, otherActor, true, 1.50f)) {
												ShrinkTheTargetOr(giant, otherActor, stare_threshold_s, tiny_size, difference, data);
												AdjustMassLimit(tiny_size * 0.0075f, giant);
											} else {
												if (!IsActionOnCooldown(otherActor, CooldownSource::Misc_ShrinkParticle_Gaze)) {
													SpawnParticle(otherActor, 6.00f, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), otherActor->GetPosition(), tiny_size * 4.5f, 7, nullptr); 
													Runtime::PlaySoundAtNode("Magic_ProtectTinies", otherActor, 0.5f, 0.5f, "NPC Root [Root]");
													ApplyActionCooldown(otherActor, CooldownSource::Misc_ShrinkParticle_Gaze);
												}
												data->MovementSlowdown = 1.0f;
												MoanOr(giant);
											}
										}
										return;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void Task_ShrinkingGazeTask(Actor* giant) {
		std::string name = std::format("ShrinkingGaze_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			if (!HasSMT(giantref)) {
				return false;
			}

			if (!IsGtsBusy(giantref)) {	
				PerformShrinkOnActor(giantref);
			}

			return true;
		});
	}
}

namespace Gts {

	void StartShrinkingGaze(Actor* giant) {
		if (Runtime::HasPerk(giant, "ShrinkingGaze") && giant->formID == 0x14) {
			Task_ShrinkingGazeTask(giant);
		}
	}
}