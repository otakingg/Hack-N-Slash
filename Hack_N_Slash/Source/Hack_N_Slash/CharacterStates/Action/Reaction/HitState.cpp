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
    groundBounceData.Reset();

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

void UHitState::OnJumpApexReached_Implementation() { if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Apex"); }

void UHitState::OnLanded(const FHitResult& Hit)
{
    if (animInst)
    {
        if (CanBounceGround() && animInst->PlayMontageHNS(combatResComp->GetHitReactions().bounceGround)) BounceGround();
        else animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Land");
    }
}

void UHitState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::TryBounceGround) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded)
        {
            if (CanBounceGround() && animInst->PlayMontageHNS(combatResComp->GetHitReactions().bounceGround)) BounceGround();
            else animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Land");
        }
    }
    else if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::TryLand) && animInst)
    {
        bool bGrounded = false;
        if (iCmbtInst) bGrounded = iCmbtInst->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Land");
    }
}

void UHitState::ReceiveHit_Implementation(const FAtkHitData& HitData)
{
    Super::ReceiveHit_Implementation(HitData);

    if (!ownerStateMachineComp) return;

    if (!animInst || !combatResComp)
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
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown || HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BounceGround)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = nullptr;
        if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch) hitReaction = combatResComp->GetHitReactions().launch;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;
        else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BounceGround)
        {
            hitReaction = combatResComp->GetHitReactions().knockDown;
            groundBounceData.damager = HitData.damager;
            groundBounceData.damagerLoc = HitData.damager ? HitData.damager->GetActorLocation() : HitData.hitLoc;
            groundBounceData.damagerRot = HitData.damager ? HitData.damager->GetActorRotation() : FRotator::ZeroRotator;
        }

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
    if (!locoComp || !ownerChar) return;

    FVector force = HitData.localDir * (HitData.distance / HitData.duration);

    // Calculate the direction from the hit location to this actor
    // Flatten hit direction to XY plane. Won't be pushed upward/downward because of the relative height difference between the owner and hit location
    //  Normalize because we only care about the direction, not the distance
    FVector dir = HitData.damager ? ownerChar->GetActorLocation() - HitData.damager->GetActorLocation() : ownerChar->GetActorLocation() - HitData.hitLoc;
    dir.Z = 0.0f;
    dir = dir.GetSafeNormal();

    FRotator Rot = dir.Rotation(); // Convert the direction vector into a rotation. EX: If "dir" points east, "Rot" will represent a rotation facing east
    force = Rot.RotateVector(force); // Convert the previously calculated LOCAL force into WORLD space. Rotates the force so it points in the direction the attacker -> this actor vector is facing
    locoComp->ApplyRootMotionSourceConstant(HitData.duration, force, HitData.velocityOnFinish, HitData.clampVelocityOnFinish, HitData.velocityOnFinishMode, HitData.strengthOverTime, HitData.bAdditive);
}

float UHitState::CalculateHitAngle(const FAtkHitData& HitData) const
{
    if (!ownerChar) return 0.0f;

    // Calculate hit direction
    FVector hitLoc = HitData.damager ? HitData.damager->GetActorLocation() : HitData.hitImpactPoint;
    FVector hitDir = (hitLoc - ownerChar->GetActorLocation()).GetSafeNormal();
    
    // Flatten
    hitDir.Z = 0.f;
    hitDir = hitDir.GetSafeNormal();

    // ✅ Use ONLY yaw rotation
    FRotator yawRot = ownerChar->GetActorRotation();
    yawRot.Pitch = 0.f;
    yawRot.Roll = 0.f;

    FVector forward = yawRot.Vector();
    FVector right   = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

    float forwardDot = FVector::DotProduct(hitDir, forward);
    float rightDot   = FVector::DotProduct(hitDir, right);

    float angle = FMath::RadiansToDegrees(FMath::Atan2(rightDot, forwardDot));
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

bool UHitState::CanBounceGround() const { return groundBounceData.damager && combatResComp && ownerChar && locoComp; }

void UHitState::BounceGround()
{
    FVector ownerLoc = ownerChar->GetActorLocation();

    float heightDiffAbs = FMath::Abs(ownerLoc.Z - groundBounceData.damagerLoc.Z);
    FVector bounceLoc = ownerLoc + (ownerChar->GetActorUpVector() * (heightDiffAbs + groundBounceData.extraBounceHeight));

    double bounceDist = FVector::Dist(ownerLoc, bounceLoc);

    float duration = FMath::Clamp(bounceDist / groundBounceData.bounceSpeed, 0.1f, 1.0f);
    FVector force = (bounceLoc - ownerLoc).GetSafeNormal() * (bounceDist / duration);
    locoComp->ApplyRootMotionSourceConstant(duration, force, groundBounceData.setVelocityOnFinish, groundBounceData.clampVelocityOnFinish, groundBounceData.velocityOnFinishMode, groundBounceData.strengthOverTime, groundBounceData.bIsAdditive);
    groundBounceData.Reset();
}

FGameplayTag UHitState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}