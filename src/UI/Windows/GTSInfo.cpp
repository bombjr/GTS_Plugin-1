#include "UI/Windows/GTSInfo.hpp"
#include "UI/ImGui/ImUtil.hpp"
#include "UI/DearImGui/imgui.h"
#include "Managers/Attributes.hpp"
#include "Utils/UnitConverter.hpp"

namespace {
    bool CheckOK(RE::Actor* a_Actor) {
        if (!a_Actor) return false;
        if (!a_Actor->Get3D()) return false;
        if (!a_Actor->Is3DLoaded()) return false;
        return true;
    }
}

namespace GTS {

    void DrawGTSInfo(const GTSInfoFeatures a_featureFlags, RE::Actor* a_Actor) {

        std::ignore = Profilers::Profile("UI: DrawGTSInfo");

        if (!CheckOK(a_Actor)) {
            ImGui::Text("Actor Invalid!");
            return;
        }

        const auto& ActorTransient = Transient::GetSingleton().GetData(a_Actor);
        const auto& ActorPersistent = Persistent::GetSingleton().GetData(a_Actor);
        if (!ActorTransient || !ActorPersistent) {
            ImGui::Text("Actor Invalid!");
            return;
        }

        const auto& AttributeManager = AttributeManager::GetSingleton();
        const auto& Settings = Config::GetUI().StatusWindow;

        float CarryWeight;
        // When in god mode carry weight gets 100x'ed for some reason
        if (a_Actor->formID == 0x14 && IsInGodMode(a_Actor)) {
            CarryWeight = ActorTransient->CarryWeightBoost / 100u;
        }
        else [[likely]] {
            CarryWeight = ActorTransient->CarryWeightBoost;
        }

    	//--------- Transient Data
        const float BonusSize = ActorTransient->PotionMaxSize;
        const float StolenHealth = ActorPersistent->stolen_health;
        const float StolenMagicka = ActorPersistent->stolen_magick;
        const float StolenStamina = ActorPersistent->stolen_stamin;
        const float StolenAttributes = ActorPersistent->stolen_attributes;

        //---------- Persistent Data
        const float SizeReserve = ActorPersistent->SizeReserve;
        const float SizeEssense = Persistent::GetSingleton().GTSExtraPotionSize.value;

        //---------- Other
        const bool MassMode = Config::GetBalance().sSizeMode == "kMassBased";
        const float shrinkResist_PreCalc = 1.0f * Potion_GetShrinkResistance(a_Actor) * Perk_GetSprintShrinkReduction(a_Actor); // to make ShrinkResistance below shorter

        const float CurrentScale = get_visual_scale(a_Actor);
        const float MaxScale = get_max_scale(a_Actor);
        const float AspectOfGTS = Ench_Aspect_GetPower(a_Actor) * 100.0f;
        const float DamageResist = (1.0f - AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kHealth)) * 100.f;
        const float Speed = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kSpeedMult) - 1.0f) * 100.f;
        const float JumpHeight = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kJumpingBonus) - 1.0f) * 100.0f;
        const float Damage = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kAttackDamageMult) - 1.0f) * 100.0f;
        const float ShrinkResistance = (1.0f - shrinkResist_PreCalc) * 100.f;
        const float OnTheEdge = (GetPerkBonus_OnTheEdge(a_Actor, 0.01f) - 1.0f) * 100.f;
        const float BonusHHDamage = (GetHighHeelsBonusDamage(a_Actor, true) - 1.0f) * 100.0f;
        const float VisualProgress = MaxScale < 250.0f ? CurrentScale / MaxScale : 0.0f;

        //--------- Formatted display strings
        const std::string StringScale = hasFlag(a_featureFlags, GTSInfoFeatures::kUnitScale) ? fmt::format(" ({:.2f}x)", CurrentScale) : "";
        const std::string StringReal = hasFlag(a_featureFlags, GTSInfoFeatures::kUnitReal) ? GTS::GetFormatedHeight(a_Actor).c_str() : "";
        const std::string ResultingText = fmt::format("{}{}", StringReal, StringScale);

        //---------Total Max Size Calculation and Text Formating
        const float BonusSize_EssenceAndDragons = SizeEssense;
        const float BonusSize_TempPotionBoost = BonusSize * 100.0f;
        const float BonusSize_AspectOfGiantess = AspectOfGTS;

        std::string TotalSizeBonusCalculation = fmt::format(
            fmt::runtime("Size Essence + Absorbed Dragons: +{:.2f}x\n"
            "Potion Of Heights: +{:.0f}%%\n"
            "Aspect Of Giantess: +{:.0f}%%\n\n"
            "- Size Essence Increases your maximum achievable size when the size limit cap is set to \"Skill Based\"\n"
            "- If Size Gain mode is in \"Mass Mode\", then Essence Bonus is reduced by {:.0f}%% \n" 
            "- You can gain Essence by killing and absorbing dragons when you have the correct perk or by consuming specific potions found all around the world."
            ),
            MassMode ? BonusSize_EssenceAndDragons * MassMode_ElixirPowerMultiplier : BonusSize_EssenceAndDragons * 1.0f,
            BonusSize_TempPotionBoost,
            BonusSize_AspectOfGiantess,
            (1.0f - MassMode_ElixirPowerMultiplier) * 100.0f
        );

        //------- Tooltip Descriptions
        
        const char* TBonusSize = TotalSizeBonusCalculation.c_str();

        const char* TDamageResist = "This is your damage resistance in percentage. Some GTS perks may further increase it.";
        const char* TDamageBonus =  "This is your non-size related damage multiplier. It affects both Physical and Magic damage.";

        const char* THHDamage = "Extra foot damage multiplier when wearing high heels.";

        const char* TShrinkResist = "Shrink Resistance reduces the effectiveness of any shrinking spell and/or effect on you.";

        const char* TOnTheEdge = "When your health drops below 60%%:\n"
                                 "- All growth gained becomes stronger the less health you have.\n"
                                 "- Hostile shrinking is less effective the less health you have.\n\n"
    							 "Maximum effect is achieved at 10%% HP remaining or lower.";

        const char* TSizeReserve = "This is the amount of size stored by the size reserve perk.\n"
								   "You gain size reserve by eating/absorbing/crushing others";

        const char* TAspectOfGTS = "This is the strength of Aspect of the Giantess enchantment\n"
                                    "Aspect Of Giantess affects:\n"
                                    "- Maximal Size, power of Shrink and Size Steal spells\n"
                                    "- Size-Related damage, minimal shrink threshold from quest/balance mode\n"
                                    "- Growth Rate and Growth Chance from Random Growth\n"
                                    "- Power of Shrink Outburst and Hit Growth size gain\n"
                                    "- Shrink resistance towards hostile shrink sources\n\n"
                                    "Enchantment can be obtained from 'Amulet of Giants', which can be randomly found inside boss chests.";
        const char* TStoredAttributes = 
                                    "Stored Attributes are permanent Health/Magicka/Stamina attribute boosts that weren't absorbed by you yet\n"
                                    "They'll be randomly distributed between your three main attributes\n"
                                    "Complete perk requirements to fully absorb them and convert to Absorbed Attributes";
        const char* TAbsorbedAttributes = 
                                    "Absorbed Attributes are permanent Health/Magicka/Stamina attribute boosts of your character\n"
                                    "They're coming from 'Size Conversion' and 'Full Assimilation' perks";

        // Visual Scale (Progress) Bar
        ImUtil::CenteredProgress(VisualProgress, ImVec2(hasFlag(a_featureFlags, GTSInfoFeatures::kAutoSize) ?
            0.0f :
            Settings.fFixedWidth, 0.0f), ResultingText.c_str(), Settings.fSizeBarHeightMult
        );

        // Set up the table with 2 columns: Stat name and value
        if (ImGui::BeginTable("GTSInfoTable", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Hideable)) {

            // Max Size
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kMaxSize)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Max Scale:");
                ImGui::TableSetColumnIndex(1);
                if (MaxScale > 250.0f) {
                    ImGui::TextUnformatted("Infinite");
                }
                else {
                    ImGui::Text("%.2fx", MaxScale);
                }
            }

            // Bonus Size
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kBonusSize)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Size:");
                ImUtil::Tooltip(TBonusSize, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.0f%% + %.2Fx", (BonusSize * 100.0f) + AspectOfGTS, 
                    MassMode ? SizeEssense * MassMode_ElixirPowerMultiplier : SizeEssense * 1.0f
                );
            }

            // Only for the player
            if (a_Actor->formID == 0x14) {

                // Shrink Resist
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kShrinkResist)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Shrink Resist:");
                    ImUtil::Tooltip(TShrinkResist, true);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.1f%%", ShrinkResistance);
                }

                // On The Edge
                if (Runtime::HasPerk(a_Actor, "GTSPerkOnTheEdge") && hasFlag(a_featureFlags, GTSInfoFeatures::kOnTheEdge)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("On The Edge:");
                    ImUtil::Tooltip(TOnTheEdge, true);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.1f%%", OnTheEdge);
                }

                // Size Reserve
                if (Runtime::HasPerk(a_Actor, "GTSPerkSizeReserve") && hasFlag(a_featureFlags, GTSInfoFeatures::kSizeReserve)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Size Reserve:");
                    ImUtil::Tooltip(TSizeReserve, true);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.2fx", SizeReserve);
                }
            }

            // Weight
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kWeight)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Weight:");
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(GTS::GetFormatedWeight(a_Actor).c_str());
            }

            // Aspect of GTS
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kAspect)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Aspect of GTS:");
                ImUtil::Tooltip(TAspectOfGTS, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.0f%%", AspectOfGTS);
                
            }

            // Damage Resist
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kDamageResist)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Damage Reduction:");
                ImUtil::Tooltip(TDamageResist, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", DamageResist);
            }

            // High Heel Damage
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kHeelsBonusDamage) && BonusHHDamage > 0.0f) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("High Heel Damage:");
                ImUtil::Tooltip(THHDamage, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("+%.0f%%", BonusHHDamage);
            }

            // Damage Multiplier
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kDamageMultiplier)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Damage:");
                ImUtil::Tooltip(TDamageBonus, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", Damage);
            }

            // Carry Weight
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kCarryWeight)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Carry Weight:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f", CarryWeight);
            }

            // Speed Multiplier
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kSpeedMultiplier)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Speed:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", Speed);
            }

            // Jump Multiplier
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kJumpMultiplier)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Jump Height:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", JumpHeight);
            }

            // Stolen Attributes for Size Conversion perk
            if (Runtime::HasPerk(a_Actor, "GTSPerkSizeConversion") && hasFlag(a_featureFlags, GTSInfoFeatures::kStolenAttributes)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Stored Attributes:");
                ImUtil::Tooltip(TStoredAttributes, true);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("+%.2f", StolenAttributes);
            }

            // Soul Vore perk data
            if (Runtime::HasPerk(a_Actor, "GTSPerkFullAssimilation")) {
                // Stolen Health
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kStolenHealth)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Absorbed Attributes:");
                    ImUtil::Tooltip(TAbsorbedAttributes, true);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("HP: +%.2f, MP: +%.2f, SP: +%.2f", StolenHealth, StolenMagicka, StolenStamina);
                }

                // Stolen Magicka
                /*if (hasFlag(a_featureFlags, GTSInfoFeatures::kStolenMagicka)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Absorbed Magicka:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("+%.2f", StolenMagicka);
                }

                // Stolen Stamina
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kStolenStamina)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Absorbed Stamina:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("+%.2f", StolenStamina);
                }*/
            }

            ImGui::EndTable();
        }
    }
}
