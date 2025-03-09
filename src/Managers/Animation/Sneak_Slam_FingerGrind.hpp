#pragma once

namespace GTS {

	class Animation_SneakSlam_FingerGrind {
		public:
			static void RegisterEvents();
			static void RegisterTriggers();
	};
	
    void TrackMatchingHand(Actor* giant, CrawlEvent kind, bool enable);
    void StopStaminaDrain(Actor* giant);
}