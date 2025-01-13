#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/modscale.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;
using namespace SKSE;


namespace {

	static auto ptrOffset = REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_629) == std::strong_ordering::less ? -0xB8 : -0xC0;
	// Credits to ERSH

	void Headtracking_ManageSpineToggle(Actor* actor) {
		if (actor && actor->Is3DLoaded()) { // Player is handled inside HeadTracking.cpp -> SetGraphVariableBool hook
			std::string taskname = std::format("SpineBool_{}", actor->formID);
			ActorHandle giantHandle = actor->CreateRefHandle();

			double Start = Time::WorldTimeElapsed();

			TaskManager::RunFor(taskname, 1.0f, [=](auto& progressData){
				if (!giantHandle) {
					return false;
				}

				double Finish = Time::WorldTimeElapsed();

				double timepassed = Finish - Start;
				if (timepassed > 0.10) {
					auto giant = giantHandle.get().get();
					bool Disable = !(IsCrawling(giant) || IsProning(giant));
					giant->SetGraphVariableBool("bHeadTrackSpine", Disable);
					//log::info("Setting {} for {}", Disable, giant->GetDisplayFullName());
					return false;
				}
				return true;
			});
		}
	}

	void ForceLookAtCleavage(Actor* actor, NiPoint3& target) { // Forces someone to look at breasts
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

	bool HasHeadTrackingTarget(Actor* giant) {
		auto process = giant->GetActorRuntimeData().currentProcess;
		if (process) {
			auto high = process->high;
			if (high) {
				if (process->GetHeadtrackTarget()) {
					return true;
				} else {
					return false;
				}
			}
		}
		return false;
	}

	bool KnockedDown(Actor* giant) {
		return static_cast<int>(giant->AsActorState()->GetKnockState()) != 0; // Another way of checking ragdoll just in case
	}

	bool IsinRagdollState(Actor* giant) {
		bool ragdolled = IsRagdolled(giant) || KnockedDown(giant);
		return ragdolled;
	}

	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			if (giant->Is3DLoaded()) {
				if (HasHeadTrackingTarget(giant)) { // Apply it ONLY when targeting someone (when locking on Enemy with TDM for example)
					//|| giant->formID != 0x14 && !HasHeadTrackingTarget(giant)) { 
					// ^ needs to be enabled if experimenting with ForceLookAtCleavage() function, else they double-apply
					if (IsinRagdollState(giant) || IsDragon(giant)) {  // Dragons seem to behave funny if we edit them...sigh...
						// For some Bethesda™ reason - it breaks tiny ragdoll (their skeleton stretches :/) when they're small, so they fly into the sky.
						return original;      // We really want to prevent that, so we return original value in this case.
					}
					float fix = original * ((get_giantess_scale(giant)) / game_getactorscale(giant)); // game_getscale() is used here by the game, so we want to / it again

					return fix;
				}
				// ^ Compensate it, since SetScale() already affects HT by default
			}
		}
		return original;
	}

	void SetHeadtrackTargetImpl(Actor* actor, NiPoint3& target) {
		if (actor) {
			if (actor->Is3DLoaded()) {
				if (!HasHeadTrackingTarget(actor)) {// || actor->formID != 0x14) { // Alter it ONLY when target is nullptr or if Actor is not a player
					//                                    ^ Needs to be enabled if experimenting with ForceLookAtCleavage() so dll will allow pos override
					if (!IsinRagdollState(actor)) { // ^ Needed to fix TDM bugs with deforming Meshes of Actors when we lock onto someone
					
						// log::info("Actor: {}", actor->GetDisplayFullName());
						auto headPos = actor->GetLookingAtLocation();
						// log::info("headPos: {}", Vector2Str(headPos));
						auto model = actor->Get3D();
						if (model) {
							auto trans = model->world;
							auto transInv = trans.Invert();
							auto scale = get_visual_scale(actor) / game_getactorscale(actor);

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
}

namespace Hooks
{

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		static CallHook<float(TESObjectREFR* param_1)>Alter_Headtracking( 
			REL::RelocationID(37129, 37364), REL::Relocate(0x24, 0x5E),
			[](auto* param_1) {
				// Applied only when we TDM track someone
				// ----------------- SE:
				// FUN_140615030 : 37129
				// 0x140615054 - 0x140615030 = 0x24

				//------------------ AE:
				// FUN_1405ffc50: 37364
				// 0x1405ffcae - 0x1405ffc50 = 0x5E
  
				float result = Alter_Headtracking(param_1);
				float alter = affect_by_scale(param_1, result); 

				return alter;
            }
        );

		static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> 
			SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
				[](auto* a_this, auto* a_owner, auto& a_targetPosition) {
				// Applied in all other cases
				SetHeadtrackTargetImpl(a_owner, a_targetPosition);
				SetHeadtrackTarget(a_this, a_owner, a_targetPosition);
				return;
			}
		);

		static FunctionHook<bool(IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const bool a_in)> SetGraphVariableBool(
            REL::RelocationID(32141, 32885),
            [](IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const bool a_in){
                //log::info("SetGraph hooked");
                if (a_variableName == "bHeadTrackSpine") { // Disable weird spine rotation during crawl/prone
				    // Done through hook since TDM seems to adjust it constantly
                    auto actor = skyrim_cast<Actor*>(graph);
                    if (actor) {
                        //log::info("Holder found: {}", actor->GetDisplayFullName());
                        bool Disable = !(IsCrawling(actor) || IsProning(actor));
                        return SetGraphVariableBool(graph, a_variableName, Disable);
                    }
                }
            	return SetGraphVariableBool(graph, a_variableName, a_in);
            }
        );

		static CallHook<bool(RE::ActorState* a_this, int16_t a_flag)>Sneak_AddMovementFlags( // Mostly from DynamicCollisionAdjustment
			REL::RelocationID(36926, 37951), REL::Relocate(0xE4, 0xA0),
			[](RE::ActorState* a_this, int16_t a_flag) {
				auto actor = SKSE::stl::adjust_pointer<RE::Actor>(a_this, ptrOffset); // Some black magic from Ersh
				if (actor) {
					Headtracking_ManageSpineToggle(actor); // Toggle spine HT on/off based on GTS state
				}

				return Sneak_AddMovementFlags(a_this, a_flag);
            }
        );

		static CallHook<bool(RE::ActorState* a_this, int16_t a_flag)>Sneak_RemoveMovementFlags( // Mostly from DynamicCollisionAdjustment
			REL::RelocationID(36926, 37951), REL::Relocate(0xEB, 0xB2),
			[](RE::ActorState* a_this, int16_t a_flag) {
				auto actor = SKSE::stl::adjust_pointer<RE::Actor>(a_this, ptrOffset); // Some black magic from Ersh
				if (actor) {
					Headtracking_ManageSpineToggle(actor); // Toggle spine HT on/off based on GTS state
				}

				return Sneak_RemoveMovementFlags(a_this, a_flag);
            }
        );
	}


}

