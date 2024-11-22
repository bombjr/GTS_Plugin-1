#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class AnimationUnderStomp {
		public:
            static bool ShouldStompUnder(Actor* giant);

			static void RegisterEvents();

			static void RegisterTriggers();
	};
}