#include "HitState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UHitState::Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    Super::Initialize_Implementation(InSM, InOwner);

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
    enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
}

void UHitState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Move);
    }

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (moveComp) moveComp->StopMovementImmediately();
}

void UHitState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Move);
    }

    Super::ExitState_Implementation();
}

void UHitState::OnLanded(const FHitResult& Hit) { if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround"); }

void UHitState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::Grounded) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
    }
}

void UHitState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!ownerStateMachineComp) return;

    if (!ownerChar || !animInst || !combatResComp)
    {
        ownerStateMachineComp->ClearActionState();
        return;
    }

    if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Flinch || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Stagger)
    {
        float angle = CalculateHitAngle(HitData);

        FName sectionName;

        if (angle >= -45.f && angle <= 45.f) sectionName = "Front";
        else if (angle > 45.f && angle < 135.f) sectionName = "Right";
        else if (angle < -45.f && angle > -135.f) sectionName = "Left";
        else sectionName = "Back";

        if (bDebug)
        {
            FString SectionString = sectionName.ToString();
            UE_LOG(LogTemp, Warning, TEXT("Section: %s"), *SectionString);
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Section: %s"), *SectionString));
        }

        UAnimMontage* hitReaction = (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Flinch) ? combatResComp->GetHitReactions().flinch : combatResComp->GetHitReactions().stagger;
        animInst->PlayMontageHNS(hitReaction, sectionName);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Air)
    {
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().air);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().launch);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback) ? combatResComp->GetHitReactions().knockBack : combatResComp->GetHitReactions().knockDown;
        animInst->PlayMontageHNS(hitReaction);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak) HandleBlockBreak(HitData);
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Countered)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().countered);
    }
}

void UHitState::ApplyHitForce(const FAtkHitData& HitData)
{
    if (!locoComp || !HitData.damager) return;

    FVector force = HitData.localDir * (HitData.distance / HitData.duration);

    FVector dir = ownerChar->GetActorLocation() - HitData.damager->GetActorLocation();
    dir.Z = 0.0f;
    dir = dir.GetSafeNormal();

    FRotator Rot = dir.Rotation();
    force = Rot.RotateVector(force);
    locoComp->ApplyRootMotionSourceConstant(HitData.duration, force, HitData.velocityOnFinish, HitData.clampVelocityOnFinish, HitData.velocityOnFinishMode, HitData.strengthOverTime, HitData.bAdditive);
}

float UHitState::CalculateHitAngle(const FAtkHitData& HitData) const
{
    // Calculate hit direction
    FVector hitDir = FVector::ZeroVector;
    if (HitData.attacker) hitDir = (HitData.attacker->GetActorLocation() - ownerChar->GetActorLocation()).GetSafeNormal();
    else hitDir = (HitData.hitLoc - ownerChar->GetActorLocation()).GetSafeNormal();
    
    // Flatten
    hitDir.Z = 0.f;
    hitDir.Normalize();

    // ✅ Use ONLY yaw rotation
    FRotator YawRot = ownerChar->GetActorRotation();
    YawRot.Pitch = 0.f;
    YawRot.Roll = 0.f;

    FVector Forward = YawRot.Vector();
    FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    float ForwardDot = FVector::DotProduct(hitDir, Forward);
    float RightDot   = FVector::DotProduct(hitDir, Right);

    float angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    return angle;
}

void UHitState::FaceDamageSource(AActor* Actor, FVector Location)
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

FGameplayTag UHitState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}