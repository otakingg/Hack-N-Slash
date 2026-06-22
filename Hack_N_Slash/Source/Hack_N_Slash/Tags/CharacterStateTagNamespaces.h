#pragma once
#include "GameplayTagContainer.h"

namespace ActionTags
{
    extern FGameplayTag None;
}

namespace ReactionTags
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
    extern FGameplayTag Countered;
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

namespace MovementTags
{
    extern FGameplayTag Walk;
    extern FGameplayTag Grind;
    extern FGameplayTag Climb;
    extern FGameplayTag WallRun;
    extern FGameplayTag Fall;
    extern FGameplayTag Fly;
}