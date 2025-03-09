#pragma once
#include "Data/BasicRecord.hpp"

// Module that holds data that is persistent across saves

namespace GTS {

	//AS OF Version 8 actor data is 100 bytes (0x64) + 4 to store the formid
	//Each actordata cosave entry is thus 104 bytes.

	struct ActorData {

		/// --------- V1
		float PAD_00 = 0.0f; 
		float visual_scale = 1.0f;
		float visual_scale_v = 0.0f;
		float target_scale = 1.0f;
		float max_scale = 65535.0f;

		/// --------- V2
		float half_life = 1.0f;

		/// --------- V3
		float anim_speed = 1.0f;

		/// --------- V4
		float PAD_1C = 0.0f;

		/// --------- V5
		float PAD_20 = 0.0f;
		float PAD_24 = 0.0f;
		float PAD_28 = 0.0f;

		/// --------- V6
		float smt_run_speed = 0.0f;
		float NormalDamage = 1.0f;
		float SprintDamage = 1.0f;
		float FallDamage = 1.0f;
		float HHDamage = 1.0f;
		float PAD_40 = 0.0f;
		float PAD_44 = 0.0f;
		float SizeReserve = 0.0f;

		/// --------- V7
		float target_scale_v = 0.0f;

		/// --------- V8
		float PAD_50 = 0.0f;
		float stolen_attributes = 0.0f;
		float stolen_health = 0.0f;
		float stolen_magick = 0.0f;
		float stolen_stamin = 0.0f;

		/// --------- V9
		//Add New Stuff Here if needed / PAD data is all used up.
		//Must Be Read and written in the same order as found here

		ActorData() = default;
		explicit ActorData(RE::Actor* actor) {}
	};

	class Persistent : public EventListener {

		public:

			virtual void Reset() override {

				std::unique_lock lock(this->_lock);
				this->ActorDataMap.clear();

				// Ensure we reset them back to inital scales
				// if they are loaded into game memory
				// since skyrim only lazy loads actors
				// that are already in memory it won't reload
				// their nif scales otherwise
				for (auto actor : find_actors()) {
					ResetToInitScale(actor);
				}
			}

			virtual void ResetActor(Actor* actor) override;

			virtual std::string DebugName() override {
				return "::Persistent";
			}

			static void OnRevert(SerializationInterface*) {
				GetSingleton().Reset();
			}

			[[nodiscard]] inline static Persistent& GetSingleton() noexcept {
				static Persistent Instance;
				return Instance;
			}

			//SKSE Callbacks
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			void EraseUnloadedPersistentData();


			ActorData* GetActorData(Actor& actor);
			ActorData* GetActorData(Actor* actor);
			ActorData* GetData(TESObjectREFR* refr);
			ActorData* GetData(TESObjectREFR& refr);

			//bool -> 1 byte
			//int -> 4 bytes
			//float -> 4 bytes
			//double -> 8 bytes

			//------ Actor Record Struct
			constexpr static inline uint8_t ActorStructVersion = 8;
			const static inline uint32_t ActorDataRecord = _byteswap_ulong('ACTD');

			//----- Camera
			BasicRecord<int, 'TCST'> TrackedCameraState = 0;

			//----- Crawk/Sneak State
			BasicRecord<bool, 'ECPL'> EnableCrawlPlayer = false;
			BasicRecord<bool, 'ECFL'> EnableCrawlFollower = false;

			//----- Max Size Related
			BasicRecord<float, 'GBPS'> GTSExtraPotionSize = 0.0f;
			BasicRecord<float, 'GTSL'> GTSGlobalSizeLimit = 1.0f;
			BasicRecord<float, 'GMBS'> GTSMassBasedSizeLimit = 0.0f;

			// ---- Quest Progression
			BasicRecord<float, 'QHSR'> HugStealCount = 0.0f;
			BasicRecord<float, 'QSSR'> StolenSize = 0.0f;
			BasicRecord<float, 'QCCR'> CrushCount = 0.0f;
			BasicRecord<float, 'QSTR'> STNCount = 0.0f;
			BasicRecord<float, 'QHCR'> HandCrushed = 0.0f;
			BasicRecord<float, 'QVRR'> VoreCount = 0.0f;
			BasicRecord<float, 'QGCR'> GiantCount = 0.0f;

			// ---- Guide Messages Seen
			BasicRecord<bool, 'MSTC'> MSGSeenTinyCamity = false;
			BasicRecord<bool, 'MSGS'> MSGSeenGrowthSpurt = false;
			BasicRecord<bool, 'MSAG'> MSGSeenAspectOfGTS = false;


		private:

			Persistent() = default;
			mutable std::mutex _lock;
			std::unordered_map<FormID, ActorData> ActorDataMap;

			static void LoadPersistent(SerializationInterface* serde);
			static void SavePersistent(SerializationInterface* serde);

			//Actor Data Load
			static void LoadActorData(SKSE::SerializationInterface* serde, const uint32_t RecordType, const uint32_t RecordVersion);
			static void LoadActorRecordFloat(SKSE::SerializationInterface* serde, float* a_Data, uint32_t RecordVersion, uint32_t MinVersion, float DefaultValue);
			static void DummyReadFloat(SKSE::SerializationInterface* serde);

			//Actor Data Save
			static void WriteActorData(SKSE::SerializationInterface* serde, uint8_t Version);
			static void WriteActorRecordFloat(SKSE::SerializationInterface* serde, const float* Data);
			static void WriteActorRecordFormID(SKSE::SerializationInterface* serde, const FormID* Id);
			static void DummyWriteFloat(SKSE::SerializationInterface* serde);

	};
}
