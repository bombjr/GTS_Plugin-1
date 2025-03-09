#pragma once

namespace GTS {
	std::vector<Actor*> GrabAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PotentialPrey);
	void GrabAI_Start(Actor* a_Performer, Actor* a_Prey);
}
