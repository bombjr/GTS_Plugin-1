#pragma once

/*
	Input Conditions.
	If a condition returns true it assumed a bound action can play or some other form of feedback will be displayed.
	eg. A cooldown message.
	If false is returned. Input manager assumes the bound action even if called won't do anyhing.
	effectively the idea is to move all the check logic here and only keep the cooldown/doing the action in the callback.
	
	If true is returned inputmanager assumes the bound trigger will lead to an action and thus will block the relevant key inputs from being passed on to the game.

*/

//----------------------
// ButtCrush
//---------------------

enum class ActionState {
	kNone = 0,
	kAnimating = 1
};


static bool ButtCrushConditions(ActionState a_state) {
	Actor* target = GetPlayerOrControlled();

	if (
		target->formID == 0x14 
		&& IsFirstPerson()
		|| IsGtsBusy(target)
		|| IsChangingSize(target)
		|| !CanPerformAnimation(target, AnimationCondition::kGrabAndSandwich)
		) {
		return;
	}
	if (IsGtsBusy(target) || IsChangingSize(target) || !CanPerformAnimation(target, AnimationCondition::kGrabAndSandwich)) {
		return;
	}

	//auto grabbedActor = Grab::GetHeldActor(target);
	//if (grabbedActor && !IsCrawling(player)) { // IF we have someone in hands, allow only when we crawl
	//	return;
	//}

	//if (Runtime::HasPerk(player, "ButtCrush_NoEscape")) {
	//	auto& ButtCrush = ButtCrushController::GetSingleton();

	//	std::vector<Actor*> preys = ButtCrush.GetButtCrushTargets(player, 1);
	//	for (auto prey : preys) {
	//		ButtCrush.StartButtCrush(player, prey); // attaches actors to AnimObjectB
	//	}
	//	return;
	//}
	//else if (CanDoButtCrush(player, true) && !Runtime::HasPerk(player, "ButtCrush_NoEscape")) {
	//	float WasteStamina = 100.0f * GetButtCrushCost(player, false);
	//	DamageAV(player, ActorValue::kStamina, WasteStamina);
	//	AnimationManager::StartAnim("ButtCrush_StartFast", player);
	//}
	//else if (!CanDoButtCrush(player, false) && !Runtime::HasPerk(player, "ButtCrush_NoEscape")) {
	//	ButtCrushController::ButtCrush_OnCooldownMessage(player);
	//}




	return false;
}

static bool Action() {
	return false;
}

