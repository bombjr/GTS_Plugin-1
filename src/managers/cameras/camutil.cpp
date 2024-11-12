#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "scale/modscale.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

#include "node.hpp"


using namespace RE;
using namespace Gts;

namespace {
	enum class CameraDataMode {
		State,
		Transform,
	};

	const CameraDataMode currentMode = CameraDataMode::State;

	void PerformRaycastOnCamera(NiPoint3& LocalCoords, PlayerCamera* playerCamera) {
		// Get our computed local-space xyz offset.
		const auto cameraLocal = LocalCoords;
		// Get the base world position for the camera which we will offset with the local-space values.

		bool success = false;
		NiPoint3 endpos = NiPoint3();
		NiPoint3 ray_start = playerCamera->cameraRoot->world.translate;
		const std::vector<NiPoint3> directions = {
			{0,0, -1},
			{0,0, 1},
			{0,-1,0},
			{0,1,0},
			{-1,0,0},
			{1,0,0},
		};

		float ray_length = 30 * get_visual_scale(PlayerCharacter::GetSingleton());
		for (auto Directions: directions) {
			NiPoint3 endpos = CastRayStatics(PlayerCharacter::GetSingleton(), ray_start, Directions, ray_length, success);
		}
		if (success) {
			log::info("Raycast is true");
			LocalCoords = endpos;
			// I can't get the logic for it right, behaves incorrectly.
		}
	}

	void CameraTest(NiPoint3 coords) {
		auto player = PlayerCharacter::GetSingleton();
		auto playerCamera = RE::PlayerCamera::GetSingleton();
		auto thirdPersonState = reinterpret_cast<RE::ThirdPersonState*>(playerCamera->cameraStates[RE::CameraState::kThirdPerson].get());
		auto isInThirdPerson = playerCamera->currentState->id == RE::CameraState::kThirdPerson;
		//log::info("Current Zoom Offset: {}", thirdPersonState->currentZoomOffset);
		if (isInThirdPerson) {

			float shift = 100 * (get_visual_scale(player) - get_natural_scale(player));

			if (coords.Length() <= 0) {
				coords = {0, -shift, shift}; // If no Bone Tracking mode is selected, just scale the camera up and backwards
			}
			
			// Else apply bone tracking offsets
 			//NiPoint3& expected_pos = thirdPersonState->posOffsetExpected;
			//expected_pos = coords;
			// Problems:
			    // 
				// -1: When looking above the character, camera moves forward despite offsetting it backwards based on size
				// -2: rotating the camera around the player results in weird distance shifts
		}
	}
}

namespace Gts {

	float HighHeelOffset() {
		Actor* player = PlayerCharacter::GetSingleton();
		float hh = 0.0f;
		if (player) {
			hh = HighHeelManager::GetBaseHHOffset(player).z;
			hh *= HighHeelManager::GetHHMultiplier(player);
			if (IsFootGrinding(player) || IsTrampling(player) || IsStomping(player) || IsVoring(player)) {
				hh = 0.0f;
			}
		}
		return hh;
	}

	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	float GetINIFloat(std::string_view name) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			return setting->data.f;
		}
		return -1.0f;
	}

	void EnsureINIFloat(std::string_view name, float value) {
		auto currentValue = GetINIFloat(name);
		if (fabs(currentValue - value) > 1e-3) {
			SetINIFloat(name, value);
		}
	}

	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = GetCameraActor();
		if (camera && player) {
			camera->UpdateThirdPerson(player->AsActorState()->IsWeaponDrawn());
		}
	}

	void ResetIniSettings() {
		EnsureINIFloat("fOverShoulderPosX:Camera", 30.0f);
		EnsureINIFloat("fOverShoulderPosY:Camera", 30.0f);
		EnsureINIFloat("fOverShoulderPosZ:Camera", -10.0f);
		EnsureINIFloat("fOverShoulderCombatPosX:Camera", 0.0f);
		EnsureINIFloat("fOverShoulderCombatPosY:Camera", 0.0f);
		EnsureINIFloat("fOverShoulderCombatPosZ:Camera", 20.0f);
		EnsureINIFloat("fVanityModeMaxDist:Camera", 600.0f);
		EnsureINIFloat("fVanityModeMinDist:Camera", 155.0f);
		EnsureINIFloat("fMouseWheelZoomSpeed:Camera", 0.8000000119f);
		EnsureINIFloat("fMouseWheelZoomIncrement:Camera", 0.075000003f);
		UpdateThirdPerson();
	}

	NiCamera* GetNiCamera() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		return niCamera;
	}
	void UpdateWorld2ScreetMat(NiCamera* niCamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
	}

	Actor* GetCameraActor() {
		auto camera = PlayerCamera::GetSingleton();
		return camera->cameraTarget.get().get();
	}


	void UpdateSceneManager(NiPoint3 camLoc) {
		auto sceneManager = UI3DSceneManager::GetSingleton();
		if (sceneManager) {
			// Cache
			sceneManager->cachedCameraPos = camLoc;

			/*#ifdef ENABLED_SHADOW
			   // Shadow Map
			   auto shadowNode = sceneManager->shadowSceneNode;
			   if (shadowNode) {
			        shadowNode->GetRuntimeData().cameraPos = camLoc;
			   }
			 #endif*/

			// Camera
			auto niCamera = sceneManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateRenderManager(NiPoint3 camLoc) {
		auto renderManager = UIRenderManager::GetSingleton();
		if (renderManager) {
			/*#ifdef ENABLED_SHADOW
			   // Shadow Map
			   auto shadowNode = renderManager->shadowSceneNode;
			   if (shadowNode) {
			        shadowNode->GetRuntimeData().cameraPos = camLoc;
			   }
			 #endif*/

			// Camera
			auto niCamera = renderManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateNiCamera(NiPoint3 camLoc) {
		auto niCamera = GetNiCamera();
		if (niCamera) {
			niCamera->world.translate = camLoc;
			UpdateWorld2ScreetMat(niCamera);
			update_node(niCamera);
		}

		/*#ifdef ENABLED_SHADOW
		   auto shadowNode = GetShadowMap();
		   if (shadowNode) {
		        shadowNode->GetRuntimeData().cameraPos = camLoc;
		   }
		 #endif*/
	}

	NiTransform GetCameraWorldTransform() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto cameraRoot = camera->cameraRoot;
			if (cameraRoot) {
				return cameraRoot->world;
			}
		}
		return NiTransform();
	}

	void UpdatePlayerCamera(NiPoint3 camLoc) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto cameraRoot = camera->cameraRoot;
			if (cameraRoot) {
				auto cameraState = reinterpret_cast<ThirdPersonState*>(camera->currentState.get());
				cameraRoot->local.translate = camLoc;
				cameraRoot->world.translate = camLoc;
				update_node(cameraRoot.get());
			}
		}
	}

	NiPoint3 GetCameraPosition() {
		NiPoint3 cameraLocation;
		switch (currentMode) {
			case CameraDataMode::State: {
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto currentState = camera->currentState;
					if (currentState) {
						currentState->GetTranslation(cameraLocation);
					}
				}
			}
			case CameraDataMode::Transform: {
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto cameraRoot = camera->cameraRoot;
					if (cameraRoot) {
						cameraLocation = cameraRoot->world.translate;
					}
				}
			}
		}
		return cameraLocation;
	}

	NiMatrix3 GetCameraRotation() {
		NiMatrix3 cameraRot;
		switch (currentMode) {
			case CameraDataMode::State: {
				//log::info("Camera State: State");
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto currentState = camera->currentState;
					if (currentState) {
						NiQuaternion cameraQuat;
						currentState->GetRotation(cameraQuat);
						cameraRot = QuatToMatrix(cameraQuat);
					}
				}
			}
			case CameraDataMode::Transform: {
				//log::info("Camera State: Transform");
				auto camera = PlayerCamera::GetSingleton();
				if (camera) {
					auto cameraRoot = camera->cameraRoot;
					if (cameraRoot) {
						cameraRot = cameraRoot->world.rotate;
					}
				}
			}
		}
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto currentState = camera->currentState;
			if (currentState) {
				NiQuaternion cameraQuat;
				currentState->GetRotation(cameraQuat);
				cameraRot = QuatToMatrix(cameraQuat);
			}
		}
			
		return cameraRot;
	}

	// Get's camera position relative to the player
	NiPoint3 GetCameraPosLocal() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			NiPointer<TESObjectREFR> Target = camera->cameraTarget.get();

			auto currentState = camera->currentState;
			if (currentState) {
				auto player = GetCameraActor();
				if (player) {
					auto model = player->Get3D(false);
					if (model) {
						NiPoint3 cameraLocation = GetCameraPosition();
						auto playerTrans = model->world;
						playerTrans.scale = model->parent ? model->parent->world.scale : 1.0f; // Only do translation/rotation
						auto playerTransInve = playerTrans.Invert();
						// Get Scaled Camera Location
						return playerTransInve*cameraLocation;
					}
				}
			}
		}
		return NiPoint3();
	}

	NiMatrix3 QuatToMatrix(const NiQuaternion& q){
		float sqw = q.w*q.w;
		float sqx = q.x*q.x;
		float sqy = q.y*q.y;
		float sqz = q.z*q.z;

		// invs (inverse square length) is only required if quaternion is not already normalised
		float invs = 1 / (sqx + sqy + sqz + sqw);
		float m00 = ( sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
		float m11 = (-sqx + sqy - sqz + sqw)*invs;
		float m22 = (-sqx - sqy + sqz + sqw)*invs;

		float tmp1 = q.x*q.y;
		float tmp2 = q.z*q.w;
		float m10 = 2.0f * (tmp1 + tmp2)*invs;
		float m01 = 2.0f * (tmp1 - tmp2)*invs;

		tmp1 = q.x*q.z;
		tmp2 = q.y*q.w;
		float m20 = 2.0f * (tmp1 - tmp2)*invs;
		float m02 = 2.0f * (tmp1 + tmp2)*invs;
		tmp1 = q.y*q.z;
		tmp2 = q.x*q.w;
		float m21 = 2.0f * (tmp1 + tmp2)*invs;
		float m12 = 2.0f * (tmp1 - tmp2)*invs;

		return NiMatrix3(
			NiPoint3(m00, m01, m02),
			NiPoint3(m10, m11, m12),
			NiPoint3(m20, m21, m22)
			);
	}

	NiPoint3 FirstPersonPoint() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kFirstPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}
	NiPoint3 ThirdPersonPoint() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kThirdPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}

	float ZoomFactor() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kThirdPerson].get();
		if (camState) {
			ThirdPersonState* tpState = skyrim_cast<ThirdPersonState*>(camState);
			if (tpState) {
				return tpState->currentZoomOffset;
			}
		}
		return 0.0f;
	}
	float MaxZoom() {
		return GetINIFloat("fVanityModeMaxDist:Camera");
	}

	NiPoint3 CompuleLookAt(float zoomScale) {
		NiPoint3 cameraTrans = GetCameraPosition();

		NiMatrix3 cameraRotMat = GetCameraRotation();

		float zoomOffset = ZoomFactor() * MaxZoom() * zoomScale;
		NiPoint3 zoomOffsetVec = NiPoint3(0.0f, zoomOffset, 0.0f);
		return cameraRotMat * zoomOffsetVec + cameraTrans;
	}

	void UpdateCamera(float scale, NiPoint3 cameraLocalOffset, NiPoint3 playerLocalOffset) {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot;
		auto player = GetCameraActor();
		auto currentState = camera->currentState;

		float value = Runtime::GetFloatOr("cameraAlternateX", 1.0f);

		if (cameraRoot) {
			if (currentState) {
				auto cameraWorldTranform = GetCameraWorldTransform();
				NiPoint3 cameraLocation;
				currentState->GetTranslation(cameraLocation);
				if (player) {
					if (scale > 1e-4) {
						auto model = player->Get3D(false);
						if (model) {
							auto playerTrans = model->world;
							playerTrans.scale = model->parent ? model->parent->world.scale : 1.0f;  // Only do translation/rotation
							auto playerTransInve = playerTrans.Invert();

							// Make the transform matrix for our changes
							NiTransform adjustments = NiTransform();
							adjustments.scale = scale;
							// Adjust by scale reports 1.0f / naturalscale (Which includes RaceMenu and GetScale)


							adjustments.translate = playerLocalOffset;

							// Get Scaled Camera Location
							auto targetLocationWorld = playerTrans*(adjustments*(playerTransInve*cameraLocation));

							// Get shifted Camera Location
							cameraWorldTranform.translate = targetLocationWorld; // Update with latest position
							NiTransform adjustments2 = NiTransform();
							adjustments2.translate = cameraLocalOffset * scale;
							auto worldShifted =  cameraWorldTranform * adjustments2 * NiPoint3();

							// Convert to local space
							auto parent = cameraRoot->parent;
							NiTransform transform = parent->world.Invert();
							auto localShifted = transform * worldShifted;
							auto targetLocationLocalShifted = localShifted;

							//PerformRaycastOnCamera(targetLocationLocalShifted, camera); // Doesn't work as intended
							//CameraTest(targetLocationLocalShifted);

							UpdatePlayerCamera(targetLocationLocalShifted);
							UpdateNiCamera(targetLocationLocalShifted);

							//UpdateSceneManager(targetLocationLocalShifted);
							//UpdateRenderManager(targetLocationLocalShifted);
						}
					}
				}
			}
		}
	}
}