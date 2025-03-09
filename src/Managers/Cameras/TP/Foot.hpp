#pragma once

#include "Managers/Cameras/TPState.hpp"

namespace GTS {

	class Foot : public ThirdPersonCameraState {
		public:
			virtual void EnterState() override;

			BoneTarget GetBoneTarget() override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetPlayerLocalOffsetCrawling(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetFootPos();

		protected:
			Spring3 smoothFootPos = Spring3(NiPoint3(0.0f, 0.0f, 0.0f), 0.5f);
			Spring smoothScale = Spring(1.0f, 0.5f);
		private:
			const BoneTarget FootTarget{ .boneNames = {"NPC L Calf [LClf]" ,"NPC R Calf [RClf]"} };
	};
}
