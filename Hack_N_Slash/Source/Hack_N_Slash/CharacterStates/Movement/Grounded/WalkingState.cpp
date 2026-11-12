#include "WalkingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"

bool UWalkingState::CanEnterState_Implementation(const UCharacterState* CurrentState) const
{ return Super::CanEnterState_Implementation(CurrentState) && moveComp->IsMovingOnGround(); }

void UWalkingState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (!ownerChar) return;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    locoComp->UpdateLastGroundedTime();
    if (!moveComp->IsMovingOnGround()) moveComp->SetMovementMode(MOVE_Walking);
}

void UWalkingState::ExitState_Implementation()
{
    if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (locoComp) locoComp->UpdateLastGroundedTime();

    Super::ExitState_Implementation();
}

FGameplayTag UWalkingState::ResolvePlayerInput_Implementation(EPlayerInput PlayerInput, const FVector2D& LookVector, const FVector2D& MoveVector)
{
    switch(PlayerInput)
    {
        case EPlayerInput::BlockDodgeTrigger:
            if (MoveVector.IsNearlyZero()) return Tags::PlayerAction::BlockTrigger;
            else return Tags::PlayerAction::None;

        case EPlayerInput::BlockDodgeStart:
            if (MoveVector.IsNearlyZero()) return Tags::PlayerAction::BlockStart;
            else return Tags::PlayerAction::Dodge;

        case EPlayerInput::BlockDodgeComplete:
            if (MoveVector.IsNearlyZero()) return Tags::PlayerAction::BlockRelease;
            else return Tags::PlayerAction::None;
            
        default:
            return Super::ResolvePlayerInput_Implementation(PlayerInput, LookVector, MoveVector);
    }
}