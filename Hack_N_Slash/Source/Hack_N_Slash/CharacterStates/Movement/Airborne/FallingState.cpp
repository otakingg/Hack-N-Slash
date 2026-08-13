#include "FallingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UFallingState::CanEnterState_Implementation(const UCharacterState* PreviousState) const { return moveComp && moveComp->IsFalling(); }

void UFallingState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (!ownerChar) return;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    if (!moveComp->IsFalling()) moveComp->SetMovementMode(MOVE_Falling);
}