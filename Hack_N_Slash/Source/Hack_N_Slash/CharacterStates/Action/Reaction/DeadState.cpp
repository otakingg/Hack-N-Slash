#include "DeadState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Tags/CharacterStateTags.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/Damageable.h"
#include "../../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/OverrideTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

bool UDeadState::CanEnterState_Implementation(const UCharacterState* CurrentState) const
{
    const IDamageable* Damageable = Cast<IDamageable>(ownerChar);
    return !Damageable || !Damageable->IsAlive();
}

void UDeadState::Initialize_Implementation(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize_Implementation(InSM, InOwner);

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
    enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
    enemyController = ownerChar ? Cast<AEnemyController>(ownerChar->GetController()) : nullptr;
}

void UDeadState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(OverrideTags::NoAtk);
        iCmbtInst->AddTag(OverrideTags::NoBlock);
        iCmbtInst->AddTag(OverrideTags::NoDodge);
        iCmbtInst->AddTag(OverrideTags::NoJump);
        iCmbtInst->AddTag(OverrideTags::NoLockOn);
        iCmbtInst->AddTag(OverrideTags::NoLook);
        iCmbtInst->AddTag(OverrideTags::NoMove);
    }
}

void UDeadState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(OverrideTags::NoAtk);
        iCmbtInst->RemoveTag(OverrideTags::NoBlock);
        iCmbtInst->RemoveTag(OverrideTags::NoDodge);
        iCmbtInst->RemoveTag(OverrideTags::NoJump);
        iCmbtInst->RemoveTag(OverrideTags::NoLockOn);
        iCmbtInst->RemoveTag(OverrideTags::NoLook);
        iCmbtInst->RemoveTag(OverrideTags::NoMove);
    }

    Super::ExitState_Implementation();
}

void UDeadState::OnLanded(const FHitResult& Hit)
{
    if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
    if (ownerChar) ownerChar->SetActorEnableCollision(false);
}

void UDeadState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if (NotifyTag.MatchesTagExact(StateMachineTags::Grounded) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
        if (ownerChar) ownerChar->SetActorEnableCollision(false);
    }
    else if (NotifyTag.MatchesTagExact(StateMachineTags::DeathFreeze) && animInst) animInst->Montage_Pause();
}

void UDeadState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!ownerChar || !animInst || !combatResComp) return;

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (locoComp) locoComp->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();
    if (enemyController)
    {
        enemyController->StopMovement();
        enemyController->ClearFocusHNS();
    }

    if (HitData.resolvedReaction == StateReactionTags::Air)
    {
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().air);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == StateReactionTags::Launch || HitData.resolvedReaction == StateReactionTags::Knockback || HitData.resolvedReaction == StateReactionTags::Knockdown)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = nullptr;
        if (HitData.resolvedReaction == StateReactionTags::Launch) hitReaction = combatResComp->GetHitReactions().launch;
        else if (HitData.resolvedReaction == StateReactionTags::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
        else if (HitData.resolvedReaction == StateReactionTags::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;

        animInst->PlayMontageHNS(hitReaction);
        ApplyHitForce(HitData);
    }
    else
    {
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().death);
        ownerChar->SetActorEnableCollision(false);
    }
}

void UDeadState::ApplyHitForce(const FAtkHitData& HitData)
{
    if (!locoComp || !HitData.damager) return;

    FVector force = HitData.localDir.GetSafeNormal() * (HitData.distance / HitData.duration);

    FVector dir = ownerChar->GetActorLocation() - HitData.damager->GetActorLocation();
    dir.Z = 0.0f;
    dir = dir.GetSafeNormal();

    FRotator Rot = dir.Rotation();
    force = Rot.RotateVector(force);
    locoComp->ApplyRootMotionSourceConstant(HitData.duration, force, HitData.velocityOnFinish, HitData.clampVelocityOnFinish, HitData.velocityOnFinishMode, HitData.strengthOverTime, HitData.bAdditive);
}

void UDeadState::FaceDamageSource(AActor* Actor, FVector Location)
{
    if (!ownerChar) return;
    else if (Actor)
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Actor->GetActorLocation());
        desiredRot.Pitch = 0.0f;
        desiredRot.Roll = 0.0f;
        ownerChar->SetActorRotation(desiredRot);
    }
    else
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Location);
        desiredRot.Pitch = 0.0f;
        desiredRot.Roll = 0.0f;
        ownerChar->SetActorRotation(desiredRot);
    }
}

FGameplayTag UDeadState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector) { return CharacterActionTags::None; }