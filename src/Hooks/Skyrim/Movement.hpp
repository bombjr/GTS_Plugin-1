#pragma once
/*
	Hooks on various GetScale calls
 */

namespace Hooks {
	class Hook_Movement {
		public:
			static void Hook(Trampoline& trampoline);
	};
}