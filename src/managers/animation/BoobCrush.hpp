#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	struct BoobCrushData {
		public:
			BoobCrushData(Actor* tiny);
			Actor* tiny;
	};
	class AnimationBoobCrush :  public EventListener {
		public:

			[[nodiscard]] static AnimationBoobCrush& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			static void RegisterEvents();

			static void AttachActor(Actor* giant, Actor* tiny);
			static Actor* GetBoobCrushVictim(Actor* giant);

			static float GetBoobCrushDamage(Actor* actor);

			std::unordered_map<Actor*, BoobCrushData> data;
	};
}
