#include "CharacterStateTagNamespaces.h"

namespace ActionTags
{
    FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");
}

namespace HitTags
{
    FGameplayTag Hit        = FGameplayTag::RequestGameplayTag("State.Action.Hit");
    FGameplayTag Flinch     = FGameplayTag::RequestGameplayTag("State.Action.Hit.Flinch");
    FGameplayTag Stagger    = FGameplayTag::RequestGameplayTag("State.Action.Hit.Stagger");
    FGameplayTag StaggerAir = FGameplayTag::RequestGameplayTag("State.Action.Hit.StaggerAir");
    FGameplayTag Launch     = FGameplayTag::RequestGameplayTag("State.Action.Hit.Launch");
    FGameplayTag Knockback  = FGameplayTag::RequestGameplayTag("State.Action.Hit.Knockback");
    FGameplayTag Knockdown  = FGameplayTag::RequestGameplayTag("State.Action.Hit.Knockdown");
    FGameplayTag Dead       = FGameplayTag::RequestGameplayTag("State.Action.Hit.Dead");
}

namespace CombatTags
{
    FGameplayTag Attack = FGameplayTag::RequestGameplayTag("State.Action.Combat.Attack");
    FGameplayTag Block  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Block");
    FGameplayTag Dodge  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Dodge");
    FGameplayTag Jump   = FGameplayTag::RequestGameplayTag("State.Action.Combat.Jump");
    FGameplayTag Parry  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Parry");
}