#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    ACharacter* ownerChar = Cast<ACharacter>(pawnOwner);
    
    animData.character = ownerChar;
    animData.moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;
    iCmbtInst = Cast<ICombatInstigator>(pawnOwner);
}

void UBaseCharAnimInstance::BuildMovementData()
{
    ACharacter* ownerChar = animData.character;
    UCharacterMovementComponent* moveComp = animData.moveComp;

    if (!ownerChar || !moveComp) return;

    animData.velocity = ownerChar->GetVelocity();
    animData.speed = animData.velocity.Size();
    animData.speed2D = animData.velocity.Size2D();
    
    animData.accel = moveComp->GetCurrentAcceleration();
    animData.bHasAcceleration = animData.accel.SizeSquared() > KINDA_SMALL_NUMBER;
}

void UBaseCharAnimInstance::BuildTags()
{
    animData.stateTags.Reset();
    if (iCmbtInst) animData.stateTags = iCmbtInst->GetTags();
}

bool UBaseCharAnimInstance::HasStateTag(const FGameplayTag& Tag) const { return animData.stateTags.HasTag(Tag); }

float UBaseCharAnimInstance::PlayMontageHNS(UAnimMontage* Montage, FName Section)
{
    if (!Montage || (Section != NAME_None && !Montage->IsValidSectionName(Section))) return 0.0f;
    
    float const duration = Montage_Play(Montage);
    if (duration > 0.0f && Section != NAME_None) Montage_JumpToSection(Section, Montage);
    return duration;
}