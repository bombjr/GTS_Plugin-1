#pragma once

namespace GTS {
	std::string_view GetDeathNodeName(DamageSource cause);
	void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause, bool vore_absorbed = false);
}