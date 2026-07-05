#include "DeadState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
    const IDamageable* Damageable = Cast<IDamageable>(ownerChar);
    return !Damageable || !Damageable->IsAlive();
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

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Attack);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Block);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Dodge);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Jump);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::LockOnOff);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Look);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Move);
    }
}

void UDeadState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Attack);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Block);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Dodge);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Jump);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::LockOnOff);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Look);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Move);
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

    if (NotifyTag.MatchesTagExact(MyTags::NotifyEvent::StateMachine::Grounded) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
        if (ownerChar) ownerChar->SetActorEnableCollision(false);
    }
    else if (NotifyTag.MatchesTagExact(MyTags::NotifyEvent::StateMachine::DeathFreeze) && animInst) animInst->Montage_Pause();
}

void UDeadState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!ownerChar || !animInst || !combatResComp) return;

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (locoComp) locoComp->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();

    if (HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Air)
    {
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().air);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Launch || HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Knockback || HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Knockdown)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = nullptr;
        if (HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Launch) hitReaction = combatResComp->GetHitReactions().launch;
        else if (HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
        else if (HitData.resolvedReaction == MyTags::StateMachine::Action::Reaction::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;

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

FGameplayTag UDeadState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector) { return MyTags::PlayerAction::None; }