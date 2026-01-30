// AirRisingState.cpp
#include "AirRisingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../StateMachineComponent.h"

static constexpr float ZVelEpsilon = 5.f;

void UAirRisingState::EnterState()
{
    Super::EnterState();

    // Nothing required here for motion matching.
    // Your AnimBP / motion matching can simply read the movement state tag (Air.Rising) and pick the correct database/pose search context.
}

bool UAirRisingState::CanEnterAirMode_Implementation(const UCharacterState* PreviousState) const
{
    if (!moveComp) return false;
    return moveComp->IsFalling() && (moveComp->Velocity.Z > ZVelEpsilon);
}

void UAirRisingState::OnJumpApexReached()
{
    if (ownerStateMachineComp) ownerStateMachineComp->ClearAirborneMode();
}


/*OnReachedJumpApex is great when it fires, but it won’t always cover:
    Launches with odd curves
    Network situations
    Custom movement modes / gravity changes
    Some cases where apex notify timing doesn’t align with the desired switch
This acts as a fallback if you’re in Rising and velocity is no longer positive, request falling.*/
void UAirRisingState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    Super::OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);

    if (!moveComp) return;
    if (moveComp->IsFalling() && moveComp->Velocity.Z <= ZVelEpsilon)
    {
        if (ownerStateMachineComp) ownerStateMachineComp->ClearAirborneMode();
    }
}