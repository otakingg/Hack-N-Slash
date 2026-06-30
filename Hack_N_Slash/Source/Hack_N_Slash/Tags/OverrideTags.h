#pragma once
#include "GameplayTagContainer.h"

// Overrides (Can have many active)
namespace OverrideTags
{
    extern FGameplayTag MoveStats; // Use alternate stats (for things like buffs/debuffs, separate from profile changes)
    extern FGameplayTag NoAtk; // Ignore attack input
    extern FGameplayTag NoBlock; // Ignore block input
    extern FGameplayTag NoDodge; // Ignore dodge input
    extern FGameplayTag NoJump; // Ignore Jump Input
    extern FGameplayTag NoLockOn; // Ignore lock on input
    extern FGameplayTag NoLook; // Ignore look input
    extern FGameplayTag NoMove; // Ignore move input
}