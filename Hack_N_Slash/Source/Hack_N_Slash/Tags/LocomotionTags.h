#pragma once
#include "NativeGameplayTags.h"

// Profiles (one active)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_None);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Ground_Walk);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Ground_Jog);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Ground_Sprint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Grind);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Climb);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Fly);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Airborne);

// Overrides (0..many)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Lock);    // ignore move input
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_NoJump);  // block JumpPressed
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Slow);    // apply multiplier (optional)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Root);    // speed = 0