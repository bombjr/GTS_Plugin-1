#pragma once

namespace GTS {

	class HugAnimationController : public EventListener  {
		public:
			[[nodiscard]] static HugAnimationController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void Hugs_OnCooldownMessage(Actor* giant);
			std::vector<Actor*> GetHugTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			bool CanHug(Actor* pred, Actor* prey);
			static void StartHug(Actor* pred, Actor* prey);

			void AllowMessage(bool allow);
		private:
			bool allow_message = false;	
	};
}