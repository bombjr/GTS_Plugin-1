#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ClothManager : public EventListener {
		public:
			[[nodiscard]] static ClothManager& GetSingleton() noexcept;
			virtual std::string DebugName() override;

			void CheckClothingRip(Actor* a_actor);
			bool ShouldPreventReEquip(Actor* a_actor, RE::TESBoundObject* a_object);

			const float rip_threshold = 1.5;                //Threshold Singular Clothes get unequiped
			const float rip_toobig = 2.5;                       //Threshold All Clothes get unequiped

	};
}
