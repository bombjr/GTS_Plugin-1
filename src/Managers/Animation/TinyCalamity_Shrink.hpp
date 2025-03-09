#pragma once

namespace GTS {

	class Animation_TinyCalamity : public EventListener {
		public:

            [[nodiscard]] static Animation_TinyCalamity& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void RegisterEvents();
            static void RegisterTriggers();

			virtual void ResetActors(Actor* actor);

            static void AddToData(Actor* giant, Actor* tiny, float until);
            static std::vector<Actor*> GetShrinkActors(Actor* giant);
            static float GetShrinkUntil(Actor* giant);

    };
}

