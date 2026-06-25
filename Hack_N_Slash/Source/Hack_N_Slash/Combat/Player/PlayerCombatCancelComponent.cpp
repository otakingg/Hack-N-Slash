#include "PlayerCombatCancelComponent.h"
#include "../../Tags/CharacterStateTags.h"

UPlayerCombatCancelComponent::UPlayerCombatCancelComponent() { PrimaryComponentTick.bCanEverTick = false; }

bool UPlayerCombatCancelComponent::CanCancel(const FGameplayTag& CurrentStateTag, const TArray<FGameplayTag>& AllowedStates) const
{
    return CurrentStateTag.MatchesTagExact(StateCombatTags::Jump) || CurrentStateTag.MatchesTagExact(StateCombatTags::Block) ||
    (bCanCancelCurrentAction && AllowedStates.Contains(CurrentStateTag));
}

void UPlayerCombatCancelComponent::SetCanCancelCurrentAction(bool bCanCancel) { bCanCancelCurrentAction = bCanCancel; }