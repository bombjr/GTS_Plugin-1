#include "Hooks/Hooks.hpp"
#include "Hooks/Skyrim/HeadTracking_Graph.hpp"
#include "Hooks/Skyrim/PreventAnimations.hpp"
#include "Hooks/Skyrim/ActorEquipManager.hpp"
#include "Hooks/Skyrim/hkbBehaviorGraph.hpp"
#include "Hooks/Skyrim/PlayerCharacter.hpp"
#include "Hooks/Skyrim/ActorRotation.hpp"
#include "Hooks/Skyrim/HeadTracking.hpp"
#include "Hooks/Skyrim/PlayerCamera.hpp"
#include "Hooks/Skyrim/MagicTarget.hpp"
#include "Hooks/Skyrim/Detection.hpp"
#include "Hooks/Skyrim/Projectiles.hpp"
#include "Hooks/Skyrim/CameraState.hpp"
#include "Hooks/Skyrim/D3DPresent.hpp"
#include "Hooks/Skyrim/Character.hpp"
#include "Hooks/Skyrim/Movement.hpp"
#include "Hooks/Skyrim/Controls.hpp"
#include "Hooks/Skyrim/PushAway.hpp"
#include "Hooks/Skyrim/Pushback.hpp"
#include "Hooks/Skyrim/RaceMenu.hpp"
#include "Hooks/Skyrim/Stealth.hpp"
#include "Hooks/Skyrim/Console.hpp"
#include "Hooks/Skyrim/Impact.hpp"
#include "Hooks/Skyrim/Damage.hpp"
#include "Hooks/Skyrim/Actor.hpp"
#include "Hooks/Skyrim/Input.hpp"
#include "Hooks/Skyrim/Havok.hpp"
#include "Hooks/Skyrim/Main.hpp"
#include "Hooks/Skyrim/Sink.hpp"
#include "Hooks/Skyrim/Jump.hpp"
#include "Hooks/Skyrim/VM.hpp"

#include "Hooks/Experiments.hpp"

namespace Hooks
{
	void InstallControls() {

		log::info("Applying Control Hooks...");

		Hook_Controls<ActivateHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ActivateHandler[0]));
		Hook_Controls<JumpHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_JumpHandler[0]));
		Hook_Controls<ShoutHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ShoutHandler[0]));
		Hook_Controls<SneakHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_SneakHandler[0]));
		Hook_Controls<ToggleRunHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ToggleRunHandler[0]));

		// Hooked so it's impossible to sheathe/unsheathe during transition anims
		Hook_Controls<AttackBlockHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_AttackBlockHandler[0]));

		//Hook_Controls<AutoMoveHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_AutoMoveHandler[0]));
		//Hook_Controls<MovementHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_MovementHandler[0]));
		//Hook_Controls<ReadyWeaponHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ReadyWeaponHandler[0]));
		//Hook_Controls<RunHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_RunHandler[0]));
		//Hook_Controls<ThirdPersonState>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_ThirdPersonState[0]));
		//Hook_Controls<SprintHandler>::Hook(REL::Relocation<std::uintptr_t>(RE::VTABLE_SprintHandler[0]));
		
		log::info("Applied Control Hooks");
	}

	void Install(){

		log::info("Applying hooks");

		Trampoline& SKSETrampoline = SKSE::GetTrampoline();
		SKSETrampoline.create(384);

		Hook_VM::Hook();
		Hook_Character::Hook();
		Hook_Projectiles::Hook();      // Experimental stuff with scaling arrows and other projectiles. It works but mostly visually.
		Hook_BGSImpactManager::Hook();
		Hook_PlayerCharacter::Hook();
		Hook_hkbBehaviorGraph::Hook();

		Hook_MainUpdate::Hook(SKSETrampoline);
		Hook_Input::Hook(SKSETrampoline);
		Hook_Renderer::Hook(SKSETrampoline);
		Hook_BGSImpactManager::Hook(SKSETrampoline);
		Hook_Havok::Hook(SKSETrampoline);
		Hook_Actor::Hook(SKSETrampoline);
		Hook_ActorEquipManager::Hook(SKSETrampoline);
		Hook_Sinking::Hook(SKSETrampoline);
		Hook_Jumping::Hook(SKSETrampoline);
		Hook_Damage::Hook(SKSETrampoline);
		Hook_PushBack::Hook(SKSETrampoline);
		Hook_PushAway::Hook(SKSETrampoline);
		Hook_Stealth::Hook(SKSETrampoline);
		Hook_Movement::Hook(SKSETrampoline);
		Hook_HeadTracking::Hook(SKSETrampoline);
		Hook_HeadTrackingGraph::Hook(SKSETrampoline);
		Hook_PreventAnimations::Hook(SKSETrampoline);
		Hook_RaceMenu::Hook(SKSETrampoline);
		Hook_Console::Hook(SKSETrampoline);
		Hook_Detection::Hook(SKSETrampoline);

		HookCameraStates();
		InstallControls();

		//if (REL::Module::IsSE()) { // Used when something is not RE'd yet for AE

		//}

		//Hook_MagicTarget::Hook();
		//Hook_ActorRotation::Hook(SKSETrampoline);
		//Hook_Experiments::Hook(SKSETrampoline);

		log::info("Finished applying hooks");
		log::info("Default Trampoline Used: {}/{} Bytes", SKSETrampoline.allocated_size(), SKSETrampoline.capacity());
		


	}
}