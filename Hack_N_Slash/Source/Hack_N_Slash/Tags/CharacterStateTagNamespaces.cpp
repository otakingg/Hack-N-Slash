#include "CharacterStateTagNamespaces.h"

namespace HitTags
{
    FGameplayTag None       = FGameplayTag::RequestGameplayTag("State.Action.None");
    FGameplayTag Flinch     = FGameplayTag::RequestGameplayTag("State.Action.Hit.Flinch");
    FGameplayTag Stagger    = FGameplayTag::RequestGameplayTag("State.Action.Hit.Stagger");
    FGameplayTag Launch     = FGameplayTag::RequestGameplayTag("State.Action.Hit.Launch");
    FGameplayTag Knockback  = FGameplayTag::RequestGameplayTag("State.Action.Hit.Knockback");
    FGameplayTag Knockdown  = FGameplayTag::RequestGameplayTag("State.Action.Hit.Knockdown");
}