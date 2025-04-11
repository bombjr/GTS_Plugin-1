
#include "Hooks/Skyrim/Actor.hpp"
#include "Managers/Attributes.hpp"
#include "Managers/Animation/AnimationManager.hpp"

#include "Managers/AI/AIFunctions.hpp"

using namespace GTS;

namespace Hooks {

	void Hook_Actor::Hook(Trampoline& trampoline) {

		log::info("Hooking Actor");

		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_Actor[0] };

		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
		_Move = Vtbl.write_vfunc(REL::Relocate(0x0C8, 0x0C8, 0x0CA), Move);

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_Actor[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
		_SetBaseActorValue = Vtbl5.write_vfunc(0x04, SetBaseActorValue);
	}

	void Hook_Actor::HandleHealthDamage(Actor* a_this, Actor* a_attacker, float a_damage) {
		if (a_attacker) {
			if (Runtime::HasPerkTeam(a_this, "GTSPerkSizeReserveAug1")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
			}
		}
		_HandleHealthDamage(a_this, a_attacker, a_damage);  // Just reports the value, can't override it.
	}

	void Hook_Actor::AddPerk(Actor* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_Actor::RemovePerk(Actor* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_Actor::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight
		float value = _GetActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterGetAv(a_this, a_akValue, value);
				}
				if (a_akValue == ActorValue::kSpeedMult) {
					value = GetNPCSpeedOverride(a_this, value);
				}
			}
		}
		return value;
	}

	float Hook_Actor::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Health
		float value = _GetBaseActorValue(a_owner, a_akValue);
		float bonus = 0.0f;
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					bonus = AttributeManager::AlterGetBaseAv(a_this, a_akValue, value);
				}
			}
		}
		return value + bonus;
	}

	void Hook_Actor::SetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue, float value) {
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterSetBaseAv(a_this, a_akValue, value);
				}
			}
		}
		_SetBaseActorValue(a_owner, a_akValue, value);
	}

	float Hook_Actor::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Damage
		float value = _GetPermanentActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				if (a_akValue == ActorValue::kCarryWeight) {
					value = AttributeManager::AlterGetPermenantAv(a_this, a_akValue, value);
				}
			}
		}
		return value;
	}

	void Hook_Actor::Move(Actor* a_this, float a_arg2, const NiPoint3& a_position) { // Override Movement Speed
		if (a_this->IsInKillMove()) {
			return _Move(a_this, a_arg2, a_position); // Do nothing in Kill moves
		}
		float bonus = AttributeManager::AlterMovementSpeed(a_this, a_position);
		return _Move(a_this, a_arg2, a_position * bonus);
	}
}
