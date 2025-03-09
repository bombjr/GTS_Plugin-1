#pragma once

namespace GTS {
    void PlayCrushSound(Actor* giant, NiAVObject* node, bool only_once, bool StrongSound);
    void PlayMatchingSound(Actor* giant, NiAVObject* node, bool strong, int crushed, float size) ;
}