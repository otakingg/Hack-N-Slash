#include "AirFallingState.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UAirFallingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    if (!moveComp) return false;
    return moveComp->IsFalling() && (moveComp->Velocity.Z <= ZVelEpsilon);
}