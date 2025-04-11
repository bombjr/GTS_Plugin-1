#include "Managers/Audio/AudioObtainer.hpp"
#include "Config/Config.hpp"

using namespace GTS;

namespace GTS {


    float volume_function(float scale, const VolumeParams& params) {
        float k = params.k;
        float a = params.a;
        float n = params.n;
        float s = params.s;
        // https://www.desmos.com/calculator/ygoxbe7hjg
        return k*pow(s*(scale-a), n);
    }

    float frequency_function(float scale, const VolumeParams& params) {
        float a = params.a;
        return soft_core(scale, 0.01f, 1.0f, 1.0f, a, 0.0f)*0.5f+0.5f;
    }

    std::string ObtainMoanLaughSound(float scale, std::string Construct) {
        std::string SoundResult = Construct;
        std::string size_range = "_x2";
        if (scale < 2.0f || !Config::GetAudio().bMoanLaughSizeVariants) {
            return Construct; // We're at 'normal' size
        }

        // Else construct matching size audio for moan/laughs
        
        else if (scale >= 96.0f)    {size_range = "_x96";}
        else if (scale >= 48.0f)    {size_range = "_x48";}
        else if (scale >= 24.0f)    {size_range = "_x24";}
        else if (scale >= 12.0f)    {size_range = "_x12";}
        else if (scale >= 8.0f)     {size_range = "_x8";}
        else if (scale >= 4.0f)     {size_range = "_x4";}
        else if (scale >= 2.0f)     {size_range = "_x2";}

        SoundResult += size_range;
        log::info("Sound Result: {}", SoundResult); 
        return SoundResult;
    }

    BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("GTSSoundFootstep_L");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("GTSSoundFootstep_L");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::JumpLand:
                return Runtime::GetSound("GTSSoundFootstepLand_L");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("GTSSoundFootstep_XL"); 
            break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("GTSSoundFootstep_XL");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("GTSSoundRumble");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("GTSSoundRumble");
                break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("GTSSoundRumble");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("GTSSoundFootstep_Sprint");
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("GTSSoundFootstep_Sprint");
                break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("GTSSoundFootstepLand_L");
                break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return Runtime::GetSound("GTSSoundFootstep_XXL");
            break;
            case FootEvent::Right:
            case FootEvent::Back:
                return Runtime::GetSound("GTSSoundFootstep_XXL");
            break;
            case FootEvent::JumpLand:
                return Runtime::GetSound("GTSSoundFootstep_XXL");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* GetNormalSound(float scale) {
        if (scale == 2.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_2x");
        } else if (scale == 4.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_4x");
        } else if (scale == 8.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_8x");
        } else if (scale == 12.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_12x");
        } else if (scale == 24.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_24x");
        } else if (scale == 48.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_48x");
        } else if (scale == 96.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_96x");
        } else if (scale > 96.0f) {
            return Runtime::GetSound("GTSSoundFootstepNormal_Mega");
        }
        return nullptr;
    }
    BSISoundDescriptor* GetNormalSound_Jump(float scale) {
        if (scale == 2.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_2x");
        } else if (scale == 4.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_4x");
        } else if (scale == 8.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_8x");
        } else if (scale == 12.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_12x");
        } else if (scale == 24.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_24x");
        } else if (scale == 48.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_48x");
        } else if (scale == 96.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_96x");
        } else if (scale > 96.0f) {
            return Runtime::GetSound("GTSSoundFootstepLandNormal_Mega");
        }
        return nullptr;
    }

    BSISoundDescriptor* GetHHSound_Normal(const int scale) {
        switch (scale) {
            case 2: 
                return Runtime::GetSound("GTSSoundFootstepHighHeels_2x");
            case 4:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_4x");
            case 8:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_8x");
            case 12:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_12x");
            case 24:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_24x");
            case 48:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_48x");
            case 96:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_96x");
            case 128:
                return Runtime::GetSound("GTSSoundFootstepHighHeels_Mega");
            break;
        }
        return nullptr;
    }
    BSISoundDescriptor* GetHHSound_Jump(const int scale) {
        switch (scale) {
            case 2: 
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_2x");
            case 4:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_4x");
            case 8:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_8x");
            case 12:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_12x");
            case 24:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_24x");
            case 48:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_48x");
            case 96:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_96x");
            case 128:
                return Runtime::GetSound("GTSSoundFootstepLandHighHeels_Mega");
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind, const int scale) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return GetHHSound_Normal(scale);
            case FootEvent::Right:
            case FootEvent::Back:
                return GetHHSound_Normal(scale);
            case FootEvent::JumpLand:
                return GetHHSound_Jump(scale);
            break;
        }
        return nullptr;
    }

    BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind, float scale) {
        switch (foot_kind) {
            case FootEvent::Left:
            case FootEvent::Front:
                return GetNormalSound(scale);
                break;
            case FootEvent::Right:
            case FootEvent::Back:
                return GetNormalSound(scale);
                break;
            case FootEvent::JumpLand:
                return GetNormalSound_Jump(scale);
                break;
        }
        return nullptr;
    }
}