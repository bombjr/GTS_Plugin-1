#pragma once

namespace GTS {
	vector<Actor*> DevourmentAI_FilterList(Actor* a_Pred, const vector<Actor*>& a_PotentialPrey);
	void DevourmentAI_Start(Actor* a_Predator, const vector<Actor*>& a_PotentialPrey);
}