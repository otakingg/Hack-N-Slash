#include "JumpState.h"
#include "GameFramework/Character.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached_Implementation()
{
    if (!ownerStateMachineComp) ownerChar->FindComponentByClass<UStateMachineComponent>();
    if (ownerStateMachineComp) ownerStateMachineComp->ClearActionState();
}

FGameplayTag UJumpState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}