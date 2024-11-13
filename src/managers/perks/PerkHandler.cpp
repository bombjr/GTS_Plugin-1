#include "managers/GtsSizeManager.hpp"
#include "managers/perks/PerkHandler.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
    float Perk_Acceleration_GetBonus(Actor* giant) {
        float bonus = std::clamp(GetGtsSkillLevel(giant) - 75.0f, 0.0f, 25.0f) * 0.01f;
        return 1.05f + bonus;
    }

    float Perk_LifeAbsorption_GetBonus(Actor* giant) {
        float bonus = std::clamp(GetGtsSkillLevel(giant) - 75.0f, 0.0f, 25.0f) * 0.03f;
        return 0.25f + bonus;
    }

    void ManageSpellPerks(const AddPerkEvent& evt) {
        if (evt.actor->formID == 0x14) {
            if (evt.perk == Runtime::GetPerk("ColossalGrowth")) {
                CallHelpMessage();
            }
            if (evt.perk == Runtime::GetPerk("GrowthDesirePerkAug")) {
                PrintMessageBox("You're now able to grow and shrink yourself manually at will. By default, press L.Shift + 1 or 2. You can affect followers by pressing L.Shift + Left Arrow + Arrow Up, and can also affect self by pressing Left Arrow + Arrow Up");
            }
            if (evt.perk == Runtime::GetPerk("FastShrink") && !Runtime::HasSpell(evt.actor, "ShrinkBolt")) {
                Runtime::AddSpell(evt.actor, "ShrinkBolt");
            }
            if (evt.perk == Runtime::GetPerk("LethalShrink") && !Runtime::HasSpell(evt.actor, "ShrinkStorm")) {
                Runtime::AddSpell(evt.actor, "ShrinkStorm");
            }
            if (evt.perk == Runtime::GetPerk("CalamityPerk")) {
                AddCalamityPerk();
            }
        }
    }

    void ManageOtherPerks(const AddPerkEvent& evt) {
        Actor* actor = evt.actor;
        if (actor) {
            if (actor->formID == 0x14 || IsTeammate(actor)) {
                auto data = Transient::GetSingleton().GetActorData(actor);
                if (data) {
                    if (evt.perk == Runtime::GetPerk("Acceleration")) {
                        data->Perk_BonusActionSpeed = Perk_Acceleration_GetBonus(actor);
                    }
                }
            }
        }
    }

    void Perks_UpdateAccelerationPerk(TempActorData* data, Actor* giant) {
        if (data && Runtime::HasPerkTeam(giant, "Acceleration")) {
            data->Perk_BonusActionSpeed = Perk_Acceleration_GetBonus(giant);
        }
    }

    void StartStackDecayTask(Actor* giant, float stack_power, TempActorData* data) {
        std::string name = std::format("StackDecay_{}_{}", giant->formID, Time::WorldTimeElapsed());
        ActorHandle gianthandle = giant->CreateRefHandle();
        double Start = Time::WorldTimeElapsed();

        double stack_duration = 240.0;
        if (!data) {
            return;
        }
        if (data->Perk_lifeForceStolen < 0.0f) {
            data->Perk_lifeForceStolen = 0.0f;
        }
        if (data->Perk_lifeForceStacks < 0) {
            data->Perk_lifeForceStacks = 0;
        }

        data->Perk_lifeForceStacks += 1;
        data->Perk_lifeForceStolen += stack_power;

        //log::info("Life force stolen: {}, added power: {}", data->Perk_lifeForceStolen, stack_power);

        TaskManager::Run(name, [=](auto& progressData) {
            if (!gianthandle) {
                return false;
            }
            Actor* giantref = gianthandle.get().get();
            double Finish = Time::WorldTimeElapsed();

            if (Finish - Start >= stack_duration) {
                if (data) {
                    if (data->Perk_lifeForceStacks > 0) {
                        data->Perk_lifeForceStacks -= 1;
                        data->Perk_lifeForceStolen -= stack_power;
                    } else if (data->Perk_lifeForceStacks == 0) {
                        return false;
                    }
                }
                return false;
            }
            return true;
        });
    }


    void Perks_UpdateLifeForceAbsorptionPerk(TempActorData* data, Actor* giant) {
        if (giant) {
            if (Runtime::HasPerkTeam(giant, "LifeAbsorption")) {
                int stack_limit = 25;
                if (data) {
                    if (data->Perk_lifeForceStacks < stack_limit) {
                        log::info("Stacks < 25");
                        StartStackDecayTask(giant, Perk_LifeAbsorption_GetBonus(giant), data);
                    } else {
                        data->Perk_lifeForceStacks -= 1;
                        data->Perk_lifeForceStolen -= Perk_LifeAbsorption_GetBonus(giant);
                        // Just Refresh it
                        StartStackDecayTask(giant, Perk_LifeAbsorption_GetBonus(giant), data);
                    }
                }
            }
        }
    }
}

namespace Gts {
    PerkHandler& PerkHandler::GetSingleton() noexcept {
        static PerkHandler instance;

        static std::atomic_bool initialized;
        static std::latch latch(1);
        if (!initialized.exchange(true)) {
            latch.count_down();
        }
        latch.wait();

        return instance;
    }

    std::string PerkHandler::DebugName() {
        return "PerkHandler";
    }

    void PerkHandler::OnAddPerk(const AddPerkEvent& evt) {
        ManageSpellPerks(evt);
        ManageOtherPerks(evt);
    }

    void PerkHandler::OnRemovePerk(const RemovePerkEvent& evt) {
        Actor* actor = evt.actor;
        if (actor) {
            if (actor->formID == 0x14 || IsTeammate(actor)) {
                auto data = Transient::GetSingleton().GetActorData(actor);
                if (data) {
                    if (evt.perk == Runtime::GetPerk("Acceleration")) {
                        data->Perk_BonusActionSpeed = 1.0f;
                    }
                    if (evt.perk == Runtime::GetPerk("LifeAbsorption")) {
                        data->Perk_lifeForceStolen = 0.0f;
                        data->Perk_lifeForceStacks = 0;
                    }
                }
            }
        }
    }

    void PerkHandler::UpdatePerkValues(Actor* giant, PerkUpdate Type) {
        if (giant) {
            auto data = Transient::GetSingleton().GetActorData(giant);
            switch (Type) {
                case PerkUpdate::Perk_Acceleration: 
                    Perks_UpdateAccelerationPerk(data, giant);
                break;
                case PerkUpdate::Perk_LifeForceAbsorption:
                    Perks_UpdateLifeForceAbsorptionPerk(data, giant); 
                    // ^ Isn't called with 'KillActor' since it can be funny with Essential actors and grant more than 1 stack at once
                break;
                case PerkUpdate::Perk_None:
                break;
            }
        }
    }
}