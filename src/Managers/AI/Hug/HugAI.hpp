#pragma once

namespace GTS {
	std::vector<Actor*> HugAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PotentialPrey);
	void HugAI_Start(Actor* a_Performer, Actor* a_Prey);
}