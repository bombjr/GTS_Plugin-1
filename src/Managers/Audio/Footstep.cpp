#include "Managers/Audio/Footstep.hpp"

#include "Config/Config.hpp"

#include "Managers/Audio/AudioObtainer.hpp"
#include "Managers/Highheel.hpp"

using namespace GTS;

namespace {

	// Params
	constexpr VolumeParams Params_Empty = {.a = 0.0f, .k = 0.0f, .n = 0.0f, .s = 0.0f};

	constexpr VolumeParams xlFootstep_Params = {.a = 12.0f, .k = 0.50f, .n = 0.5f, .s = 1.0f};
	constexpr VolumeParams xxlFootstep_Params = {.a = 20.0f, .k = 0.50f,  .n = 0.5f, .s = 1.0f};
	constexpr VolumeParams lJumpLand_Params = {.a = 1.2f, .k = 0.65f,  .n = 0.7f, .s = 1.0f};

	constexpr VolumeParams xlRumble_Params = {.a = 12.0f, .k = 0.50f, .n = 0.5f, .s = 1.0f};

	constexpr VolumeParams Footstep_2_Params = {.a = 1.35f, .k = 1.0f, .n = 0.75f, .s = 1.0f};    // https://www.desmos.com/calculator/6yzmmrg3oi
	constexpr VolumeParams Footstep_4_Params = {.a = 3.0f, .k = 1.0f, .n = 1.15f, .s = 1.0f};     // https://www.desmos.com/calculator/ighurmlanl
	constexpr VolumeParams Footstep_8_Params = {.a = 6.0f, .k = 0.26f, .n = 1.94f, .s = 1.0f};    // https://www.desmos.com/calculator/d3gsgj6ocs
	constexpr VolumeParams Footstep_12_Params = {.a = 12.0f, .k = 0.27f, .n = 1.9f, .s = 1.0f};   // https://www.desmos.com/calculator/akoyl4cxch
	constexpr VolumeParams Footstep_24_Params = {.a = 16.0f, .k = 0.36f, .n = 1.5f, .s = 0.25f};  // https://www.desmos.com/calculator/bh4fhfrji6
	constexpr VolumeParams Footstep_48_Params = {.a = 36.0f, .k = 0.20f, .n = 1.5f, .s = 0.25f};  // https://www.desmos.com/calculator/3q4qgkrker
	constexpr VolumeParams Footstep_96_Params = {.a = 80.0f, .k = 0.66f, .n = 0.90f, .s = 0.1f};  // https://www.desmos.com/calculator/ufdbieymdi
	constexpr VolumeParams Footstep_128_Params = {.a = 124.0f, .k = 1.0f, .n = 1.15f, .s = 1.0f}; //{.a = 118.0f, .k = 0.19f, .n = 1.5f, .s = 0.1f}; // https://www.desmos.com/calculator/pvrge5dejz
	// Params end

	constexpr float limitless = 0.0f;
	constexpr float limit_x2 = 2.0f;
	constexpr float limit_x4 = 4.0f;
	constexpr float limit_x8 = 8.0f;
	constexpr float limit_x12 = 12.0f;
	constexpr float limit_x14 = 14.0f;
	constexpr float limit_x24 = 24.0f;
	constexpr float limit_x48 = 48.0f;
	constexpr float limit_x96 = 96.0f;
	constexpr float limit_mega = 128.0f;

	std::string GetFootstepName(Actor* giant, bool right) {
		std::string tag;
		if (!giant->AsActorState()->IsSneaking()) {
			right ? tag = "FootScuffRight" : tag = "FootScuffLeft";
		} else {
			right ? tag = "FootRight" : tag = "FootLeft";
		}
		return tag;
	}

	BSSoundHandle get_sound(float movement_mod, NiAVObject* foot, const float& scale, const float& scale_limit, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, const VolumeParams& blend_with, std::string_view tag, float mult, bool blend) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		if (foot) {
			if (sound_descriptor && audio_manager) {
				float volume = volume_function(scale, params);
				float frequency = frequency_function(scale, params);
				float falloff = Sound_GetFallOff(foot, mult);
				float intensity = volume * falloff * movement_mod;
				if (scale_limit > 0.02f && scale > scale_limit) {
					return result; // Return empty sound in that case
				}

				intensity = std::clamp(intensity, 0.0f, 1.0f);
				
				if (blend) {
					float exceeded = volume_function(scale, blend_with);
					if (exceeded > 0.02f) {
						intensity -= exceeded;
					}
				}

				if (intensity > 0.05f) {

					// log::trace("  - Playing {} with volume: {}, falloff: {}, intensity: {}", tag, volume, falloff, intensity);
					audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
					result.SetVolume(intensity);
					result.SetFrequency(frequency);
					NiPoint3 pos;
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;
					result.SetPosition(pos);
					result.SetObjectToFollow(foot);
				}
			}
		}
		return result;
	}
}
namespace GTS {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	std::string FootStepManager::DebugName() {
		return "::FootStepManager";
	}

	void FootStepManager::OnImpact(const Impact& impact) {
		if (impact.actor) {

			if (!impact.actor->Is3DLoaded()) {
				return;
			} 
			if (!impact.actor->GetCurrent3D()) {
				return;
			}

			auto profiler = Profilers::Profile("FootStepManager: OnImpact");

			float scale = impact.scale;
			auto actor = impact.actor;
			
			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 2.5f; // Affect Sound threshold itself
			}

			const bool LegacySounds = Config::GetAudio().bUseOldSounds;  // Determine if we should play old pre 2.00 update sounds
			// ^ Currently forced to true: there's not a lot of sounds yet.
			bool WearingHighHeels = HighHeelManager::IsWearingHH(actor);
			if (scale > 1.2f && !actor->AsActorState()->IsSwimming()) {

				float modifier = Volume_Multiply_Function(actor, impact.kind) * impact.modifier; // Affects the volume only!
				FootEvent foot_kind = impact.kind;
				
				if (Config::GetAudio().bFootstepSounds) {

					for (NiAVObject* foot: impact.nodes) {

						if (foot) {
							FootStepManager::PlayLegacySounds(modifier, foot, foot_kind, scale);
							return; // New sounds are disabled for now

							if (!LegacySounds) { // Use new sounds that we've commissioned
								if (WearingHighHeels) { // Play high heel sounds that are being slowly worked on
									FootStepManager::PlayHighHeelSounds(modifier, foot, foot_kind, scale);
									return;
								} else { // Play non high heel sounds that are being slowly worked on
									FootStepManager::PlayNormalSounds(modifier, foot, foot_kind, scale);
									return;
								}
							} else { // Else Play old sounds
								FootStepManager::PlayLegacySounds(modifier, foot, foot_kind, scale);
								return;
							}
						}
					}
				}
			}
		}
	}

	void FootStepManager::PlayLegacySounds(float modifier, NiAVObject* foot, FootEvent foot_kind, float scale) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		auto profiler = Profilers::Profile("FootStepManager: PlayLegacySounds");

		BSSoundHandle xlFootstep   = get_sound(modifier, foot, scale, limit_x14, get_xlFootstep_sounddesc(foot_kind), xlFootstep_Params, Params_Empty, "XL: Footstep", 1.0f, false);
		BSSoundHandle xxlFootstep = get_sound(modifier, foot, scale, limit_x14, get_xxlFootstep_sounddesc(foot_kind), xxlFootstep_Params, Params_Empty, "XXL Footstep", 1.0f, false);
		// These stop to appear at x14
		BSSoundHandle lJumpLand    = get_sound(modifier, foot, scale, limitless, get_lJumpLand_sounddesc(foot_kind), lJumpLand_Params, Params_Empty, "L Jump", 1.0f, false);

		BSSoundHandle xlRumble     = get_sound(modifier, foot, scale, limitless, get_xlRumble_sounddesc(foot_kind), xlRumble_Params, Params_Empty, "XL Rumble", 1.0f, false);
		//BSSoundHandle xlSprint     = get_sound(modifier, foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.50, .n = 0.5, .s = 1.0}, "XL Sprint", 1.0);
        //  ^ Same normal sounds but a tiny bit louder: 319060: Sound\fx\GTS\Effects\Footsteps\Original\Movement
		BSSoundHandle Footstep_2   = get_sound(modifier, foot, scale, limit_x4, get_footstep_highheel(foot_kind, 2), Footstep_2_Params, Footstep_4_Params, "x2 Footstep", 1.0f, true);
		// Stops at x4
		BSSoundHandle Footstep_4  = get_sound(modifier, foot, scale, limit_x8, get_footstep_highheel(foot_kind, 4), Footstep_4_Params, Footstep_8_Params, "x4 Footstep", 1.0f, true);
		// ^ Stops at ~x12
		BSSoundHandle Footstep_8  = get_sound(modifier, foot, scale, limit_x14, get_footstep_highheel(foot_kind, 8), Footstep_8_Params, Footstep_12_Params, "x8 Footstep", 1.33f, true);
		// ^ Stops at ~x14
		BSSoundHandle Footstep_12 = get_sound(modifier, foot, scale, limit_x24, get_footstep_highheel(foot_kind, 12), Footstep_12_Params, Footstep_24_Params, "x12 Footstep", 2.0f, true);
		// ^ Stops at ~x24
		BSSoundHandle Footstep_24 = get_sound(modifier, foot, scale, limit_x48, get_footstep_highheel(foot_kind, 24), Footstep_24_Params, Footstep_48_Params, "x24 Footstep", 5.0f, true);
		// ^ Stops at ~x44
		BSSoundHandle Footstep_48 = get_sound(modifier, foot, scale, limit_x96, get_footstep_highheel(foot_kind, 48), Footstep_48_Params, Footstep_96_Params, "x48 Footstep", 8.0f, true);
		// ^ Stops at ~x88
		BSSoundHandle Footstep_96 = get_sound(modifier, foot, scale, limit_mega, get_footstep_highheel(foot_kind, 96), Footstep_96_Params, Footstep_128_Params, "x96 Footstep", 12.0f, true);
		// ^ Stops at X126
		BSSoundHandle Footstep_128 = get_sound(modifier, foot, scale, limitless, get_footstep_highheel(foot_kind, 128), Footstep_128_Params, Params_Empty, "Mega Footstep", 18.0f, false);

		if (xlFootstep.soundID != BSSoundHandle::kInvalidID) { 
			// 271EF4: Sound\fx\GTS\Foot\Effects  (Stone sounds)
			xlFootstep.Play();
		}
		if (xxlFootstep.soundID != BSSoundHandle::kInvalidID) { 
			// 16FB25: Sound\fx\GTS\Effects\Footsteps\Original\Rumble (Distant foot sounds)
			xxlFootstep.Play();
		}

		if (lJumpLand.soundID != BSSoundHandle::kInvalidID) { // Jump Land audio: 
			// 183F43: Sound\fx\GTS\Effects\Footsteps\Original\Fall
			lJumpLand.Play();
		}
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) { // Rumble when walking at huge scale: 
			// 36A06D: Sound\fx\GTS\Foot\Effects\Rumble1-4.wav
			xlRumble.Play();
		}

		//=================================== Custom Commissioned Sounds =========================================
		if (Footstep_2.soundID != BSSoundHandle::kInvalidID) { // x1.35 + Custom audio
			Footstep_2.Play();
		}
		if (Footstep_4.soundID != BSSoundHandle::kInvalidID) { // x4 Custom audio
			Footstep_4.Play();
		}
		if (Footstep_8.soundID != BSSoundHandle::kInvalidID) { // x8 Custom audio
			Footstep_8.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) { // x12 Custom audio
			Footstep_12.Play();
		}
		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) { // x24 Custom audio
			Footstep_24.Play();
		}
		if (Footstep_48.soundID != BSSoundHandle::kInvalidID) { // x48 Custom audio
			Footstep_48.Play();
		}
		if (Footstep_96.soundID != BSSoundHandle::kInvalidID) { // x96 Custom audio
			Footstep_96.Play();
		}
		if (Footstep_128.soundID != BSSoundHandle::kInvalidID) { // Mega sounds custom audio
			Footstep_128.Play();
		}
	}

	void FootStepManager::PlayHighHeelSounds(float modifier, NiAVObject* foot, FootEvent foot_kind, float scale) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		// 2024.04.23: Only 2 sets are done for now: x8, x12 and x24 (still wip)
		/*BSSoundHandle xlRumble     = get_sound(modifier, foot, scale, limitless, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = 12.0f,            .k = 0.50f, .n = 0.5f, .s = 1.0f}, "XL Rumble", 1.0f);

		BSSoundHandle Footstep_2  = get_sound(modifier, foot, scale, limit_x4, get_footstep_highheel(foot_kind, 2),  VolumeParams { .a = 1.2f,           .k = 0.45f,  .n = 0.7f, .s = 1.0f}, "x2 Footstep", 1.0f);
		BSSoundHandle Footstep_4  = get_sound(modifier, foot, scale, limit_x8, get_footstep_highheel(foot_kind, 4),  VolumeParams { .a = 4.0f,           .k = 0.45f, .n = 0.55f, .s = 1.0f}, "x4 Footstep", 1.5f);
		BSSoundHandle Footstep_8  = get_sound(modifier, foot, scale, limit_x14, get_footstep_highheel(foot_kind, 8),  VolumeParams { .a = 8.0f,           .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x8 Footstep", 2.0f);
		BSSoundHandle Footstep_12 = get_sound(modifier, foot, scale, limit_x24, get_footstep_highheel(foot_kind, 12),  VolumeParams { .a = 12.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x12 Footstep", 2.5f);
		BSSoundHandle Footstep_24 = get_sound(modifier, foot, scale, limit_x48, get_footstep_highheel(foot_kind, 24),  VolumeParams { .a = 22.0f,          .k = 0.30f, .n = 0.55f, .s = 1.0f}, "x24 Footstep", 5.0f);
		BSSoundHandle Footstep_48 = get_sound(modifier, foot, scale, limit_x96, get_footstep_highheel(foot_kind, 48),  VolumeParams { .a = 48.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x48 Footstep", 10.0f);
		BSSoundHandle Footstep_96 = get_sound(modifier, foot, scale, limit_mega, get_footstep_highheel(foot_kind, 96),  VolumeParams { .a = 96.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x96 Footstep", 120.0f);
		BSSoundHandle Footstep_Mega = get_sound(modifier, foot, scale, limitless, get_footstep_highheel(foot_kind, 98),  VolumeParams { .a = 110.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "Mega Footstep", 40.0f);
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
		}
		if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		}
		if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		}
		if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) {
			Footstep_12.Play();
		}
		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) {
			Footstep_24.Play();
		}
		if (Footstep_48.soundID != BSSoundHandle::kInvalidID) {
			Footstep_48.Play();
		}
		if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
		if (Footstep_Mega.soundID != BSSoundHandle::kInvalidID) {
			Footstep_Mega.Play();
		}*/
	}

	void FootStepManager::PlayNormalSounds(float modifier, NiAVObject* foot, FootEvent foot_kind, float scale) {
		//https://www.desmos.com/calculator/wh0vwgljfl
		// 2024.04.23:  There's no sounds in this set of sounds yet. It will be worked on after high heel sounds will be done
		/*BSSoundHandle xlRumble     = get_sound(modifier, foot, scale, limitless, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = 12.0f,            .k = 0.50f, .n = 0.5f, .s = 1.0f}, "XL Rumble", 1.0f);

		BSSoundHandle Footstep_2  = get_sound(modifier, foot, scale, limit_x4, get_footstep_normal(foot_kind, 2),  VolumeParams { .a = 1.2f,           .k = 0.45f,  .n = 0.7f, .s = 1.0f}, "x2 Footstep", 1.0f);
		BSSoundHandle Footstep_4  = get_sound(modifier, foot, scale, limit_x8, get_footstep_normal(foot_kind, 4),  VolumeParams { .a = 4.0f,           .k = 0.45f, .n = 0.55f, .s = 1.0f}, "x4 Footstep", 1.5f);
		BSSoundHandle Footstep_8  = get_sound(modifier, foot, scale, limit_x12, get_footstep_normal(foot_kind, 8),  VolumeParams { .a = 8.0f,           .k = 0.45f, .n = 0.55f, .s = 1.0f}, "x8 Footstep", 2.0f);
		BSSoundHandle Footstep_12 = get_sound(modifier, foot, scale, limit_x24, get_footstep_normal(foot_kind, 12),  VolumeParams { .a = 12.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x12 Footstep", 2.5f);
		BSSoundHandle Footstep_24 = get_sound(modifier, foot, scale, limit_x48, get_footstep_normal(foot_kind, 24),  VolumeParams { .a = 22.0f,          .k = 0.30f, .n = 0.55f, .s = 1.0f}, "x24 Footstep", 3.0f);
		BSSoundHandle Footstep_48 = get_sound(modifier, foot, scale, limit_x96, get_footstep_normal(foot_kind, 48),  VolumeParams { .a = 48.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x48 Footstep", 6.0f);
		BSSoundHandle Footstep_96 = get_sound(modifier, foot, scale, limit_mega, get_footstep_normal(foot_kind, 96),  VolumeParams { .a = 96.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "x96 Footstep", 12.0f);
		BSSoundHandle Footstep_Mega = get_sound(modifier, foot, scale, limitless, get_footstep_normal(foot_kind, 98),  VolumeParams { .a = 110.0f,          .k = 0.40f, .n = 0.55f, .s = 1.0f}, "Mega Footstep", 24.0f);

		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
		}
		if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		}
		if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		}
		if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		}
		if (Footstep_12.soundID != BSSoundHandle::kInvalidID) {
			Footstep_12.Play();
		}
		if (Footstep_24.soundID != BSSoundHandle::kInvalidID) {
			Footstep_24.Play();
		}
		if (Footstep_48.soundID != BSSoundHandle::kInvalidID) {
			Footstep_48.Play();
		}
		if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
		if (Footstep_Mega.soundID != BSSoundHandle::kInvalidID) {
			Footstep_Mega.Play();
		}*/
	}

	float FootStepManager::Volume_Multiply_Function(Actor* actor, FootEvent Kind) {
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

	void FootStepManager::PlayVanillaFootstepSounds(Actor* giant, bool right) {
		if (get_visual_scale(giant) > 2.05f) { // No need to play it past this size
			return;
		}

		ActorHandle giantHandle = giant->CreateRefHandle();
		std::string tag = GetFootstepName(giant, right);

		BSTEventSource<BGSFootstepEvent>* eventSource = nullptr;
		auto foot_event = BGSFootstepEvent();

		foot_event.actor = giantHandle;
		foot_event.pad04 = 10000001; // Mark as custom .dll event, so our dll won't listen to it
		foot_event.tag = tag;

		BGSImpactManager::GetSingleton()->ProcessEvent(&foot_event, eventSource); // Make the game play vanilla footstep sound
	}

	void FootStepManager::DoStrongSounds(Actor* giant, float animspeed, std::string_view feet) {
		float scale = get_visual_scale(giant);
		float bonus = 1.0f;
		

		if (HasSMT(giant)) {
			bonus = 8.0f;
			scale += 0.6f;
		}

		if (scale > 1.25f) {
			float volume = 0.14f * bonus * (scale - 1.10f) * animspeed;
			if (volume > 0.05f) {
				Runtime::PlaySoundAtNode("GTSSoundHeavyStomp", giant, volume, 1.0f, feet);
				Runtime::PlaySoundAtNode("GTSSoundFootstep_XL", giant, volume, 1.0f, feet);
				Runtime::PlaySoundAtNode("GTSSoundRumble", giant, volume, 1.0f, feet);
			}
		}
	}
}
