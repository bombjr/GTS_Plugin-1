#pragma once

namespace GTS {

	class AnimationUnderStomp {
		public:
			static bool ShouldStompUnder_NPC(Actor* giant, float distance);
            static bool ShouldStompUnder(Actor* giant);

			static void RegisterEvents();

			static void RegisterTriggers();


			static inline float Remap(float x, float in_min, float in_max, float out_min, float out_max) {
				return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
			}

	};
}