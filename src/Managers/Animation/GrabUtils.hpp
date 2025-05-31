#pragma once

namespace GTS {
    bool IsCurrentlyReattaching(Actor* giant);
    bool HandleGrabLogic(Actor* giantref, Actor* tinyref, ActorHandle gianthandle, ActorHandle tinyhandle);
    void ReattachTiny(Actor* giant, Actor* tiny);
    bool FailSafeAbort(Actor* giantref, Actor* tinyref);
}