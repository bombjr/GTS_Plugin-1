#include "Papyrus/Papyrus.hpp"
#include "Papyrus/Plugin.hpp"
#include "papyrus/Scale.hpp"
#include "papyrus/Height.hpp"
#include "papyrus/ModEvents.hpp"
#include "papyrus/TotalControl.hpp"

using namespace GTS;
using namespace RE::BSScript;

namespace GTS {

	bool register_papyrus(IVirtualMachine* vm) {
		register_papyrus_plugin(vm);
		register_papyrus_scale(vm);
		register_papyrus_height(vm);
		register_papyrus_events(vm);
		register_total_control(vm);
		return true;
	}

}
