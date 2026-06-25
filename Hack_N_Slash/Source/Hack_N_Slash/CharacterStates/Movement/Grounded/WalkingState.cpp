#include "WalkingState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"

bool UWalkingState::CanEnterState_Implementation(const UCharacterState* PreviousState) const { return moveComp && moveComp->IsMovingOnGround(); }

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