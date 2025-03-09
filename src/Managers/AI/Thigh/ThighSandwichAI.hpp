#pragma once

namespace GTS {
	std::vector<Actor*> ThighSandwichAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PotentialPrey);
	void ThighSandwichAI_DecideAction(Actor* a_Performer, bool a_HavePrey);
	void ThighSandwichAI_Start(Actor* a_Performer, const std::vector<Actor*>& a_PreyList);
}