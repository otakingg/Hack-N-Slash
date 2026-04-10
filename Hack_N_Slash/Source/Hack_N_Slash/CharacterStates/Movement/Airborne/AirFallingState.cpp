#include "AirFallingState.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UAirFallingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    return (moveComp && moveComp->IsFalling()) && (moveComp->Velocity.Z <= ZVelEpsilon);
}