#pragma once
#include "GameplayTagContainer.h"

namespace CharacterActionTags
{
    extern FGameplayTag None;
    extern FGameplayTag Attack;
    extern FGameplayTag AttackHeavyStart;
    extern FGameplayTag AttackHeavyOngoing;
    extern FGameplayTag AttackHeavyCompleted;
    extern FGameplayTag AttackLightStart;
    extern FGameplayTag AttackLightOngoing;
    extern FGameplayTag AttackHeavyCompleted;
    extern FGameplayTag BlockStart;
    extern FGameplayTag BlockStop;
    extern FGameplayTag FlyStart;
    extern FGameplayTag FlyStop;
    extern FGameplayTag Grind;
    extern FGameplayTag GrindJump;
    extern FGameplayTag JumpStart;
    extern FGameplayTag JumpStop;
    extern FGameplayTag Look;
    extern FGameplayTag Move;
    extern FGameplayTag MoveFly;
    extern FGameplayTag TetherMeTo;  // Pull you toward something
    extern FGameplayTag TetherToMe;  // Pull something to you. Get Over Here!
    extern FGameplayTag TetherSwing; // Spider-Man Swing
    extern FGameplayTag WallJump;
    extern FGameplayTag WallRun;
}