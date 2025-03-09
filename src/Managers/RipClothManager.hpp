#pragma once
// Module that handles AttributeValues


namespace GTS {


	class ClothManager : public EventListener {
		public:
			[[nodiscard]] static ClothManager& GetSingleton() noexcept;
			virtual std::string DebugName() override;

			void CheckClothingRip(Actor* a_actor) const;
			static bool ShouldPreventReEquip(Actor* a_actor, RE::TESBoundObject* a_object);
			float ReConstructOffset(Actor* a_actor, float scale) const;

			const float rip_randomOffsetMax = 0.10f;

	};
}
