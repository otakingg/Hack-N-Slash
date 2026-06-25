#include "JumpState.h"
#include "GameFramework/Character.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached()
{
    if (!ownerStateMachineComp) ownerChar->FindComponentByClass<UStateMachineComponent>();
    if (ownerStateMachineComp) ownerStateMachineComp->ClearActionState();
}