#pragma once 

namespace GTS {
	std::vector<Actor*> ButtCrushAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_ViablePreyList);
	void ButtCrushAI_Start(Actor* A_Performer, Actor* a_Prey);
}