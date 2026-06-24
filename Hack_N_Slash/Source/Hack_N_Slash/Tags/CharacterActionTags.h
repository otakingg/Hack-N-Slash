#pragma once
#include "GameplayTagContainer.h"

namespace CharacterActionTags
{
    extern FGameplayTag None;
    extern FGameplayTag Attack;
    extern FGameplayTag AttackHeavyStart;
    extern FGameplayTag AttackHeavyHold;
    extern FGameplayTag AttackHeavyRelease;
    extern FGameplayTag AttackLightStart;
    extern FGameplayTag AttackLightHold;
    extern FGameplayTag AttackLightRelease;
    extern FGameplayTag BlockStart;
    extern FGameplayTag BlockRelease;
    extern FGameplayTag Dodge;
    extern FGameplayTag Fly;
    extern FGameplayTag Grind;
    extern FGameplayTag GrindJump;
    extern FGameplayTag JumpStart;
    extern FGameplayTag JumpRelease;
    extern FGameplayTag LookMouse;
    extern FGameplayTag LookStick;
    extern FGameplayTag Move;
    extern FGameplayTag TetherMeTo;  // Pull you toward something
    extern FGameplayTag TetherToMe;  // Pull something to you. Get Over Here!
    extern FGameplayTag TetherSwing; // Spider-Man Swing
    extern FGameplayTag WallJump;
    extern FGameplayTag WallRun;
}