// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/StateMachineComponent.h"

void UBaseCharAnimInstance::NativeInitializeAnimation()
{
    owningPawn = TryGetPawnOwner();
    if (!owningPawn) return;

    movementComp = owningPawn->FindComponentByClass<UCharacterMovementComponent>();
    stateMachineComp = owningPawn->FindComponentByClass<UStateMachineComponent>();
}

void UBaseCharAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (!owningPawn) owningPawn = TryGetPawnOwner();
    if (!owningPawn) return;

    if (!stateMachineComp) stateMachineComp = owningPawn->FindComponentByClass<UStateMachineComponent>();
    if (!stateMachineComp) return;

    if (!movementComp) movementComp = owningPawn->FindComponentByClass<UCharacterMovementComponent>();
    if (!movementComp) return;

    velocity = movementComp->Velocity;
    speed = static_cast<float>(velocity.Size());
    bIsFalling = movementComp->IsFalling();
}

FGameplayTag UBaseCharAnimInstance::GetActiveStateTag() const {return stateMachineComp ? stateMachineComp->GetCurrentStateTag() : FGameplayTag();}