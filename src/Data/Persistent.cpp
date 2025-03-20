
#include "Data/Persistent.hpp"
#include "Managers/GtsSizeManager.hpp"
#include "Utils/ItemDistributor.hpp"

namespace GTS {

	//-----------------
	// Persistent
	//-----------------

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);
		SizeManager::GetSingleton().Reset();
		DistributeChestItems();
		FixAnimationsAndCamera(); // Call it from ActorUtils, needed to fix Grab anim on save-reload
		LoadPersistent(serde);
	}


	void Persistent::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);
		SavePersistent(serde);
	}

	void Persistent::LoadPersistent(SerializationInterface* serde) {

		std::uint32_t RecordType;
		std::uint32_t RecordSize;
		std::uint32_t RecordVersion;

		auto& Persistent = Persistent::GetSingleton();

		logger::info("De-Serializing Persistent...");

		while (serde->GetNextRecordInfo(RecordType, RecordVersion, RecordSize)) {

			//----- Actor Data Struct
			LoadActorData(serde, RecordType, RecordVersion);
			LoadKillCountData(serde, RecordType, RecordVersion);

			//----- Camera
			Persistent.TrackedCameraState.Load(serde, RecordType, RecordVersion, RecordSize);

			//----- Crawk/Sneak State
			Persistent.EnableCrawlPlayer.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.EnableCrawlFollower.Load(serde, RecordType, RecordVersion, RecordSize);

			//----- Max Size Related
			Persistent.GTSMassBasedSizeLimit.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.GTSExtraPotionSize.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.GTSGlobalSizeLimit.Load(serde, RecordType, RecordVersion, RecordSize);

			// ---- Quest Progression
			Persistent.HugStealCount.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.StolenSize.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.CrushCount.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.STNCount.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.HandCrushed.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.VoreCount.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.GiantCount.Load(serde, RecordType, RecordVersion, RecordSize);

			// ---- Ability Info
			Persistent.MSGSeenTinyCamity.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.MSGSeenGrowthSpurt.Load(serde, RecordType, RecordVersion, RecordSize);
			Persistent.MSGSeenAspectOfGTS.Load(serde, RecordType, RecordVersion, RecordSize);

		}

	}

	void Persistent::SavePersistent(SerializationInterface* serde) {

		auto& Persistent = Persistent::GetSingleton();
		logger::info("Serializing Persistent...");

		//----- Actor Data Struct
		WriteActorData(serde, ActorStructVersion);
		WriteKillCountData(serde, KillCountStructVersion);

		//----- Camera
		Persistent.TrackedCameraState.Save(serde);

		//----- Crawk/Sneak State
		Persistent.EnableCrawlPlayer.Save(serde);
		Persistent.EnableCrawlFollower.Save(serde);

		//----- Max Size Related
		Persistent.GTSMassBasedSizeLimit.Save(serde);
		Persistent.GTSExtraPotionSize.Save(serde);
		Persistent.GTSGlobalSizeLimit.Save(serde);

		// ---- Quest Progression
		Persistent.HugStealCount.Save(serde);
		Persistent.StolenSize.Save(serde);
		Persistent.CrushCount.Save(serde);
		Persistent.STNCount.Save(serde);
		Persistent.HandCrushed.Save(serde);
		Persistent.VoreCount.Save(serde);
		Persistent.GiantCount.Save(serde);

		// ---- Ability Info
		Persistent.MSGSeenTinyCamity.Save(serde);
		Persistent.MSGSeenGrowthSpurt.Save(serde);
		Persistent.MSGSeenAspectOfGTS.Save(serde);
	}
	
	//-----------------
	// ActorData Read
	//-----------------

	void Persistent::LoadActorData(SKSE::SerializationInterface* serde, const uint32_t RecordType, const uint32_t RecordVersion) {

		if (RecordType != ActorDataRecord) {
			return;
		}

		if (RecordVersion < 1) {
			return;
		}

		std::size_t RecordCount = 0;
		serde->ReadRecordData(&RecordCount, sizeof(RecordCount));

		for (; RecordCount > 0; --RecordCount) {
			ActorData Data = {};

			//V1
			//FormID Offset 0x00
			RE::FormID ReadFormID;       //FormID Stored in the Cosave;
			serde->ReadRecordData(&ReadFormID, sizeof(ReadFormID));

			//V1
			DummyReadFloat(serde);                                                          //0x04
			LoadActorRecordFloat(serde, &Data.visual_scale, RecordVersion, 1, 1.0f);        //0x08
			LoadActorRecordFloat(serde, &Data.visual_scale_v, RecordVersion, 1, 0.0f);      //0x1C
			LoadActorRecordFloat(serde, &Data.target_scale, RecordVersion, 1, 1.0f);        //0x10
			LoadActorRecordFloat(serde, &Data.max_scale, RecordVersion, 1, 65535.0f);       //0x14

			//V2
			LoadActorRecordFloat(serde, &Data.half_life, RecordVersion, 2, 1.0f);           //0x18

			//V3
			LoadActorRecordFloat(serde, &Data.anim_speed, RecordVersion, 3, 1.0f);          //0x1C

			//V4
			DummyReadFloat(serde);                                                          //0x20

			//V5
			DummyReadFloat(serde);                                                          //0x24
			DummyReadFloat(serde);                                                          //0x28
			DummyReadFloat(serde);                                                          //0x2C

			//V6
			LoadActorRecordFloat(serde, &Data.smt_run_speed, RecordVersion, 6, 0.0f);       //0x30
			LoadActorRecordFloat(serde, &Data.NormalDamage, RecordVersion, 6, 1.0f);        //0x34
			LoadActorRecordFloat(serde, &Data.SprintDamage, RecordVersion, 6, 1.0f);        //0x38
			LoadActorRecordFloat(serde, &Data.FallDamage, RecordVersion, 6, 1.0f);          //0x3C
			LoadActorRecordFloat(serde, &Data.HHDamage, RecordVersion, 6, 1.0f);            //0x40
			DummyReadFloat(serde);                                                          //0x44
			DummyReadFloat(serde);                                                          //0x48
			LoadActorRecordFloat(serde, &Data.SizeReserve, RecordVersion, 6, 0.0f);         //0x4C

			//V7
			LoadActorRecordFloat(serde, &Data.target_scale_v, RecordVersion, 7, 0.0f);      //0x50

			//V8
			DummyReadFloat(serde);                                                          //0x54
			LoadActorRecordFloat(serde, &Data.stolen_attributes, RecordVersion, 8, 0.0f);   //0x58
			LoadActorRecordFloat(serde, &Data.stolen_health, RecordVersion, 8, 0.0f);       //0x5C
			LoadActorRecordFloat(serde, &Data.stolen_magick, RecordVersion, 8, 0.0f);       //0x60
			LoadActorRecordFloat(serde, &Data.stolen_stamin, RecordVersion, 8, 0.0f);       //0x64

			//Do this last. If we continue early we'll have shifted the read pointer by 4 bytes for the next  read
			//Corrupting any future deserialization
			RE::FormID CorrectedFormID;  //Load order may have changed. This is the New FormID
			if (serde->ResolveFormID(ReadFormID, CorrectedFormID)) {

				log::trace("LoadActorData() Actor Persistent data loaded for FormID {:08X}", ReadFormID);

				if (Actor* ActorForm = TESForm::LookupByID<Actor>(CorrectedFormID)) {
					if (ActorForm) {
						Persistent::GetSingleton().ActorDataMap.insert_or_assign(CorrectedFormID, Data);
					}
				}
				else {
					log::warn("LoadActorData() Actor FormID {:08X} could not be found after loading the save.", CorrectedFormID);
					Persistent::GetSingleton().ActorDataMap.erase(CorrectedFormID);
				}
			}
			else {
				log::warn("LoadActorData() Actor FormID {:08X} could not be resolved. Not adding to ActorDataMap.", ReadFormID);
			}
		}
	}

	void Persistent::LoadActorRecordFloat(SKSE::SerializationInterface* serde, float* a_Data, uint32_t RecordVersion, uint32_t MinVersion, float DefaultValue) {

		if (RecordVersion >= MinVersion) {
			serde->ReadRecordData(a_Data, sizeof(float));
		}
		else {
			*a_Data = DefaultValue;
		}

		if (std::isnan(*a_Data)) {
			*a_Data = DefaultValue;
		}

	}

	void Persistent::DummyReadFloat(SKSE::SerializationInterface* serde) {
		float Dummy;
		serde->ReadRecordData(&Dummy, sizeof(float));
	}

	//-----------------
	// ActorData Write
	//-----------------

	void Persistent::WriteActorData(SKSE::SerializationInterface* serde, const uint8_t Version) {

		const size_t NumOfActorRecords = GetSingleton().ActorDataMap.size();
		
		if (!serde->OpenRecord(ActorDataRecord, Version)) {
			log::critical("Unable to open ActorDataRecord in CoSave. Something is really wrong, your save is probably broken!");
			return;
		}

		serde->WriteRecordData(&NumOfActorRecords, sizeof(NumOfActorRecords));

		for (auto const& [ActorFormID, Data] : GetSingleton().ActorDataMap) {

			//V1
			WriteActorRecordFormID(serde, &ActorFormID);                    //0x00 - FORMID
			DummyWriteFloat(serde);                                         //0x04 - PAD
			WriteActorRecordFloat(serde, &Data.visual_scale);               //0x08
			WriteActorRecordFloat(serde, &Data.visual_scale_v);             //0x1C
			WriteActorRecordFloat(serde, &Data.target_scale);               //0x10
			WriteActorRecordFloat(serde, &Data.max_scale);                  //0x14

			//V2
			WriteActorRecordFloat(serde, &Data.half_life);                  //0x18

			//V3
			WriteActorRecordFloat(serde, &Data.anim_speed);                 //0x1C

			//V4
			DummyWriteFloat(serde);                                         //0x20 - PAD

			//V5
			DummyWriteFloat(serde);                                         //0x24 - PAD
			DummyWriteFloat(serde);                                         //0x28 - PAD
			DummyWriteFloat(serde);                                         //0x2C - PAD

			//V6
			WriteActorRecordFloat(serde, &Data.smt_run_speed);              //0x30
			WriteActorRecordFloat(serde, &Data.NormalDamage);               //0x34
			WriteActorRecordFloat(serde, &Data.SprintDamage);               //0x38
			WriteActorRecordFloat(serde, &Data.FallDamage);                 //0x3C
			WriteActorRecordFloat(serde, &Data.HHDamage);                   //0x40
			DummyWriteFloat(serde);                                         //0x44 - PAD
			DummyWriteFloat(serde);                                         //0x48 - PAD
			WriteActorRecordFloat(serde, &Data.SizeReserve);                //0x4C

			//V7
			WriteActorRecordFloat(serde, &Data.target_scale_v);             //0x50

			//V8
			DummyWriteFloat(serde);                                         //0x54 - PAD
			WriteActorRecordFloat(serde, &Data.stolen_attributes);          //0x58
			WriteActorRecordFloat(serde, &Data.stolen_health);              //0x5C
			WriteActorRecordFloat(serde, &Data.stolen_magick);              //0x60
			WriteActorRecordFloat(serde, &Data.stolen_stamin);              //0x64

			log::trace("Persistent data serialized for Actor FormID {:08X}", ActorFormID);
		}
	}

	void Persistent::WriteActorRecordFloat(SKSE::SerializationInterface* serde, const float* Data) {
		serde->WriteRecordData(Data, sizeof(float));
	}

	void Persistent::WriteActorRecordFormID(SKSE::SerializationInterface* serde, const FormID* Id) {
		serde->WriteRecordData(Id, sizeof(FormID));
	}

	void Persistent::DummyWriteFloat(SKSE::SerializationInterface* serde) {
		constexpr float Dummy = 0.0f;
		serde->WriteRecordData(&Dummy, sizeof(Dummy));
	}

	//-----------------
	// ActorData Other
	//-----------------

	ActorData* Persistent::GetActorData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return this->GetActorData(*actor);
	}

	ActorData* Persistent::GetActorData(Actor& actor) {
		std::unique_lock lock(this->_lock);
		auto key = actor.formID;

		// Lambda to add new ActorData if conditions are met
		auto addActorData = [&]() -> ActorData* {
			if (!actor.Is3DLoaded()) {
				return nullptr;
			}
			if (get_scale(&actor) < 0.0f) {
				return nullptr;
			}
			auto [iter, inserted] = this->ActorDataMap.try_emplace(key, &actor);
			return &(iter->second);
		};

		// Attempt to find the actor's data in the map
		auto it = this->ActorDataMap.find(key);
		if (it != this->ActorDataMap.end()) {
			return &(it->second);
		}

		// ActorData not found; attempt to add it
		return addActorData();
		
	}

	ActorData* Persistent::GetData(TESObjectREFR* refr) {
		if (!refr) {
			return nullptr;
		}
		return this->GetData(*refr);
	}

	ActorData* Persistent::GetData(TESObjectREFR& refr) {
		auto key = refr.formID;
		try {
			if (!this->ActorDataMap.contains(key)) {
				return nullptr;
			}
			return &this->ActorDataMap.at(key);
		}
		catch (const std::out_of_range&) {
			return nullptr;
		}
		
	}

	void Persistent::ResetActor(Actor* actor) {
		// Fired after a TESReset event
		// This event should be when the game attempts to reset their
		// actor values etc when the cell resets
		auto data = this->GetData(actor);
		if (data) {
			data->PAD_00 = 0.0f;
			data->visual_scale = 1.0f; 
			data->target_scale = 1.0f; 
			data->max_scale = 65535.0f;
			data->visual_scale_v = 0.0f;
			data->half_life = 1.0f;
			data->anim_speed = 1.0f;
			data->PAD_1C = 0.0f;
			data->PAD_20 = 0.0f;
			data->PAD_24 = 0.0f;
			data->PAD_28 = 0.0f;
			data->smt_run_speed = 0.0f;
			data->NormalDamage = 1.0f;
			data->SprintDamage = 1.0f;
			data->FallDamage = 1.0f;
			data->HHDamage = 1.0f;
			data->PAD_40 = 0.0f;
			data->PAD_44 = 0.0f;
			data->SizeReserve = 0.0f;
			data->PAD_50 = 0.0f;
			data->stolen_attributes = 0.0f;
			data->stolen_health = 0.0f;
			data->stolen_magick = 0.0f;
			data->stolen_stamin = 0.0f;
		}
		ResetToInitScale(actor);
	}

	void Persistent::EraseUnloadedPersistentData() {
		std::unique_lock lock(_lock);
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
		for (auto it = ActorDataMap.begin(); it != ActorDataMap.end(); ) {
			if (!allowedFormIDs.contains(it->first)) {
				it = ActorDataMap.erase(it);  // erase returns the next iterator.
			}
			else {
				++it;
			}
		}

		logger::critical("All Unloaded actors have beeen purged from persistent.");

	}

	//----------------------
	// KillCountData Read
	//----------------------

	void Persistent::LoadKillCountData(SKSE::SerializationInterface* serde, const uint32_t RecordType, const uint32_t RecordVersion) {

		if (RecordType != KillCountDataRecord) {
			return;
		}

		if (RecordVersion != KillCountStructVersion) {
			return;
		}

		size_t RecordCount = 0;
		uint32_t RecordSize = 0;
		serde->ReadRecordData(&RecordCount, sizeof(size_t));
		serde->ReadRecordData(&RecordSize, sizeof(uint32_t));

		//Killcount data must be as big or larger than the value stored in the cosave otherwise we'll write out of struct bounds and corrupt adjacent memory
		if (sizeof(KillCountData) + sizeof(FormID) < RecordSize || RecordSize == 0) {
			ReportAndExit("KillCountData structure size missmatch, proceeding will lead to broken save data.\nThe Game will now close.");
		}

		for (; RecordCount > 0; --RecordCount) {

			KillCountData Data = {};
			RE::FormID ReadFormID;

			serde->ReadRecordData(&ReadFormID, sizeof(FormID));        //FormID Offset 0x00 (Size 4)
			serde->ReadRecordData(&Data, RecordSize - sizeof(FormID)); //Struct Offset 0x04 (Size 76 As of V1)

			//Do this last. If we continue early we'll have shifted the read pointer by 4 bytes for the next  read
			//Corrupting any future deserialization
			RE::FormID CorrectedFormID;  //Load order may have changed. This is the New FormID
			if (serde->ResolveFormID(ReadFormID, CorrectedFormID)) {

				log::trace("LoadKillCountData() Actor KillCountData loaded for FormID {:08X}", ReadFormID);

				if (Actor* ActorForm = TESForm::LookupByID<Actor>(CorrectedFormID)) {
					if (ActorForm) {
						Persistent::GetSingleton().KillCountDataMap.insert_or_assign(CorrectedFormID, Data);
					}
				}
				else {
					log::warn("LoadKillCountData() Actor FormID {:08X} could not be found after loading the save.", CorrectedFormID);
					Persistent::GetSingleton().KillCountDataMap.erase(CorrectedFormID);
				}
			}
			else {
				log::warn("LoadKillCountData() Actor FormID {:08X} could not be resolved. Not adding to KillCountData.", ReadFormID);
			}
		}
	}

	//----------------------
	// KillCountData Write
	//----------------------

	void Persistent::WriteKillCountData(SKSE::SerializationInterface* serde, const uint8_t Version) {

		const size_t NumOfActorRecords = GetSingleton().KillCountDataMap.size();
		constexpr uint32_t SizeOfDataToWrite = sizeof(KillCountData) + sizeof(FormID);

		if (!serde->OpenRecord(KillCountDataRecord, Version)) {
			log::critical("Unable to open KillCountDataRecord in CoSave. Something is really wrong, your save is probably broken!");
			return;
		}

		serde->WriteRecordData(&NumOfActorRecords, sizeof(size_t));
		serde->WriteRecordData(&SizeOfDataToWrite, sizeof(uint32_t));

		for (auto const& [ActorFormID, Data] : GetSingleton().KillCountDataMap) {

			//V1
			WriteActorRecordFormID(serde, &ActorFormID);                    //0x00 - FORMID
			serde->WriteRecordData(&Data, SizeOfDataToWrite);

			log::trace("Persistent KillCountData serialized for Actor FormID {:08X}", ActorFormID);
		}
	}

	//----------------------
	// KillCountData Other
	//----------------------

	KillCountData* Persistent::GetKillCountData(Actor& actor) {
		std::unique_lock lock(this->_lock);
		auto key = actor.formID;
		auto it = this->KillCountDataMap.find(key);

		if (it != this->KillCountDataMap.end()) {
			return &it->second;
		}

		// Key not found, add new entry
		if (!actor.Is3DLoaded()) {
			return nullptr;
		}
		auto [newIt, inserted] = this->KillCountDataMap.try_emplace(key, &actor);
		return &newIt->second;
	}

	KillCountData* Persistent::GetKillCountData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return this->GetKillCountData(*actor);
	}
}
