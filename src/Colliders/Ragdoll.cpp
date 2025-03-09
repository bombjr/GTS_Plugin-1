#include "Colliders/Ragdoll.hpp"
#include "Colliders/Common.hpp"

namespace GTS {

	RagdollData::RagdollData(hkaRagdollInstance* ragdoll) {
		if (!ragdoll) {
			return;
		}

		for (auto rb: ragdoll->rigidBodies) {
			AddRB(rb);
		}
	}
}
