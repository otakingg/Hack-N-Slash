#include "PlayerCombatCancelComponent.h"
#include "../../Utility/Tags.h"

UPlayerCombatCancelComponent::UPlayerCombatCancelComponent() { PrimaryComponentTick.bCanEverTick = false; }

bool UPlayerCombatCancelComponent::CanCancel(const FGameplayTag& CurrentStateTag, const TArray<FGameplayTag>& AllowedStates) const
{
    return CurrentStateTag.MatchesTagExact(Tags::StateMachine::Action::Combat::Jump) || CurrentStateTag.MatchesTagExact(Tags::StateMachine::Action::Combat::Block) ||
    (bCanCancelCurrentAction && AllowedStates.Contains(CurrentStateTag));
}

void UPlayerCombatCancelComponent::SetCanCancelCurrentAction(bool bCanCancel) { bCanCancelCurrentAction = bCanCancel; }