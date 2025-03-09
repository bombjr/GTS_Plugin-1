#pragma once
/*
 *  Hooks on various GetScale calls
 */

namespace Hooks {

	class Hook_RaceMenu {
		public:
			static void Hook(Trampoline& trampoline);
	};
}