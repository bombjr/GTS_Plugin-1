#pragma once

namespace Hooks {

	class Hook_Input{
		public:
		static void Hook(Trampoline& trampoline);

		private:
		static void DispatchEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events);
		static inline REL::Relocation<decltype(DispatchEvent)> _DispatchEvent;
	};
}
