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
    if (!iCmbtInst) return;

    TMap<FGameplayTag, int32> tags = iCmbtInst->GetTags();

    for (const TPair<FGameplayTag, int32>& pair : tags)
    {
        if (pair.Value > 0) animData.stateTags.AddTag(pair.Key);
    }
}

//bool UBaseCharAnimInstance::HasStateTag(const FGameplayTag& Tag, bExact) const { return animData.stateTags.HasTag(Tag); }
bool UBaseCharAnimInstance::HasStateTag(const FGameplayTag& Tag, bool bExact) const { return iCmbtInst ? iCmbtInst->HasTag(Tag, bExact) : false; }
float UBaseCharAnimInstance::PlayMontageHNS(UAnimMontage* Montage, FName Section)
{
    if (!Montage || (Section != NAME_None && !Montage->IsValidSectionName(Section))) return 0.0f;
    
    float const duration = Montage_Play(Montage);
    if (duration > 0.0f && Section != NAME_None) Montage_JumpToSection(Section, Montage);
    return duration;
}