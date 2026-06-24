#include "FallingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"

bool UFallingState::CanEnterState_Implementation(const UCharacterState* PreviousState) const { return moveComp && moveComp->IsFalling(); }

void UFallingState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    if (!moveComp->IsFalling()) moveComp->SetMovementMode(MOVE_Falling);
}