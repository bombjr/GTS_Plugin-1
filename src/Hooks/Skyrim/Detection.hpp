#pragma once

namespace Hooks {

	class Hook_Detection {
		public:
		static void Hook(SKSE::Trampoline& trampoline);

		private:
		static std::uint8_t* Detection(RE::Actor* a_source, RE::Actor* a_target, std::int32_t& a_detectionValue, std::uint8_t& a_unk04, std::uint8_t& a_unk05, std::uint32_t& a_unk06, RE::NiPoint3& a_pos, float& a_unk08, float& a_unk09, float& a_unk10);
		static inline REL::Relocation<decltype(Detection)> _Detection;

	};
}