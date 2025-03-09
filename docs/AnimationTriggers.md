# Animation Triggers

This readme provides a reference for in-game console commands that can be used to manually play some of this mod's animations. Note that some commands marked with `(*)` may not work properly when called manually due to some logic within the mod being skipped.

---

## Usage Example:
```bash
player.sae GTSBEH_HeavyKickLow_R
```

---

## STOMPS:

**Main Actions**

- `GtsModStompAnimRight -> GTSBEH_StartFootGrindR`
- `GtsModStompAnimLeft -> GTSBEH_StartFootGrindL`

**UnderStomp Variants:**
- `GTSBeh_UnderStomp_StartR` - Understomp range depends on the Behavior float 'GTS_StompBlend' (0.0 = under self, 1.0 = far)
- `GTSBeh_UnderStomp_StartL` - Same as above
- `GTSBeh_UnderStomp_Start_StrongR` - Same logic as UnderStomp_StartL/R
- `GTSBeh_UnderStomp_Start_StrongL` - Same logic as UnderStomp_StartL/R

**Strong Stomps:**
- `GTSBeh_StrongStomp_StartRight`
- `GTSBeh_StrongStomp_StartLeft`

---

## KICKS/CRAWL/SNEAK ATTACKS:

**Heavy Kicks:**
- `GTSBEH_HeavyKickLow_L` - Medium leg kick
- `GTSBEH_HeavyKickLow_R` - Leg kick

**Swipe Attacks (Universal crawl/sneak/standing):**
- `GTSBeh_SwipeLight_L` - Light Kick when standing
- `GTSBeh_SwipeLight_R` - Light Kick when standing
- `GTSBeh_SwipeHeavy_R` - Strong Kick when standing
- `GTSBeh_SwipeHeavy_R` - Strong Kick when standing

---

## TRAMPLE:

- `GTSBeh_Trample_L -> GTSBEH_Trample_Start_L`
- `GTSBeh_Trample_R -> GTSBEH_Trample_Start_R`

---

## BUTT/BREAST/KNEE CRUSH (Universal Crawl/Sneak/Standing):

- `GTSBeh_ButtCrush_Start` - Triggers `GTSBeh_ButtCrush_Attack` or `GTSBeh_ButtCrush_Grow`
- `GTSBeh_ButtCrush_StartFast`

---

## THIGH CRUSH:

- `GTSBeh_TriggerSitdown`
- `GTSBeh_StartThighCrush`
- `GTSBeh_LeaveSitdown`
- `GTSBeh_ThighAnimationFull`

---

## HUGS:
- *These do not work at all when manually called.*

---

## THIGH SANDWICH (*):

- `GTSBeh_ThighSandwich_Start`
- `GTSBeh_ThighSandwich_Attack`
- `GTSBeh_ThighSandwich_Attack_H`
- `GTSBeh_ThighSandwich_ExitLoop`

---

## GRAB (*):

- `GTSBeh_GrabStart`
- `GTSBeh_GrabVore`
- `GTSBeh_GrabAttack`
- `GTSBeh_GrabThrow`
- `GTSBeh_GrabRelease`
- `GTSBeh_BreastsAdd`
- `GTSBeh_BreastsRemove`
- `GTSBeh_T_Remove`

---

## PRONE:

- `GTSBeh_ProneStart`
- `GTSBeh_ProneStop`
- `GTSBeh_ProneStart_Dive`

---

## TINY CALAMITY (*):

- `GTSBEH_TC_Shrink`

---

## VORE (*):

- `GTSBeh_StartVore`

---

## GROWTH/SHRINK:

- `GTSBeh_Grow_Random` - Random Growth anims by Lajest.
- `GTSBeh_Shrink_Trigger` - Manual Growth
- `GTSBeh_Grow_Trigger` - Manual Shrink

---

