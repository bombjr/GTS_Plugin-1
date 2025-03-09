#pragma once 
#include "Config/Config.hpp"

namespace GTS {

	class AIManager : public EventListener {
		public:

		[[nodiscard]] static AIManager& GetSingleton() noexcept {
			static AIManager Instance;
			return Instance;
		}

		std::string DebugName() override {
			return "::AIManager";
		}

		Timer BeginNewActionTimer = Timer(3.0f);

		void Update() override;
		void TryStartAction(Actor* a_Performer) const;

		SettingsAI& AISettings = Config::GetAI();

	};
}

