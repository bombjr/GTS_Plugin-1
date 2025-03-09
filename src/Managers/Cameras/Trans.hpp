#pragma once
#include "Managers/Cameras/State.hpp"

namespace GTS {

	class TransState : public CameraState {
		public:
			virtual float GetScale() override;
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPosLocal) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPosLocal) override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool IsCrawling) override;
			virtual NiPoint3 GetPlayerLocalOffsetCrawling(const NiPoint3& cameraPosLocal) override;

			virtual bool PermitManualEdit() override;
			virtual bool PermitTransition() override;
			virtual bool PermitCameraTransforms() override;

			TransState(CameraState* stateA, CameraState* stateB);

			bool IsDone() const;
		private:
			CameraState* stateA;
			CameraState* stateB;
			Spring smoothIn = Spring(0.0f, 0.4f);
	};
}
