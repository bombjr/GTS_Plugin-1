#pragma once

#include "Managers/Cameras/TP/Foot.hpp"

namespace GTS {
	class FootL : public Foot {
		public:
			BoneTarget GetBoneTarget() override;
			virtual NiPoint3 GetFootPos() override;

			private:
			const BoneTarget FootTarget{ .boneNames = {"NPC L Calf [LClf]"} };
	};
}
