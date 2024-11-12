#include "managers/animation/Controllers/ThighCrushController.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/ai/ai_PerformAction.hpp"
#include "managers/ai/ai_SelectAction.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/ai_Manager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "utils/random.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {
	void RecordSneakingState(Actor* giant, Actor* tiny) {
		bool Crawling = IsCrawling(giant);
		bool Sneaking = giant->IsSneaking();

		tiny->SetGraphVariableBool("GTS_Hug_Sneak_Tny", Sneaking); // Is Sneaking?
		tiny->SetGraphVariableBool("GTS_Hug_Crawl_Tny", Crawling); // Is Crawling?
	}

	bool AI_CanHugCrush(Actor* giant, Actor* tiny, int rng) {
		int crush_rng = RandomInt(0, 4);

		float health = GetHealthPercentage(tiny);
		float HpThreshold = GetHugCrushThreshold(giant, tiny, true);

		bool low_hp = (health <= HpThreshold);
		bool allow_perform = (tiny->formID != 0x14 && IsHostile(giant, tiny)) || (rng <= 1);
		bool Can_HugCrush = (low_hp && allow_perform);

		float stamina = GetStaminaPercentage(giant);
		bool Can_Force = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles") && IsHostile(giant, tiny);

		if (Can_Force && crush_rng <= 1 && stamina >= 0.75f) {
			return true;
		}
		if (Can_HugCrush) {
			return true;
		}
		return false;
	}

	void AI_HealOrShrink(Actor* giant, Actor* tiny, int rng) {
		bool hostile = IsHostile(giant, tiny);
		bool IsCrushing = IsHugCrushing(giant);
		
		if (!IsCrushing && (hostile || rng <= 1)) { // chance to get drained by follower, it is always 1 without Loving Embrace perk

			float sizedifference = GetSizeDifference(giant, tiny, SizeType::TargetScale, false, true);
			if (!IsHugHealing(giant) && sizedifference >= GetHugShrinkThreshold(giant)) {
				AbortHugAnimation(giant, tiny); // Cancel anim if below shrink threshold
				log::info("Sizediff is > threshold, aborting");
			} else {
				AnimationManager::StartAnim("Huggies_Shrink", giant);
				AnimationManager::StartAnim("Huggies_Shrink_Victim", tiny);
			}
		} else { // else heal
			StartHealingAnimation(giant, tiny);
		}
	}

	void AI_SelectActionToPlay(Actor* pred, Actor* prey, int rng, int butt_rng, int action_rng) {
		if (IsGtsBusy(pred)) {
			return;
		}

		if (rng <= 2 && butt_rng <= 2) {
			AI_ButtCrush(pred, prey);
			return;
		} else if (rng <= 3) {
			AI_StrongStomp(pred, action_rng);
			return;
		} else if (rng <= 6) {
			AI_LightStomp(pred, action_rng);
			return;
		} else if (rng <= 8) {
			AI_Kicks(pred, action_rng);
			return;
		} else if (rng <= 9) {
			AI_Tramples(pred, action_rng);
			return;
		}
	}
}

namespace Gts {

	void AI_TryAction(Actor* actor) {
		float scale = std::clamp(get_visual_scale(actor), 1.0f, 6.0f);
		if (GetAV(actor, ActorValue::kHealth) < 0) {
			log::info("Action: {} Health is < 0", actor->GetDisplayFullName());
			return;
		}
		if (!IsGtsBusy(actor)) {
			int rng = RandomInt(0, 100);
			if (rng > 7 && rng < 33 * scale) {
				AI_DoStomp_Kick_ButtCrush(actor);
				return;
			} else if (rng > 3 && rng < 7) {
				AI_DoSandwich(actor);
				return;
			} else if (rng <= 3) {
				int HugsOrThigh = RandomInt(0, 10);
				if (HugsOrThigh > 5) {
					AI_DoHugs(actor);
				} else {
					AI_DoThighCrush(actor);
				}
			}
		}
		// Random Vore is managed inside Vore.cpp, RandomVoreAttempt(Actor* pred) function
	}

	void AI_DoStomp_Kick_ButtCrush(Actor* pred) {
		int rng = RandomInt(0,10);
        int butt_rng = RandomInt(0,10);
        int action_rng = RandomInt(0, 10);
        std::size_t amount = 6;
        std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
        for (auto prey: preys) {
            AI_SelectActionToPlay(pred, prey, rng, butt_rng, action_rng);
        }
    }

	void AI_DoSandwich(Actor* pred) {
		if (!Persistent::GetSingleton().Sandwich_Ai || IsCrawling(pred)) {
			return;
		}
		auto& Sandwiching = ThighSandwichController::GetSingleton();

		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, 1);
		for (auto prey: preys) {
			if (CanPerformAnimationOn(pred, prey, false)) { // player check is done inside CanSandwich()
				Sandwiching.StartSandwiching(pred, prey);
				auto node = find_node(pred, "GiantessRune", false);
				if (node) {
					node->local.scale = 0.01f;
					update_node(node);
				}
			}
		}
	}

	void AI_DoHugs(Actor* pred) {
		if (!Persistent::GetSingleton().Hugs_Ai || IsCrawling(pred)) {
			return;
		}
		int rng = RandomInt(0, 7);
		if (rng >= 2) {
			if (CanDoPaired(pred) && !IsSynced(pred) && !IsTransferingTiny(pred)) {
				auto& hugs = HugAnimationController::GetSingleton();
				std::vector<Actor*> preys = hugs.GetHugTargetsInFront(pred, 1);
				for (auto prey: preys) {
					// ^ If Size > 0.92f (minimum) && Size < 2.5f + perk bonus (maximum) threshold basically
					AI_StartHugs(pred, prey);
				}
			}
		}
	}

	void AI_StartHugs(Actor* pred, Actor* prey) {
		auto& hugging = HugAnimationController::GetSingleton();
		auto& persist = Persistent::GetSingleton();
		if (!pred->IsInCombat() && persist.vore_combatonly) {
			return;
		}
		if (prey->formID != 0x14 && !IsHostile(pred, prey) && !IsTeammate(pred)) {
			return;
		}
		if (prey->formID == 0x14 && !persist.vore_allowplayervore) {
			return;
		}

		RecordSneakingState(pred, prey); // Needed to determine which hugs to play: sneak or crawl ones (when sneaking)
		
		HugShrink::GetSingleton().HugActor(pred, prey);

		AnimationManager::StartAnim("Huggies_Try", pred);

		if (pred->IsSneaking()) {
			AnimationManager::StartAnim("Huggies_Try_Victim_S", prey); // GTSBEH_HugAbsorbStart_Sneak_V
		} else {
			AnimationManager::StartAnim("Huggies_Try_Victim", prey); //   GTSBEH_HugAbsorbStart_V
		}
		AI_StartHugsTask(pred, prey);
	}

	void AI_StartHugsTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("Huggies_Forced_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		static Timer ActionTimer = Timer(2.5f);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			bool AllyHugged;
			bool IsDead = (tinyref->IsDead() || giantref->IsDead());
			tinyref->GetGraphVariableBool("GTS_IsFollower", AllyHugged);

			if (!HugShrink::GetHuggiesActor(giantref)) {
				if (!AllyHugged && tinyref->formID != 0x14) {
					PushActorAway(giantref, tinyref, 1.0f);
				}
				return false;
			}
			if (ActionTimer.ShouldRunFrame()) {
				int rng = RandomInt(0, 20);
				if (rng < 12) {
					if (!Runtime::HasPerkTeam(giantref, "HugCrush_LovingEmbrace")) {
						rng = 1; // always force crush and always shrink
					}	
					
					if (AI_CanHugCrush(giantref, tinyref, rng)) {
						AnimationManager::StartAnim("Huggies_HugCrush", giantref);
						AnimationManager::StartAnim("Huggies_HugCrush_Victim", tinyref);
					} else {
						log::info("HealOrShrink");
						AI_HealOrShrink(giant, tiny, rng);
					}
				}
			}
			if (IsDead) {
				return false;
			}
			return true;
		});
	}

	void AI_DoThighCrush(Actor* giant) {
		if (Persistent::GetSingleton().Thigh_Ai == false) {
			return;
		}
		std::vector<Actor*> tinies = ThighCrushController::GetSingleton().GetThighTargetsInFront(giant, 1);
		if (!tinies.empty()) {
			Actor* tiny = tinies[0];
			if (tiny) {
				ThighCrushController::GetSingleton().StartThighCrush(giant, tiny);
			}
		}
	}

	void AI_StartThighCrushTask(Actor* giant) {
		std::string name = std::format("ThighCrush_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		double Start = Time::WorldTimeElapsed();
		static Timer ActionTimer = Timer(6.0f);

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			Actor* giantref = gianthandle.get().get();
			double Finish = Time::WorldTimeElapsed();

			if (Finish - Start > 0.10) {
				if (!IsThighCrushing(giantref)) {
					return false;
				}

				if (ActionTimer.ShouldRunFrame()) {

					bool ForceAbort = GetAV(giantref, ActorValue::kStamina) <= 2.0f;
					DamageAV(giantref, ActorValue::kStamina, 0.025f);

					if (ForceAbort) {
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					}

					std::vector<Actor*> targets = ThighCrushController::GetSingleton().GetThighTargetsInFront(giantref, 1);
					if (targets.empty()) {
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					} else if (!targets.empty() && !ThighCrushController::GetSingleton().CanThighCrush(giantref, targets[0])) {
						AnimationManager::StartAnim("ThighLoopExit", giantref);
						return true;
					} else {
						AnimationManager::StartAnim("ThighLoopAttack", giantref);
						return true;
					}
					return true;
				}
				return true;
			}
			return true;
		});
	}
}
