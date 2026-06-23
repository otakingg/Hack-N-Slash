#pragma once
#include "GameplayTagContainer.h"

namespace StateActionTags
{
    extern FGameplayTag None;
}

namespace StateReactionTags
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

namespace StateCombatTags
{
    extern FGameplayTag Attack;
    extern FGameplayTag Block;
    extern FGameplayTag Dodge;
    extern FGameplayTag Jump;
    extern FGameplayTag Parry;
}

namespace StateMovementTags
{
    extern FGameplayTag Walk;
    extern FGameplayTag Grind;
    extern FGameplayTag Climb;
    extern FGameplayTag WallRun;
    extern FGameplayTag Fall;
    extern FGameplayTag Fly;
}