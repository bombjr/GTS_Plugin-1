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
        const float CurrentScale = get_visual_scale(a_Actor);
        const float MaxScale = get_max_scale(a_Actor);
        const float AspectOfGTS = Ench_Aspect_GetPower(a_Actor) * 100.0f;
        const float DamageResist = (1.0f - AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kHealth)) * 100.f;
        const float Speed = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kSpeedMult) - 1.0f) * 100.f;
        const float JumpHeight = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kJumpingBonus) - 1.0f) * 100.0f;
        const float Damage = (AttributeManager.GetAttributeBonus(a_Actor, ActorValue::kAttackDamageMult) - 1.0f) * 100.0f;
        const float ShrinkResistance = (1.0f - Potion_GetShrinkResistance(a_Actor)) * 100.f;
        const float OnTheEdge = (GetPerkBonus_OnTheEdge(a_Actor, 0.01f) - 1.0f) * 100.f;
        const float BonusHHDamage = (GetHighHeelsBonusDamage(a_Actor, true) - 1.0f) * 100.0f;
        const float VisualProgress = MaxScale < 250.0f ? CurrentScale / MaxScale : 0.0f;

        //--------- Formatted display strings
        const std::string StringScale = hasFlag(a_featureFlags, GTSInfoFeatures::kUnitScale) ? fmt::format(" ({:.2f}x)", CurrentScale) : "";
        const std::string StringReal = hasFlag(a_featureFlags, GTSInfoFeatures::kUnitReal) ? GTS::GetFormatedHeight(a_Actor).c_str() : "";
        const std::string ResultingText = fmt::format("{}{}", StringReal, StringScale);

        //------- Tooltip Descriptions
        const char* TEssence = "Essence Increases your maximum achievable size when the size limit cap is set to \"Skill Based\"\n"
							   "You can gain essence by killing and absorbing dragons when you have the correct perk or by consuming specific potions found all around the world.";

        const char* TBonusSize = "Bonus size shows any temporary increases to your maximum size when gained through potions or other sources.";

        const char* THHDamage = "Extra foot damage done when wearing high heels";

        const char* TShrinkResist = "Shrink Resistance reduces the effectiveness of any shrinking spell and/or effect.";

        const char* TOnTheEdge = "When your health drops below 60%% all growth gained becomes stronger the less health you have.\n"
    							 "Maximum growth gain happens at 10%% HP remaining or lower.";

        const char* TSizeReserve = "This is the amount of size stored by the size reserve perk.\n"
								   "You gain size reserve by eating/absorbing/crushing others";

        const char* TAspectOfGTS = "This is the strength of the Aspect of the Giantess enchantment";

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
                ImGui::Text("%.2fx", BonusSize);
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

            // Only for the player
            if (a_Actor->formID == 0x14) {
                // Essence
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kEssence)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Essence:");
                    ImUtil::Tooltip(TEssence, true);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("+%.2fx", SizeEssense);
                }

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
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.0f%%", AspectOfGTS);
                ImUtil::Tooltip(TAspectOfGTS);
            }

            // Damage Resist
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kDamageResist)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Damage Resist:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", DamageResist);
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

            // Damage Multiplier
            if (hasFlag(a_featureFlags, GTSInfoFeatures::kDamageMultiplier)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Bonus Damage:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f%%", Damage);
            }

            // Stolen Attributes for Size Conversion perk
            if (Runtime::HasPerk(a_Actor, "GTSPerkSizeConversion") && hasFlag(a_featureFlags, GTSInfoFeatures::kStolenAttributes)) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Stored Attributes:");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("+%.2f", StolenAttributes);
            }

            // Soul Vore perk data
            if (Runtime::HasPerk(a_Actor, "GTSPerkFullAssimilation")) {
                // Stolen Health
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kStolenHealth)) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Absorbed Health:");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("+%.2f", StolenHealth);
                }

                // Stolen Magicka
                if (hasFlag(a_featureFlags, GTSInfoFeatures::kStolenMagicka)) {
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
                }
            }

            ImGui::EndTable();
        }
    }
}
