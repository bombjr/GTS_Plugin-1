#pragma once

#include "API/Impl/SKEEBodyMorphInterface.hpp"

namespace GTS {

	class Racemenu final {

		public:
		static void Register();
		static void SetMorph(RE::Actor* a_actor, const char* a_morphName, float a_value, bool a_immediate);
		static float GetMorph(RE::Actor* a_actor, const char* a_morphName);
		static void ClearAllMorphs(RE::Actor* a_actor);
		static void ClearMorphs(RE::Actor* a_actor);
		static void ClearMorph(RE::Actor* a_actor, const char* a_morphName);
		static void ApplyMorphs(RE::Actor* a_actor);

		[[nodiscard]] static inline bool Loaded() {
			return RaceMenuInterface != nullptr;
		}

		private:

		static inline SKEE::IBodyMorphInterface* RaceMenuInterface = nullptr;
		static constexpr std::string MorphKey = "GTSPlugin";

	};
}