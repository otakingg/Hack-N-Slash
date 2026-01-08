// Fill out your copyright notice in the Description page of Project Settings.
#include "GroundLocomotionState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UGroundLocomotionState::EnterState()
{
    Super::EnterState();

    if (!ownerChar || !moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UGroundLocomotionState] EnterState: ownerChar or movement comp is null."));
        return;
    }

    // Rotation policy
    moveComp->bOrientRotationToMovement = bOrientRotationToMovement;
    moveComp->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
    moveComp->RotationRate = rotationRate;

    // Basic tuning
    moveComp->MaxWalkSpeed = maxWalkSpeed;
    moveComp->MaxAcceleration = maxAcceleration;
    moveComp->BrakingDecelerationWalking = brakingDecelerationWalking;
}

void UGroundLocomotionState::ExitState()
{
    Super::ExitState();
}

bool UGroundLocomotionState::OnInputLook(const FVector2D &Look)
{
    if (!ownerChar) return false;
    Super::OnInputLook(Look);
    
    ownerChar->AddControllerYawInput(Look.X * turnRate * ownerChar->GetWorld()->GetDeltaSeconds());
    ownerChar->AddControllerPitchInput(Look.Y * lookUpRate * ownerChar->GetWorld()->GetDeltaSeconds());

    return false;
}

bool UGroundLocomotionState::OnInputMove(const FVector2D &InputVector)
{
    if (!ownerChar) return false;
    Super::OnInputMove(InputVector);

	FRotator controlRotA {ownerChar->GetControlRotation()};
	controlRotA.Pitch = 0.0;
	ownerChar->AddMovementInput(UKismetMathLibrary::GetRightVector(controlRotA), InputVector.X);

	FRotator controlRotB {ownerChar->GetControlRotation()};
	controlRotB.Roll = 0.0f;
	controlRotB.Pitch = 0.0f;
	ownerChar->AddMovementInput(UKismetMathLibrary::GetForwardVector(controlRotB), InputVector.Y);

    return false;
}
