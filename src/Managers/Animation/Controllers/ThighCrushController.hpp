#pragma once

namespace GTS {

	class ThighCrushController : public EventListener  {
		public:
			[[nodiscard]] static ThighCrushController& GetSingleton() noexcept;

			virtual std::string DebugName() override;
            
			std::vector<Actor*> GetThighTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartThighCrush(Actor* pred, Actor* prey);
			bool CanThighCrush(Actor* pred, Actor* prey);
	};
}