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
    const IDamageable* damageable = Cast<IDamageable>(ownerChar);
    return !damageable || !damageable->IsAlive();
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
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::LockOnOff);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Look);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Move);
    }

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (locoComp) locoComp->ClearAllRootMotionSources();
    if (moveComp) moveComp->StopMovementImmediately();
}

void UDeadState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::LockOnOff);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Look);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Move);
    }

    Super::ExitState_Implementation();
}

void UDeadState::OnLanded(const FHitResult& Hit)
{
    if (ownerChar) ownerChar->SetActorEnableCollision(false);
    if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Land");
}

void UDeadState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if ((NotifyTag.MatchesTag(Tags::NotifyEvent::StateMachine::TryBounceGround) ||  NotifyTag.MatchesTag(Tags::NotifyEvent::StateMachine::TryLand)) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded)
        {
            if (ownerChar) ownerChar->SetActorEnableCollision(false);
            animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Land");
        }
    }
    else if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::IfDeadPauseMontage) && animInst) animInst->Montage_Pause();
}

void UDeadState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!animInst || !combatResComp) return;

    if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Air)
    {
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().air);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BounceGround)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = nullptr;
        if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch) hitReaction = combatResComp->GetHitReactions().launch;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BounceGround) hitReaction = combatResComp->GetHitReactions().knockDown; // If dead don't ground bounce

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
    if (!ownerChar || !locoComp) return;

    FVector force = HitData.localDir * (HitData.distance / HitData.duration);

    // Calculate the direction from the damager to this actor
    // This means the hit direction is calculated only on the XY plane
    // The actor won't be pushed upward/downward because of the relative height difference between the two actors
    //  Normalize because we only care about the direction, not the distance
    FVector dir = HitData.damager ? ownerChar->GetActorLocation() - HitData.damager->GetActorLocation() : ownerChar->GetActorLocation() - HitData.hitLoc;
    dir.Z = 0.0f;
    dir = dir.GetSafeNormal();

    FRotator Rot = dir.Rotation(); // Convert the direction vector into a rotation. EX: If "dir" points east, "Rot" will represent a rotation facing east
    force = Rot.RotateVector(force); // Convert the previously calculated LOCAL force into WORLD space. Rotates the force so it points in the direction the attacker -> this actor vector is facing
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

FGameplayTag UDeadState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}