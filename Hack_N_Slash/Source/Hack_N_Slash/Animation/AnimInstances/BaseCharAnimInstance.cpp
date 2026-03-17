#include "BaseCharAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Characters/StateMachineComponent.h"

void UBaseCharAnimInstance::InitializeAnimation()
{
    CacheOwner();
	CachedStateMachineComp = CachedCharacter ? CachedCharacter->FindComponentByClass<UStateMachineComponent>() : nullptr;
}

void UBaseCharAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    if (!CachedCharacter || !CachedMoveComp)
    {
        CacheOwner();
        if (!CachedCharacter || !CachedMoveComp) return;
    }

    BuildMovementData(DeltaSeconds);
    BuildTags();

    if (bDebug) UE_LOG(LogTemp, Verbose, TEXT("StateTags: %s"), *AnimData.StateTags.ToString());
}

void UBaseCharAnimInstance::CacheOwner()
{
    APawn* PawnOwner = TryGetPawnOwner();
    CachedCharacter = Cast<ACharacter>(PawnOwner);
    CachedMoveComp = CachedCharacter ? CachedCharacter->GetCharacterMovement() : nullptr;

    AnimData.Character = CachedCharacter;
    AnimData.MoveComp = CachedMoveComp;
}

void UBaseCharAnimInstance::BuildMovementData(float DeltaSeconds)
{
    AnimData.VelocityWS = CachedCharacter->GetVelocity();
    AnimData.Speed = AnimData.VelocityWS.Size();
    AnimData.Speed2D = FVector(AnimData.VelocityWS.X, AnimData.VelocityWS.Y, 0.f).Size();

    AnimData.AccelWS = CachedMoveComp->GetCurrentAcceleration();
    AnimData.bHasAcceleration = AnimData.AccelWS.SizeSquared() > KINDA_SMALL_NUMBER;
    AnimData.bIsFalling = CachedMoveComp->IsFalling();
    AnimData.bIsGrounded = CachedMoveComp->IsMovingOnGround();
}

void UBaseCharAnimInstance::BuildTags()
{
    AnimData.StateTags.Reset();
    AnimData.AnimContextTags.Reset();

    // --- Pull from your improved state tag system ---
    // Example patterns (pick one):

    // (A) If your character has an interface like IStateTagProvider:
    // if (IStateTagProvider* Provider = Cast<IStateTagProvider>(CachedCharacter)) Provider->GetStateTags(AnimData.StateTags);

    // (B) If you have a StateMachineComponent that stores current tags:
    if (!CachedStateMachineComp && CachedCharacter) CachedStateMachineComp = CachedCharacter->FindComponentByClass<UStateMachineComponent>();
    if (CachedStateMachineComp) AnimData.StateTags = CachedStateMachineComp->GetActiveStateTags();

    // (C) If you’re using GameplayTags on an ASC, you can query those too:
    // CachedASC->GetOwnedGameplayTags(AnimData.StateTags);

    GatherAnimContextTags(AnimData.AnimContextTags);
}

void UBaseCharAnimInstance::GatherAnimContextTags(FGameplayTagContainer& OutTags) const
{
    // Default: none.
    // Child classes can add tags like:
    // - stance: standing/crouch
    // - weapon: sword/2h
    // - locomotion: strafe/forward
    // - overlay: injured/wet/etc
}

bool UBaseCharAnimInstance::HasStateTag(FGameplayTag Tag) const { return AnimData.StateTags.HasTag(Tag); }
bool UBaseCharAnimInstance::HasAnyStateTags(const FGameplayTagContainer& Tags) const { return AnimData.StateTags.HasAny(Tags); }

float UBaseCharAnimInstance::PlayMontageHNS(UAnimMontage* Montage, float PlayRate, FName Section)
{
    if (!Montage) return 0.0f;

    PlayRate = FMath::Clamp(PlayRate, 0.0f, 100.0f);
    Montage->RateScale = PlayRate;
    
    float const duration = Montage_Play(Montage, PlayRate);
    if (duration > 0.0f && Section != NAME_None) Montage_JumpToSection(Section, Montage);

    return duration;
}
