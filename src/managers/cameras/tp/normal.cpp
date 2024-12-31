#include "managers/cameras/tp/normal.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Normal::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraX"),
			0, //Normal::ZOffset,
			Runtime::GetFloat("cameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraX"),
			0, //Normal::ZOffset,
			Runtime::GetFloat("combatCameraY")
			);
	}

	NiPoint3 Normal::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraX"),
			0, //Normal::ZOffset,
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffsetProne(const NiPoint3& cameraPos)  {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraX"),
			0, //Normal::ZOffset,
			Runtime::GetFloat("proneCombatCameraY")
			);
	}

	// fVanityModeMaxDist:Camera Changes The Offset Value We Need So we need to take this value into account;
	void Normal::SetZOff(float Offset) {
		//The 0.15 was found through testing different fVanityModeMaxDist values
		Normal::ZOffset = Offset - (0.15f * Gts::MaxZoom());
	}

	BoneTarget Normal::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		int MCM_Mode = Runtime::GetInt("NormalCameraTarget");
		CameraTracking_MCM Camera_MCM = static_cast<CameraTracking_MCM>(MCM_Mode);
		CameraTracking Camera_Anim = sizemanager.GetTrackedBone(player);

		return GetBoneTargets(Camera_Anim, Camera_MCM);
	}
}
