#pragma once
#include "NativeGameplayTags.h"

// Profiles (one active)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_None);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Grounded);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Grind);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Climb);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Fly);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Profile_Falling);

// Overrides (Can have many active)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_Lock);    // Ignore move input
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_MoveStats); // Use alternate stats (for things like buffs/debuffs, separate from profile changes)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Move_Override_NoJump);  // Block JumpPressed