#pragma once

namespace GTS {
	void SurvivalMode_RemoveAllSpells(Actor* actor, SpellItem* stage0, SpellItem* stage1, SpellItem* stage2, SpellItem* stage3, SpellItem* stage4, SpellItem* stage5);
	void SurvivalMode_RefreshSpells(Actor* actor, float currentvalue);
	void SurvivalMode_AdjustHunger(Actor* giant, float tinyscale, bool IsLiving, bool Finished);
}
