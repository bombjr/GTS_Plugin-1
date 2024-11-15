#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	const float GetGrowthFormula(float a_giantScale, float a_tinyScale, bool a_devourment);

    void VoreMessage_SwallowedAbsorbing(Actor* pred, Actor* prey);
    void VoreMessage_Absorbed(Actor* pred, std::string_view prey);

    void CantVorePlayerMessage(Actor* giant, Actor* tiny, float sizedifference);
    void Vore_AdvanceQuest(Actor* pred, Actor* tiny, bool WasDragon, bool WasGiant);

    VoreInformation GetVoreInfo(Actor* giant, Actor* tiny, float growth_mult);

    void Task_Vore_FinishVoreBuff(const VoreInformation& VoreInfo, int amount_of_tinies, bool Devourment);
    void Task_Vore_StartVoreBuff(Actor* giant, Actor* tiny, int amount_of_tinies);

    void DevourmentBonuses(Actor* Pred, Actor* Prey, bool Digested, float mult);	

    // Make Devourment events apply all bonuses
    void Devourment_Compatibility(Actor* Pred, Actor* Prey, bool Digested);
}