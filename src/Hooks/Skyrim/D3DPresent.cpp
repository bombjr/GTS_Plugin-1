#include "Hooks/Skyrim/D3DPresent.hpp"
#include "UI/UIManager.hpp"

namespace Hooks {

	//Win32 Window Messages
	LRESULT Hook_Renderer::WndProcHandler(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {
		if (a_msg == WM_KILLFOCUS) {

			auto& UIMgr = UIManager::GetSingleton();

			if (UIMgr.Ready()) {

				UIMgr.OnFocusLost();

				//IO can only be retrieved if a valid imguicontext exists.
				auto& io = ImGui::GetIO();

				io.ClearInputCharacters();
				io.ClearInputKeys();

			}
		}
		return _WndProcHandler(a_hwnd, a_msg, a_wParam, a_lParam);
	}

	//DXGI Swapchain
	void Hook_Renderer::CreateD3DAndSwapChain() {
		_CreateD3DAndSwapChain();
		UIManager::GetSingleton().Init();
	}

	//Register Window Class
	WORD Hook_Renderer::RegisterClassA(WNDCLASSA* a_wndClass) {
		_WndProcHandler = reinterpret_cast<uintptr_t>(a_wndClass->lpfnWndProc);
		a_wndClass->lpfnWndProc = &WndProcHandler;

		return _RegisterClassA(a_wndClass);
	}

	//DXGISwapchain::Present
	void Hook_Renderer::Present(uint32_t a1) {
		_Present(a1);

		//Hack: Without this Settings draws over the hud and while its fine for 99% of cases sometimes something will draw over the hud and thus over the settings.
		//I don't know how to register a "HUD" element that could draw over all other hud elements so we'll do it this way.
		//Does checking for an atomic bool hurt perf. i honestly don't know.
		//The proper way to do this would be to have 2 imgui contexts for each "layer" but managing 2 of them needs a semi rewrite of all the custom helper functions
		//And thus is an absolute pain in the ass.
		if (UIManager::ShouldDrawOverTop) {
			UIManager::GetSingleton().Update();
		}
	}

	//HudMenu Present
	struct Hook_HUDMenu_Present {

		static void thunk (RE::IMenu* a_menu) {
			func(a_menu);

			if (!UIManager::ShouldDrawOverTop) {
				UIManager::GetSingleton().Update();
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;

		static void Install() {
			REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_HUDMenu[0] };
			func = vtbl.write_vfunc(0x06, thunk);
			logger::info("Hooked IMenu HUDMenu[0] Vtable");
		}

	};

	void Hook_Renderer::Hook(Trampoline& trampoline) {

		//Credits to ershin for the hooks here
		const REL::Relocation<uintptr_t> registerWindowHook{ REL::VariantID(75591, 77226, 0xDC4B90) };  // D71F00, DA3850, DC4B90
		const REL::Relocation<uintptr_t> created3d11Hook{ REL::VariantID(75595, 77226, 0xDC5530) };     // D72810, DA3850, DC5530
		const REL::Relocation<uintptr_t> presentHook{ REL::VariantID(75461, 77246, 0xDBBDD0) };         // D6A2B0, DA5BE0, DBBDD0

		_RegisterClassA = *reinterpret_cast<uintptr_t*>(trampoline.write_call<6>(registerWindowHook.address() + REL::VariantOffset(0x8E, 0x15C, 0x99).offset(), RegisterClassA));
		logger::info("Hooked RegisterClassA");

		_CreateD3DAndSwapChain = trampoline.write_call<5>(created3d11Hook.address() + REL::VariantOffset(0x9, 0x275, 0x9).offset(), CreateD3DAndSwapChain);
		logger::info("Hooked CreateD3DAndSwapChain");

		_Present = trampoline.write_call<5>(presentHook.address() + REL::VariantOffset(0x9, 0x9, 0x15).offset(), Present);
		logger::info("Hooked DXGISwapchain Present");

		//Updating earlier in the render pass allows us to auto hide everything when
		//skyrim is doing its fade to black
		Hook_HUDMenu_Present::Install();

	}

}
