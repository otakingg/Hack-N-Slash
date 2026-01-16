// AirRisingState.cpp
#include "AirRisingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../StateMachineComponent.h"

void UAirRisingState::EnterState()
{
    Super::EnterState();

    // Nothing required here for motion matching.
    // Your AnimBP / motion matching can simply read the movement state tag (Air.Rising) and pick the correct database/pose search context.
}

bool UAirRisingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    if (!moveComp) return false;

    // Must be in air + moving upward
    return moveComp->IsFalling() && (moveComp->Velocity.Z > 0.f);
}

void UAirRisingState::OnJumpApexReached()
{
    // On apex: switch to falling mode
    if (ownerStateMachineComp) ownerStateMachineComp->ClearAirMode();
}
