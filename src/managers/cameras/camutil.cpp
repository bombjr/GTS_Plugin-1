#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/cameras/state.hpp"
#include "managers/camera.hpp"
#include "managers/highheel.hpp"
#include "ActionSettings.hpp"
#include "api/APIManager.hpp"
#include "scale/modscale.hpp"
#include "rays/raycast.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "events.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace {
	enum class CameraDataMode {
		State,
		Transform,
	};

	const CameraDataMode currentMode = CameraDataMode::State;

	const BoneTarget GetBoneTarget_Anim(CameraTracking Camera_Anim) {
		switch (Camera_Anim) {
			case CameraTracking::None: {
				return BoneTarget();
			}
			case CameraTracking::Butt: {
				return BoneTarget {
					.boneNames = {"NPC L Butt","NPC R Butt",},
					.zoomScale = ZoomIn_butt,
				};
			}
			case CameraTracking::Knees: {
				return BoneTarget {
					.boneNames = {"NPC L Calf [LClf]","NPC R Calf [RClf]",},
					.zoomScale = ZoomIn_knees,
				};
			}
			case CameraTracking::Breasts_02: {
				return BoneTarget {
					.boneNames = {"L Breast02","R Breast02",},
					.zoomScale = ZoomIn_Breast02,
				};
			}
			case CameraTracking::Thigh_Crush: {
				return BoneTarget {
					.boneNames = {"NPC R PreRearCalf","NPC R Foot [Rft ]","NPC L PreRearCalf","NPC L Foot [Lft ]",},
					.zoomScale = ZoomIn_ThighCrush,
				};
			}
			case CameraTracking::Thigh_Sandwich: {
				return BoneTarget {
					.boneNames = {"AnimObjectA",},
					.zoomScale = ZoomIn_ThighSandwich,
				};
			}
			case CameraTracking::Hand_Right: {
				return BoneTarget {
					.boneNames = {"NPC R Hand [RHnd]",},
					.zoomScale = ZoomIn_RightHand,
				};
			}
			case CameraTracking::Hand_Left: {
				return BoneTarget {
					.boneNames = {"NPC L Hand [LHnd]",},
					.zoomScale = ZoomIn_LeftHand,
				};
			}
			case CameraTracking::Grab_Left: {
				return BoneTarget {
					.boneNames = {"NPC L Finger02 [LF02]",},
					.zoomScale = ZoomIn_GrabLeft,
				};
			}
			case CameraTracking::L_Foot: {
				return BoneTarget {
					.boneNames = {"NPC L Foot [Lft ]",},
					.zoomScale = ZoomIn_LeftFoot,
				};
			}
			case CameraTracking::R_Foot: {
				return BoneTarget {
					.boneNames = {"NPC R Foot [Rft ]",},
					.zoomScale = ZoomIn_RightFoot,
				};
			}
			case CameraTracking::Mid_Butt_Legs: {
				return BoneTarget {
					.boneNames = {"NPC L Butt","NPC R Butt","NPC L Foot [Lft ]","NPC R Foot [Rft ]",},
					.zoomScale = ZoomIn_ButtLegs,
				};
			}
			case CameraTracking::VoreHand_Right: {
				return BoneTarget {
					.boneNames = {"AnimObjectA",},
					.zoomScale = ZoomIn_VoreRight,
				};
			}
			case CameraTracking::Finger_Right: {
				return BoneTarget {
					.boneNames = {"NPC R Finger12 [RF12]",},
					.zoomScale = ZoomIn_FingerRight,
				};
			}
			case CameraTracking::Finger_Left: {
				return BoneTarget {
					.boneNames = {"NPC L Finger12 [LF12]",},
					.zoomScale = ZoomIn_FingerLeft,
				};
			}
			case CameraTracking::ObjectA: {
				return BoneTarget {
					.boneNames = {"AnimObjectA",},
					.zoomScale = ZoomIn_ObjectA,
				};
			}
			case CameraTracking::ObjectB: {
				return BoneTarget {
					.boneNames = {"AnimObjectB",},
					.zoomScale = ZoomIn_ObjectB,
				};
			}
		}
		return BoneTarget();
	}

	const BoneTarget GetBoneTarget_MCM(CameraTracking_MCM Camera_MCM) {
		switch (Camera_MCM) {
			case CameraTracking_MCM::None: {
				return BoneTarget();
			}
			case CameraTracking_MCM::Spine: {
				return BoneTarget {
					.boneNames = {"NPC Spine2 [Spn2]","NPC Neck [Neck]",},
					.zoomScale = ZoomIn_Cam_Spine,
				};
			}
			case CameraTracking_MCM::Clavicle: {
				return BoneTarget {
					.boneNames = {"NPC R Clavicle [RClv]","NPC L Clavicle [LClv]",},
					.zoomScale = ZoomIn_Cam_Clavicle,
				};
			}
			case CameraTracking_MCM::Breasts_01: {
				return BoneTarget {
					.boneNames = {"NPC L Breast","NPC R Breast",},
					.zoomScale = ZoomIn_Cam_Breasts_01,
				};
			}
			case CameraTracking_MCM::Breasts_02: {
				return BoneTarget {
					.boneNames = {"L Breast02","R Breast02",},
					.zoomScale = ZoomIn_Cam_Breasts_02,
				};
			}
			case CameraTracking_MCM::Breasts_03: {
				return BoneTarget {
					.boneNames = {"L Breast03","R Breast03",},
					.zoomScale = ZoomIn_Cam_Breasts_03,
				};
			}
			case CameraTracking_MCM::Neck: {
				return BoneTarget {
					.boneNames = {"NPC Neck [Neck]",},
					.zoomScale = ZoomIn_Cam_Neck,
				};
			}
			case CameraTracking_MCM::Butt: {
				return BoneTarget {
					.boneNames = {"NPC L Butt","NPC R Butt",},
					.zoomScale = ZoomIn_Cam_Butt,
				};
			}
		}
		return BoneTarget();
	}

	NiPoint3 CameraStateToCoords(Actor* giant) {
		int cameraMode = Runtime::GetInt("CameraMode");
		NiPoint3 result = NiPoint3();
		switch (cameraMode) {
			case 3: // Between Foot
				for (auto Foot: {"NPC L Foot [Lft ]", "NPC R Foot [Rft ]"}) {
					auto node = find_node(giant, Foot);
					if (Foot) {
						result += node->world.translate / 2;
					}
				}
			break; 
			case 4: { // Left Foot
				auto node = find_node(giant, "NPC L Foot [Lft ]");
				if (node) {
					result = node->world.translate;
				}
			}
			break;
			case 5: { // Right Foot
				auto node = find_node(giant, "NPC R Foot [Rft ]");
				if (node) {
					result = node->world.translate;
				}
			break;
			}
		}
		return result;
	}

	void ReadBoneTargets(Actor* giant, NiPoint3& point) {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		int MCM_Mode = Runtime::GetInt("AltCameraTarget");
		CameraTracking_MCM Camera_MCM = static_cast<CameraTracking_MCM>(MCM_Mode);
		CameraTracking Camera_Anim = sizemanager.GetTrackedBone(player);

		BoneTarget targets = BoneTarget();

		NiPoint3 FootPos = CameraStateToCoords(giant);
		
		if (FootPos.Length() > 0.0f) {
			point = FootPos;
			// Just update foot coords
		} else {
			if (Camera_Anim != CameraTracking::None) {
				targets = GetBoneTarget_Anim(Camera_Anim);
			} else {
				targets = GetBoneTarget_MCM(Camera_MCM);
			}

			if (!targets.boneNames.empty()) {
				for (auto node_name: targets.boneNames) {
					auto node = find_node_any(giant, node_name);
					if (node) {
						point += node->world.translate / targets.boneNames.size();
					}
				}
			}
		}
	}

	void UpdateNiFrustum(Actor* cameraActor, float hullMult) {
		//TODO Wait For IC 2.0
									
		if (!IsFirstPerson() && !IsFakeFirstPerson()) {
			if (get_visual_scale(cameraActor) < get_natural_scale(cameraActor)) {
				auto niCamera = GetNiCamera();
				if (niCamera) {
					//CamHull Should be the same as FNearDistance
					//TODO Find the Offset For that value and assign CamHull to it.
					//TODO Also Connect this to the camera rotation matrix Z coord. it should be somethin like: camhullSize * (LookUpDownAngle Remaped to something like 0.8-1.0 * min(visual scale, 1.0)).
					//TODO Also Move this to its own func.
					niCamera->GetRuntimeData2().viewFrustum.fNear = camhullSize * hullMult;
				}
			}
		}
	}
}

namespace Gts {

	BoneTarget GetBoneTargets(CameraTracking Camera_Anim, CameraTracking_MCM Camera_MCM) {
		if (Camera_Anim != CameraTracking::None) { // must take priority
			return GetBoneTarget_Anim(Camera_Anim);
		} else {
			return GetBoneTarget_MCM(Camera_MCM);
		}
	}

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
		auto camState = camera->cameraStates[RE::CameraState::kFirstPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}

	NiPoint3 ThirdPersonPoint() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[RE::CameraState::kThirdPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}

	float ZoomFactor() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[RE::CameraState::kThirdPerson].get();
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

									//ReadBoneTargets(cameraActor, rayStart);

									auto hullMult = min(get_visual_scale(cameraActor), 1.0f);
									//UpdateNiFrustum(cameraActor, hullMult);

									//offset Height by camera hull size. Fixes cases where the bone is closer to the ground than the hull size.
									rayStart.z += max(camhullSize * hullMult, 3.0f);

									if (IsDebugEnabled()) {
										DebugAPI::DrawSphere(glm::vec3(rayStart.x, rayStart.y, rayStart.z), 1.0f, 10, { 0.5f, 1.0f, 0.0f, 1.0f }, 10.0f);
									}

									//Cast a ray from the bone to the new camera pos in worldspace as the camera. If the ray hits move the camera to the pos of the hit
									localShifted = ComputeRaycast(rayStart, localShifted, hullMult);

							}

							UpdatePlayerCamera(localShifted);
							UpdateNiCamera(localShifted);
						}
					}
				}
			}
		}
	}
}