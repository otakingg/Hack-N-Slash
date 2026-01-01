// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Characters/StateMachineComponent.h"

void UBaseCharAnimInstance::NativeInitializeAnimation()
{
    CacheOwnerRefs();
    bInitialized = (charOwner != nullptr && moveComp != nullptr);

    charOwner = Cast<ACharacter>(TryGetPawnOwner());
    if (!charOwner) return;

    moveComp = charOwner->FindComponentByClass<UCharacterMovementComponent>();
    stateMachineComp = charOwner->FindComponentByClass<UStateMachineComponent>();
}

void UBaseCharAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (!bInitialized)
	{
		CacheOwnerRefs();
		bInitialized = (charOwner != nullptr && moveComp != nullptr);
		if (!bInitialized) return;
	}

	UpdateLocomotionData(DeltaSeconds);
}


void UBaseCharAnimInstance::CacheOwnerRefs()
{
	charOwner = Cast<ACharacter>(TryGetPawnOwner());
	if (!charOwner) return;

	moveComp = charOwner->GetCharacterMovement();
	stateMachineComp = charOwner->FindComponentByClass<UStateMachineComponent>();
}

void UBaseCharAnimInstance::UpdateLocomotionData(float /*DeltaSeconds*/)
{
	if (!moveComp || !charOwner) return;

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