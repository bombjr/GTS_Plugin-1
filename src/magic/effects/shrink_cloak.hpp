#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShrinkCloak : public Magic {
		public:
			virtual void OnStart() override;

			virtual void OnUpdate() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

			ShrinkCloak(ActiveEffect* effect);

		private:
			float power = 0.0f;
			float time_mult = 3.0f;
	};
}