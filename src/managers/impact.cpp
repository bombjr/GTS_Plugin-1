#include "managers/animation/Utils/CooldownManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/explosion.hpp"
#include "managers/modevent.hpp"
#include "managers/audio/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "events.hpp"


#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	bool CanDoImpact(Actor* actor, FootEvent kind) { // This function is needed to prevent sound spam from followers at large sizes
		if (IsTeammate(actor) && actor->formID != 0x14) {
			if (get_visual_scale(actor) < 6.0f) {
				return true;
			}
			if (kind == FootEvent::Right) {
				if (IsActionOnCooldown(actor, CooldownSource::Footstep_Right)) {
					return false;
				}
				ApplyActionCooldown(actor, CooldownSource::Footstep_Right);
			} else if (kind == FootEvent::Left) {
				if (IsActionOnCooldown(actor, CooldownSource::Footstep_Left)) {
					return false;
				}
				ApplyActionCooldown(actor, CooldownSource::Footstep_Left);
			}
		} else {
			return true;
		}
		return true;
	}

	FootEvent get_foot_kind(Actor* actor, std::string_view tag) {
		auto profiler = Profilers::Profile("Impact: Get Foot Kind");
		FootEvent foot_kind = FootEvent::Unknown;
		bool is_jumping = actor ? IsJumping(actor) : false;
		bool in_air = actor ? actor->IsInMidair() : false;
		bool hugging = actor ? IsHuggingFriendly(actor) : false; 
		// Hugging is needed to fix missing footsteps once we do friendly release
		// Footsteps aren't seen by the dll without it (because actor is in air)

		bool allow = (!is_jumping || hugging);

		if (matches(tag, ".*Foot.*Left.*") && allow) {
			foot_kind = FootEvent::Left;
		} else if (matches(tag, ".*Foot.*Right.*") && allow) {
			foot_kind = FootEvent::Right;
		} else if (matches(tag, ".*Foot.*Front.*") && allow) {
			foot_kind = FootEvent::Front;
		} else if (matches(tag, ".*Foot.*Back.*") && allow) {
			foot_kind = FootEvent::Back;
		} else if (matches(tag, ".*Jump.*(Down|Land).*") && !in_air) {
			foot_kind = FootEvent::JumpLand;
		}
		return foot_kind;
	}

	std::vector<NiAVObject*> get_landing_nodes(Actor* actor, const FootEvent& foot_kind) {
		auto profiler = Profilers::Profile("Impact: Get Landing Nodes");
		std::vector<NiAVObject*> results;
		const std::string_view left_foot = "NPC L Foot [Lft ]";
		const std::string_view right_foot = "NPC R Foot [Rft ]";
		const std::string_view left_arm = "NPC L Hand [LHnd]";
		const std::string_view right_arm = "NPC R Hand [RHnd]";

		NiAVObject* result;
		switch (foot_kind) {
			case FootEvent::Left:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Right:
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Front:
				result = find_node(actor, left_arm);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_arm);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Back:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::JumpLand:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
		}
		return results;
	}

	void DoExplosionAndSound(Actor* actor, FootEvent kind) {
		Impact impact_data = Impact {
			.actor = actor,
			.kind = kind,
			.scale = get_visual_scale(actor),
			.modifier = 1.0f,
			.nodes = get_landing_nodes(actor, kind),
		};

		EventDispatcher::DoOnImpact(impact_data); // Calls Explosions and sounds. A Must.
	}

	void ApplyPerkBonuses(Actor* actor, float& launch, float& radius) {
		if (actor->AsActorState()->IsWalking()) {
			launch = 0.8f;
		}
		// We already *= sneak damage by 0.70 inside DoSizeDamage() function, no point in doing it here
		if (actor->AsActorState()->IsSprinting()) {
			launch *= 1.150f;
			radius *= 1.175f;
			if (Runtime::HasPerkTeam(actor, "DevastatingSprint")) {
				launch *= 1.250f;
				radius *= 1.275f;
			}
		}
	}

	DamageSource EventToSource(FootEvent kind) {
		DamageSource source = DamageSource::WalkLeft;
		if (kind == FootEvent::Right) {
			source = DamageSource::WalkRight;
		}
		return source;
	}

	void DoJumpLandEffects(Actor* actor) {
		auto& sizemanager = SizeManager::GetSingleton();
		float perk = GetPerkBonus_Basics(actor);
		float fallmod = GetFallModifier(actor);
		
		float damage = sizemanager.GetSizeAttribute(actor, SizeAttribute::JumpFall) * fallmod; // get jump damage boost

		std::string name = std::format("JumpLandT_{}", actor->formID);
		ActorHandle gianthandle = actor->CreateRefHandle();
		double Start = Time::WorldTimeElapsed();
		
		TaskManager::Run(name, [=](auto& progressData) { // Delay it a bit since it often happens in the air
			if (!gianthandle) {
				return false; // end task
			}
			auto giant = gianthandle.get().get();
			double timepassed = Time::WorldTimeElapsed() - Start;

			if (timepassed >= 0.15) {
				DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default * fallmod, 20, 0.25f, FootEvent::Left, 1.0f, DamageSource::CrushedLeft, true);
				DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default * fallmod, 20, 0.25f, FootEvent::Right, 1.0f, DamageSource::CrushedRight, true);

				DoLaunch(giant, 1.20f * perk * fallmod, 1.75f * fallmod, FootEvent::Left);
				DoLaunch(giant, 1.20f * perk * fallmod, 1.75f * fallmod, FootEvent::Right);
				return false;
			}
			return true;
		});
	}

	void DoDamageAndLaunch(Actor* actor, FootEvent kind, float launch, float radius) {
		if (kind != FootEvent::JumpLand) { // If just walking
			DoDamageEffect(actor, Damage_Walk_Defaut, Radius_Walk_Default * radius, 25, 0.25f, kind, 1.25f, EventToSource(kind), true);
			DoLaunch(actor, 1.05f * launch, 1.10f * radius, kind);
		} else { // If jump landing
			DoJumpLandEffects(actor);
		}
	}
}
namespace Gts {
	ImpactManager& ImpactManager::GetSingleton() noexcept {
		static ImpactManager instance;
		return instance;
	}

	void ImpactManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		// Applied when Foot Events such as FootScuffLeft/FootScuffRight and FootLeft/FootRight are seen on Actors
		if (a_event) {
			auto profiler = Profilers::Profile("Impact: HookProcess");
			auto actor = a_event->actor.get().get();

			auto id = a_event->pad04;
			if (id != 10000001) { // .dll sends fake footstep events to fix missing foot sounds during some animations
			    // If it matches that number = we don't want to do anything. Done inside FootStepManager::PlayVanillaFootstepSounds function
				std::string tag = a_event->tag.c_str();
				auto event_manager = ModEventManager::GetSingleton();
				event_manager.m_onfootstep.SendEvent(actor,tag);

				auto kind = get_foot_kind(actor, tag);

				if (CanDoImpact(actor, kind)) { // Prevents earrape and effect spam from followers when they're large
					float launch = 1.0f;
					float radius = 1.0f;
					
					DoExplosionAndSound(actor, kind);
					ApplyPerkBonuses(actor, launch, radius);
					DoDamageAndLaunch(actor, kind, launch, radius);
				}	
			}
		}
	}
}
