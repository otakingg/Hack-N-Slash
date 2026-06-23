#pragma once
#include "GameplayTagContainer.h"

namespace CharacterActionTags
{
    extern FGameplayTag None;
    extern FGameplayTag Attack;
    extern FGameplayTag AttackHeavyStart;
    extern FGameplayTag AttackHeavyOngoing;
    extern FGameplayTag AttackHeavyFinish;
    extern FGameplayTag AttackLightStart;
    extern FGameplayTag AttackLightOngoing;
    extern FGameplayTag AttackHeavyFinish;
    extern FGameplayTag BlockStart;
    extern FGameplayTag BlockFinish;
    extern FGameplayTag Dodge;
    extern FGameplayTag FlyStart;
    extern FGameplayTag FlyFinish;
    extern FGameplayTag Grind;
    extern FGameplayTag GrindJump;
    extern FGameplayTag JumpStart;
    extern FGameplayTag JumpFinish;
    extern FGameplayTag LookMouse;
    extern FGameplayTag LookStick;
    extern FGameplayTag Move;
    extern FGameplayTag MoveFly;
    extern FGameplayTag TetherMeTo;  // Pull you toward something
    extern FGameplayTag TetherToMe;  // Pull something to you. Get Over Here!
    extern FGameplayTag TetherSwing; // Spider-Man Swing
    extern FGameplayTag WallJump;
    extern FGameplayTag WallRun;
}