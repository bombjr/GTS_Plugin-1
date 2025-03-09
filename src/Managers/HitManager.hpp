#pragma once

namespace GTS {

	class HitManager : public EventListener {
		public:
			[[nodiscard]] static HitManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			void HitEvent(const TESHitEvent* evt) override;
		private:
			bool CanGrow = false;
			bool Balance_CanShrink = false;
			bool BlockEffect = false;
			inline static float BonusPower = 1.0f;
			inline static float GrowthTick = 0.0f;
			inline static float AdjustValue = 1.0f;
	};
}
