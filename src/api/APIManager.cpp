#include "SmoothCamAPI.hpp"
#include "APIManager.hpp"

using namespace RE;
using namespace SKSE;
namespace {
	static inline SmoothCamAPI::IVSmoothCam3* SmoothCam = nullptr;
	static bool Smoothcam_HaveCamera = false;
	static bool Smoothcam_Enable = true;
}
namespace Gts {
	bool SmoothCamLoaded() {
		return SmoothCam != nullptr && Smoothcam_Enable;
	}

	void RegisterAPIs() {

		logger::info("Registering Smoothcam API");
		if (!SmoothCam) {
			if (!SmoothCamAPI::RegisterInterfaceLoaderCallback(SKSE::GetMessagingInterface(), [](void* interfaceInstance, SmoothCamAPI::InterfaceVersion interfaceVersion) {
				if (interfaceVersion >= SmoothCamAPI::InterfaceVersion::V3) {
					SmoothCam = reinterpret_cast<SmoothCamAPI::IVSmoothCam3*>(interfaceInstance);
					logger::info("Obtained SmoothCamAPI");
				}
				else {
					logger::warn("Unable to acquire requested SmoothCamAPI interface version");
				}
				})) {
				logger::warn("SmoothCamAPI::RegisterInterfaceLoaderCallback reported an error");
			}
			if (!SmoothCamAPI::RequestInterface(
				SKSE::GetMessagingInterface(),
				SmoothCamAPI::InterfaceVersion::V3)) {
				//Set back to null incase it got set but requesting the interface failed.
				SmoothCam = nullptr;
				logger::warn("SmoothCamAPI::RequestInterface reported an error");
			}
		}
	}

	bool ReqControlFromSC() {

		if (SmoothCam && Smoothcam_Enable) {
			if (!SmoothCam->IsCameraEnabled()) {
				//Camera is disabled, We don't need to do anything
				Smoothcam_HaveCamera = false;
				return false;
			}

			if (!Smoothcam_HaveCamera) {
				auto res = SmoothCam->RequestCameraControl(SKSE::GetPluginHandle());
				if (res == SmoothCamAPI::APIResult::OK || res == SmoothCamAPI::APIResult::AlreadyGiven) {
					Smoothcam_HaveCamera = true;
				}
			}
		}

		return Smoothcam_HaveCamera;
	}

	void RetControlToSC() {
		if (SmoothCam && Smoothcam_Enable) {

			if (!SmoothCam->IsCameraEnabled()) {
				//Camera is disabled, We don't need to do anything
				Smoothcam_HaveCamera = false;
				return;
			}

			if (Smoothcam_HaveCamera) {
				auto res = SmoothCam->ReleaseCameraControl(SKSE::GetPluginHandle());
				Smoothcam_HaveCamera = false;
			}
		}
	}
}