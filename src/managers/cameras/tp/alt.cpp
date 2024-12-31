#include "managers/cameras/camutil.hpp"
#include "managers/cameras/tp/alt.hpp"
#include "managers/GtsSizeManager.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0, //Alt::ZOffset,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}

	// fVanityModeMaxDist:Camera Changes The Offset Value We Need So we need to take this value into account;
	void Alt::SetZOff(float Offset) {
		// The 0.15 was found through testing different fVanityModeMaxDist values
		Alt::ZOffset = Offset - (0.15f * Gts::MaxZoom());
	}

	BoneTarget Alt::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		int MCM_Mode = Runtime::GetInt("AltCameraTarget");
		CameraTracking_MCM Camera_MCM = static_cast<CameraTracking_MCM>(MCM_Mode);
		CameraTracking Camera_Anim = sizemanager.GetTrackedBone(player);

		return GetBoneTargets(Camera_Anim, Camera_MCM);
	}
}
