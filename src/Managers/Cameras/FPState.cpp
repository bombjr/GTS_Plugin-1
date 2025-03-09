#include "Managers/Cameras/FPState.hpp"
#include "CamUtil.hpp"

namespace GTS {

	void FirstPersonCameraState::ExitState() {}

	float FirstPersonCameraState::GetScale() {
		return 1.0f;
	}

	bool FirstPersonCameraState::PermitManualEdit() {
		return false;
	}

	bool FirstPersonCameraState::PermitTransition() {
		return false;
	}

	bool FirstPersonCameraState::PermitCameraTransforms() {
		return false;
	}
}
