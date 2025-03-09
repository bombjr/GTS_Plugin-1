#include "Hooks/Skyrim/Pushback.hpp"

using namespace GTS;

namespace {

    float GetPushMult(Actor* giant) {
		float result = 1.0f;
		if (giant->formID == 0x14 || IsTeammate(giant)) {
			auto tranData = Transient::GetSingleton().GetData(giant);
			if (tranData) {
				result = tranData->PushForce;
			} else {
				float size = get_giantess_scale(giant);
				if (HasSMT(giant)) {
					size *= 2.5f;
				}
				result = std::clamp(1.0f / (size*size*size*size), 0.01f, 1.0f);
			}

			if (result <= 0.025f) {
				return 0.0f;
			}
		}

		return result;
	}
}

namespace Hooks {


	void Hook_PushBack::Hook(Trampoline& trampoline) {

		// Reduces amount of CharController being pushed back on being hit based on size

        static FunctionHook<void(bhkCharacterController* controller, hkVector4& a_from, float time)>HavokPushHook (      
			REL::RelocationID(76442, 78282), 
			[](bhkCharacterController* controller, hkVector4& a_from, float time) { // SE: DC0930
				// TO-DO: Somwehow improve performance instead of looping through all actors
                auto profiler = Profilers::Profile("H: HavokPush");
				Actor* giant = GetCharContActor(controller);
				float scale = 1.0f;
				if (giant) {
					scale = GetPushMult(giant);
				}
				hkVector4 Push = hkVector4(a_from) * scale;
				// Size difference is recorded only outside of TGM!
				// In TGM effect isn't applied because of that
				
				return HavokPushHook(controller, Push, time); 
            }
        );

		/*static FunctionHook<void(AIProcess *ai, Actor* actor, NiPoint3& direction, float force)>PushActorAwayHook (      
			REL::RelocationID(38858, 39895), 
			[](AIProcess *ai, Actor* actor, NiPoint3& direction, float force) { // Use it to cache size difference between 2 actors
				
				//Cache size difference and then use it inside hook above
				return PushActorAwayHook(ai, actor, direction, force); 
            }
        );*/
    }
}