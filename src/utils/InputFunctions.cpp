#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "Utils/InputConditions.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/InputFunctions.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "data/plugin.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "utils/av.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace {
	void ReportScaleIntoConsole(Actor* actor, bool enemy) {
		float hh = HighHeelManager::GetBaseHHOffset(actor)[2]/100;
		float gigantism = Ench_Aspect_GetPower(actor) * 100;
		float naturalscale = get_natural_scale(actor, true);
		float scale = get_visual_scale(actor);
		float maxscale = get_max_scale(actor);

		Actor* player = PlayerCharacter::GetSingleton();

		float BB = GetSizeFromBoundingBox(actor);
		if (enemy) {
			Cprint("{} Bounding Box To Size: {:.2f}, GameScale: {:.2f}", actor->GetDisplayFullName(), BB, game_getactorscale(actor));
			Cprint("{} Size Difference With the Player: {:.2f}", actor->GetDisplayFullName(), GetSizeDifference(player, actor, SizeType::VisualScale, false, true));
		} else {
			Cprint("{} Height: {:.2f} m / {:.2f} ft; Weight: {:.2f} kg / {:.2f} lb;", actor->GetDisplayFullName(), GetActorHeight(actor, true), GetActorHeight(actor, false), GetActorWeight(actor, true), GetActorWeight(actor, false));
		}

		if (maxscale > 250.0f * naturalscale) {
			Cprint("{} Scale: {:.2f}  (Natural Scale: {:.2f}; Bounding Box: {}; Size Limit: Infinite; Aspect Of Giantess: {:.1f}%)", actor->GetDisplayFullName(), scale, naturalscale, BB, gigantism);
		} else {
			Cprint("{} Scale: {:.2f}  (Natural Scale: {:.2f}; Bounding Box: {}; Size Limit: {:.2f}; Aspect Of Giantess: {:.1f}%)", actor->GetDisplayFullName(), scale, naturalscale, BB, maxscale, gigantism);
		}
		if (hh > 0.0f) { // if HH is > 0, print HH info
			Cprint("{} High Heels: {:.2f} (+{:.2f} cm / +{:.2f} ft)", actor->GetDisplayFullName(), hh, hh, hh*3.28f);
		}
	}
	void ReportScale(bool enemy) {
		for (auto actor: find_actors()) {
			if (actor->formID != 0x14) {
				if (enemy && !IsTeammate(actor)) {
					ReportScaleIntoConsole(actor, enemy);
				} else if (IsTeammate(actor)) {
					ReportScaleIntoConsole(actor, false);
				}
			} else {
				if (!enemy) {
					ReportScaleIntoConsole(actor, false);
				}
			}
		}
	}

	void regenerate_health(Actor* giant, float value) {
		if (Runtime::HasPerk(giant, "SizeReserveAug2")) {
			float maxhp = GetMaxAV(giant, ActorValue::kHealth);
			float regenerate = maxhp * 0.25f * value; // 25% of health

			giant->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, regenerate * TimeScale());
		}
	}

	void TotalControlGrowEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			float scale = get_visual_scale(player);
			float stamina = std::clamp(GetStaminaPercentage(player), 0.05f, 1.0f);

			float perk = Perk_GetCostReduction(player);

			DamageAV(player, ActorValue::kStamina, 0.15f * perk * (scale * 0.5f + 0.5f) * stamina * TimeScale());
			Grow(player, 0.0010f * stamina, 0.0f);
			float Volume = std::clamp(get_visual_scale(player)/16.0f, 0.20f, 2.0f);
			Rumbling::Once("ColossalGrowth", player, 0.15f, 0.05f);
			static Timer timergrowth = Timer(2.00);
			if (timergrowth.ShouldRun()) {
				Runtime::PlaySoundAtNode("growthSound", player, Volume, 1.0f, "NPC Pelvis [Pelv]");
			}
	}
	void TotalControlShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			float scale = get_visual_scale(player);
			float stamina = std::clamp(GetStaminaPercentage(player), 0.05f, 1.0f);

			float perk = Perk_GetCostReduction(player);

			if (get_target_scale(player) > 0.12f) {
				DamageAV(player, ActorValue::kStamina, 0.07f * perk * (scale * 0.5f + 0.5f) * stamina * TimeScale());
				ShrinkActor(player, 0.0010f * stamina, 0.0f);
			} else {
				set_target_scale(player, 0.12f);
			}

			float Volume =std::clamp(get_visual_scale(player)*0.10f, 0.10f, 1.0f);
			Rumbling::Once("ColossalGrowth", player, 0.15f, 0.05f);
			static Timer timergrowth = Timer(2.00);
			if (timergrowth.ShouldRun()) {
				Runtime::PlaySound("shrinkSound", player, Volume, 1.0f);
			}
	}
	void TotalControlGrowOtherEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}
				if (actor->formID != 0x14 && (IsTeammate(actor))) {

					float perk = Perk_GetCostReduction(player);

					float npcscale = get_visual_scale(actor);
					float magicka = std::clamp(GetMagikaPercentage(player), 0.05f, 1.0f);
					DamageAV(player, ActorValue::kMagicka, 0.15f * perk * (npcscale * 0.5f + 0.5f) * magicka * TimeScale());
					Grow(actor, 0.0010f * magicka, 0.0f);
					float Volume = std::clamp(0.20f, 2.0f, get_visual_scale(actor)/16.0f);
					Rumbling::Once("TotalControlOther", actor, 0.15f, 0.05f);
					static Timer timergrowth = Timer(2.00);
					if (timergrowth.ShouldRun()) {
						Runtime::PlaySoundAtNode("growthSound", actor, Volume, 1.0f, "NPC Pelvis [Pelv]");
					}
				}
			}
	}
	void TotalControlShrinkOtherEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}
				if (actor->formID != 0x14 && (IsTeammate(actor))) {
					
					float perk = Perk_GetCostReduction(player);

					float npcscale = get_visual_scale(actor);
					float magicka = std::clamp(GetMagikaPercentage(player), 0.05f, 1.0f);
					DamageAV(player, ActorValue::kMagicka, 0.07f * perk * (npcscale * 0.5f + 0.5f) * magicka * TimeScale());
					ShrinkActor(actor, 0.0010f * magicka, 0.0f);
					float Volume = std::clamp(get_visual_scale(actor) * 0.10f, 0.10f, 1.0f);
					Rumbling::Once("TotalControlOther", actor, 0.15f, 0.05f);
					static Timer timergrowth = Timer(2.00);
					if (timergrowth.ShouldRun()) {
						Runtime::PlaySound("shrinkSound", actor, Volume, 1.0f);
					}
				} 
			}
	}

	void RapidGrowthEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			float target = get_target_scale(player);
			float max_scale = get_max_scale(player);// * get_natural_scale(player);
			if (target >= max_scale) {
				NotifyWithSound(player, "You can't grow any further");
				Rumbling::Once("CantGrow", player, 0.25f, 0.05f);
				return;
			}
			AnimationManager::StartAnim("TriggerGrowth", player);
	}
	void RapidShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
			float target = get_target_scale(player);
			if (target <= Minimum_Actor_Scale) {
				NotifyWithSound(player, "You can't shrink any further");
				Rumbling::Once("CantGrow", player, 0.25f, 0.05f);
				return;
			}
			AnimationManager::StartAnim("TriggerShrink", player);
	}

	void SizeReserveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto Cache = Persistent::GetSingleton().GetData(player);
		if (!Cache) {
			return;
		}
		if (Cache->SizeReserve > 0.0f) {
			bool Attacking = false;
			player->GetGraphVariableBool("GTS_IsGrabAttacking", Attacking);

			if (!Attacking) {
				float duration = data.Duration();
				

				if (duration >= 1.2f && Runtime::HasPerk(player, "SizeReserve") && Cache->SizeReserve > 0) {
					bool HandsBusy = Grab::GetHeldActor(player);
					if (!HandsBusy) {
						float SizeCalculation = duration - 1.2f;
						float gigantism = 1.0f + Ench_Aspect_GetPower(player);
						float Volume = std::clamp(get_visual_scale(player) * Cache->SizeReserve/10.0f, 0.10f, 2.0f);
						static Timer timergrowth = Timer(3.00);
						if (timergrowth.ShouldRunFrame()) {
							Runtime::PlaySoundAtNode("growthSound", player, Cache->SizeReserve/50 * duration, 1.0f, "NPC Pelvis [Pelv]");
							Task_FacialEmotionTask_Moan(player, 2.0f, "SizeReserve");
							PlayMoanSound(player, Volume);
						}

						float shake_power = std::clamp(Cache->SizeReserve/15 * duration, 0.0f, 2.0f);
						Rumbling::Once("SizeReserve", player, shake_power, 0.05f);

						update_target_scale(player, (SizeCalculation/80) * gigantism, SizeEffectType::kNeutral);
						regenerate_health(player, (SizeCalculation/80) * gigantism);

						Cache->SizeReserve -= SizeCalculation/80;
						if (Cache->SizeReserve <= 0) {
							Cache->SizeReserve = 0.0f; // Protect against negative values.
						}
					}
				}
			}
		}
	}

	void DisplaySizeReserveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto Cache = Persistent::GetSingleton().GetData(player);
		if (Cache) {
			if (Runtime::HasPerk(player, "SizeReserve")) {
				float gigantism = 1.0f + Ench_Aspect_GetPower(player);
				float Value = Cache->SizeReserve * gigantism;
				Notify("Reserved Size: {:.2f}", Value);
			}
		}
	}

	void PartyReportEvent(const InputEventData& data) { // Report follower scale into console
		ReportScale(false);
	}

	void DebugReportEvent(const InputEventData& data) { // Report enemy scale into console
		ReportScale(true);
	}

	void ShrinkOutburstEvent(const InputEventData& data) {

		auto player = PlayerCharacter::GetSingleton();

		bool DarkArts2 = Runtime::HasPerk(player, "DarkArts_Aug2");
		bool DarkArts3 = Runtime::HasPerk(player, "DarkArts_Aug3");

		float gigantism = 1.0f + Ench_Aspect_GetPower(player);

		float multi = GetDamageResistance(player);

		float healthMax = GetMaxAV(player, ActorValue::kHealth);
		float healthCur = GetAV(player, ActorValue::kHealth);
		float damagehp = 80.0f;

		if (DarkArts2) {
			damagehp -= 10; // less hp drain
		}
		if (DarkArts3) {
			damagehp -= 10; // even less hp drain
		}

		damagehp *= multi;
		damagehp /= gigantism;

		if (healthCur < damagehp * 1.10f) {
			Notify("Your health is too low");
			return; // don't allow us to die from own shrinking
		}

		static Timer NotifyTimer = Timer(2.0);
		bool OnCooldown = IsActionOnCooldown(player, CooldownSource::Misc_ShrinkOutburst);
		if (OnCooldown) {
			if (NotifyTimer.ShouldRunFrame()) {
				double cooldown = GetRemainingCooldown(player, CooldownSource::Misc_ShrinkOutburst);
				std::string message = std::format("Shrink Outburst is on a cooldown: {:.1f} sec", cooldown);
				shake_camera(player, 0.75f, 0.35f);
				NotifyWithSound(player, message);
			}
			return;
		}
		ApplyActionCooldown(player, CooldownSource::Misc_ShrinkOutburst);
		DamageAV(player, ActorValue::kHealth, damagehp);
		ShrinkOutburstExplosion(player, false);
	}

	void ProtectSmallOnesEvent(const InputEventData& data) {
		static Timer ProtectTimer = Timer(5.0);
		if (ProtectTimer.ShouldRunFrame()) {
			bool balance = IsInBalanceMode();
			Utils_ProtectTinies(balance);
		}
	}

	void AnimSpeedUpEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.045f); // Increase speed and power
	}
	void AnimSpeedDownEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(-0.045f); // Decrease speed and power
	}
	void AnimMaxSpeedEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.090f); // Strongest attack speed buff
	}

	void VoreInputEvent(const InputEventData& data) {
		static Timer voreTimer = Timer(0.25);
		auto pred = PlayerCharacter::GetSingleton();
		if (IsGtsBusy(pred)) {
			return;
		}

		if (voreTimer.ShouldRunFrame()) {
			auto& VoreManager = Vore::GetSingleton();

			std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, 1);
			for (auto prey: preys) {
				VoreManager.StartVore(pred, prey);
			}
		}
	}

	void VoreInputEvent_Follower(const InputEventData& data) {
		Actor* player = PlayerCharacter::GetSingleton();
		ForceFollowerAnimation(player, FollowerAnimType::Vore);
	}
}

namespace Gts
{
	void InputFunctions::RegisterEvents() {
		InputManager::RegisterInputEvent("SizeReserve", SizeReserveEvent, SizeReserveCondition);
		InputManager::RegisterInputEvent("DisplaySizeReserve", DisplaySizeReserveEvent, SizeReserveCondition);
		InputManager::RegisterInputEvent("PartyReport", PartyReportEvent);
		InputManager::RegisterInputEvent("DebugReport", DebugReportEvent);
		InputManager::RegisterInputEvent("AnimSpeedUp", AnimSpeedUpEvent);
		InputManager::RegisterInputEvent("AnimSpeedDown", AnimSpeedDownEvent);
		InputManager::RegisterInputEvent("AnimMaxSpeed", AnimMaxSpeedEvent);
		InputManager::RegisterInputEvent("RapidGrowth", RapidGrowthEvent, RappidGrowShrinkCondition);
		InputManager::RegisterInputEvent("RapidShrink", RapidShrinkEvent, RappidGrowShrinkCondition);
		InputManager::RegisterInputEvent("ShrinkOutburst", ShrinkOutburstEvent, ShrinkOutburstCondition);
		InputManager::RegisterInputEvent("ProtectSmallOnes", ProtectSmallOnesEvent, ProtectSmallOnesCondition);

		InputManager::RegisterInputEvent("TotalControlGrow", TotalControlGrowEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("TotalControlShrink", TotalControlShrinkEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("TotalControlGrowOther", TotalControlGrowOtherEvent, TotalControlCondition);
		InputManager::RegisterInputEvent("TotalControlShrinkOther", TotalControlShrinkOtherEvent, TotalControlCondition);

		InputManager::RegisterInputEvent("Vore", VoreInputEvent, VoreCondition);
		InputManager::RegisterInputEvent("PlayerVore", VoreInputEvent_Follower, AlwaysBlock);
	}
}