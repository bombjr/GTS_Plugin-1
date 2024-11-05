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
	 * The class which manages some perk bonuses
	 */
	class PerkHandler : public EventListener  {
		public:
            [[nodiscard]] static PerkHandler& GetSingleton() noexcept;
            virtual std::string DebugName() override;

			virtual void OnAddPerk(const AddPerkEvent& evt) override;
            virtual void OnRemovePerk(const RemovePerkEvent& evt) override;
			static void UpdatePerkValues(Actor* giant, PerkUpdate Type);
	};
}