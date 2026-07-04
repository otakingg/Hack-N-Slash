#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Interfaces/CombatInstigator.h"

void UBaseCharAnimInstance::InitializeAnimation() { CacheOwner(); }

void UBaseCharAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    if (!animData.character || !animData.moveComp || !iCmbtInst) CacheOwner();
    BuildMovementData();
    BuildTags();

    if (bDebug) UE_LOG(LogTemp, Verbose, TEXT("StateTags: %s"), *animData.stateTags.ToString());
}

void UBaseCharAnimInstance::CacheOwner()
{
    APawn* pawnOwner = TryGetPawnOwner();
    ACharacter* charPtr = Cast<ACharacter>(pawnOwner);
    iCmbtInst = Cast<ICombatInstigator>(pawnOwner);
    
    animData.character = charPtr;
    animData.moveComp = charPtr ? charPtr->GetCharacterMovement() : nullptr;
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
}

void UBaseCharAnimInstance::BuildTags()
{
    animData.stateTags.Reset();
    animData.stateTags = iCmbtInst->GetTags();
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