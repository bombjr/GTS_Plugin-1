#pragma once

// Module that handles changing spectator targets

namespace GTS {

	class SpectatorManager : public EventListener {
		public:

		[[nodiscard]] static SpectatorManager& GetSingleton() noexcept {
			static SpectatorManager Instance;
			return Instance;
		}

		std::string DebugName() override {
			return "::SpectatorManager";
		}

		virtual void DataReady() override {

		}

		static void SetCameraTarget(RE::Actor* a_Actor, bool aTemporary);
		virtual void Update() override;
		static bool IsCameraTargetPlayer();
		static void ResetTarget(bool aDoFullReset);
		virtual void Reset() override;
		virtual void ResetActor(Actor* actor) override;

		private:
		static inline FormID Target = 0x14;
		static inline volatile bool TrackedActorLost = false;

	};
}
