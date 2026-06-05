#pragma once
#include "GameplayTagContainer.h"

// Profiles (one active)
namespace ProfileTags
{
    extern FGameplayTag None;
    extern FGameplayTag Grounded;
    extern FGameplayTag Grind;
    extern FGameplayTag Climb;
    extern FGameplayTag WallRun;
    extern FGameplayTag Falling;
    extern FGameplayTag Fly;
}

// Overrides (Can have many active)
namespace OverrideTags
{
    extern FGameplayTag Lock; // Ignore move input
    extern FGameplayTag MoveStats; // Use alternate stats (for things like buffs/debuffs, separate from profile changes)
    extern FGameplayTag NoJump; // Ignore Jump Input
}