#pragma once

// Module that handles AttributeValues

namespace GTS {

	class RandomGrowth : public EventListener {
		public:
			[[nodiscard]] static RandomGrowth& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			static void RestoreStats(Actor* actor, float multiplier);
	};
}
