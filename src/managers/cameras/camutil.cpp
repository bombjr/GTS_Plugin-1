#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "scale/modscale.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;

namespace {
	enum class CameraDataMode {
		State,
		Transform,
	};

	const CameraDataMode currentMode = CameraDataMode::State;

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
		for (auto& child : cameraRoot->GetChildren()) {
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

	static NiTransform GetCameraWorldTransform() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto& cameraRoot = camera->cameraRoot;
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

	static NiPoint3 GetCameraPosition() {
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
					auto& currentState = camera->currentState;
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
					auto& cameraRoot = camera->cameraRoot;
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

	// Get's camera position relative to the cameraActor
	NiPoint3 GetCameraPosLocal() {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			NiPointer<TESObjectREFR> Target = camera->cameraTarget.get();

			auto& currentState = camera->currentState;
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
		auto& cameraRoot = camera->cameraRoot;
		auto cameraActor = GetCameraActor();
		auto& currentState = camera->currentState;

		if (cameraRoot) {
			if (currentState) {
				auto cameraWorldTranform = GetCameraWorldTransform();
				NiPoint3 cameraLocation;
				currentState->GetTranslation(cameraLocation);
				if (cameraActor) {
					if (scale > 1e-4) {
						auto model = cameraActor->Get3D(false);
						if (model) {
							auto playerTrans = model->world;
							playerTrans.scale = model->parent ? model->parent->world.scale : 1.0f;  // Only do translation/rotation
							auto playerTransInve = playerTrans.Invert();

							// Make the transform matrix for our changes
							NiTransform adjustments = NiTransform();
							adjustments.scale = scale;

							// Adjust by scale reports 1.0 / naturalscale (Which includes RaceMenu and GetScale)
							adjustments.translate = playerLocalOffset;

							// Get Scaled Camera Location
							auto targetLocationWorld = playerTrans*(adjustments*(playerTransInve*cameraLocation));

							// Get shifted Camera Location
							cameraWorldTranform.translate = targetLocationWorld; // Update with latest position
							NiTransform adjustments2 = NiTransform();
							adjustments2.translate = cameraLocalOffset * scale;
							auto worldShifted = cameraWorldTranform * adjustments2 * NiPoint3();

							// Convert to local space
							auto parent = cameraRoot->parent;
							NiTransform transform = parent->world.Invert();
							auto localShifted = transform * worldShifted;

							//Fix Camera Collision
							//Anchor to Pelvis bone. Works Fine.
							//This method relies on the camera having a target as we cast from the target to the camera
							//upside this only needs 1 raycast, downside you need to have a target

							if (auto node = find_node_any(cameraActor, "NPC Pelvis [Pelv]")) {

									auto rayStart = node->world.translate;
									auto hullMult = min(get_visual_scale(cameraActor), 1.0f);

									//offset Height by camera hull size. Fixes cases where the bone is closer to the ground than the hull size.
									rayStart.z += max(camhullSize * hullMult, 3.0f);

									//TODO The distances at small scales become so small that we run against the camera fnear clip
                                    //TODO Scale FnearDistance... This will be fun considering mods like IC also mess with it constantly... 
									//TODO Wait For IC to fix this on their end.
									
									if (!IsFirstPerson() && !IsFakeFirstPerson()) {
										auto niCamera = GetNiCamera();
										if (niCamera) {
											//CamHull Should be the same as FNearDistance
											//TODO Find the Offset For that value and assign CamHull to it.
											//TODO Also Connect this to the camera rotation matrix Z coord. it should be somethin like: camhullSize * (LookUpDownAngle Remaped to something like 0.8-1.0 * min(visual scale, 1.0)).
											//TODO Also Move this to its own func.
											niCamera->GetRuntimeData2().viewFrustum.fNear = camhullSize * hullMult;
										}
									}


									if (IsDebugEnabled()) {
										DebugAPI::DrawSphere(glm::vec3(rayStart.x, rayStart.y, rayStart.z), 1.0f, 10, { 0.5f, 1.0f, 0.0f, 1.0f }, 10.0f);
									}

									//Cast a ray from the bone to the new camera pos in worldspace as the camera. If the ray hits move the camera to the pos of the hit
									localShifted = ComputeRaycast(rayStart, localShifted, hullMult);

							}

							UpdatePlayerCamera(localShifted);
							UpdateNiCamera(localShifted);
							UpdateSceneManager(localShifted);
							UpdateRenderManager(localShifted);
						}
					}
				}
			}
		}
	}
}