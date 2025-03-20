#pragma once
// Module that holds data that is not persistent across saves

namespace GTS {
	struct KillCountData { // Once done, order of data CANNOT be changed
		// But we can expand the data by adding new data at the end of current 
		uint16_t iEntrySize = sizeof(KillCountData); // Not used in Data

		int iTotalKills = 0; // Total kill count when we don't expand all the info

		int iShrunkToNothing = 0; // Mostly with spells
		int iOtherSources = 0; 
		//^  Colliding with someone with Tiny Calamity (leads to exploding the tiny) 
		// or inflicting too much damage with weapons when size difference is gigantic (also explodes tiny)

		//Breast data
		int iBreastAbsorbed = 0;
		int iBreastCrushed = 0;
		// Hug Data
		int iHugCrushed = 0;
		// Grab Data
		int iGrabCrushed = 0;
		// Butt Crush Data
		int iButtCrushed = 0;
		//Thigh Sandwich/Crush
		int iThighCrushed = 0;
		int iThighSuffocated = 0; // When dying from DOT damage under thighs
		int iThighSandwiched = 0; // When dying from Thigh Sandwich
		int iThighGrinded = 0; // We with Nick plan Thigh Grind anim, so it may be used later

		int iFingerCrushed = 0;

		int iErasedFromExistence = 0; // Wrathful Calamity Finisher

		int iAbsorbed = 0; // Unused for now, may be useful later
		int iCrushed = 0; // Used in most crush sources
		int iEaten = 0; // When fully voring someone

		int iKicked = 0; // Kicked and crushed to death at same time
		int iGrinded = 0; // Grinded to death
	};
	struct TempActorData {

		float BaseHeight = 0.0f;
		float SMTBonusDuration = 0.0f;
		float SMTPenaltyDuration = 0.0f;
		float CarryWeightBoost = 0.0f;
		float HealthBoost = 0.0f;
		float FallTimer = 1.0f;
		float HugAnimationSpeed = 1.0f;
		float ThrowSpeed = 0.0f;
		float PotionMaxSize = 0.0f;
		float ButtCrushMaxSize = 0.0f;
		float ButtCrushStartScale = 0.0f;
		float SizeVulnerability = 0.0f;
		float PushForce = 1.0f;
		float OtherScales = 1.0f;
		float VoreRecordedScale = 1.0f;
		float WorldFOVDefault = 0.0f;
		float FPFOVDefault = 0.0f;
		float ButtCrushGrowthAmount = 0.0f;
		float MovementSlowdown = 1.0f;
		float ShrinkResistance = 0.0f;
		float MightValue = 0.0f;
		float ShrinkTicks = 0.0f;
		float ShrinkTicksCalamity = 0.0f;
		float PerkBonusSpeed = 1.0f;
		float PerkLifeForceStolen = 0.0f;
		float ClothRipLastScale = -1.0f;
		float ClothRipOffset = -1.0f;
		bool TemporaryDamageImmunity = false;
		float ShrinkUntil = 0.0f;
		float BreastSizeBuff = 0.0f;

		int PerkLifeForceStacks = 0;
		int CrushedTinies = 0;

		bool ThrowWasThrown = false;
		bool CanDoVore = true;
		bool CanBeCrushed = true;
		bool CanBeVored = true;
		bool BeingHeld = false;
		bool BetweenBreasts = false;
		bool AboutToBeEaten = false;
		bool DragonWasEaten = false;
		bool BeingFootGrinded = false;
		bool SMTReachedMaxSpeed = false;
		bool OverrideCamera = false;
		bool WasReanimated = false;
		bool FPCrawling = false;
		bool FPProning = false;
		bool Protection = false;
		bool GrowthPotion = false;
		bool DevourmentDevoured = false;
		bool DevourmentEaten = false;
		bool WasSneaking = false;
		bool EmotionModifierBusy = false;
		bool EmotionPhonemeBusy = false;
		bool ImmuneToBreastOneShot = true;
		bool IsSlowGrowing = false;

		bool ReattachingTiny = false;

		NiPoint3 BoundingBoxCache = { 0.0f, 0.0f, 0.0f };
		NiPoint3 POSLastLegL = { 0.0f, 0.0f, 0.0f };
		NiPoint3 POSLastLegR = { 0.0f, 0.0f, 0.0f };
		NiPoint3 POSLastHandL = { 0.0f, 0.0f, 0.0f };
		NiPoint3 POSLastHandR = { 0.0f, 0.0f, 0.0f };

		Actor* IsInControl = nullptr;

		TESObjectREFR* DisableColissionWith = nullptr;
		TESObjectREFR* ThrowOffender = nullptr;

		AttachToNode AttachmentNode = AttachToNode::None;
		BusyFoot FootInUse = BusyFoot::None;

		Timer GameModeIntervalTimer = Timer(0);
		Timer ActionTimer = Timer(0);

		std::vector<Actor*> shrinkies;

		KillCountData KillData;


		explicit TempActorData(Actor* a_Actor) {
			const auto _BoundValues = get_bound_values(a_Actor);
			const auto _Scale = get_scale(a_Actor);

			BaseHeight = unit_to_meter(_BoundValues[2] * _Scale);
			BoundingBoxCache = _BoundValues;
		}
	};

	class Transient : public EventListener {

		public:

			[[nodiscard]] static Transient& GetSingleton() noexcept;

			TempActorData* GetData(TESObjectREFR* a_Object);
			TempActorData* GetActorData(Actor* actor);
			std::vector<FormID> GetForms() const;

			virtual std::string DebugName() override;
			virtual void ActorLoaded(RE::Actor* actor) override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			void EraseUnloadedTransientData();

		private:

			mutable std::mutex TransientLock;
			std::unordered_map<FormID, TempActorData> TempActorDataMap;
	};
}
