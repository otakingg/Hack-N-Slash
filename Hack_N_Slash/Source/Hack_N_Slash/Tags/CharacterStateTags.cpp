#include "CharacterStateTags.h"

namespace StateActionTags
{
    FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");
}

namespace StateReactionTags
{
    FGameplayTag Hit        = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit");
    FGameplayTag Flinch     = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Flinch");
    FGameplayTag Stagger    = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Stagger");
    FGameplayTag Air        = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Air");
    FGameplayTag Launch     = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Launch");
    FGameplayTag Knockback  = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Knockback");
    FGameplayTag Knockdown  = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Knockdown");
    FGameplayTag BlockBreak = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.BlockBreak");
    FGameplayTag BlockHit   = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Block");
    FGameplayTag Countered  = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Countered");
    FGameplayTag Dead       = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Dead");
}

namespace StateCombatTags
{
    FGameplayTag Attack = FGameplayTag::RequestGameplayTag("State.Action.Combat.Attack");
    FGameplayTag Block  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Block");
    FGameplayTag Dodge  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Dodge");
    FGameplayTag Jump   = FGameplayTag::RequestGameplayTag("State.Action.Combat.Jump");
    FGameplayTag Parry  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Parry");
}


namespace StateMovementTags
{
    FGameplayTag Walk         = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Walking");
    FGameplayTag Grind        = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Grinding");
    FGameplayTag Climb        = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Climbing");
    FGameplayTag WallRun      = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.WallRunning");
    FGameplayTag Fall         = FGameplayTag::RequestGameplayTag("State.Movement.Airborne.Falling");
    FGameplayTag Fly          = FGameplayTag::RequestGameplayTag("State.Movement.Airborne.Flying");
}