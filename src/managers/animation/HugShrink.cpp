#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "colliders/charcontroller.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"


#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;


namespace {
	bool CanHugCrush(Actor* giant, Actor* huggedActor) {
		bool ForceCrush = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles");
		float staminapercent = GetStaminaPercentage(giant);
		float stamina = GetAV(giant, ActorValue::kStamina);
		if (ForceCrush && staminapercent >= 0.75) {
			AnimationManager::StartAnim("Huggies_HugCrush", giant);
			AnimationManager::StartAnim("Huggies_HugCrush_Victim", huggedActor);
			DamageAV(giant, ActorValue::kStamina, stamina * 1.10);
			return true;
		}
		return false;
	}

	void ShrinkPulse_DecreaseSize(Actor* tiny, float scale) {
		float min_scale = 0.04;
		float target_scale = get_target_scale(tiny);
		if (target_scale > min_scale) {
			set_target_scale(tiny, scale*0.48);
		} else {
			set_target_scale(tiny, min_scale);
		}
	}

	void ShrinkPulse_GainSize(Actor* giant, Actor* tiny, bool task) {
		float increase = 1.0;
		if (Runtime::HasPerkTeam(giant, "HugCrush_Greed")) {
			increase = 1.15;
		}

		if (!task) {
			float steal = get_visual_scale(tiny) * 0.035 * increase * 0.6;
			if (IsCrawling(giant)) {
				steal *= 0.8; // Crawl has one more shrink event so we compensate
			}
			mod_target_scale(giant, steal);
		} else {
			float Start = Time::WorldTimeElapsed();
			ActorHandle gianthandle = giant->CreateRefHandle();
			float original_scale = Vore::ReadOriginalScale(tiny);
			std::string name = std::format("HugCrushGrowth_{}_{}", giant->formID, tiny->formID);
			
			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}

				auto giantref = gianthandle.get().get();

				float Elapsed = Time::WorldTimeElapsed() - Start;
				float formula = bezier_curve(Elapsed, 0.2, 1.9, 0, 0, 3.0, 4.0); // Reuse formula from GrowthAnimation::Growth_2/5
				// https://www.desmos.com/calculator/reqejljy19
				if (formula >= 1.0) {
					formula = 1.0;
				}
				
				float grow = 0.000235 * 8 * original_scale * increase * TimeScale() * formula * 0.6;

				if (Elapsed <= 0.95) {
					override_actor_scale(giantref, grow, SizeEffectType::kNeutral);
					return true;
				} 
				return false;
			});
		}
	}

	void Hugs_ShakeCamera(Actor* giant) {
		if (giant->formID == 0x14) {
			shake_camera(giant, 0.75, 0.35);
		} else {
			Rumbling::Once("HugGrab_L", giant, Rumble_Hugs_Catch, 0.15, "NPC L Hand [LHnd]", 0.0);
			Rumbling::Once("HugGrab_R", giant, Rumble_Hugs_Catch, 0.15, "NPC R Hand [RHnd]", 0.0);
		}
	}

	void Hugs_ManageFriendlyTiny(ActorHandle gianthandle, ActorHandle tinyhandle) {
		Actor* giantref = gianthandle.get().get();
		Actor* tinyref = tinyhandle.get().get();

		AnimationManager::StartAnim("Huggies_Spare", giantref);
		AnimationManager::StartAnim("Huggies_Spare", tinyref);

		ForceRagdoll(tinyref, false);
		if (!HugAttach(gianthandle, tinyhandle)) {
			return;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// E V E N T S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTS_Hug_Catch(AnimationEventData& data) {
		auto giant = &data.giant;
		if (giant->IsSneaking()) {
			Hugs_ShakeCamera(giant);
		}

		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (huggedActor) {
			DisableCollisions(huggedActor, giant);
			Vore::RecordOriginalScale(huggedActor);
		}
	} // Used for Sneak Hugs only

	void GTS_Hug_Grab(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}

		SetBeingHeld(huggedActor, true);
		HugShrink::AttachActorTask(giant, huggedActor);

		if (!giant->IsSneaking()) {
			Hugs_ShakeCamera(giant);
		}
	}

	void GTS_Hug_Grow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}

		if (!IsTeammate(huggedActor)) {
			Attacked(huggedActor, giant);
		}

		HugShrink::ShrinkOtherTask(giant, huggedActor);
	}

	void GTS_Hug_Moan(AnimationEventData& data) {
		auto giant = &data.giant;
		PlayMoanSound(giant, 1.0);
		Task_FacialEmotionTask_Moan(giant, 2.75, "HugMoan");
	}

	void GTS_Hug_Moan_End(AnimationEventData& data) {
	}

	void GTS_Hug_FacialOn(AnimationEventData& data) { // Smug or something
		AdjustFacialExpression(&data.giant, 2, 1.0, "expression");
	}

	void GTS_Hug_FacialOff(AnimationEventData& data) { // Disable smug
		AdjustFacialExpression(&data.giant, 2, 0.0, "expression");
	}

	void GTS_Hug_PullBack(AnimationEventData& data) { // When we pull actor back to chest, used to play laugh
		int Random = RandomInt(0, 5);
		if (Random >= 4) {
			PlayLaughSound(&data.giant, 1.0, 1);
			Task_FacialEmotionTask_Smile(&data.giant, 2.25, "HugSmile");
		}
	}

	void GTSBEH_HugAbsorbAtk(AnimationEventData& data) {}

	void GTS_Hug_ShrinkPulse(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (huggedActor) {
			auto scale = get_visual_scale(huggedActor);
			float sizedifference = get_visual_scale(giant)/scale;

			Attacked(huggedActor, giant);

			ShrinkPulse_DecreaseSize(huggedActor, scale);
			ShrinkPulse_GainSize(giant, huggedActor, false);

			Rumbling::For("ShrinkPulse", giant, Rumble_Hugs_Shrink, 0.10, "NPC COM [COM ]", 0.50 / AnimationManager::GetAnimSpeed(giant), 0.0);
			ModSizeExperience(giant, scale/6);
		}
	}

	void GTS_Hug_RunShrinkTask(AnimationEventData& data) {}

	void GTS_Hug_StopShrinkTask(AnimationEventData& data) {}

	void GTS_Hug_CrushTiny(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		HugCrushOther(giant, huggedActor);
		PrintDeathSource(giant, huggedActor, DamageSource::Hugs);
		Rumbling::For("HugCrush", giant, Rumble_Hugs_HugCrush, 0.10, "NPC COM [COM ]", 0.15, 0.0);
		HugShrink::DetachActorTask(giant);

		AdjustFacialExpression(giant, 0, 0.0, "phenome");
		AdjustFacialExpression(giant, 0, 0.0, "modifier");
		AdjustFacialExpression(giant, 1, 0.0, "modifier");

		Task_ApplyAbsorbCooldown(giant); // Start Cooldown right after crush
		ShrinkPulse_GainSize(giant, huggedActor, true);

		if (giant->formID == 0x14) {
			auto caster = giant;
			float target_scale = get_visual_scale(huggedActor);
			AdjustSizeReserve(caster, 0.0225);
			AdjustSizeLimit(0.0060, caster);
			AdjustMassLimit(0.0060, caster);
		}
		HugShrink::Release(giant);
	}

	void GTSBeh_HugCrushEnd(AnimationEventData& data) {
		SetSneaking(&data.giant, false, 0); // Go back into sneaking if Actor was sneaking
	}

	void GTS_Hug_SwitchToObjectA(AnimationEventData& data) {
		Attachment_SetTargetNode(&data.giant, AttachToNode::ObjectA);
	}

	void GTS_Hug_SwitchToDefault(AnimationEventData& data) {
		/*Actor* giant = &data.giant;
		Actor* tiny = HugShrink::GetHuggiesActor(giant);

		if (IsHugCrushing(giant)) {
			Attachment_SetTargetNode(giant, AttachToNode::None);
		} else {
			if (tiny) {
				Task_FixTinyPosition(giant, tiny);
			}
		}*/
	}

	void GTS_CH_Tiny_FXStart(AnimationEventData& data) { // Spawn Runes on Tiny
		float scale = get_visual_scale(&data.giant) * 0.33;
		for (std::string_view hand_nodes: {"NPC L Hand [LHnd]", "NPC R Hand [RHnd]"}) {
			auto node = find_node(&data.giant, hand_nodes);
			if (node) {
				NiPoint3 position = node->world.translate;
				SpawnParticle(&data.giant, 6.00, "GTS/gts_tinyrune_bind.nif", NiMatrix3(), position, scale, 7, node); 
			}
		}

		for (std::string_view leg_nodes: {"NPC L Foot [Lft ]", "NPC R Foot [Rft ]"}) {
			auto node = find_node(&data.giant, leg_nodes);
			if (node) {
				NiPoint3 position = node->world.translate;
				SpawnParticle(&data.giant, 6.00, "GTS/gts_tinyrune_bind_leg.nif", NiMatrix3(), position, scale, 7, node); 
			}
		}
	}

	void GTS_CH_RuneStart(AnimationEventData& data) { // GTS rune
		auto huggedActor = HugShrink::GetHuggiesActor(&data.giant);
		if (huggedActor) {
			auto ObjectB = find_node(huggedActor, "AnimObjectB");
			if (ObjectB) {
				NiPoint3 position = ObjectB->world.translate;
				SpawnParticle(huggedActor, 3.00, "GTS/gts_chugrune.nif", NiMatrix3(), position, get_visual_scale(huggedActor), 7, ObjectB); 
			}
		}
	}

	void GTS_CH_RuneEnd(AnimationEventData& data) { // Empty
	}

	void GTS_CH_BoobCameraOn(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Breasts_02);
		if (data.giant.formID == 0x14) {
			std::string name = std::format("ChangeCamera_{}", data.giant.formID);
			ActorHandle gianthandle = data.giant.CreateRefHandle();
			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}
				Actor* giantref = gianthandle.get().get();
				if (!IsHugging(giantref) && !IsHugCrushing(giantref) && !IsGtsBusy(giantref)) {
					ManageCamera(giantref, false, CameraTracking::None);
					return false;
				}
				return true;
			});
		}
	}
	void GTS_CH_BoobCameraOff(AnimationEventData& data) {
		//ManageCamera(&data.giant, false, CameraTracking::None);
	}

	

	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// I N P U T
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	void HugAttemptEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 0)) {
			return;
		}
		if (IsGtsBusy(player)) {
			return;
		}
		if (CanDoPaired(player) && !IsSynced(player) && !IsTransferingTiny(player)) {
			auto& Hugging = HugAnimationController::GetSingleton();

			std::vector<Actor*> preys = Hugging.GetHugTargetsInFront(player, 1.0);
			for (auto prey: preys) {
				Hugging.StartHug(player, prey);
			}
		}
	}

	void HugAttemptEvent_Follower(const InputEventData& data) {
		Actor* player = PlayerCharacter::GetSingleton();
		ForceFollowerAnimation(player, FollowerAnimType::Hugs);
	}

	void HugCrushEvent(const InputEventData& data) {
		Actor* player = GetPlayerOrControlled();
		auto huggedActor = HugShrink::GetHuggiesActor(player);

		if (huggedActor) {
			if (!IsActionOnCooldown(player, CooldownSource::Action_AbsorbOther)) {
				float health = GetHealthPercentage(huggedActor);
				float HpThreshold = GetHugCrushThreshold(player, huggedActor, true);
				if (health <= HpThreshold) {
					AnimationManager::StartAnim("Huggies_HugCrush", player);
					AnimationManager::StartAnim("Huggies_HugCrush_Victim", huggedActor);
					return;
				} else if (HasSMT(player)) {
					AnimationManager::StartAnim("Huggies_HugCrush", player);
					AnimationManager::StartAnim("Huggies_HugCrush_Victim", huggedActor);
					AddSMTPenalty(player, 10.0); // Mostly called inside ShrinkUntil
					DamageAV(player, ActorValue::kStamina, 60);
					return;
				} else {
					if (CanHugCrush(player, huggedActor)) { // Force hug crush in this case
						return;
					}
					std::string message = std::format("{} is too healthy to be hug crushed", huggedActor->GetDisplayFullName());
					shake_camera(player, 0.45, 0.30);
					NotifyWithSound(player, message);

					Notify("Health: {:.0f}%; Requirement: {:.0f}%", health * 100.0, HpThreshold * 100.0);
				}
			} else {
				float cooldown = GetRemainingCooldown(player, CooldownSource::Action_AbsorbOther);
                std::string message = std::format("Hug Crush is on a cooldown: {:.1f} sec", cooldown);
				NotifyWithSound(player, message);
			}
		}
	}

	void HugShrinkEvent(const InputEventData& data) {
		Actor* player = GetPlayerOrControlled();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		if (GetSizeDifference(player, huggedActor, SizeType::VisualScale, false, true) >= GetHugShrinkThreshold(player)) {
			if (!IsHugCrushing(player) && !IsHugHealing(player)) {
				NotifyWithSound(player, "All available size was drained");
				shake_camera(player, 0.45, 0.30);
			}
			return;
		}

		AnimationManager::StartAnim("Huggies_Shrink", player);
		AnimationManager::StartAnim("Huggies_Shrink_Victim", huggedActor);
	}

	void HugHealEvent(const InputEventData& data) {
		Actor* player = GetPlayerOrControlled();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}

		if (GetSizeDifference(player, huggedActor, SizeType::VisualScale, false, true) >= GetHugShrinkThreshold(player)) {
			if (!IsHugCrushing(player) && !IsHugHealing(player)) {
				NotifyWithSound(player, "All available size was drained");
				shake_camera(player, 0.45, 0.30);
			}
			return;
		}

		if (Runtime::HasPerkTeam(player, "HugCrush_LovingEmbrace")) {
			if (!IsHostile(huggedActor, player) && (IsTeammate(huggedActor) || huggedActor->formID == 0x14)) {
				StartHealingAnimation(player, huggedActor);
				return;
			} else {
				AnimationManager::StartAnim("Huggies_Shrink", player);
				AnimationManager::StartAnim("Huggies_Shrink_Victim", huggedActor);
			}
		} else {
			AnimationManager::StartAnim("Huggies_Shrink", player);
			AnimationManager::StartAnim("Huggies_Shrink_Victim", huggedActor);
			UpdateFriendlyHugs(player, huggedActor, true);
		}
	}
	void HugReleaseEvent(const InputEventData& data) {
		Actor* player = GetPlayerOrControlled();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (huggedActor) {
			if (IsHugCrushing(player) || IsHugHealing(player)) {
				return; // disallow manual release when it's true
			}

			bool Hugging;
			player->GetGraphVariableBool("GTS_HuggingTeammate", Hugging);

			AbortHugAnimation(player, huggedActor);

			if (!Hugging) { // we don't want to stop task if it returns true
				HugShrink::DetachActorTask(player);
			}
		}
	}
}

namespace Gts {
	HugShrink& HugShrink::GetSingleton() noexcept {
		static HugShrink instance;
		return instance;
	}

	std::string HugShrink::DebugName() {
		return "HugShrink";
	}

	void HugShrink::DetachActorTask(Actor* giant) {
		std::string name = std::format("Huggies_{}", giant->formID);
		std::string name_2 = std::format("Huggies_Shrink_{}", giant->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name_2);
	}

	void HugShrink::ShrinkOtherTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_Shrink_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();

		UpdateFriendlyHugs(giant, tiny, true);

		const float duration = 2.0;
		TaskManager::RunFor(name, duration, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			float sizedifference = GetSizeDifference(giantref, tinyref, SizeType::VisualScale, false, true);
			float steal = GetHugStealRate(giantref) * 0.85;
			
			float stamina = 0.35;
			float shrink = 14.0;
			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				shrink *= 1.25;
				stamina *= 0.75;
			}
			stamina *= Perk_GetCostReduction(giantref);

			if (sizedifference >= GetHugShrinkThreshold(giantref)) {
				std::string_view message = std::format("{} stole all available size", giantref->GetDisplayFullName());
				Notify(message);
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, (0.60 * TimeScale())); // Drain Stamina
			DamageAV(giantref, ActorValue::kStamina, 0.50 * stamina * TimeScale()); // Damage GTS Stamina
			
			TransferSize(giantref, tinyref, false, shrink, steal, false, ShrinkSource::Hugs); // Shrink foe, enlarge gts
			ModSizeExperience(giantref, 0.00020);
			Attacked(tinyref, giantref); // make it look like we attack the tiny
			Rumbling::Once("HugSteal", giantref, Rumble_Hugs_Shrink, 0.12, "NPC COM [COM ]", 0.0);
			
			return true;
		});
	}

	void HugShrink::AttachActorTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}

		std::string name = std::format("Huggies_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto tinyref = tinyhandle.get().get();
			auto giantref = gianthandle.get().get();
			
			bool GTS_HuggingAlly = false;
			bool Tiny_HuggedAsAlly = false;
			float DrainReduction = 3.4;
			tinyref->GetGraphVariableBool("GTS_IsFollower", Tiny_HuggedAsAlly);
			giantref->GetGraphVariableBool("GTS_HuggingTeammate", GTS_HuggingAlly);

			ApplyActionCooldown(giantref, CooldownSource::Action_Hugs); // Send Hugs on cooldown non-stop

			bool HuggingAlly = GTS_HuggingAlly && Tiny_HuggedAsAlly;

			if (HuggingAlly) {
				DrainReduction *= 1.5; // less stamina drain for friendlies
			}

			float sizedifference = GetSizeDifference(giantref, tinyref, SizeType::VisualScale, false, true);

			ShutUp(tinyref);
			ShutUp(giantref);

			if (!FaceOpposite(giantref, tinyref)) { // Makes the actor face us
				// If face towards fails then actor is invalid
				AbortHugAnimation(giantref, tinyref);
				return false;
			}

			if (!IsHugging(giantref)) { // If for some reason we're not in the hug anim and actor is still attached to us: cancel it
				AbortHugAnimation(giantref, tinyref);
				return false;
			}
			
			GrabStaminaDrain(giantref, tinyref, sizedifference * DrainReduction);
			DamageAV(tinyref, ActorValue::kStamina, 0.125 * TimeScale()); // Drain Tiny Stamina
			ModSizeExperience(giantref, 0.00005);
			
			bool TinyAbsorbed;
			giantref->GetGraphVariableBool("GTS_TinyAbsorbed", TinyAbsorbed);

			float stamina = GetAV(giantref, ActorValue::kStamina);

			/*log::info("----{} Is Hugging {}", giantref->GetDisplayFullName(), tinyref->GetDisplayFullName());
			log::info("Anim Speed: GTS: {}, Tiny: {}", AnimationManager::GetAnimSpeed(giantref), AnimationManager::GetAnimSpeed(tinyref));*/

			Utils_UpdateHugBehaviors(giantref, tinyref); // Record GTS/Tiny Size-Difference value for animation blending
			Anims_FixAnimationDesync(giantref, tinyref, false); // Share GTS Animation Speed with hugged actor to avoid de-sync

			if (IsHugHealing(giantref)) {
				ForceRagdoll(tinyref, false);
				if (!HugAttach(gianthandle, tinyhandle)) {
					AbortHugAnimation(giantref, tinyref);
					return false;
				}
				return true; // do nothing while we heal actor
			}
			
			bool GotTiny = HugShrink::GetHuggiesActor(giantref);
			bool IsDead = (giantref->IsDead() || tinyref->IsDead());
			
			if (!IsHugCrushing(giantref)) {
				if (sizedifference < Action_Hug || IsDead || stamina <= 2.0 || !GotTiny) {
					if (HuggingAlly) { 
						// this is needed to still attach the actor while we have ally hugged (with Loving Embrace Perk)
					    // It fixes the Tiny not being moved around during Gentle Release animation for friendlies
						// If it will be disabled, it will look off during gentle release
						Hugs_ManageFriendlyTiny(gianthandle, tinyhandle);
						return true;
					}
					AbortHugAnimation(giantref, tinyref);
					return false;
				}
			} else if (IsHugCrushing(giantref) && !TinyAbsorbed) {
				if (IsDead || !GotTiny) {
					AbortHugAnimation(giantref, tinyref);
					return false;
				}
			}
			// Ensure they are NOT in ragdoll
			ForceRagdoll(tinyref, false);
			if (IsCrawling(giantref)) { // Always attach to ObjectA during Crawling (Crawl anims are configured for ObjectA)
				if (!AttachToObjectA(giantref, tinyref)) {
					return false;
				}
			} else {
				if (!HugAttach(gianthandle, tinyhandle)) { // Else use default hug attach logic
					AbortHugAnimation(giantref, tinyref);
					return false;
				}
			}

			// All good try another frame
			return true;
		});
	}


	void HugShrink::HugActor_Actor(Actor* giant, TESObjectREFR* tiny, float strength) {
		HugShrink::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void HugShrink::HugActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		HugShrink::HugActor_Actor(giant, tiny, 1.0);
	}

	void HugShrink::Reset() {
		this->data.clear();
	}

	void HugShrink::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void HugShrink::Release(Actor* giant) {
		HugShrink::GetSingleton().data.erase(giant);
	}

	void HugShrink::CallRelease(Actor* giant) {
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		std::string_view message = std::format("{} was saved from hugs of {}", huggedActor->GetDisplayFullName(), giant->GetDisplayFullName());
		float sizedifference = get_visual_scale(giant)/get_visual_scale(huggedActor);
		if (giant->formID == 0x14) {
			shake_camera(giant, 0.25 * sizedifference, 0.35);
		} else {
			Rumbling::Once("HugRelease", giant, Rumble_Hugs_Release, 0.10);
		}
		Notify(message);
		AbortHugAnimation(giant, huggedActor);
	}

	TESObjectREFR* HugShrink::GetHuggiesObj(Actor* giant) {
		try {
			auto& me = HugShrink::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* HugShrink::GetHuggiesActor(Actor* giant) {
		auto obj = HugShrink::GetHuggiesObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void HugShrink::RegisterEvents() {
		InputManager::RegisterInputEvent("HugPlayer", HugAttemptEvent_Follower);
		InputManager::RegisterInputEvent("HugAttempt", HugAttemptEvent);
		InputManager::RegisterInputEvent("HugRelease", HugReleaseEvent);
		InputManager::RegisterInputEvent("HugShrink", HugShrinkEvent);
		InputManager::RegisterInputEvent("HugHeal", HugHealEvent);
		InputManager::RegisterInputEvent("HugCrush", HugCrushEvent);

		AnimationManager::RegisterEvent("GTS_Hug_Catch", "Hugs", GTS_Hug_Catch);
		AnimationManager::RegisterEvent("GTS_Hug_Grab", "Hugs", GTS_Hug_Grab);
		AnimationManager::RegisterEvent("GTS_Hug_Grow", "Hugs", GTS_Hug_Grow);
		AnimationManager::RegisterEvent("GTS_Hug_Moan", "Hugs", GTS_Hug_Moan);
		AnimationManager::RegisterEvent("GTS_Hug_Moan_End", "Hugs", GTS_Hug_Moan_End);
		AnimationManager::RegisterEvent("GTS_Hug_PullBack", "Hugs", GTS_Hug_PullBack);
		AnimationManager::RegisterEvent("GTS_Hug_FacialOn", "Hugs", GTS_Hug_FacialOn);
		AnimationManager::RegisterEvent("GTS_Hug_FacialOff", "Hugs", GTS_Hug_FacialOff);
		AnimationManager::RegisterEvent("GTS_Hug_CrushTiny", "Hugs", GTS_Hug_CrushTiny);
		AnimationManager::RegisterEvent("GTS_Hug_ShrinkPulse", "Hugs", GTS_Hug_ShrinkPulse);
		AnimationManager::RegisterEvent("GTS_Hug_RunShrinkTask", "Hugs", GTS_Hug_RunShrinkTask);
		AnimationManager::RegisterEvent("GTS_Hug_StopShrinkTask", "Hugs", GTS_Hug_StopShrinkTask);

		AnimationManager::RegisterEvent("GTSBeh_HugCrushEnd", "Hugs", GTSBeh_HugCrushEnd);

		AnimationManager::RegisterEvent("GTSBEH_HugAbsorbAtk", "Hugs", GTSBEH_HugAbsorbAtk);

		AnimationManager::RegisterEvent("GTS_Hug_SwitchToObjectA", "Hugs", GTS_Hug_SwitchToObjectA);
		AnimationManager::RegisterEvent("GTS_Hug_SwitchToDefault", "Hugs", GTS_Hug_SwitchToDefault);


		AnimationManager::RegisterEvent("GTS_CH_Tiny_FXStart", "Hugs", GTS_CH_Tiny_FXStart);
		AnimationManager::RegisterEvent("GTS_CH_RuneStart", "Hugs", GTS_CH_RuneStart);
		AnimationManager::RegisterEvent("GTS_CH_RuneEnd", "Hugs", GTS_CH_RuneEnd);

		AnimationManager::RegisterEvent("GTS_CH_BoobCameraOn", "Hugs", GTS_CH_BoobCameraOn);
		AnimationManager::RegisterEvent("GTS_CH_BoobCameraOff", "Hugs", GTS_CH_BoobCameraOff);
	}

	void HugShrink::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Try", "Hugs", "GTSBEH_HugAbsorbStart_A");
		AnimationManager::RegisterTrigger("Huggies_Try_Victim", "Hugs", "GTSBEH_HugAbsorbStart_V");
		AnimationManager::RegisterTrigger("Huggies_Try_Victim_S", "Hugs", "GTSBEH_HugAbsorbStart_Sneak_V");
		AnimationManager::RegisterTrigger("Huggies_Shrink", "Hugs", "GTSBEH_HugAbsorbAtk");
		AnimationManager::RegisterTrigger("Huggies_Shrink_Victim", "Hugs", "GTSBEH_HugAbsorbAtk_V");
		AnimationManager::RegisterTrigger("Huggies_Spare", "Hugs", "GTSBEH_HugAbsorbExitLoop");
		AnimationManager::RegisterTrigger("Huggies_Cancel", "Hugs", "GTSBEH_PairedAbort");
		AnimationManager::RegisterTrigger("Huggies_HugCrush", "Hugs", "GTSBEH_HugCrushStart_A");
		AnimationManager::RegisterTrigger("Huggies_HugCrush_Victim", "Hugs", "GTSBEH_HugCrushStart_V");
	}

	HugShrinkData::HugShrinkData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}
