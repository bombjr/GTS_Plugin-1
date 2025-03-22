#include "Managers/SpectatorManager.hpp"

using namespace RE;

namespace {
	Actor* GetCameraActor() {
		auto camera = PlayerCamera::GetSingleton();
		return camera->cameraTarget.get().get();
	}
}

namespace GTS {

	void SpectatorManager::SetCameraTarget(RE::Actor* a_Actor, bool aTemporary) {

		if (!a_Actor) {
			return;
		}

		auto Camera = PlayerCamera::GetSingleton();
		if (Camera) {
			auto NewTarget = a_Actor->CreateRefHandle();
			if (NewTarget) {
				Camera->cameraTarget = NewTarget;
				if (!aTemporary) {
					Target = a_Actor->formID;
				}
			}
		}
	}

	void SpectatorManager::Update() {

		std::ignore = Profilers::Profile("SpectatorManager::Update");

		if (Plugin::Live()) {

			auto currentCameraTarget = GetCameraActor();

			// Handle case when there's no camera target
			if (!currentCameraTarget) {
				ResetTarget(true);
				return;
			}

			// Case 1: Current camera target is not loaded - switch to player temporarily if not already
			// Normaly the player never is unloaded so if the target is the player this case should never trigger.
			// even if it did though it wont do anything undesired.
			if (!currentCameraTarget->Is3DLoaded() || !currentCameraTarget->Get3D() || !currentCameraTarget->Get3D1(false)) {
				// Only temporarily reset to player without changing Target
				if (const auto Player = PlayerCharacter::GetSingleton()) {
					SetCameraTarget(Player, true);
					//TrackedActorLost = true;
				}
				return;
			}

			// Case 2: Camera target differs from our stored target
			// if the target was the player we can assume something external changed the target so we should respect that change
			if (currentCameraTarget->formID != Target) {

				if (Target == 0x14) {
					Target = currentCameraTarget->formID;
				}

				auto intendedTarget = TESForm::LookupByID<Actor>(Target);

				if (!intendedTarget) {
					ResetTarget(true);
					return;
				}

				if (intendedTarget->Is3DLoaded() && intendedTarget->Get3D() && intendedTarget->Get3D1(false)) {
					// Our target is valid and loaded - switch back to it
					SetCameraTarget(intendedTarget, false);
				}
			}
		}
	}

	bool SpectatorManager::IsCameraTargetPlayer() {
		if (const auto CamTarget = GetCameraActor()) {
			return CamTarget->formID == 0x14;
		}
		return false;
	}


	void SpectatorManager::ResetTarget(bool aDoFullReset) {
		if (const auto Player = PlayerCharacter::GetSingleton()) {
			SetCameraTarget(Player, !aDoFullReset);
		}
	}

	void SpectatorManager::Reset() {
		ResetTarget(true);
	}

	void SpectatorManager::ResetActor(Actor* actor) {
		if (const auto& CameraTarget = GetCameraActor()) {
			if (actor) {
				if (CameraTarget->formID == actor->formID) {
					ResetTarget(true);
				}
			}
		}
	}
}
