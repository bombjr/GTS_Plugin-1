#include "Hooks/Skyrim/Console.hpp"

#include "Managers/Console/ConsoleManager.hpp"

namespace GTS {


	void Hook_Console::Hook(Trampoline& trampoline) {

		//SE Offset: 1408daf02
		REL::Relocation<std::uintptr_t> hook(RELOCATION_ID(52065, 52952), REL::VariantOffset(0xE2, 0x52, 0xE2).offset());
		ConsoleSub_1408daf02 = trampoline.write_call<5>(hook.address(), ConsoleSub);
		log::info("Hooked ConsoleSub_1408daf02");

	}

	void Hook_Console::ConsoleSub(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef) {

		const std::string Input = a_script->text;
		logger::info("Entered Console Text: \"{}\"", Input);

		//If true command was handled by the plugin
		if (ConsoleManager::Process(Input)) {
			return;
		}

		ConsoleSub_1408daf02(a_script, a_compiler, a_name, a_targetRef);
	}

}
