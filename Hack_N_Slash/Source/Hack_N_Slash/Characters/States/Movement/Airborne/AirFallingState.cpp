#include "AirFallingState.h"
#include "GameFramework/CharacterMovementComponent.h"

static constexpr float ZVelEpsilon = 5.f;

void UAirFallingState::EnterState()
{
    Super::EnterState();

    if (!moveComp) return;

    moveComp->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
    moveComp->bOrientRotationToMovement     = bOrientRotationToMovement;
    moveComp->RotationRate                  = rotationRate;
}

bool UAirFallingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    if (!moveComp) return false;
    return moveComp->IsFalling() && (moveComp->Velocity.Z <= ZVelEpsilon);
}