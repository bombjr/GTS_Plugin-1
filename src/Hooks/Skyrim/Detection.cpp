#include "Hooks/Skyrim/Detection.hpp"
#include "Managers/Animation/Grab.hpp"

namespace Hooks{
	std::uint8_t * Hook_Detection::Detection(RE::Actor * a_source, RE::Actor * a_target, std::int32_t & a_detectionValue, std::uint8_t & a_unk04, std::uint8_t & a_unk05, std::uint32_t & a_unk06, RE::NiPoint3 & a_pos, float& a_unk08, float& a_unk09, float& a_unk10) {

		std::ignore = Profilers::Profile("Hook: Detection");

		if (a_source) {

			if (IsHuman(a_source)) {
				if (Grab::GetHeldActor(a_source)) {
					a_detectionValue = -1000;
					return nullptr;
				}
			}
		}

		//Doesn't work
		//if (a_target) {
		//	//Scale the detection value based on the target's scale
		//	a_detectionValue = static_cast<int32_t>(static_cast<float>(a_detectionValue) * get_visual_scale(a_target));
		//}

		return _Detection(a_source, a_target, a_detectionValue, a_unk04, a_unk05, a_unk06, a_pos, a_unk08, a_unk09, a_unk10);
	}

	void Hook_Detection::Hook(Trampoline & trampoline) {
		//Credits to Po3 for the hook
		const REL::Relocation<uintptr_t> detectionhook{ RELOCATION_ID(41659, 42742)};

		_Detection = trampoline.write_call<5>(detectionhook.address() + REL::VariantOffset(0x526, 0x67B, 0x0).offset(), Detection);
		logger::info("Hooked Detection");

	}
}
