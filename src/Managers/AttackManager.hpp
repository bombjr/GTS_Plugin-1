#pragma once

namespace GTS {

	class AttackManager : public EventListener  {
		public:

			[[nodiscard]] static AttackManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void PreventAttacks(Actor* a_Giant, Actor* a_Tiny);
	};
}