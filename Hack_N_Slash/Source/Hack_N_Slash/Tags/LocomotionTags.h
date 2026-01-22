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

// Overrides (Can have many active)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Lock);    // Ignore move input
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_NoJump);  // Block JumpPressed
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Slow);    // Apply multiplier (optional)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Root);    // Speed = 0