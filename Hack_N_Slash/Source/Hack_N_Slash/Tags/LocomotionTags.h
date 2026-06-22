#pragma once
#include "GameplayTagContainer.h"

// Overrides (Can have many active)
namespace OverrideTags
{
    extern FGameplayTag Lock; // Ignore move input
    extern FGameplayTag MoveStats; // Use alternate stats (for things like buffs/debuffs, separate from profile changes)
    extern FGameplayTag NoJump; // Ignore Jump Input
}