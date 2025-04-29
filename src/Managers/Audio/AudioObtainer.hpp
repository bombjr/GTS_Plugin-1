#pragma once
// Module that handles footsteps

namespace GTS {

    struct VolumeParams {
        float a;
        float k;
        float n;
        float s;
    };
    
    float volume_function(float scale, const VolumeParams& params);
    float frequency_function(float scale, const VolumeParams& params);

    std::string ObtainGTSMoanLaughSound(float scale, const std::string& basestring);
    std::string ObtainSLMoanSound(uint8_t index);

    BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind);
    BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind);

    BSISoundDescriptor* GetNormalSound(float scale);
    BSISoundDescriptor* GetNormalSound_Jump(float scale);

    BSISoundDescriptor* GetHHSound_NormalAlt(const int scale);
    BSISoundDescriptor* GetHHSound_JumpAlt(const int scale);

    BSISoundDescriptor* GetHHSound_Normal(const int scale);
    BSISoundDescriptor* GetHHSound_Jump(const int scale);

    BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind, const int scale, const bool alt);
    BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind, float scale);

}