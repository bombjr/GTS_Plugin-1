#include "UI/Categories/Widgets.hpp"
#include "UI/ImGUI/ImUtil.hpp"
#include "UI/Windows/GTSInfo.hpp"
#include "UI/Windows/WindowStatus.hpp"

namespace GTS {

	void CategoryWidgets::DrawLeft() {

		// ------- Player Stats Widget

		ImUtil_Unique{

			const char* T0 = "Toggle visibility of extra information for the player character.";
			const char* T1 = "Adjust the opacity of the stats window.";

			const char* T2 = "Automatically handle positioning and size for the stats window.\n"
							 "Disabling this allows you to move and resize the settings window manually.\n"
							 "Otherwise, if left enabled you can adjust the position offsets below.";

			const char* T3 = "Choose the alignment point for the window on screen";
			const char* T4 = "Adjust horizontal offset from the selected anchor point.";
			const char* T5 = "Adjust vertical offset from the selected anchor point.";
			const char* T6 = "Adjust the opacity of the stats window's backround.";
			const char* T7 = "Toggle whether the stats widget fades if the players scale or max scale don't change for a while.";
			const char* T8 = "Change how long the widget menu should be shown for after a value has changed.";
			const char* T9 = "Set the width of the stats window. Works only if auto size is disabled.";
			const char* T10 = "Set the height multiplier of the size bar.";
			const char* T11 = "Set the required change (ie Delta) in size for the window to reappear if inactivity fade is enabled.";

			if (ImGui::CollapsingHeader("Status Widget",ImUtil::HeaderFlagsDefaultOpen)) {
				ImUtil::CheckBox("Show Player Stats Widget", &Settings.StatusWindow.bVisible, T0);

				ImGui::Spacing();

				ImUtil::CheckBox("Inactivity Fade", &Settings.StatusWindow.bEnableFade,T7);
				ImUtil::SliderF("Fade After", &Settings.StatusWindow.fFadeAfter, 0.5f, 10.0f, T8, "After %.1f Seconds", !Settings.StatusWindow.bEnableFade);
				ImUtil::SliderF("Reappear Delta", &Settings.StatusWindow.fFadeDelta, 0.0, 0.5f, T11, "After a %.2fx Difference", !Settings.StatusWindow.bEnableFade);

				ImGui::Spacing();

				ImUtil::SliderF("Widget Alpha", &Settings.StatusWindow.fAlpha, 0.1f, 1.0f, T1,"%.1fx");
				ImUtil::SliderF("Widget BG Alpha", &Settings.StatusWindow.fBGAlphaMult, 0.0f, 1.0f, T6, "%.1fx");

				ImGui::Spacing();

				ImUtil::SliderF("Widget Fixed Width", &Settings.StatusWindow.fFixedWidth, 120.0f, 600.0f, T9, "%.0f");
				ImUtil::SliderF("Bar Height Mult", &Settings.StatusWindow.fSizeBarHeightMult, 0.1f, 2.5f, T10, "%.2fx");

				ImGui::Spacing();

				ImUtil::CheckBox("Lock Widget Position", &Settings.StatusWindow.bLock, T2);

				ImGui::BeginDisabled(!Settings.StatusWindow.bLock);
				{
					ImUtil::ComboEx<ImWindow::WindowAnchor>("Anchor", Settings.StatusWindow.sAnchor, T3);
					ImGui::BeginDisabled(Settings.StatusWindow.sAnchor == "kCenter");
					{
						ImUtil::SliderF("Offset (Left/Right)", &Settings.StatusWindow.f2Offset.at(0), 0.0f, 1280.0f, T4, "%.1f%");
						ImUtil::SliderF("Offset (Up/Down)", &Settings.StatusWindow.f2Offset.at(1), 0.0f, 720.0f, T5,"%.1f%");
					}
					ImGui::EndDisabled();
				}
				ImGui::Spacing();

				ImGui::EndDisabled();
			}

			if (ImGui::CollapsingHeader("Status Widget Content Toggles", ImUtil::HeaderFlagsDefaultOpen)) {
				ImUtil::Bitfield<GTSInfoFeatures>(&Settings.StatusWindow.iFlags);
			}
		}
	}

	void CategoryWidgets::DrawRight() {

		// ------ UnderStomp Angle Window

		ImUtil_Unique{

			const char* T0 = "Toggle the visibility of the understomp angle window.";
			const char* T1 = "Adjust the opacity of the window.";

			const char* T2 = "Automatically handle positioning and size for the the understomp angle window.\n"
								"Disabling this allows you to move and resize the settings window manually.\n"
								"Otherwise, if left enabled you can adjust the position offsets below.";

			const char* T3 = "Choose the alignment point for the window on screen";
			const char* T4 = "Adjust horizontal offset from the selected anchor point.";
			const char* T5 = "Adjust vertical offset from the selected anchor point.";
			const char* T6 = "Adjust the opacity of the stats window's backround.";
			const char* T7 = "Toggle whether the the understomp angle widget fades if the players scale or max scale don't change for a while.";
			const char* T8 = "Change how long the widget menu should be shown for after a value has changed.";
			const char* T9 = "Set the width of the the understomp angle window. Works only if auto size is disabled.";
			const char* T10 = "Set the height multiplier of the size bar.";
			const char* T11 = "Set the required change (ie Delta) in size for the window to reappear if inactivity fade is enabled.";

			if (ImGui::CollapsingHeader("Understomp Status Widget",ImUtil::HeaderFlagsDefaultOpen)) {
				ImUtil::CheckBox("Show Understomp Status Widget", &Settings.UnderstompWindow.bVisible, T0);

				ImGui::Spacing();

				ImUtil::CheckBox("Inactivity Fade", &Settings.UnderstompWindow.bEnableFade,T7);
				ImUtil::SliderF("Fade After", &Settings.UnderstompWindow.fFadeAfter, 0.5f, 10.0f, T8, "After %.1f Seconds", !Settings.UnderstompWindow.bEnableFade);
				ImUtil::SliderF("Reappear Delta", &Settings.UnderstompWindow.fFadeDelta, 0.0, 0.5f, T11, "After a %.2fx Difference", !Settings.UnderstompWindow.bEnableFade);

				ImGui::Spacing();

				ImUtil::SliderF("Widget Alpha", &Settings.UnderstompWindow.fAlpha, 0.1f, 1.0f, T1,"%.1fx");
				ImUtil::SliderF("Widget BG Alpha", &Settings.UnderstompWindow.fBGAlphaMult, 0.0f, 1.0f, T6, "%.1fx");

				ImGui::Spacing();

				ImUtil::SliderF("Widget Fixed Width", &Settings.UnderstompWindow.fFixedWidth, 120.0f, 600.0f, T9, "%.0f");
				ImUtil::SliderF("Bar Height Mult", &Settings.UnderstompWindow.fSizeBarHeightMult, 0.1f, 2.5f, T10, "%.2fx");

				ImGui::Spacing();

				ImUtil::CheckBox("Lock Widget Position", &Settings.UnderstompWindow.bLock, T2);
				ImGui::BeginDisabled(!Settings.UnderstompWindow.bLock);
				{
					ImUtil::ComboEx<ImWindow::WindowAnchor>("Anchor", Settings.UnderstompWindow.sAnchor, T3);
					ImGui::BeginDisabled(Settings.UnderstompWindow.sAnchor == "kCenter");
					{
						ImUtil::SliderF("Offset (Left/Right)", &Settings.UnderstompWindow.f2Offset.at(0), 0.0f, 1280.0f, T4, "%.1f%");
						ImUtil::SliderF("Offset (Up/Down)", &Settings.UnderstompWindow.f2Offset.at(1), 0.0f, 720.0f, T5,"%.1f%");
					}
					ImGui::EndDisabled();
				}
				ImGui::EndDisabled();
			}
		}
	}
}