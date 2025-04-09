#pragma once

#include "Managers/Cameras/TPState.hpp"

namespace GTS {
	class Alt : public ThirdPersonCameraState {
		public:
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;
			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPos) override;
			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPos) override;
			virtual BoneTarget GetBoneTarget() override;
	};
}
