#include "JumpState.h"
#include "GameFramework/Character.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached()
{
    if (!ownerStateMachineComp) ownerChar->FindComponentByClass<UStateMachineComponent>();
    if (ownerStateMachineComp) ownerStateMachineComp->ClearActionState();
}

FGameplayTag UJumpState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}