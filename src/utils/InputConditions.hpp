#pragma once
#include "utils/actorUtils.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/animation/HugHeal.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"

/*
	Input Conditions.
	If a condition returns true it assumed a bound action can play or some other form of feedback will be displayed.
	eg. A cooldown message.
	If false is returned. Input manager assumes the bound action even if called won't do anyhing.
	effectively the idea is to move all the check logic here and only keep the cooldown/doing the action in the callback.
	
	If true is returned inputmanager assumes the bound trigger will lead to an action and thus will block the relevant key inputs from being passed on to the game.

*/

//---------------------
// ButtCrush
//---------------------

static bool ButtCrushCondition_Start() {
	Actor* target = GetPlayerOrControlled();
	auto player = PlayerCharacter::GetSingleton();

	if (IsPlayerFirstPerson(target) || IsGtsBusy(target) || IsChangingSize(target) || !CanPerformAnimation(target, AnimationCondition::kGrabAndSandwich)) {
		return false;
	}

	auto grabbedActor = Grab::GetHeldActor(target);
	if (grabbedActor && !IsCrawling(target)) { // IF we have someone in hands, allow only when we crawl
		return false;
	}

	if (target->formID != 0x14) {
		if (IsBeingHeld(target, player)) {
			return false;
		}
	}

	return true;
}

static bool ButtCrushCondition_Grow() {
	Actor* target = GetPlayerOrControlled();
	if (IsPlayerFirstPerson(target)) {
		return false;
	}
	if (IsButtCrushing(target) && !IsChangingSize(target) && Runtime::HasPerkTeam(target, "ButtCrush_GrowingDisaster")) {
		return true;
	}
	return false;
}

static bool ButtCrushCondition_Attack() {
	Actor* target = GetPlayerOrControlled();
	if (IsButtCrushing(target)) {
		return true;
	}
	return false;
}


//---------------------
// Cleavage
//---------------------


static bool CleavageCondition() {
	Actor* target = GetPlayerOrControlled();
	if (target) {
		if (Runtime::HasPerkTeam(target, "Breasts_Intro")) {
			Actor* tiny = Grab::GetHeldActor(target);
			if (tiny && IsBetweenBreasts(tiny)) {
				return true;
			}
		}
	}
	return false;
}

//---------------------
// Swipe
//---------------------

static bool SwipeCondition() {
	auto target = PlayerCharacter::GetSingleton();
	if (!CanPerformAnimation(target, AnimationCondition::kStompsAndKicks) || IsGtsBusy(target)) {
		return false;
	}
	if (!target->IsSneaking()) {
		return false;
	}
	return true;
}

//---------------------
// Trample
//---------------------

static bool TrampleCondition() {
	auto target = PlayerCharacter::GetSingleton();
	if (!CanPerformAnimation(target, AnimationCondition::kStompsAndKicks) || IsGtsBusy(target)) {
		return false;
	}
	if (IsCrawling(target) || target->IsSneaking() || IsProning(target)) {
		return false;
	}
	return true;
}

//---------------------
// Hug
//---------------------

static bool HugCondition_Start() {
	auto target = GetPlayerOrControlled();
	if (!CanPerformAnimation(target, AnimationCondition::kHugs)) {
		return false;
	}
	if (IsGtsBusy(target)) {
		return false;
	}
	if (CanDoPaired(target) && !IsSynced(target) && !IsTransferingTiny(target)) {
		return true;
	}
	return false;
}

static bool HugCondition_Action() {
	Actor* target = GetPlayerOrControlled();
	auto huggedActor = HugShrink::GetHuggiesActor(target);
	if (!huggedActor) {
		return false;
	}
	return true;
}

static bool HugCondition_Release() {
	Actor* target = GetPlayerOrControlled();
	auto huggedActor = HugShrink::GetHuggiesActor(target);
	if (!huggedActor || IsHugCrushing(target) || IsHugHealing(target)) {
		return false;
	}
	return true;
}

//------------------
// Size Reseve
//------------------

static bool SizeReserveCondition() {
	auto target = PlayerCharacter::GetSingleton();
	return Runtime::HasPerk(target, "SizeReserve") && Persistent::GetSingleton().GetData(target);
}

//---------------------
// Rappid Grow/Shrink
//---------------------

static bool RappidGrowShrinkCondition() {
	auto target = PlayerCharacter::GetSingleton();

	if (!Runtime::HasPerk(target, "GrowthDesirePerkAug")) {
		return false;
	}
	if (!IsGtsBusy(target) && !IsChangingSize(target)) {
		return true;
	}
	return false;
}


//---------------------
// Shrink Outburst
//---------------------

static bool ShrinkOutburstCondition() {
	auto target = PlayerCharacter::GetSingleton();
	bool DarkArts = Runtime::HasPerk(target, "DarkArts");
	if (!DarkArts) {
		return false; // no perk, do nothing
	}
	return true;
}

//---------------------
// Protect Small Ones
//---------------------

static bool ProtectSmallOnesCondition() {
	auto target = PlayerCharacter::GetSingleton();

	if (CanPerformAnimation(target, AnimationCondition::kOthers)) {
		return true;
	}
	return false;
}



//---------------------------
// Total Control Grow/Shrink
//---------------------------

static bool TotalControlCondition() {
	auto target = PlayerCharacter::GetSingleton();

	if (!Runtime::HasPerk(target, "GrowthDesirePerkAug")) {
		return false;
	}
	return true;
}


//---------------------------
// Kicks
//---------------------------

static bool KickCondition() {
	auto target = PlayerCharacter::GetSingleton();
	if (!CanPerformAnimation(target, AnimationCondition::kStompsAndKicks) || IsGtsBusy(target)) {
		return false;
	}

	if (!target->IsSneaking() && !target->AsActorState()->IsSprinting()) {
		return true;
	}
	return false;
}

//---------------------------
// Stomps
//---------------------------

static bool StompCondition() {
	auto target = PlayerCharacter::GetSingleton();
	if (!CanPerformAnimation(target, AnimationCondition::kStompsAndKicks) || IsGtsBusy(target)) {
		return false;
	}

	return true;
}

//---------------------------
// ThighCrush
//---------------------------

static bool ThighCrushCondition_Start() {
	auto target = PlayerCharacter::GetSingleton();
	if (!CanPerformAnimation(target, AnimationCondition::kGrabAndSandwich)) {
		return false;
	}
	return true;
}

//---------------------------
// ThighSandwich
//---------------------------

static bool ThighSandwitchCondition_Start() {
	auto target = PlayerCharacter::GetSingleton();


	if (!CanPerformAnimation(target, AnimationCondition::kGrabAndSandwich)) {
		return false;
	}
	if (IsGtsBusy(target)) {
		return false;
	}
	if (IsCrawling(target)) {
		return false;
	}

	return true;
}

//---------------------------
// Grab
//---------------------------

static bool GrabCondition_Start() {
	auto target = GetPlayerOrControlled();
	auto grabbedActor = Grab::GetHeldActor(target);
	if (grabbedActor) { //If we have actor, don't pick anyone up.
		return false;
	}
	if (!CanPerformAnimation(target, AnimationCondition::kStompsAndKicks)) {
		return false;
	}
	if (IsGtsBusy(target) || IsEquipBusy(target) || IsTransitioning(target)) {
		return false; // Disallow Grabbing if Behavior is busy doing other stuff.
	}
	return true;
}

static bool GrabCondition_Attack() {
	auto target = GetPlayerOrControlled();

	if (IsGtsBusy(target) && !IsUsingThighAnimations(target)) {
		return false;
	}
	if (!IsStomping(target) && !IsTransitioning(target)) {
		auto grabbedActor = Grab::GetHeldActor(target);
		if (!grabbedActor) {
			return false;
		}
		return true;
	}
	return false;
}

static bool GrabCondition_Vore() {
	auto target = GetPlayerOrControlled();

	if (!CanPerformAnimation(target, AnimationCondition::kVore)) {
		return false;
	}
	if (IsGtsBusy(target) && !IsUsingThighAnimations(target)) {
		return false;
	}

	if (!IsTransitioning(target)) {
		auto grabbedActor = Grab::GetHeldActor(target);
		if (!grabbedActor) {
			return false;
		}
		if (IsInsect(grabbedActor, true) || IsBlacklisted(grabbedActor) || IsUndead(grabbedActor, true)) {
			return false; // Same rules as with Vore
		}
		return true;
	}
	return false;
}

static bool GrabCondition_Throw() {
	auto target = GetPlayerOrControlled();

	if (IsGtsBusy(target) && !IsUsingThighAnimations(target)) {
		return false;
	}

	if (!IsTransitioning(target)) {
		auto grabbedActor = Grab::GetHeldActor(target);
		if (!grabbedActor) {
			return false;
		}
		return true;
	}
	return false;
}


static bool GrabCondition_Release() {
	auto target = GetPlayerOrControlled();

	auto grabbedActor = Grab::GetHeldActor(target);
	if (!grabbedActor) {
		return false;
	}
	if (IsGtsBusy(target) && !IsUsingThighAnimations(target) || IsTransitioning(target)) {
		return false;
	}
	if (!target->AsActorState()->IsWeaponDrawn()) {
		return true;
	}

	return false;
}

static bool GrabCondition_Breasts() {
	auto target = GetPlayerOrControlled();

	auto grabbedActor = Grab::GetHeldActor(target);
	if (!grabbedActor || IsTransitioning(target)) {
		return false;
	}

	return true;
}


//---------------------------
// Vore
//---------------------------

static bool VoreCondition() {
	auto target = GetPlayerOrControlled();

	if (!CanPerformAnimation(target, AnimationCondition::kVore)) {
		return false;
	}
	return true;
}


static inline bool AlwaysBlock() {
	return true;
}

static inline bool NeverBlock() {
	return false;
}
