#pragma once

namespace Hooks {

	class Hook_TESCamera {
		public:
			static void Hook();
		private:
			static void Update(TESCamera* a_this);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
