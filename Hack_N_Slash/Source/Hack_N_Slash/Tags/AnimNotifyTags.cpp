#include "AnimNotifyTags.h"


namespace StateMachineTags
{
    FGameplayTag ClearActionState = FGameplayTag::RequestGameplayTag("Notify.StateMachine.ClearActionState");
    FGameplayTag DeathFreeze      = FGameplayTag::RequestGameplayTag("Notify.StateMachine.DeathFreeze");
    FGameplayTag Grounded         = FGameplayTag::RequestGameplayTag("Notify.StateMachine.Grounded");
    FGameplayTag Jump             = FGameplayTag::RequestGameplayTag("Notify.StateMachine.Jump");
}

namespace EnemyBrainTags
{
    FGameplayTag AdvanceSequence  = FGameplayTag::RequestGameplayTag("Notify.EnemyBrain.AdvanceSequence");
    FGameplayTag ClearFocus       = FGameplayTag::RequestGameplayTag("Notify.EnemyBrain.ClearFocus");
    FGameplayTag Dash             = FGameplayTag::RequestGameplayTag("Notify.EnemyBrain.Dash");
    FGameplayTag RequestEvaluate  = FGameplayTag::RequestGameplayTag("Notify.EnemyBrain.RequestEvaluate");
    FGameplayTag SetFocus         = FGameplayTag::RequestGameplayTag("Notify.EnemyBrain.SetFocus");
}