#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles Growth Spurt


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class GrowthSpurt : public Magic {
		public:
			virtual void OnUpdate() override;
			virtual void OnStart() override;
			virtual void OnFinish() override;


			virtual std::string GetName() override;

			GrowthSpurt(ActiveEffect* effect);

			virtual void DoGrowth(Actor* actor, float value);
			virtual void DoShrink(Actor* actor);
		private:
			float power = 0.0f;
			bool AllowStacking = true;
			float grow_limit = 1.0f;
			Timer timer = Timer(2.33f);
			Timer timerSound = Timer(0.7f);
	};
}
