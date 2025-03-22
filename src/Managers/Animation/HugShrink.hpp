#pragma once

namespace GTS {

	struct HugShrinkData {
		public:
			HugShrinkData(TESObjectREFR* tiny, float strength);
			TESObjectREFR* tiny;
			float strength;
	};

	class HugShrink : public EventListener
	{
		public:
			[[nodiscard]] static HugShrink& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void RegisterEvents();
			static void RegisterTriggers();

			static void DetachActorTask(Actor* giant);
			static void ShrinkOtherTask(Actor* giant, Actor* tiny);
			static void AttachActorTask(Actor* giant, Actor* tiny);
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			// Streangth is meant to be for a calculation of
			// escape chance currently unused
			static void HugActor_Actor(Actor* giant, TESObjectREFR* tiny, float strength);
			static void HugActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);


			static void CallRelease(Actor* giant);
			// Get object being held
			static TESObjectREFR* GetHuggiesObj(Actor* giant);
			// Same as `GetHeldObj` but with a conversion to actor if possible
			static Actor* GetHuggiesActor(Actor* giant);

			[[nodiscard]] inline bool IsTinyInDataList(Actor* aTiny) {
				std::unique_lock lock(_lock);
				if (!aTiny) {
					return false;
				}

				for (auto& val : data | views::values) {
					if (val.tiny) {
						if (val.tiny->formID == aTiny->formID) {
							return true;
						}
					}
				}
				return false;
			}

			private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, HugShrinkData> data;
	};
}