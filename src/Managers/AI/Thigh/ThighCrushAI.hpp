#pragma once

namespace GTS {
	std::vector<Actor*> ThighCrushAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PreyList);
	void ThighCrushAI_Start(Actor* a_Performer);
}