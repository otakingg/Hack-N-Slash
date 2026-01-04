// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/StateMachineComponent.h"

void UBaseCharAnimInstance::NativeInitializeAnimation()
{
	CacheOwnerRefs();
}

void UBaseCharAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	UpdateLocomotionData(DeltaSeconds);
}


void UBaseCharAnimInstance::CacheOwnerRefs()
{
	if (bInitialized) return;

    charOwner = Cast<ACharacter>(TryGetPawnOwner());
    if (!charOwner) return;

    moveComp = charOwner->FindComponentByClass<UCharacterMovementComponent>();
	if (!moveComp) return;

    stateMachineComp = charOwner->FindComponentByClass<UStateMachineComponent>();
	if (!stateMachineComp) return;
	
	bInitialized = true;
}

void UBaseCharAnimInstance::UpdateLocomotionData(float DeltaSeconds)
{
	if (!bInitialized) {CacheOwnerRefs();}
	if (!bInitialized) return;

	velocity = charOwner->GetVelocity();
	speed = static_cast<float>(velocity.Size());

	// Acceleration: CharacterMovement has current acceleration
	acceleration = static_cast<float>(moveComp->GetCurrentAcceleration().Size());
	bHasAcceleration = acceleration > KINDA_SMALL_NUMBER;

	bIsFalling = moveComp->IsFalling();
}

float UBaseCharAnimInstance::PlayActionMontage(UAnimMontage* Montage, float PlayRate, FName StartSection)
{
	if (!Montage) return 0.f;

	const float Duration = Montage_Play(Montage, PlayRate);
	if (Montage->IsValidSectionName(StartSection)) Montage_JumpToSection(StartSection, Montage);
	return Duration;
}

void UBaseCharAnimInstance::StopAllMontages(float BlendOutTime) {Montage_Stop(BlendOutTime);}