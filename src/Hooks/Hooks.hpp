#pragma once

// Hooks into skyrim engine

#include "Hooks/Detours/FunctionHook.hpp"
#include "Hooks/Fudge/CallHook.hpp"
#include "Hooks/Fudge/BranchHook.hpp"

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

namespace Hooks {
	void InstallControls();
	void Install();
}
