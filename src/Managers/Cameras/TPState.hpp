#pragma once

#include "Managers/Cameras/State.hpp"

namespace GTS {

	class ThirdPersonCameraState : public CameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;
			virtual NiPoint3 GetPlayerLocalOffsetCrawling(const NiPoint3& cameraPos) override;
			virtual BoneTarget GetBoneTarget();
			virtual NiPoint3 CrawlAdjustment(const NiPoint3& cameraPos);

			Spring SpringSmoothScale = Spring(1.0f, 0.5f);
			Spring3 SpringSmoothedBonePos = Spring3(NiPoint3(0.0f, 0.0f, 0.0f), 0.5f);

	};
}
