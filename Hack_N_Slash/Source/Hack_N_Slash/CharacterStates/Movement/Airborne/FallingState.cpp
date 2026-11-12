#include "FallingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UFallingState::CanEnterState_Implementation(const UCharacterState* CurrentState) const
{ return Super::CanEnterState_Implementation(CurrentState) && moveComp->IsFalling(); }

void UFallingState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (!ownerChar) return;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    if (!moveComp->IsFalling()) moveComp->SetMovementMode(MOVE_Falling);
}

FGameplayTag UFallingState::ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& LookVector, const FVector2D& MoveVector)
{
    switch(PlayerInput)
    {
        case EPlayerInput::BlockDodgeTrigger:
            return Tags::PlayerAction::None;

        case EPlayerInput::BlockDodgeStart:
            return Tags::PlayerAction::Dodge;

        case EPlayerInput::BlockDodgeComplete:
            return Tags::PlayerAction::None;

        default:
            return Super::ResolvePlayerInput_Implementation(PlayerInput, LookVector, MoveVector);
    }
}
