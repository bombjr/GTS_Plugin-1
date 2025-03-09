#include "Data/Transient.hpp"

namespace GTS {

	Transient& Transient::GetSingleton() noexcept {
		static Transient Instance;
		return Instance;
	}

	TempActorData* Transient::GetData(TESObjectREFR* a_Object) {

		std::unique_lock lock(this->TransientLock);

		if (!a_Object) {
			return nullptr;
		}

		auto ActorKey = a_Object->formID;
		TempActorData* result;

		try {
			result = &this->TempActorDataMap.at(ActorKey);
		}
		catch (const std::exception&) {
			return nullptr;
		}

		return result;
	}

	TempActorData* Transient::GetActorData(Actor* actor) {

		std::unique_lock lock(this->TransientLock);

		if (!actor) {
			return nullptr;
		}

		auto ActorKey = actor->formID;

		try {
			return &this->TempActorDataMap.at(ActorKey);
		}
		catch (const std::out_of_range&) {

			// Try to add
			if (!actor) {
				return nullptr;
			}
			if (get_scale(actor) < 0.0f) {
				return nullptr;
			}

			TempActorData NewTempActorData = TempActorData(actor);
			auto [iter, inserted] = this->TempActorDataMap.try_emplace(ActorKey, NewTempActorData);
			return &(iter->second);
		}
		catch (const exception& e) {
			logger::warn("Transient Exeption GetActorData {}", e.what());
			return nullptr;
		}
	}

	std::vector<FormID> Transient::GetForms() const {
		std::unique_lock lock(this->TransientLock);
		std::vector<FormID> keys;
		keys.reserve(this->TempActorDataMap.size());
		for(const auto data : this->TempActorDataMap | views::keys) {
			keys.push_back(data);
		}
		return keys;
	}


	std::string Transient::DebugName() {
		return "::Transient";
	}

	void Transient::ActorLoaded(RE::Actor* actor) {
		std::unique_lock lock(this->TransientLock);

		if (!actor) {
			return;
		}
		try {
			//Try accessing the element if it doesnt exist. Create it.
			const FormID ActorID = actor->formID;
			std::ignore = this->TempActorDataMap.at(ActorID);
		}
		catch (const std::out_of_range&) {

			// Try to add
			if (!actor) {
				return;
			}

			if (get_scale(actor) < 0.0f) {
				return;
			}

			const FormID ActorKey = actor->formID;
			TempActorData NewTempActorData = TempActorData(actor);
			this->TempActorDataMap.try_emplace(ActorKey, NewTempActorData);
		}
		catch (const exception& e) {
			logger::warn("Transient Exeption ActorLoaded {}",e.what());
		}
	}

	void Transient::Reset() {
		std::unique_lock lock(this->TransientLock);
		log::info("Transient was reset");
		this->TempActorDataMap.clear();
	}

	void Transient::ResetActor(Actor* actor) {
		std::unique_lock lock(this->TransientLock);
		if (actor) {
			auto key = actor->formID;
			this->TempActorDataMap.erase(key);
		}
	}

	void Transient::EraseUnloadedTransientData() {
		std::unique_lock lock(this->TransientLock);

		// Create a set to hold the whitelisted FormIDs.
		std::unordered_set<FormID> allowedFormIDs;

		// Always keep FormID 0x14 (Player).
		allowedFormIDs.insert(0x14);

		// Get preserve all currently loaded actors
		for (const Actor* ActorToNotDelete : find_actors()) {
			if (ActorToNotDelete) {
				allowedFormIDs.insert(ActorToNotDelete->formID);
			}
		}

		// Iterate through ActorDataMap and remove entries whose key is not in allowedFormIDs.
		for (auto it = TempActorDataMap.begin(); it != TempActorDataMap.end(); ) {
			if (!allowedFormIDs.contains(it->first)) {
				it = TempActorDataMap.erase(it);  // erase returns the next iterator.
			}
			else {
				++it;
			}
		}
		logger::critical("All Unloaded actors have beeen purged from transient.");
	}
}
