#include "Managers/Perks/PerkHandler.hpp"

using namespace GTS;

namespace {

    float Perk_Acceleration_GetBonus(Actor* giant) {
        float bonus = std::clamp(GetGtsSkillLevel(giant) - 75.0f, 0.0f, 25.0f) * 0.01f;
        return 1.05f + bonus;
    }

    float Perk_LifeAbsorption_GetBonus(Actor* giant) {
        float bonus = std::clamp(GetGtsSkillLevel(giant) - 75.0f, 0.0f, 25.0f) * 0.02f;
        return 0.30f + bonus;
    }

    void ManageSpellPerks(const AddPerkEvent& evt) {
        if (evt.actor->formID == 0x14) {
            /*if (evt.perk == Runtime::GetPerk("GTSPerkColossalGrowth")) {
                PrintMessageBox("You've learned to rapidly change the size of yourself and your followers, as well as unlocked an ability to set size limit to any value. To change it, Open the settings and go to the balance page.");
            }*/
            if (evt.perk == Runtime::GetPerk("GTSPerkGrowthDesireAug")) {
                PrintMessageBox("You're now able to grow and shrink yourself manually at will. By default, press L.Shift + 1 or 2. You can affect followers by pressing L.Shift + Left Arrow + Arrow Up, and can also affect self by pressing Left Arrow + Arrow Up");
            }
            if (evt.perk == Runtime::GetPerk("GTSPerkShrinkAdept") && !Runtime::HasSpell(evt.actor, "GTSSpellShrinkBolt")) {
                Runtime::AddSpell(evt.actor, "GTSSpellShrinkBolt");
            }
            if (evt.perk == Runtime::GetPerk("GTSPerkShrinkExpert") && !Runtime::HasSpell(evt.actor, "GTSSpellShrinkStorm")) {
                Runtime::AddSpell(evt.actor, "GTSSpellShrinkStorm");
            }
            if (evt.perk == Runtime::GetPerk("GTSPerkTinyCalamity")) {
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
                    if (evt.perk == Runtime::GetPerk("GTSPerkAcceleration")) {
                        data->PerkBonusSpeed = Perk_Acceleration_GetBonus(actor);
                    }
                }
            }
        }
    }

    void Perks_UpdateAccelerationPerk(TempActorData* data, Actor* giant) {
        if (data && Runtime::HasPerkTeam(giant, "GTSPerkAcceleration")) {
            data->PerkBonusSpeed = Perk_Acceleration_GetBonus(giant);
        }
    }

    void StartStackDecayTask(Actor* giant, float stack_power, TempActorData* data) {
        std::string name = std::format("StackDecay_{}_{}", giant->formID, Time::WorldTimeElapsed());
        ActorHandle gianthandle = giant->CreateRefHandle();
        double Start = Time::WorldTimeElapsed();

        double stack_duration = 300.0;
        if (data) {
            if (data->PerkLifeForceStolen < 0.0f) {
                data->PerkLifeForceStolen = 0.0f;
            }
            if (data->PerkLifeForceStacks < 0) {
                data->PerkLifeForceStacks = 0;
            }

            data->PerkLifeForceStacks += 1;
            data->PerkLifeForceStolen += stack_power;

            //log::info("Life force stolen: {}, added power: {}", data->Perk_lifeForceStolen, stack_power);

            TaskManager::Run(name, [=](auto& progressData) {
                if (!gianthandle) {
                    return false;
                }
                Actor* giantref = gianthandle.get().get();
                double Finish = Time::WorldTimeElapsed();

                if (Finish - Start >= stack_duration) {
                    if (data) {
                        if (data->PerkLifeForceStacks > 0) {
                            data->PerkLifeForceStacks -= 1;
                            data->PerkLifeForceStolen -= stack_power;
                        } else if (data->PerkLifeForceStacks == 0) {
                            return false;
                        }
                    }
                    return false;
                }
                return true;
            });
        }
    }


    void Perks_UpdateLifeForceAbsorptionPerk(TempActorData* data, Actor* giant) {
        if (giant) {
            if (Runtime::HasPerkTeam(giant, "GTSPerkLifeAbsorption")) {
                int stack_limit = 25;
                if (data) {
                    if (data->PerkLifeForceStacks < stack_limit) {
                        StartStackDecayTask(giant, Perk_LifeAbsorption_GetBonus(giant), data);
                    } else {
                        data->PerkLifeForceStacks -= 1;
                        data->PerkLifeForceStolen -= Perk_LifeAbsorption_GetBonus(giant);
                        // Just Refresh it
                        StartStackDecayTask(giant, Perk_LifeAbsorption_GetBonus(giant), data);
                    }
                }
            }
        }
    }
}

namespace GTS {
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
        return "::PerkHandler";
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
                    if (evt.perk == Runtime::GetPerk("GTSPerkAcceleration")) {
                        data->PerkBonusSpeed = 1.0f;
                    }
                    if (evt.perk == Runtime::GetPerk("GTSPerkLifeAbsorption")) {
                        data->PerkLifeForceStolen = 0.0f;
                        data->PerkLifeForceStacks = 0;
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