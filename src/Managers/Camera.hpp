#pragma once

// Module that handles the Camera

#include "Managers/Cameras/State.hpp"
#include "Managers/Cameras/Trans.hpp"

#include "Managers/Cameras/TP/Alt.hpp"
#include "Managers/Cameras/TP/Normal.hpp"
#include "Managers/Cameras/TP/Foot.hpp"
#include "Managers/Cameras/TP/FootL.hpp"
#include "Managers/Cameras/TP/footR.hpp"

#include "Managers/Cameras/FP/normal.hpp"

namespace GTS {

	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void DataReady() override;
			virtual void Start() override;

			virtual void CameraUpdate() override;

			CameraState* GetCameraState();

			void AdjustUpDown(float amt);
			void ResetUpDown();

			void AdjustLeftRight(float amt);
			void ResetLeftRight();

		private:
			CameraState* GetCameraStateTP();
			CameraState* GetCameraStateFP();

			CameraState scaledVanillaState;  // Like vanilla only scaled

			Normal normalState;
			Alt altState;
			Foot footState;
			FootR footRState;
			FootL footLState;

			FirstPerson fpState;

			NiPoint3 manualEdit;

			Timer initimer = Timer(3.00);

			Spring smoothScale = Spring(0.30f, 0.50f);
			Spring3 smoothOffset = Spring3(NiPoint3(0.30f, 0.30f, 0.30f), 0.50f);
			float CameraDelay = 0.0f;

			CameraState* currentState = nullptr;
			std::unique_ptr<TransState> transitionState = std::unique_ptr<TransState>(nullptr);
	};
}