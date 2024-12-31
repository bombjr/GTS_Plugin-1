#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace RE;

namespace Gts {
	/**
	 * The class which tracks gts size effects.
	 */
	class AttackManager : public EventListener  {
		public:

			/**
			 * Get the singleton instance of the <code>AttackManager</code>.
			 */
			[[nodiscard]] static AttackManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void PreventAttacks(Actor* giant, Actor* tiny);
	};
}