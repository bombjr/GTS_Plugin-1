#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "magic/effects/TinyCalamity.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "utils/random.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "utils/debug.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	float GetShrinkPenalty(float size) {
		// https://www.desmos.com/calculator/pqgliwxzi2
		SoftPotential launch {
			.k = 0.98f,
			.n = 0.82f,
			.s = 0.70f,
			.a = 0.0f,
		};
		float balance = GameModeManager::GetSingleton().GetBalanceModeInfo(BalanceModeInfo::ShrinkRate_Base) * 1.6f;
		float power = soft_power(size, launch) * balance;
		return power;
	}

	float Aspect_GetEfficiency(float aspect) {
		float k = 0.75f;
		float a = 0.0f;
		float n = 0.85f;
		float s = 1.0f;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		float result = k*pow(s*(aspect-a), n);
		return result;
	}
}

namespace Gts {

	GameModeManager& GameModeManager::GetSingleton() noexcept {
		static GameModeManager instance;
		return instance;
	}

	std::string GameModeManager::DebugName() {
		return "GameModeManager";
	}

	float GameModeManager::GetBalanceModeInfo(BalanceModeInfo info) {
		auto& Persist = Persistent::GetSingleton();
		switch (info) {
			case BalanceModeInfo::SizeGain_Penalty: // 1.0
				return Persist.BalanceMode_SizeGain_Penalty;
			case BalanceModeInfo::ShrinkRate_Base: // 1.0
				return Persist.BalanceMode_ShrinkRate_Base;
			case BalanceModeInfo::ShrinkRate_Combat: // 0.08
				return Persist.BalanceMode_ShrinkRate_Combat;
			break;
		}
			
		return 1.0f;
	}

	void GameModeManager::ApplyGameMode(Actor* actor, const ChosenGameMode& game_mode, const float& GrowthRate, const float& ShrinkRate)  {
		auto profiler = Profilers::Profile("Manager: ApplyGameMode");
		const float EPS = 1e-7f;
		if (game_mode != ChosenGameMode::None) {
			auto player = PlayerCharacter::GetSingleton();
			float natural_scale = get_natural_scale(actor, true);

			float Scale = std::clamp(get_visual_scale(actor) * 0.25f, 1.0f, 10.0f);
			
			float targetScale = get_target_scale(actor);
			float maxScale = get_max_scale(actor);
			
			if (IsFemale(actor)) {
				if (Runtime::GetFloat("MultiplyGameModePC") == 0 && actor == player) {
					Scale = 1.0f;
				}
				if (Runtime::GetFloat("MultiplyGameModeNPC") == 0 && actor != player) {
					Scale = 1.0f;
				}

				switch (game_mode) {
					case ChosenGameMode::Grow: {
						float modAmount = Scale * (0.00010f + (GrowthRate * 0.25f)) * 60 * Time::WorldTimeDelta();
						if (fabs(GrowthRate) < EPS) {
							return;
						}
						if ((targetScale + modAmount) < maxScale) {
							update_target_scale(actor, modAmount, SizeEffectType::kGrow);
						} else if (targetScale < maxScale) {
							set_target_scale(actor, maxScale);
						} // else let spring handle it
						break;
					}
					case ChosenGameMode::Shrink: {
						float modAmount = -(0.00025f + (ShrinkRate * 0.25f) * Scale) * 60 * Time::WorldTimeDelta();
						if (fabs(ShrinkRate) < EPS) {
							return;
						}
						if ((targetScale + modAmount) > natural_scale) {
							update_target_scale(actor, modAmount, SizeEffectType::kShrink);
						} else if (targetScale > natural_scale || targetScale < natural_scale) {
							set_target_scale(actor, natural_scale);
						} // Need to have size restored by someone
						break;
					}
					case ChosenGameMode::Standard: {
						if (actor->IsInCombat()) {
							float modAmount = Scale * (0.00008f + (GrowthRate * 0.17f)) * 60 * Time::WorldTimeDelta();
							if (fabs(GrowthRate) < EPS) {
								return;
							}
							if ((targetScale + modAmount) < maxScale) {
								update_target_scale(actor, modAmount, SizeEffectType::kGrow);
							} else if (targetScale < maxScale) {
								set_target_scale(actor, maxScale);
							} // else let spring handle it
						} else {
							float modAmount = Scale * -(0.00029f + (ShrinkRate * 0.34f)) * 60 * Time::WorldTimeDelta();
							if (fabs(ShrinkRate) < EPS) {
								return;
							}
							if ((targetScale + modAmount) > natural_scale) {
								update_target_scale(actor, modAmount, SizeEffectType::kShrink);
							} else if (targetScale > natural_scale) {
								set_target_scale(actor, natural_scale);
							} // Need to have size restored by someone
						}
						break;
					}
					case ChosenGameMode::StandardNoShrink: {
						if (actor->IsInCombat()) {
							float modAmount = Scale * (0.00008f + (GrowthRate * 0.17f)) * 60 * Time::WorldTimeDelta();
							if (fabs(GrowthRate) < EPS) {
								return;
							}
							if ((targetScale + modAmount) < maxScale) {
								update_target_scale(actor, modAmount * 0.33f, SizeEffectType::kGrow);
							} else if (targetScale < maxScale) {
								set_target_scale(actor, maxScale);
							} // else let spring handle it
						}
						break;
					}
					case ChosenGameMode::CurseOfGrowth: {
						float GtsSkillLevel = GetGtsSkillLevel(actor);                                                   // Based on GTS skill level
						float MaxSize = Runtime::GetFloat("CurseOfGrowthMaxSize");                                       // Slider that determines max size cap.
						float sizelimit = std::clamp(1.0f * (GtsSkillLevel/100.0f * MaxSize), 1.0f, MaxSize);            // Size limit between 1 and [Slider]], based on GTS Skill. Cap is Slider value.
						int Random = RandomInt(0, 20);                                                                   // Randomize power
						int GrowthTimer = RandomInt(0, 6);                                                               // Randomize 're-trigger' delay, kinda
						int StrongGrowthChance = RandomInt(0, 6);                                                        // Self-explanatory
						int MegaGrowth = RandomInt(0, 20);                                                               // A chance to multiply growth again
						float GrowthPower = GtsSkillLevel*0.00240f / Random;                                              // Randomized strength of growth
						static Timer timer = Timer(1.40 * GrowthTimer);                                                  // How often it procs
						if (targetScale >= sizelimit || Random <= 0 || GrowthTimer <= 0) {
							return; // Protections against infinity
						}
						if (timer.ShouldRunFrame()) {
							if (StrongGrowthChance >= 19 && MegaGrowth >= 19.0f) {
								GrowthPower *= 4.0f;                                                                       // Proc super growth if conditions are met
							}
							if (StrongGrowthChance >= 19.0f) {
								GrowthPower *= 4.0f;                                                                       // Stronger growth if procs
								Rumbling::Once("CurseOfGrowth", actor, GrowthPower * 40, 0.10f);
							}
							if (targetScale >= sizelimit) {
								set_target_scale(actor, sizelimit);
							}
							if (((StrongGrowthChance >= 19 && Random >= 19.0f) || (StrongGrowthChance >= 19 && MegaGrowth >= 19.0f)) && Runtime::GetFloat("AllowMoanSounds") == 1.0f) {
								PlayMoanSound(actor, targetScale/4);
								Task_FacialEmotionTask_Moan(actor, 2.0f, "GameMode");
							}
							if (targetScale < maxScale) {
								update_target_scale(actor, GrowthPower, SizeEffectType::kGrow);
								Rumbling::Once("CurseOfGrowth", actor, GrowthPower * 20, 0.10f);
								Runtime::PlaySoundAtNode("growthSound", actor, GrowthPower * 6, 1.0f, "NPC Pelvis [Pelv]");
							}
						}
						break;
					}
					case ChosenGameMode::Quest: {
						float modAmount = -ShrinkRate * Time::WorldTimeDelta();
						if (fabs(ShrinkRate) < EPS) {
							return;
						}

						float Aspect = Ench_Aspect_GetPower(actor);
						float gigantism = Aspect_GetEfficiency(Aspect) * 0.5f;
						float default_scale = natural_scale * (1.0f + gigantism);
						
						if ((targetScale + modAmount) > default_scale) {
							update_target_scale(actor, modAmount, SizeEffectType::kShrink);
						} else if (targetScale > default_scale) {
							set_target_scale(actor, default_scale);
						} // Need to have size restored by something
					}
					break;
				}
			}
		}
	}

	void GameModeManager::GameMode(Actor* actor)  {
		auto profiler = Profilers::Profile("Manager: GameMode");
		if (!actor) {
			return;
		}
		ChosenGameMode gameMode = ChosenGameMode::None;
		float growthRate = 0.0f;
		float shrinkRate = 0.0f;
		int game_mode_int = 0;
		float QuestStage = Runtime::GetStage("MainQuest");
		float BalanceMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_visual_scale(actor);
		float BonusShrink = 7.4f;
		float bonus = 1.0f;
		
		if (BalanceMode >= 2.0f) {
			BonusShrink *= GetShrinkPenalty(scale);
		}

		if (QuestStage < 100.0f || BalanceMode >= 2.0f) {
			if (actor->formID == 0x14 || IsTeammate(actor)) {
				game_mode_int = 6; // QuestMode
				if (QuestStage < 40) {
					shrinkRate = 0.00086f * BonusShrink * 2.0f;
				} else if (QuestStage >= 40 && QuestStage < 80) {
					shrinkRate = 0.00086f * BonusShrink * 1.6f;
				} else if (BalanceMode >= 2.0f && QuestStage >= 80) {
					shrinkRate = 0.00086f * BonusShrink * 1.40f;
				}

				shrinkRate *= Potion_GetShrinkResistance(actor);
				shrinkRate /= (1.0f + Ench_Aspect_GetPower(actor));

				if (Potion_IsUnderGrowthPotion(actor)) {
					shrinkRate *= 0.0f; // prevent shrinking in that case
				} 
				if (HasGrowthSpurt(actor)) {
					shrinkRate *= 0.25f;
				}
				if (actor->IsInCombat() && BalanceMode == 1.0f) {
					shrinkRate = 0.0f;
				} else if (SizeManager::GetSingleton().GetGrowthSpurt(actor) > 0.01f) {
					shrinkRate = 0.0f;
				} else if (actor->IsInCombat() && BalanceMode >= 2.0f) {
					shrinkRate *= GameModeManager::GetSingleton().GetBalanceModeInfo(BalanceModeInfo::ShrinkRate_Combat); // shrink at 6% rate
				}

				if (fabs(shrinkRate) <= 1e-6) {
					game_mode_int = 0; // Nothing to do
				}
			}
		} else if (QuestStage > 100.0f && BalanceMode <= 1.0f) {
			if (actor->formID == 0x14) {
				if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
					bonus = scale * 0.25f + 0.75f;
				}
				game_mode_int = Runtime::GetInt("ChosenGameMode");
				growthRate = Runtime::GetFloat("GrowthModeRate");
				shrinkRate = Runtime::GetFloat("ShrinkModeRate");

			} else if (actor->formID != 0x14 && IsTeammate(actor)) {
				if (Runtime::HasMagicEffect(actor, "EffectSizeAmplifyPotion")) {
					bonus = scale * 0.25f + 0.75f;
				}
				game_mode_int = Runtime::GetInt("ChosenGameModeNPC");
				growthRate = Runtime::GetFloat("GrowthModeRateNPC") * bonus;
				shrinkRate = Runtime::GetFloat("ShrinkModeRateNPC");
			}
		}

		if (game_mode_int >=0 && game_mode_int <= 6) {
			gameMode = static_cast<ChosenGameMode>(game_mode_int);
		}

		shrinkRate *= Perk_GetSprintShrinkReduction(actor); // up to 20% reduction

		GameModeManager::GetSingleton().ApplyGameMode(actor, gameMode, growthRate/2, shrinkRate);
	}
}
