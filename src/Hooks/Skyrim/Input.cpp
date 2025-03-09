#include "Hooks/Skyrim/Input.hpp"
#include "Managers/Input/InputManager.hpp"
#include "UI/UIManager.hpp"

using namespace GTS;

namespace Hooks {

	void Hook_Input::DispatchEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events) {

		if (!a_events) {
			_DispatchEvent(a_dispatcher, a_events);
			return;
		}

		if (UIManager::GetSingleton().InputUpdate(a_events)) {
			constexpr RE::InputEvent* const dummy[] = { nullptr };
            _DispatchEvent(a_dispatcher, const_cast<RE::InputEvent**>(dummy));
			return;
		}

		InputManager::GetSingleton().ProcessEvents(a_events);

		_DispatchEvent(a_dispatcher, a_events);

	}

	void Hook_Input::Hook(Trampoline& trampoline) {
		const REL::Relocation<uintptr_t> inputHook{ REL::VariantID(67315, 68617, 0xC519E0) };
		_DispatchEvent = trampoline.write_call<5>(inputHook.address() + REL::VariantOffset(0x7B, 0x7B, 0x81).offset(), DispatchEvent);
		logger::info("Hooked Input::DispatchEvent");
	}

}
