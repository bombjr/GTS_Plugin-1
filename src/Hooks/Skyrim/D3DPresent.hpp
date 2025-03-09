#pragma once

namespace Hooks {

	class Hook_Renderer {
		public:
		static void Hook(Trampoline& trampoline);

		private:
		static WORD RegisterClassA(WNDCLASSA* a_wndClass);
		static inline REL::Relocation<decltype(RegisterClassA)> _RegisterClassA;

		static void CreateD3DAndSwapChain();
		static inline REL::Relocation<decltype(CreateD3DAndSwapChain)> _CreateD3DAndSwapChain;

		static LRESULT WndProcHandler(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam);
		static inline REL::Relocation<decltype(WndProcHandler)> _WndProcHandler;

		static void Present(uint32_t a1);
		static inline REL::Relocation<decltype(Present)> _Present;
	};

}
