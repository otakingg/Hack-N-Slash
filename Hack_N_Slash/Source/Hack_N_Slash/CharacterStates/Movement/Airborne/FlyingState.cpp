#include "FlyingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UFlyingState::CanEnterState_Implementation(const UCharacterState* PreviousState) const { return moveComp && moveComp->IsFlying(); }

void UFlyingState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (!ownerChar) return;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    if (!moveComp->IsFlying()) moveComp->SetMovementMode(MOVE_Flying);
}