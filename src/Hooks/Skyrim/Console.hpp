#pragma once

namespace GTS {

	class Hook_Console {

		public:

		static void Hook(Trampoline& trampoline);

		private:

		static void ConsoleSub(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef);
		static inline REL::Relocation<decltype(ConsoleSub)> ConsoleSub_1408daf02;

	};
}