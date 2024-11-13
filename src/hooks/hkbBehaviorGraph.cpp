#include "managers/animation/AnimationManager.hpp"
#include "hooks/hkbBehaviorGraph.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"


using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	float Animation_GetSpeedCorrection(Actor* actor) { // Fixes Hug animation de-sync by copying Gts anim speed to Tiny
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			if (transient->Hug_AnimSpeed < 1.0f) {
				return transient->Hug_AnimSpeed;
			}
			return AnimationManager::GetAnimSpeed(actor);
		} 
		return AnimationManager::GetAnimSpeed(actor);
	}

	void AffectByPerk(Actor* giant, float& anim_speed) {
		auto data = Transient::GetSingleton().GetActorData(giant);
		if (data) {
			float speed = data->Perk_BonusActionSpeed;
			if (speed > 1.0f) {
				bool CanApply = IsStomping(giant) || IsFootGrinding(giant) || IsVoring(giant) || IsTrampling(giant);
				if (CanApply) {
					anim_speed *= speed;
				}
			}
		}
	}
}

namespace Hooks
{
	void Hook_hkbBehaviorGraph::Hook() {
		log::info("Hooking hkbBehaviorGraph");
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_hkbBehaviorGraph[0] };

		_Update = vtbl.write_vfunc(0x05, Update);
	}

	void Hook_hkbBehaviorGraph::Update(hkbBehaviorGraph* a_this, const hkbContext& a_context, float a_timestep) {
		float anim_speed = 1.0f;
		for (auto actor: find_actors()) {
			BSAnimationGraphManagerPtr animGraphManager;
			if (actor->GetAnimationGraphManager(animGraphManager)) {
				for (auto& graph : animGraphManager->graphs) {
					if (graph) {
						if (a_this == graph->behaviorGraph) {
							float multi = Animation_GetSpeedCorrection(actor);
							AffectByPerk(actor, anim_speed);
							anim_speed *= multi;
						}
					}
				}
			}
		}
		_Update(a_this, a_context, a_timestep * anim_speed);
	}
}
