#pragma once
/*
 *  Hooks on various GetScale calls
 */

namespace Hooks {

	class Hook_Experiments{
		public:
			static void PatchShaking();
			static void Hook(Trampoline& trampoline);
	};
}
