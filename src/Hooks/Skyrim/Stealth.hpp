#pragma once
/*
 *  Hooks on various Stealth calls
 */

#include "Hooks/Hooks.hpp"




namespace Hooks
{
	class Hook_Stealth
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}