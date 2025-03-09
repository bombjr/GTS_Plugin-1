#pragma once

#include "Managers/Cameras/TPState.hpp"

namespace GTS {

	class Normal : public ThirdPersonCameraState {
		public:
			float ZOffset = -1;  // Added Back And Forth Offset

			void SetZOff(float Offset);

			virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPos) override;

			virtual BoneTarget GetBoneTarget() override;
	};
}
