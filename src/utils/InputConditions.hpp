#pragma once

/*
	Input Conditions.
	If a condition returns true it assumed a bound action can play or some other form of feedback will be displayed.
	eg. A cooldown message.
	If false is returned. Input manager assumes the bound action even if called won't do anyhing.
	effectively the idea is to move all the check logic here and only keep the cooldown/doing the action in the callback.
	
	If true is returned inputmanager assumes the bound trigger will lead to an action and thus will block the relevant key inputs from being passed on to the game.

*/

static bool Action() {
	return false;
}

