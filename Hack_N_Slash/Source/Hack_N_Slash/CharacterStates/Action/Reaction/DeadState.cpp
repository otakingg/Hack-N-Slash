#include "DeadState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/Damageable.h"
#include "../../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

bool UDeadState::CanEnterState_Implementation(const UCharacterState* CurrentState) const
{
    const IDamageable* damageable = Cast<IDamageable>(ownerChar);
    return Super::CanEnterState_Implementation(CurrentState) && !damageable || !damageable->IsAlive();
}

void UDeadState::Initialize_Implementation(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize_Implementation(InSM, InOwner);

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
    enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
}

void UDeadState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (locoComp) locoComp->ClearAllRootMotionSources();
    if (moveComp) moveComp->StopMovementImmediately();
}

void UDeadState::OnLanded(const FHitResult& Hit)
{
    if (animInst)
    {
        if (UAnimMontage* currentMontage = animInst->GetCurrentActiveMontage())
        {
            animInst->Montage_JumpToSection("Land", currentMontage);
            animInst->Montage_Resume(currentMontage);
        }
    }
    if (ownerChar) ownerChar->SetActorEnableCollision(false);
}

void UDeadState::OnAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if ((NotifyTag.MatchesTag(Tags::NotifyEvent::StateMachine::TryBounceGround) || NotifyTag.MatchesTag(Tags::NotifyEvent::StateMachine::TryLand)) && animInst)
    {
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded)
        {
            if (UAnimMontage* currentMontage = animInst->GetCurrentActiveMontage())
            {
                animInst->Montage_JumpToSection("Land", currentMontage);
                animInst->Montage_Resume(currentMontage);
            }
            if (ownerChar) ownerChar->SetActorEnableCollision(false);
        }
    }
    else if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::IfDeadPauseMontage) && animInst) animInst->Montage_Pause();
}

void UDeadState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!animInst || !combatResComp) return;

    bool bApplyKnockback = true;
    UAnimMontage* hitReaction = nullptr;

    if (HitData.resolvedReaction == EHitReaction::Air)                     hitReaction = combatResComp->GetHitReactions().air;
    else if (HitData.resolvedReaction == EHitReaction::BounceGround)       hitReaction = combatResComp->GetHitReactions().launchDown;
    else if (HitData.resolvedReaction == EHitReaction::LaunchDown)         hitReaction = combatResComp->GetHitReactions().launchDown;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdHard)      hitReaction = combatResComp->GetHitReactions().launchFwdHard;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdSoftBack)  hitReaction = combatResComp->GetHitReactions().launchFwdSoftBack;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdSoftFront) hitReaction = combatResComp->GetHitReactions().launchFwdSoftFront;
    else if (HitData.resolvedReaction == EHitReaction::LaunchUp)           hitReaction = combatResComp->GetHitReactions().launchUp;
    else
    {
        bApplyKnockback = false;
        hitReaction = combatResComp->GetHitReactions().death;
        if (ownerChar) ownerChar->SetActorEnableCollision(false);
    }

    if (animInst->PlayMontageHNS(hitReaction))
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc); // Always snap to hit direction, THIS IS A HACK-N-SLASH GAME!!!
        if (bApplyKnockback) ApplyHitForce(HitData);
    }
}

void UDeadState::FaceDamageSource(AActor* Actor, FVector Location)
{
    if (!ownerChar) return;
    else if (Actor)
    {
        if (UProjectileMovementComponent* pMoveComp = Actor->FindComponentByClass<UProjectileMovementComponent>())
        {
            FVector lookDirection = -pMoveComp->Velocity;
            lookDirection.Z = 0.0f;

            const FRotator DesiredRot = lookDirection.Rotation();
            ownerChar->SetActorRotation(FRotator(0.0f, DesiredRot.Yaw, 0.0f));
        }
        else
        {
            FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Actor->GetActorLocation());
            desiredRot.Pitch = 0.0f;
            desiredRot.Roll = 0.0f;
            ownerChar->SetActorRotation(desiredRot);
        }
    }
    else if (!Location.IsNearlyZero())
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Location);
        desiredRot.Pitch = 0.0f;
        desiredRot.Roll = 0.0f;
        ownerChar->SetActorRotation(desiredRot);
    }
}

void UDeadState::ApplyHitForce(const FAtkHitData& HitData)
{
    if (!locoComp || !ownerChar) return;

    const FVector worldVelocityOnFinish = ownerChar->GetActorTransform().TransformVectorNoScale(HitData.velocityOnFinish); // Convert local velocity into world space
    
    if (HitData.knockBackType == EKnockbackType::Constant)
    {
        FVector kbDirection = FVector::ZeroVector;
        if (HitData.attackType == Tags::Attack::Type::Projectile && HitData.damager)
        {
            if (UProjectileMovementComponent* pMoveComp = HitData.damager->FindComponentByClass<UProjectileMovementComponent>()) kbDirection = pMoveComp->Velocity.GetSafeNormal();
            else kbDirection = HitData.localDir;
        }
        else kbDirection = HitData.localDir;

        if (kbDirection == HitData.localDir)
        {
            // Calculate the direction from the hit location to this actor
            // Flatten hit direction to XY plane. Won't be pushed upward/downward because of the relative height difference between the owner and hit location
            // Normalize because we only care about the direction, not the distance
            FVector dir = HitData.damager ? ownerChar->GetActorLocation() - HitData.damager->GetActorLocation() : ownerChar->GetActorLocation() - HitData.hitLoc;
            dir.Z = 0.0f;
            dir = dir.GetSafeNormal();

            FRotator Rot = dir.Rotation(); // Convert the direction vector into a rotation. EX: If "dir" points east, "Rot" will represent a rotation facing east
            kbDirection = Rot.RotateVector(kbDirection); // Convert the previously calculated LOCAL direction into WORLD space
        }

        FVector velocity = kbDirection * (HitData.distance / HitData.duration);
        locoComp->ApplyRootMotionSourceConstant(HitData.duration, velocity, HitData.velocityOnFinishMode, worldVelocityOnFinish, HitData.clampVelocityOnFinish, HitData.strengthOverTime, HitData.bAdditive);
    }
    else locoComp->ApplyRootMotionSourceMoveTo(HitData.moveToLoc, HitData.duration, HitData.bRestrictSpeedToExpected, HitData.velocityOnFinishMode, worldVelocityOnFinish, HitData.clampVelocityOnFinish);
}

FGameplayTag UDeadState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    FGameplayTagContainer doNothingTags;
    doNothingTags.AddTag(Tags::PlayerAction::BlockRelease);
    doNothingTags.AddTag(Tags::PlayerAction::RegenChakraRelease);

    if (PlayerAction.MatchesAnyExact(doNothingTags)) return Tags::PlayerAction::None;
    else return PlayerAction;
}