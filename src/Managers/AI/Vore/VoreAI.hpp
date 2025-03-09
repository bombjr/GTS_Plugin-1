#pragma once

namespace GTS {

	vector<Actor*> VoreAI_FilterList(Actor* a_Pred, const vector<Actor*>& a_PotentialPrey);
	void VoreAI_StartVore(Actor* a_Predator, const vector<Actor*>& a_PotentialPrey);
}