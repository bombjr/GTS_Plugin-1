#include "Managers/Audio/Stomps.hpp"

#include "Config/Config.hpp"

#include "Managers/Audio/AudioObtainer.hpp"
#include "Managers/Audio/AudioParams.hpp"
#include "Managers/Highheel.hpp"

using namespace GTS;

namespace GTS_FindAudio {
	BSISoundDescriptor* GetStompSound_Light(const int scale) {
        switch (scale) {
            case 2: 
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x2");
            case 4:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x4");
            case 8:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x8");
            case 12:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x12");
            case 24:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x24");
            case 48:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x48");
            case 96:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_x96");
            case 128:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Light_Mega");
            break;
        }
        return nullptr;
    }

	BSISoundDescriptor* GetStompSound_Strong(const int scale) {
        switch (scale) {
            case 2: 
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x2");
            case 4:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x4");
            case 8:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x8");
            case 12:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x12");
            case 24:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x24");
            case 48:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x48");
            case 96:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_x96");
            case 128:
                return Runtime::GetSound("GTSSoundFootstep_Stomp_Strong_Mega");
            break;
        }
        return nullptr;
    }

	BSISoundDescriptor* get_footstep_stomp(const FootEvent& foot_kind, const int scale, const bool strong) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
            case FootEvent::Right:
            case FootEvent::Back:
                return strong ? GetStompSound_Strong(scale) : GetStompSound_Light(scale);
            case FootEvent::JumpLand:
                return GetHHSound_Jump(scale);
        }
        return nullptr;
    }
}

namespace GTS {
	StompManager& StompManager::GetSingleton() noexcept {
		static StompManager instance;
		return instance;
	}

	std::string StompManager::DebugName() {
		return "::StompManager";
	}
	void StompManager::PlayNewOrOldStomps(Actor* giant, float modifier, FootEvent foot_kind, std::string_view find_foot, bool Strong) {
		const bool UseOtherHeelSet = Config::GetAudio().bUseOtherHighHeelSet;
		log::info("Trying to do stomps");

		if (UseOtherHeelSet && HighHeelManager::IsWearingHH(giant)) {
			log::info("Other Heel Set True");
			PlayStompSounds(giant, modifier, find_foot, foot_kind, get_visual_scale(giant), Strong);
		} else {
			DoFootstepSound(giant, modifier, foot_kind, find_foot);
		}
	}

	void StompManager::PlayStompSounds(Actor* giant, float modifier, std::string_view find_foot, FootEvent foot_kind, float scale, bool Strong) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		auto profiler = Profilers::Profile("StompManager: PlayHighHeelSounds");
		log::info("Trying to play New Stomps");
		if (giant) {
			log::info("Giant True");
			modifier *= Volume_Multiply_Function(giant, foot_kind);
			auto foot = find_node(giant, find_foot);

			if (giant->formID == 0x14 && HasSMT(giant)) {
				scale *= 2.5f;
			}

			if (foot) {
				log::info("Foot True");
				BSSoundHandle Footstep_2   = get_sound(modifier, foot, scale, limit_x4, GTS_FindAudio::get_footstep_stomp(foot_kind, 2, Strong), Footstep_2_Params, Footstep_4_Params, "x2 Footstep", 1.0f, true);
				// Stops at x4
				BSSoundHandle Footstep_4  = get_sound(modifier, foot, scale, limit_x8, GTS_FindAudio::get_footstep_stomp(foot_kind, 4, Strong), Footstep_4_Params, Footstep_8_Params, "x4 Footstep", 1.0f, true);
				// ^ Stops at ~x12
				BSSoundHandle Footstep_8  = get_sound(modifier, foot, scale, limit_x14, GTS_FindAudio::get_footstep_stomp(foot_kind, 8, Strong), Footstep_8_Params, Footstep_12_Params, "x8 Footstep", 1.33f, true);
				// ^ Stops at ~x14
				BSSoundHandle Footstep_12 = get_sound(modifier, foot, scale, limit_x24, GTS_FindAudio::get_footstep_stomp(foot_kind, 12, Strong), Footstep_12_Params, Footstep_24_Params, "x12 Footstep", 2.0f, true);
				// ^ Stops at ~x24
				BSSoundHandle Footstep_24 = get_sound(modifier, foot, scale, limit_x48, GTS_FindAudio::get_footstep_stomp(foot_kind, 24, Strong), Footstep_24_Params, Footstep_48_Params, "x24 Footstep", 5.0f, true);
				// ^ Stops at ~x44
				BSSoundHandle Footstep_48 = get_sound(modifier, foot, scale, limit_x96, GTS_FindAudio::get_footstep_stomp(foot_kind, 48, Strong), Footstep_48_Params, Footstep_96_Params, "x48 Footstep", 8.0f, true);
				// ^ Stops at ~x88
				BSSoundHandle Footstep_96 = get_sound(modifier, foot, scale, limit_mega, GTS_FindAudio::get_footstep_stomp(foot_kind, 96, Strong), Footstep_96_Params, Footstep_128_Params, "x96 Footstep", 12.0f, true);
				// ^ Stops at X126
				BSSoundHandle Footstep_128 = get_sound(modifier, foot, scale, limitless, GTS_FindAudio::get_footstep_stomp(foot_kind, 128, Strong), Footstep_128_Params, Params_Empty, "Mega Footstep", 18.0f, false);

				//=================================== Custom Commissioned Sounds =========================================
				if (Footstep_2.soundID != BSSoundHandle::kInvalidID) { // x1.35 + Custom audio
					log::info("x2 found");
					Footstep_2.Play();
				}
				if (Footstep_4.soundID != BSSoundHandle::kInvalidID) { // x4 Custom audio
					log::info("x4 found");
					Footstep_4.Play();
				}
				if (Footstep_8.soundID != BSSoundHandle::kInvalidID) { // x8 Custom audio
					log::info("x8 found");
					Footstep_8.Play();
				}
				if (Footstep_12.soundID != BSSoundHandle::kInvalidID) { // x12 Custom audio
					log::info("x12 found");
					Footstep_12.Play();
				}
				if (Footstep_24.soundID != BSSoundHandle::kInvalidID) { // x24 Custom audio
					log::info("x24 found");
					Footstep_24.Play();
				}
				if (Footstep_48.soundID != BSSoundHandle::kInvalidID) { // x48 Custom audio
					log::info("x48 found");
					Footstep_48.Play();
				}
				if (Footstep_96.soundID != BSSoundHandle::kInvalidID) { // x96 Custom audio
					log::info("x96 found");
					Footstep_96.Play();
				}
				if (Footstep_128.soundID != BSSoundHandle::kInvalidID) { // Mega sounds custom audio
					log::info("Mega found");
					Footstep_128.Play();
				}
			}
		}
	}

	float StompManager::Volume_Multiply_Function(Actor* actor, FootEvent Kind) {
		float modifier = 1.0f;
		if (actor) {
			if (actor->AsActorState()->IsSprinting()) { // Sprinting makes you sound bigger
				modifier *= 1.10f;
			}
			if (actor->AsActorState()->IsWalking()) {
				modifier *= 0.70f; // Walking makes you sound quieter
			}
			if (actor->IsSneaking()) {
				modifier *= 0.70f; // Sneaking makes you sound quieter
			}

			if (Kind == FootEvent::JumpLand) {
				modifier *= 1.2f; // Jumping makes you sound bigger
			}
			modifier *= 1.0f + (Potion_GetMightBonus(actor) * 0.33f);
		}
		return modifier;
	}
}