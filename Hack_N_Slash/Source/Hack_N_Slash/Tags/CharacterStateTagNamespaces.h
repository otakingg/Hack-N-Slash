#pragma once

#include "GameplayTagContainer.h"

namespace ActionTags
{
    extern FGameplayTag None;
}

namespace HitTags
{
    extern FGameplayTag Hit;
    extern FGameplayTag Flinch;
    extern FGameplayTag Stagger;
    extern FGameplayTag StaggerAir;
    extern FGameplayTag Launch;
    extern FGameplayTag Knockback;
    extern FGameplayTag Knockdown;
    extern FGameplayTag BlockBreak;
    extern FGameplayTag BlockHit;
    extern FGameplayTag Dead;
}

namespace CombatTags
{
    extern FGameplayTag Attack;
    extern FGameplayTag Block;
    extern FGameplayTag Dodge;
    extern FGameplayTag Jump;
    extern FGameplayTag Parry;
}