#pragma once

namespace GTS {

	class GrabAnimationController : public EventListener  {
		public:
			[[nodiscard]] static GrabAnimationController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			std::vector<Actor*> GetGrabTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartGrab(Actor* pred, Actor* prey);
			bool CanGrab(Actor* pred, Actor* prey) const;

			void AllowMessage(bool allow);
		private:
			bool allow_message = false;
	};
}