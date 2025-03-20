#pragma once


namespace Hooks {

	/*
	 * Contains Re'd offsets for the values in skyrims ini settings.
	 */

	namespace LOD {

		//SE OFFSET: 141dea048
		static float* fLodDistance = reinterpret_cast<float*>(REL::RelocationID(507568, 378623).address());
		
		//SE OFFSET: 141dea060
		static float* fTalkingDistance = reinterpret_cast<float*>(REL::RelocationID(507570, 378626).address());
	};

	namespace Distance {
		static float* fActivatePickLength = reinterpret_cast<float*>(REL::RelocationID(502527, 370109).address());
		static float* fActivatePickRadius = reinterpret_cast<float*>(REL::RelocationID(502525, 370106).address());
	}

	namespace Camera {

		//SE OFFSET: 141df37d8
		static float* fVanityModeMaxDist = reinterpret_cast<float*>(REL::RelocationID(509878, 382627).address());

		//SE OFFSET: 141df37a8
		static float* fVanityModeMinDist = reinterpret_cast<float*>(REL::RelocationID(509874, 382621).address());

		//SE OFFSET: 141df3868
		static float* fMouseWheelZoomSpeed = reinterpret_cast<float*>(REL::RelocationID(509890, 382645).address());

		//SE OFFSET: 141df3850
		static float* fMouseWheelZoomIncrement = reinterpret_cast<float*>(REL::RelocationID(509888, 382642).address());
	
	};

	/*
	* Contains Re'd offsets for global vars.
	*/

	namespace Time {

		static float GGTM() {
			static float* g_SGTM = reinterpret_cast<float*>(RELOCATION_ID(511883, 388443).address());
			return *g_SGTM;
		}

		static void SGTM(float a_in) {
			static float* g_SGTM = reinterpret_cast<float*>(RELOCATION_ID(511883, 388443).address());
			*g_SGTM = a_in;
			REL::Relocation<decltype(SGTM)> func{ RELOCATION_ID(66989, 68246) };
			return;
		}
	}

}

