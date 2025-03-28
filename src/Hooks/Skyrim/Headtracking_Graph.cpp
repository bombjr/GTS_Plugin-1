#include "Hooks/Skyrim/HeadTracking_Graph.hpp"
#include "Config/Config.hpp"

using namespace GTS;
// This hooks handles behavior-graph that affects headtracking, such as adjusting TDM graph variables

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
					bool Disable = !(IsCrawling(giant) || IsProning(giant) || IsHandStomping_L(actor) || IsHandStomping_H(actor));
					giant->SetGraphVariableBool("bHeadTrackSpine", Disable);
					//log::info("Setting {} for {}", Disable, giant->GetDisplayFullName());
					return false;
				}
				return true;
			});
		}
	}
}


namespace Hooks {
    void Hook_HeadTrackingGraph::Hook(Trampoline& trampoline) {
        static FunctionHook<bool(IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const bool a_in)> SetGraphVariableBool(
            REL::RelocationID(32141, 32885),
            [](IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const bool a_in){
                //log::info("SetGraph hooked");
                if (a_variableName == "bHeadTrackSpine") { // Disable weird spine rotation during crawl/prone
				    // Done through hook since TDM seems to adjust it constantly
                    auto actor = skyrim_cast<Actor*>(graph);
                    if (actor) {
                        //log::info("Holder found: {}", actor->GetDisplayFullName());
						bool ShouldDisable = (IsCrawling(actor) || IsProning(actor) || IsHandStomping_L(actor) || IsHandStomping_H(actor));
                        if (ShouldDisable) {
							return SetGraphVariableBool(graph, a_variableName, false);
						}
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