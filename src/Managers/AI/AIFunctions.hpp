#pragma once

namespace GTS {

	float GetScareThreshold(Actor* giant);
	void Task_InitHavokTask(Actor* tiny);
	void SendDeathEvent(Actor* giant, Actor* tiny);
	void KillActor(Actor* giant, Actor* tiny, bool silent = false);
	void ForceFlee(Actor* giant, Actor* tiny, float duration, bool apply_size_difference);
	void ScareActors(Actor* giant);
}