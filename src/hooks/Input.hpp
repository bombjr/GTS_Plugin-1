#pragma once

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {
	class Hook_Input{
		public:
		static void Hook(Trampoline& trampoline);

		private:
		static void DispatchEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events);
		static inline REL::Relocation<decltype(DispatchEvent)> _DispatchEvent;
	};
}
