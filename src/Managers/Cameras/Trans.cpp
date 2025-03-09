#include "Managers/Cameras/Trans.hpp"

namespace GTS {

	TransState::TransState(CameraState* stateA, CameraState* stateB) : stateA(stateA), stateB(stateB) {
		this->smoothIn.value = 0.0f;
		this->smoothIn.target = 1.0f;
		this->smoothIn.velocity = 0.0f;
	}

	float TransState::GetScale() {
		return this->stateB->GetScale() * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetScale() * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffset(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffset(cameraPosLocal, IsCrawling) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetOffsetProne(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetOffsetProne(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetCombatOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffset(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetCombatOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffset(cameraPosLocal, IsCrawling) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetCombatOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetCombatOffsetProne(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetCombatOffsetProne(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetPlayerLocalOffset(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffset(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) {
		return this->stateB->GetPlayerLocalOffset(cameraPosLocal, IsCrawling) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffset(cameraPosLocal, IsCrawling) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	NiPoint3 TransState::GetPlayerLocalOffsetCrawling(const NiPoint3& cameraPosLocal) {
		return this->stateB->GetPlayerLocalOffsetCrawling(cameraPosLocal) * std::clamp(this->smoothIn.value, 0.0f, 1.0f) + this->stateA->GetPlayerLocalOffsetCrawling(cameraPosLocal) * (1.0f - std::clamp(this->smoothIn.value, 0.0f, 1.0f));
	}

	bool TransState::PermitManualEdit() {
		return this->stateB->PermitManualEdit() && this->stateA->PermitManualEdit();
	}

	bool TransState::PermitTransition() {
		return false;
	}

	bool TransState::PermitCameraTransforms() {
		return this->stateB->PermitCameraTransforms() && this->stateA->PermitCameraTransforms();
	}

	bool TransState::IsDone() const {
		return std::clamp(this->smoothIn.value, 0.0f, 1.0f) > 0.995f;
	}
}
