#pragma once

namespace GTS {
    void Task_QueueGrabAbortTask(Actor* giant, std::string_view name);
    bool IsCurrentlyReattaching(Actor* giant);
    bool HandleGrabLogic(Actor* giantref, Actor* tinyref, ActorHandle gianthandle, ActorHandle tinyhandle);
    void ReattachTiny(Actor* giant, Actor* tiny);
    bool FailSafeAbort(Actor* giantref, Actor* tinyref);
}