#pragma once
#include "GameplayTagContainer.h"

namespace StateMachineTags
{
    extern FGameplayTag ClearActionState;
    extern FGameplayTag DeathFreeze;
    extern FGameplayTag Grounded;
    extern FGameplayTag Jump;
}

namespace EnemyBrainTags
{
    extern FGameplayTag AdvanceSequence;
    extern FGameplayTag ClearFocus;
    extern FGameplayTag Dash;
    extern FGameplayTag SetFocus;
}