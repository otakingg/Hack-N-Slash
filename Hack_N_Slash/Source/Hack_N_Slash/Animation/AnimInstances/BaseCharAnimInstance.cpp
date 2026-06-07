// BaseCharAnimInstance.cpp
#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Characters/Shared/StateMachineComponent.h"

void UBaseCharAnimInstance::InitializeAnimation() { CacheOwner(); }

void UBaseCharAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    if (!animData.character || !animData.moveComp || !animData.stateMachineComp) CacheOwner();
    BuildMovementData();
    BuildTags();

    if (bDebug) UE_LOG(LogTemp, Verbose, TEXT("StateTags: %s"), *animData.stateTags.ToString());
}

void UBaseCharAnimInstance::CacheOwner()
{
    APawn* pawnOwner = TryGetPawnOwner();
    ACharacter* charPtr = Cast<ACharacter>(pawnOwner);
    
    animData.character = charPtr;
    animData.moveComp = charPtr ? charPtr->GetCharacterMovement() : nullptr;
    animData.stateMachineComp = charPtr ? charPtr->FindComponentByClass<UStateMachineComponent>() : nullptr;
}

void UBaseCharAnimInstance::BuildMovementData()
{
    // Safely cast to local pointers for optimal math execution speed
    ACharacter* charPtr = animData.character;
    UCharacterMovementComponent* movePtr = animData.moveComp;

    if (!charPtr || !movePtr) return;

    animData.velocityWS = charPtr->GetVelocity();
    animData.speed = animData.velocityWS.Size();
    animData.speed2D = animData.velocityWS.Size2D(); // Replaced custom manual Vector conversion with cleaner Native UE API
    
    animData.accelWS = movePtr->GetCurrentAcceleration();
    animData.bHasAcceleration = animData.accelWS.SizeSquared() > KINDA_SMALL_NUMBER;
    animData.bIsFalling = movePtr->IsFalling();
    animData.bIsGrounded = movePtr->IsMovingOnGround();
}

void UBaseCharAnimInstance::BuildTags()
{
    animData.stateTags.Reset();

    UStateMachineComponent* stateMachinePtr = animData.stateMachineComp;
    
    // Lazy fallback check using local raw pointer logic
    if (!stateMachinePtr && animData.character)
    {
        animData.stateMachineComp = animData.character->FindComponentByClass<UStateMachineComponent>();
        stateMachinePtr = animData.stateMachineComp;
    }

    if (stateMachinePtr) animData.stateTags = stateMachinePtr->GetActiveStateTags();
}

bool UBaseCharAnimInstance::HasStateTag(FGameplayTag Tag) const { return animData.stateTags.HasTag(Tag); }

bool UBaseCharAnimInstance::HasAnyStateTags(const FGameplayTagContainer& Tags) const { return animData.stateTags.HasAny(Tags); }

float UBaseCharAnimInstance::PlayMontageHNS(UAnimMontage* Montage, FName Section)
{
    if (!Montage || (Section != NAME_None && !Montage->IsValidSectionName(Section))) return 0.0f;
    
    float const duration = Montage_Play(Montage);
    if (duration > 0.0f && Section != NAME_None) Montage_JumpToSection(Section, Montage);
    return duration;
}