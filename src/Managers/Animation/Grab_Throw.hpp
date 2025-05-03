#pragma once

namespace GTS {

	class Animation_GrabThrow {
		public:
			static void Throw_Actor(const ActorHandle& giantHandle, const ActorHandle& tinyHandle, NiPoint3 startCoords, NiPoint3 endCoords, std::string_view TaskName, float speedmult = 1.0f);
			static void RegisterEvents();
	};
}