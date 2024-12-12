#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Stomp_Under_Slam.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/audio/footstep.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void GTS_UnderStomp_Crawl_CamOnR(AnimationEventData& data) {
		DrainStamina(&data.giant,"StaminaDrain_HandSlam", "DestructionBasics", true, 1.5f);
		ManageCamera(&data.giant, true, CameraTracking::Hand_Right);
	}
    void GTS_UnderStomp_Crawl_CamOnL(AnimationEventData& data) {
		DrainStamina(&data.giant,"StaminaDrain_HandSlam", "DestructionBasics", true, 1.5f);
		ManageCamera(&data.giant, true, CameraTracking::Hand_Left);
	}
    
    void GTS_UnderStomp_Crawl_ImpactR(AnimationEventData& data) {
        DoCrawlingFunctions(&data.giant, get_visual_scale(&data.giant), 1.05f, Damage_Sneak_HandSlam_Sneak, CrawlEvent::RightHand, "RightHandRumble", 0.9f, Radius_Sneak_HandSlam, 1.3f, DamageSource::HandSlamRight);
        DrainStamina(&data.giant,"StaminaDrain_HandSlam", "DestructionBasics", false, 1.5f);
    }
    void GTS_UnderStomp_Crawl_ImpactL(AnimationEventData& data) {
        DoCrawlingFunctions(&data.giant, get_visual_scale(&data.giant), 1.05f, Damage_Sneak_HandSlam_Sneak, CrawlEvent::LeftHand, "RightHandRumble", 0.9f, Radius_Sneak_HandSlam, 1.3f, DamageSource::HandSlamLeft);
        DrainStamina(&data.giant,"StaminaDrain_HandSlam", "DestructionBasics", false, 1.5f);
    }

    void GTS_UnderStomp_Crawl_CamOffR(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::Hand_Right);
	}
    void GTS_UnderStomp_Crawl_CamOffL(AnimationEventData& data) {
		ManageCamera(&data.giant, false, CameraTracking::Hand_Left);
	}
}
namespace Gts {
    void AnimationUnderStompSlam::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_CamOnR", "UnderStompSlam", GTS_UnderStomp_Crawl_CamOnR);
        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_CamOnL", "UnderStompSlam", GTS_UnderStomp_Crawl_CamOnL);

        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_CamOffR", "UnderStompSlam", GTS_UnderStomp_Crawl_CamOffR);
        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_CamOffL", "UnderStompSlam", GTS_UnderStomp_Crawl_CamOffL);
        
        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_ImpactR", "UnderStompSlam", GTS_UnderStomp_Crawl_ImpactR);
        AnimationManager::RegisterEvent("GTS_UnderStomp_Crawl_ImpactL", "UnderStompSlam", GTS_UnderStomp_Crawl_ImpactL);
	}
}