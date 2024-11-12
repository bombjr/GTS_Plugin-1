#include "magic/effects/Potions/ExperiencePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

// A potion that gives size experience

namespace Gts {
	std::string ExperiencePotion::GetName() {
		return "ExperiencePotion";
	}

	void ExperiencePotion::OnStart() {
		auto caster = GetCaster();

		if (caster) {
            int RNG = RandomInt(0, 30);
            ModSizeExperience(caster, 1.0f * (1.0f + (RNG * 0.01f)));
			Potion_Penalty(caster);

			if (caster->formID == 0x14) {
				shake_camera(caster, 0.50f, 0.50f);
			}

			float scale = get_visual_scale(caster);

			SpawnCustomParticle(caster, ParticleType::Red, NiPoint3(), "NPC COM [COM ]", scale * 2.0f); // Just some nice visuals
        }
	}
}


