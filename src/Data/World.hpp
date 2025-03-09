#pragma once

namespace GTS {

	class World {
		public:
			static const RE::GMatrix3D& WorldToCamera();
			static const RE::NiRect<float>& ViewPort();
			static const float& WorldScale();
			static const float& WorldScaleInverse();
	};
}
