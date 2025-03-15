#include "Audio.hpp"
#include "UI/DearImGui/imgui.h"
#include "UI/ImGui/ImUtil.hpp"

namespace GTS {

	void CategoryAudio::DrawLeft(){

	    ImUtil_Unique {

	        const char* T0 = "Enable footstep sounds when player size meets or exceeds a certain threshold.";
	        const char* T1 = "Enable moaning sounds during spells like Slow Growth.";

			const char* THelp = "Note: Moan/Laugh sounds are not included in the mod.\n"
			"You can add sounds by adding your own .wav files in the following folder:\n"
	    	"(Your Skyrim Folder)\\Data\\Sound\\fx\\GTS\\Others";
	        
	        if(ImGui::CollapsingHeader("Sounds",ImUtil::HeaderFlagsDefaultOpen)){
				ImGui::TextColored(ImUtil::ColorSubscript, "A Note On Sounds (?)");
				ImUtil::Tooltip(THelp ,true);

	            ImUtil::CheckBox("Footstep Sounds",&Settings.bFootstepSounds,T0);
	            ImUtil::CheckBox("Moans On Slow Growth",&Settings.bSlowGrowMoans, T1);
	            ImGui::Spacing();
	        }
	    }
	}

	void CategoryAudio::DrawRight() {

		ImUtil_Unique{

			const char* T0 = "Alter the voiceline pitch of NPCs when small/large.";
			const char* T1 = "Change the maximum voice pitch, Higher values will lower the pitch when the actor is large.\n"
							 "It's recommended to leave this at 1.0x. Anything above 1.2x doesn't sound good.";
			
			const char* T2 = "Enable/Disable actors making death sounds/screams when being absorbed by breasts.";
			const char* T3 = "Enable/Disable actors making death sounds/screams when killed through Wrathful Calamity.";
			const char* T4 = "Enable/Disable actors making death sounds/screams when killed through being Hug Crushed.";
			const char* T5 = "Enable/Disable actors making death sounds/screams when killed through vore.";
			
			if (ImGui::CollapsingHeader("Voice",ImUtil::HeaderFlagsDefaultOpen)) {
				ImUtil::CheckBox("Enable Voice Override",&Settings.bEnableVoiceOverride, T0);
				ImUtil::SliderF("Voice Pitch Max",&Settings.fMaxVoiceFrequency, 1.0f, 1.6f, T1, "%.2fx", !Settings.bEnableVoiceOverride);

				ImUtil::CheckBox("Breast Absorption: Mute Death Sound", &Settings.bMuteBreastAbsorptionDeathScreams,T2);
				ImUtil::CheckBox("Wrathful Calamity: Mute Death Sound", &Settings.bMuteFingerSnapDeathScreams,T3);
				ImUtil::CheckBox("Hug Crush: Mute Death Sound", &Settings.bMuteHugCrushDeathScreams,T4);
				ImUtil::CheckBox("Vore: Mute Death Sound", &Settings.bMuteVoreDeathScreams,T5);
				
				ImGui::Spacing();
			}
		}
	}
}