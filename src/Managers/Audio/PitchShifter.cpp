#include "Managers/Audio/PitchShifter.hpp"
#include "Config/Config.hpp"

using namespace GTS;

namespace GTS {
	 void ShiftAudioFrequency() {
	 	auto enable = Config::GetAudio().bEnableVoiceOverride;
		if (!enable) {
			return;
		}
		for (auto tiny: find_actors()) {
			if (tiny) {
				if (tiny->formID != 0x14) {
					auto ai = tiny->GetActorRuntimeData().currentProcess;
					if (ai) {
						auto high = ai->high;
						if (high) {
							auto Audio_1 = high->soundHandles[0];
							auto Audio_2 = high->soundHandles[1];

							float natural_scale = std::clamp(get_natural_scale(tiny, false), 1.0f, 5000.0f); // Don't allow < 1.0
							float scale = get_raw_scale(tiny) * natural_scale * game_getactorscale(tiny);
							// ^ Some npc's have natural scale < than 1.0 (such as children) so it's a must to alter it by natural scale...

							float volume = std::clamp(scale + 0.5f, 0.35f, 1.0f);
							float size = (scale * 0.20f) + 0.8f;
							float frequence = (1.0f / size) / (1.0f * size);
							float freq_high = 1.0f / std::clamp(Config::GetAudio().fMaxVoiceFrequency, 1.0f, 10.0f);
							float freq_low = 1.5f;
							float freq = std::clamp(frequence, freq_high, freq_low);
							// < 1  = deep voice, below 0.5 = audio bugs out, not recommended
							// > 1 = mouse-like voice, not recommended to go above 1.5	

							if (Audio_1.soundID != BSSoundHandle::kInvalidID) {
								Audio_1.SetFrequency(freq);
								Audio_1.SetVolume(volume);
							}
							if (Audio_2.soundID != BSSoundHandle::kInvalidID) {
								Audio_2.SetFrequency(freq);
								Audio_2.SetVolume(volume);
							}
						}
					}
				}
			}
		}
	}
}
