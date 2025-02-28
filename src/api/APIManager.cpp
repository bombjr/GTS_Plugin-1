#include "API/Impl/SmoothCamAPI.hpp"
#include "APIManager.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {

	APIManager& APIManager::GetSingleton() noexcept {
		static APIManager instance;
		return instance;
	}

	void APIManager::Register() {
		logger::info("Registering Smoothcam API");

		if (!SmoothCamLoaded()) {
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

	//------------------
	// Smoothcam
	//------------------

	void APIManager::ReqControlFromSC() {

		if (SmoothCamLoaded()) {
			if (!SmoothCam->IsCameraEnabled()) {
				//Camera is disabled, We don't need to do anything
				Smoothcam_HaveCamera = false;
			}

			if (!Smoothcam_HaveCamera) {
				auto res = SmoothCam->RequestCameraControl(SKSE::GetPluginHandle());
				if (res == SmoothCamAPI::APIResult::OK || res == SmoothCamAPI::APIResult::AlreadyGiven) {
					Smoothcam_HaveCamera = true;
				}
			}
		}
	}

	void APIManager::RetControlToSC() {
		
		if (SmoothCamLoaded()) {
			if (Smoothcam_HaveCamera) {
				SmoothCam->ReleaseCameraControl(SKSE::GetPluginHandle());
				Smoothcam_HaveCamera = false;
			}
		}
	}
}