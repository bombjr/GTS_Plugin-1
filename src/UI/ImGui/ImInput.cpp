#include "UI/ImGui/ImInput.hpp"

#include "UI/UIManager.hpp"
#include "UI/ImGui/ImWindowManager.hpp"

namespace GTS {

	void ImInput::ProcessInputEventQueue() {

		std::unique_lock<std::shared_mutex> mutex(InputMutex);

		ImGuiIO& io = ImGui::GetIO();

		for (auto& event : KeyEventQueue) {

			if (event.eventType == RE::INPUT_EVENT_TYPE::kChar) {
				io.AddInputCharacter(event.keyCode);
				continue;
			}

			if (event.device == RE::INPUT_DEVICE::kMouse) {
				
				if (event.keyCode > 7) {  // middle scroll
					io.AddMouseWheelEvent(0, event.value * (event.keyCode == 8 ? 1 : -1));
				}
				else {
					event.keyCode = std::min<uint32_t>(event.keyCode, 5);
					io.AddMouseButtonEvent(event.keyCode, event.IsPressed());
				}
			}

			if (event.device == RE::INPUT_DEVICE::kKeyboard) {

				uint32_t key = DIKToVK(event.keyCode);

				if (key == event.keyCode) {
					key = MapVirtualKeyEx(event.keyCode, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0));
				}

				if (!event.IsPressed()) {

					if (auto MainWindow = ImWindowManager::GetSingleton().GetWindowByName("Settings")) {
						if (key == VK_ESCAPE && MainWindow->Show && !MainWindow->Busy) {
							UIManager::CloseSettings();
							continue;
						}
					}
				}

				io.AddKeyEvent(VirtualKeyToImGuiKey(key), event.IsPressed());

				if (key == VK_LCONTROL || key == VK_RCONTROL) {
					io.AddKeyEvent(ImGuiMod_Ctrl, event.IsPressed());
				}
				else if (key == VK_LSHIFT || key == VK_RSHIFT) {
					io.AddKeyEvent(ImGuiMod_Shift, event.IsPressed());
				}
				else if (key == VK_LMENU || key == VK_RMENU) {
					io.AddKeyEvent(ImGuiMod_Alt, event.IsPressed());
				}

				//Enable Advanced Settings
				if (io.KeyAlt && io.KeyShift && io.KeyCtrl && key == VK_F12) {

					if (auto MainWindow = ImWindowManager::GetSingleton().GetWindowByName("Settings")) {
						if (MainWindow->Show && !MainWindow->Busy) {
							Config::GetHidden().IKnowWhatImDoing = true;
						}
					}
				}
			}
		}
		KeyEventQueue.clear();
	}

	void ImInput::ReleaseStuckKeys() {

		BYTE keysToRelease[] = { VK_MENU, VK_TAB, VK_CONTROL, VK_SHIFT };

		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;
		input.ki.time = 0;
		input.ki.dwExtraInfo = 0;

		for (unsigned char i : keysToRelease) {
			input.ki.wVk = i;
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
		}
	}

}
