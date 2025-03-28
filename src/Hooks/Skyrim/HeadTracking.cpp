#include "Hooks/Skyrim/HeadTracking.hpp"
#include "Config/Config.hpp"

using namespace GTS;

namespace {
	void ForceLookAtCleavage(Actor* actor, NiPoint3& target) { // Forces someone to look at breasts
		// Experimental function just for fun, it even works if enabled
		if (actor->formID != 0x14) {
			auto process = actor->GetActorRuntimeData().currentProcess;
			if (process) {
				auto high = process->high;
				if (high) {
					auto target_actor = process->GetHeadtrackTarget();
					if (target_actor) {
						auto true_target_ref = target_actor.get().get();
						if (true_target_ref && true_target_ref->formID == 0x14) {
							
							auto true_target = skyrim_cast<Actor*>(true_target_ref);
							if (true_target) {
								NiPoint3 dist_A = actor->GetPosition();
								NiPoint3 dist_B = true_target->GetPosition();
								float distance = (dist_A - dist_B).Length();
								if (distance <= 256 * get_visual_scale(true_target)) {
									auto breast_1 = find_node(true_target, "R Breast02");
									auto breast_2 = find_node(true_target, "L Breast02");
									if (breast_1 && breast_2) {
										NiPoint3 breast_pos = (breast_1->world.translate + breast_2->world.translate) / 2;
										target = breast_pos;
									}
								}
							}
						}
					} 
				}
			}
		}
	}

	float GetRacemenuScale(Actor* giant) { // Used only on NPC's since we don't want to apply Natural Scale to them
		auto actor_data = Transient::GetSingleton().GetData(giant);
		if (actor_data) {
			return actor_data->OtherScales;
		}
		return 1.0f;
	}

	float Headtracking_CalculateNewHT(Actor* giant, const float original) {
		bool Player = giant->formID == 0x14;

		if (!Player) { 	// NPC
			return original * get_raw_scale(giant) * GetRacemenuScale(giant); // Old Pre 3.0.0 method 
			// No need to bother with Race (not Racemenu!) Scale and SetScale: game already does it by default in NPC case
		} else {		// Player
			return get_visual_scale(giant);
		}
		return 1.0f;
	}

	float ScaleTargetedHeadtracking(TESObjectREFR* ref, const float original) { // Player exclusive headtracking
		// We could apply it to NPC's as well, but for some reason it introduces funny leg bug: leg slightly twitches when it's on
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			if (giant->Is3DLoaded()) {
				if (giant->formID == 0x14) {
					const bool ApplyScaling = !(IsinRagdollState(giant) || IsDragon(giant));
					// If in ragdoll / is dragon = do nothing, else bones will stretch and npc/player will cosplay slenderman
					if (ApplyScaling) {
						float fix = Headtracking_CalculateNewHT(giant, original);
						return fix; // Apply scale
					}
				}
			}
		}
		// If it fails: either actor not loaded or in ragdoll / is dragon, just return original value in that case
		return original;
	}

	void UpdateHeadtrackingPOS_Impl(Actor* actor, NiPoint3& target) { // NPC Exclusive
		if (actor) {
			if (actor->Is3DLoaded()) {
				if (actor->formID != 0x14) {
					const bool ApplyScaling = !(IsinRagdollState(actor) || IsDragon(actor)); 
					if (ApplyScaling) {
						auto headPos = actor->GetLookingAtLocation();
						auto model = actor->Get3D();
						if (model) {
							auto trans = model->world;
							auto transInv = trans.Invert();
							auto scale = Headtracking_CalculateNewHT(actor, 1.0f);

							auto unscaledHeadPos = trans * (transInv*headPos * (1.0f/scale));

							//ForceLookAtCleavage(actor, target); // If enabled, need to make sure that only one hook is affecting NPC's 

							auto direction = target - headPos;
							target = unscaledHeadPos + direction;
						}
					}
				}
			}
		}
	}

	void PrintDebugInformation(Actor* giant, float fix, float original) {
		static Timer PrintTimer = Timer(2.0);
		if (PrintTimer.ShouldRunFrame()) {
			log::info("==============={}===================", giant->GetDisplayFullName());
			log::info("Value is {}, original: {}", fix, original);
			log::info("Visual Scale: {}, raw_scale: {}", get_visual_scale(giant), get_raw_scale(giant));
			log::info("Natural + GameScale: {}, raw Natural Scale: {}", get_natural_scale(giant, true), get_natural_scale(giant, false));
			log::info("Game Scale: {}, npc node scale: {}", game_getactorscale(giant), get_npcnode_scale(giant));

			auto actor_data = Transient::GetSingleton().GetData(giant);
			if (actor_data) {
				log::info("Initial Scale: {}, Other Scale: {}", actor_data->OtherScales, GetInitialScale(giant));
				log::info("Racemenu Scale: {}", GetRacemenuScale(giant));
			}
			log::info("====================================");
		}
	}
}

namespace Hooks {

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		static CallHook<float(TESObjectREFR* param_1)>AlterHeadtracking_Player( 
			REL::RelocationID(37129, 37364), REL::Relocate(0x24, 0x5E),
			[](auto* param_1) {
				// Applied to player only because it adds minor bug to NPC's: foot funnily twitches
				// ----------------- SE:
				// FUN_140615030 : 37129
				// 0x140615054 - 0x140615030 = 0x24

				//------------------ AE:
				// FUN_1405ffc50: 37364
				// 0x1405ffcae - 0x1405ffc50 = 0x5E
  
				float result = AlterHeadtracking_Player(param_1);
				float alter = ScaleTargetedHeadtracking(param_1, result); 

				return alter;
            }
        );

		static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> 
			AlterHeadtracking_NPC(RELOCATION_ID(38850, 39887),
				[](auto* a_this, auto* a_owner, auto& a_targetPosition) {
				// Applied to NPC's only
				UpdateHeadtrackingPOS_Impl(a_owner, a_targetPosition);
				AlterHeadtracking_NPC(a_this, a_owner, a_targetPosition);
				return;
			}
		);
	}
}