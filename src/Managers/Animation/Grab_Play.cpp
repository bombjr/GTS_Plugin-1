
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Grab_Throw.hpp"
#include "Managers/OverkillManager.hpp"
#include "Managers/Animation/Grab.hpp"

#include "Managers/Animation/Grab_Play.hpp"

#include "Managers/Rumble.hpp"

#include "Managers/Input/InputManager.hpp"
#include "Utils/InputConditions.hpp"


using namespace GTS;

namespace Triggers {
	void PassAnimation(std::string anim_gts, std::string anim_tiny) {
		Actor* Player = GetPlayerOrControlled();
		Actor* Tiny = Grab::GetHeldActor(Player);
		if (Tiny) {
			AnimationManager::StartAnim(anim_gts, Player);
			AnimationManager::StartAnim(anim_tiny, Tiny);
		}
	}
	void GrabPlay_StartEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Enter", "GrabPlay_Enter_T");
	}
	void GrabPlay_ExitEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Exit", "GrabPlay_Exit_T");
	}
	void GrabPlay_CrushHeavyEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_CrushH", "GrabPlay_CrushH_T");
	}
	void GrabPlay_VoreEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Vore", "GrabPlay_Vore_T");
	}
	void GrabPlay_KissEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Kiss", "GrabPlay_Kiss_T");
	}
	void GrabPlay_KissVoreEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_KissVore", "GrabPlay_KissVore_T");
	}
	void GrabPlay_PokeEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Poke", "GrabPlay_Poke_T");
	}
	void GrabPlay_FlickEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Flick", "GrabPlay_Flick_T");
	}
	void GrabPlay_SandwichEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_Sandwich", "GrabPlay_Sandwich_T");
	}
	void GrabPlay_GrindStartEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_GrindStart", "GrabPlay_GrindStart_T");
	}
	void GrabPlay_GrindStopEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_GrindStop", "GrabPlay_GrindStop_T");
	}
	void GrabPlay_ToBoobsEvent(const ManagedInputEvent& data) {
		PassAnimation("GrabPlay_ToBoobs", "GrabPlay_ToBoobs_T");
	}
}

namespace GTS {
	void Animation_GrabPlay::RegisterEvents() {
		//AnimationManager::RegisterEvent("GtsGrowth_Moan", "Growth", GtsGrowth_Moan);

	}

	void Animation_GrabPlay::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrabPlay_Enter", "GrabPlay", "GTSBEH_Hand_Enter");
		AnimationManager::RegisterTrigger("GrabPlay_Enter_T", "GrabPlay", "GTSBEH_T_Hand_Enter");

		AnimationManager::RegisterTrigger("GrabPlay_Exit", "GrabPlay", "GTSBEH_Hand_Exit");
		AnimationManager::RegisterTrigger("GrabPlay_Exit_T", "GrabPlay", "GTSBEH_T_Hand_Exit");

		AnimationManager::RegisterTrigger("GrabPlay_CrushH", "GrabPlay", "GTSBEH_Hand_Crush_Heavy");
		AnimationManager::RegisterTrigger("GrabPlay_CrushH_T", "GrabPlay", "GTSBEH_T_Hand_Crush_Heavy");

		AnimationManager::RegisterTrigger("GrabPlay_Vore", "GrabPlay", "GTSBEH_Hand_Vore");
		AnimationManager::RegisterTrigger("GrabPlay_Vore_T", "GrabPlay", "GTSBEH_T_Hand_Vore");

		AnimationManager::RegisterTrigger("GrabPlay_KissVore", "GrabPlay", "GTSBEH_Hand_Kiss_Vore");
		AnimationManager::RegisterTrigger("GrabPlay_KissVore_T", "GrabPlay", "GTSBEH_T_Hand_Kiss_Vore");

		AnimationManager::RegisterTrigger("GrabPlay_Kiss", "GrabPlay", "GTSBEH_Hand_Kiss");
		AnimationManager::RegisterTrigger("GrabPlay_Kiss_T", "GrabPlay", "GTSBEH_T_Hand_Kiss");

		AnimationManager::RegisterTrigger("GrabPlay_Poke", "GrabPlay", "GTSBEH_Hand_Poke");
		AnimationManager::RegisterTrigger("GrabPlay_Poke_T", "GrabPlay", "GTSBEH_T_Hand_Poke");

		AnimationManager::RegisterTrigger("GrabPlay_Flick", "GrabPlay", "GTSBEH_Hand_Flick_Heavy");
		AnimationManager::RegisterTrigger("GrabPlay_Flick_T", "GrabPlay", "GTSBEH_T_Hand_Flick_Heavy");

		AnimationManager::RegisterTrigger("GrabPlay_Sandwich", "GrabPlay", "GTSBEH_Hand_Sandwich");
		AnimationManager::RegisterTrigger("GrabPlay_Sandwich_T", "GrabPlay", "GTSBEH_T_Hand_Sandwich");

		AnimationManager::RegisterTrigger("GrabPlay_GrindStart", "GrabPlay", "GTSBEH_Hand_Grind_Start");
		AnimationManager::RegisterTrigger("GrabPlay_GrindStart_T", "GrabPlay", "GTSBEH_T_Hand_Grind_Start");

		AnimationManager::RegisterTrigger("GrabPlay_GrindStop", "GrabPlay", "GTSBEH_Hand_Grind_Stop");
		AnimationManager::RegisterTrigger("GrabPlay_GrindStop_T", "GrabPlay", "GTSBEH_T_Hand_Grind_Stop");

		AnimationManager::RegisterTrigger("GrabPlay_ToBoobs", "GrabPlay", "GTSBEH_Hand_To_Boobs");
		AnimationManager::RegisterTrigger("GrabPlay_ToBoobs_T", "GrabPlay", "GTSBEH_T_Hand_To_Boobs");

		InputManager::RegisterInputEvent("GrabPlay_Start", Triggers::GrabPlay_StartEvent, GrabPlayStartCondition);
		InputManager::RegisterInputEvent("GrabPlay_Exit", Triggers::GrabPlay_ExitEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_CrushHeavy", Triggers::GrabPlay_CrushHeavyEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_Vore", Triggers::GrabPlay_VoreEvent, GrabPlayActionCondition);
		InputManager::RegisterInputEvent("GrabPlay_Kiss", Triggers::GrabPlay_KissEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_KissVore", Triggers::GrabPlay_KissVoreEvent, GrabPlayActionCondition);
		InputManager::RegisterInputEvent("GrabPlay_Poke", Triggers::GrabPlay_PokeEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_Flick", Triggers::GrabPlay_FlickEvent, GrabPlayActionCondition);
		InputManager::RegisterInputEvent("GrabPlay_Sandwich", Triggers::GrabPlay_SandwichEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_GrindStart", Triggers::GrabPlay_GrindStartEvent, GrabPlayActionCondition);
		InputManager::RegisterInputEvent("GrabPlay_GrindStop", Triggers::GrabPlay_GrindStopEvent, GrabPlayActionCondition);

		InputManager::RegisterInputEvent("GrabPlay_ToBoobs", Triggers::GrabPlay_ToBoobsEvent, GrabPlayActionCondition);

	}
}

/* TRIGGERS
----Events for the GTS hand state
            GTSBEH_Hand_Enter
            GTSBEH_Hand_Exit
            GTSBEH_Hand_Crush_Heavy
            GTSBEH_Hand_Vore
            GTSBEH_Hand_Kiss
            GTSBEH_Hand_Kiss_Vore
            GTSBEH_Hand_Poke
            GTSBEH_Hand_Flick_Heavy
            GTSBEH_Hand_Sandwich
            GTSBEH_Hand_Grind_Start
            GTSBEH_Hand_Grind_Stop
            GTSBEH_Hand_To_Boobs

            
    ---- Trigger for the Tiny hand State
            GTSBEH_T_Hand_Enter
            GTSBEH_T_Hand_Exit
            GTSBEH_T_Hand_Crush_Heavy
            GTSBEH_T_Hand_Vore
            GTSBEH_T_Hand_Kiss
            GTSBEH_T_Hand_Kiss_Vore
            GTSBEH_T_Hand_Poke
            GTSBEH_T_Hand_Flick_Heavy
            GTSBEH_T_Hand_Sandwich
            GTSBEH_T_Hand_Grind_Start
            GTSBEH_T_Hand_Grind_Stop
            GTSBEH_T_Hand_To_Boobs

*/

/* EVENTS
GTS_HS_Exit_NoTiny - Triggered when we want to exit an anim after the Tiny is dead
##Facial Anims
GTS_HS_SmileOn - Makes the GTS smile
GTS_HS_SmileOff - Stops the GTS Smiling

###SFX/Cam Stuff
GTS_HS_CamOn - Triggered when we enter the Hand state to change the camera
GTS_HS_CamOff - Triggered when we exit the Hand state to change the camera
GTS_HS_TossTiny - Intro Anim when we throw the tiny upwards
GTS_HS_TinyLand - Intro Anim when the tiny lands

###Poke -- No damage
GTS_HS_Poke_WindUp - When the GTS pulls her arm back to poke
GTS_HS_Poke_Contact - When the poke conects with the tiny
GTS_HS_Poke_TinyLand - When the Tiny falls down in the gts hand

###Flick -- Launched the Tiny out of the GTS's hand
GTS_HS_Flick_Launch - Triggered when the GTS flicks the tiny
GTS_HS_Flick_Ragdoll - Triggered when the tiny is meant to ragdoll after being flicked

###Sandwich  - Used as a Light crush Does Damage
GTS_HS_Sand_WindUp - Triggered when the GTS Raise's her hand
GTS_HS_Sand_Lower - Triggered when the GTS lower the hand to crush
GTS_HS_Sand_Hit - Triggered when the GTS Hits the Tiny
GTS_HS_Sand_Release - Triggered when the GTS lifts her hand from the Tiny

###Fist Crush -- Does Damage, Heavy Crush attack
GTS_HS_Fist_WindUp - Triggered when the GTS Raise's her hand
GTS_HS_Fist_Lower - Triggered when the GTS lower the hand to crush
GTS_HS_Fist_Hit - Triggered when the GTS Hits the Tiny
GTS_HS_Fist_Release - Triggered when the GTS lifts her hand from the Tiny   
 
###Kiss
GTS_HS_Kiss_Start - Used to trigger the Facial anims for kissing
GTS_HS_Kiss_Stop - Used to end the kissing facial anims
GTS_HS_KissSound_Play - Plays the Kiss Sound
GTS_HS_KissSound_Stop - Stops the Kiss Sound - Unused
GTS_HS_K_Vore_OpenMouth - Used to open the GTS's mouth
GTS_HS_K_Vore_CloseMouth - Used to close the GTS's mouth
GTS_HS_K_Vore_TinyYell - ideally makes the tiny yell frightenedly
GTS_HS_K_Vore_TinyMuffle - ideally muffles the tiny's voice when yelling
GTS_HS_K_Vore_SlurpTiny - ideally used to play a sound when slurping the Tiny up
GTS_HS_K_Vore_SlurpTiny_End - ideally used to play a sound  when the gts finishes slurping the tiny
GTS_HS_K_Vore_TinyInMouth - Triggered when the Tiny is fully in the GTS mouth
GTS_HS_K_Vore_SwallowTiny - Triggered when the GTS swallows the tiny

###Vore -- NOT the Kiss Vore
GTS_HS_Vore_OpenMouth - Used to open the GTS's mouth
GTS_HS_Vore_CloseMouth - Used to close the GTS's mouth
GTS_HS_Vore_TinyInMouth - Triggered when the Tiny is fully in the GTS mouth
GTS_HS_Vore_SwallowTiny - Triggered when the GTS swallows the tiny


*/