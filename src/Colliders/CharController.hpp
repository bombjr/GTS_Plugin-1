#pragma once
// Takes a char controller and extracts its collision objects
#include "Colliders/Common.hpp"

namespace GTS {

	

	class CharContData : public ColliderData {
		public:
			CharContData(bhkCharacterController* charCont);
	};
}
