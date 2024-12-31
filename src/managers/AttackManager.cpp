#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/audio/PitchShifter.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/MaxSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/AttackManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "utils/MovementForce.hpp" 
#include "utils/DynamicScale.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "scale/modscale.hpp"
#include "data/transient.hpp"
#include "ActionSettings.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "utils/camera.hpp"
#include "utils/debug.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	bool IsHumanoid(Actor* giant) {
		bool human = Runtime::HasKeyword(giant, "ActorTypeNPC");
		return human;
	}

	void DisableAttacks_Melee(Actor* giant, float SizeDiff, float threshold, bool reset) {
		float random = RandomFloat(0.0f, 20.0f);

		if (!reset && SizeDiff >= 2.5f && random <= threshold) {
			giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kAttackingDisabled);
		} else {
			giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kAttackingDisabled);
		}
	}
	void DisableAttacks_Magic(Actor* giant, float SizeDiff, float threshold, bool reset) {
		float random = RandomFloat(0.0f, 20.0f);

		if (!reset && SizeDiff >= 2.5f && random <= threshold) {
			giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kCastingDisabled);
		} else {
			giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kCastingDisabled);
		}
	}

    void PreventAttacking() { // Old LOOP version of it
		static Timer CheckTimer = Timer(0.50);
        auto profiler = Profilers::Profile("Prevent Attacking ()");
		if (CheckTimer.ShouldRunFrame()) {
			for (auto giant: find_actors()) {
				if (giant) {
					if (giant->formID != 0x14 && (IsTeammate(giant) || EffectsForEveryone(giant))) {
						auto receiverRef = giant->GetActorRuntimeData().currentCombatTarget;
						if (receiverRef && IsHumanoid(giant)) {
							auto receiver = receiverRef.get().get();
							if (receiver) {
								log::info("Victim found: {}", receiver->GetDisplayFullName());
								float SizeDiff = GetSizeDifference(giant, receiver, SizeType::VisualScale, true, false);
								float random = RandomFloat(0.0f, 20.0f);
								float threshold = 2.5f * (SizeDiff - 2.5f);
								
								float distance = (giant->GetPosition() - receiver->GetPosition()).Length();
								distance /= get_visual_scale(giant);

								if (distance <= 50.0f) {
									DisableAttacks_Melee(giant, SizeDiff, threshold, false);
									DisableAttacks_Magic(giant, SizeDiff, threshold, false);
									//log::info("Blocking attack for {}", giant->GetDisplayFullName());
								} else {
									DisableAttacks_Melee(giant, 0.0f, 0.0f, true);
									DisableAttacks_Magic(giant, 0.0f, 0.0f, true);
									//log::info("Blocking attack for {} is reset", giant->GetDisplayFullName());
								}
							}
						}
					}
				}
			}
		}
	}
}

namespace Gts {

    AttackManager& AttackManager::GetSingleton() noexcept {
        static AttackManager instance;

        static std::atomic_bool initialized;
        static std::latch latch(1);
        if (!initialized.exchange(true)) {
            latch.count_down();
        }
        latch.wait();

        return instance;
    }

    std::string AttackManager::DebugName() {
        return "AttackManager";
    }

	void AttackManager::PreventAttacks(Actor* giant, Actor* tiny) {
		if (giant && giant->formID != 0x14 && IsHumanoid(giant)) {
			if (tiny) {
				float SizeDiff = GetSizeDifference(giant, tiny, SizeType::VisualScale, true, false);
				float threshold = 2.5f * (SizeDiff - 2.5f);
				DisableAttacks_Melee(giant, SizeDiff, threshold, false);
				DisableAttacks_Magic(giant, SizeDiff, threshold, false);
			} else { // If Tiny is nullptr, we count it as 'enable attacks back'
				DisableAttacks_Melee(giant, 0.0f, 0.0f, true);
				DisableAttacks_Magic(giant, 0.0f, 0.0f, true);
			}
		}
	}
}