#include "AirFallingState.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAirFallingState::EnterState()
{
    Super::EnterState();

    if (!moveComp) return;

    // Optional behavior-only rotation policy while falling
    moveComp->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
    moveComp->bOrientRotationToMovement = bOrientRotationToMovement;
    moveComp->RotationRate = rotationRate;

    // NOTE: do NOT call AddMoveInput here (container does it).
    // NOTE: do NOT force movement mode here (container sets MOVE_Falling).
}

bool UAirFallingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    if (!moveComp) return false;
    return moveComp->IsFalling() && (moveComp->Velocity.Z <= 0.f);
}