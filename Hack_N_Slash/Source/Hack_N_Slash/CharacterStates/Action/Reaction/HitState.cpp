#include "HitState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
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

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (moveComp) moveComp->StopMovementImmediately();
}

void UHitState::ExitState_Implementation()
{
    groundBounceData.Reset();
    Super::ExitState_Implementation();
}

void UHitState::OnJumpApexReached_Implementation() { if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "Apex"); }

void UHitState::OnLanded(const FHitResult& Hit)
{
    if (animInst)
    {
        // Try to Ground Bounce, else just land
        if (CanBounceGround() && animInst->PlayMontageHNS(combatResComp->GetHitReactions().bounceGround)) BounceGround();
        else
        {
            UAnimMontage* hitMontage = animInst->GetCurrentActiveMontage();
            animInst->Montage_JumpToSection("Land", animInst->GetCurrentActiveMontage());
            animInst->Montage_Resume(hitMontage); // Just in case the montage is a pause montage
        }
    }
}

void UHitState::OnAnimNotify_Implementation(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify_Implementation(NotifyTag);

    if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::TryBounceGround) && animInst) // Try to ground bounce, else try to land
    {
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded)
        {
            if (CanBounceGround() && animInst->PlayMontageHNS(combatResComp->GetHitReactions().bounceGround)) BounceGround();
            else
            {
                UAnimMontage* hitMontage = animInst->GetCurrentActiveMontage();
                animInst->Montage_JumpToSection("Land", animInst->GetCurrentActiveMontage());
                animInst->Montage_Resume(hitMontage); // Just in case the montage is a pause montage
            }
        }
    }
    else if (NotifyTag.MatchesTagExact(Tags::NotifyEvent::StateMachine::TryLand) && animInst) // Try to land
    {
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded)
        {
            UAnimMontage* hitMontage = animInst->GetCurrentActiveMontage();
            animInst->Montage_JumpToSection("Land", animInst->GetCurrentActiveMontage());
            animInst->Montage_Resume(hitMontage); // Just in case the montage is a pause montage
        }
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

    FaceDamageSource(HitData.damager, HitData.hitLoc); // Always snap to hit direction, THIS IS A HACK-N-SLASH GAME!!!

    UAnimMontage* hitReaction = nullptr;

    if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Flinch) hitReaction = combatResComp->GetHitReactions().flinch;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Stagger) hitReaction = combatResComp->GetHitReactions().stagger;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Air) hitReaction = combatResComp->GetHitReactions().air;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Launch) hitReaction = combatResComp->GetHitReactions().launch;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BounceGround)
    {
        hitReaction = combatResComp->GetHitReactions().knockDown; // Play the knockdown anim, then play the ground bounce anim when actually hitting the ground
        groundBounceData.damager = HitData.damager;
        groundBounceData.damagerLoc = HitData.damager ? HitData.damager->GetActorLocation() : HitData.hitLoc;
        groundBounceData.damagerRot = HitData.damager ? HitData.damager->GetActorRotation() : FRotator::ZeroRotator;
        groundBounceData.extraBounceHeight = HitData.gbExtraBounceHeight;
        groundBounceData.bAdditive = HitData.bGBAdditive;
        groundBounceData.strengthOverTime = HitData.gbSOT;
        groundBounceData.clampVelocityOnFinish = HitData.gbCVOF;
    }
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::BlockBreak) HandleBlockBreak(HitData);
    else if (HitData.resolvedReaction == Tags::StateMachine::Action::Reaction::Countered) hitReaction = combatResComp->GetHitReactions().countered;

    animInst->PlayMontageHNS(hitReaction);
    ApplyHitForce(HitData);
}

void UHitState::ApplyHitForce(const FAtkHitData& HitData)
{
    if (!locoComp || !ownerChar) return;

    if (HitData.knockBackType == EKnockbackType::Constant)
    {
        FVector force = HitData.localDir * (HitData.distance / HitData.duration);

        // Calculate the direction from the hit location to this actor
        // Flatten hit direction to XY plane. Won't be pushed upward/downward because of the relative height difference between the owner and hit location
        // Normalize because we only care about the direction, not the distance
        FVector dir = HitData.damager ? ownerChar->GetActorLocation() - HitData.damager->GetActorLocation() : ownerChar->GetActorLocation() - HitData.hitLoc;
        dir.Z = 0.0f;
        dir = dir.GetSafeNormal();

        FRotator Rot = dir.Rotation(); // Convert the direction vector into a rotation. EX: If "dir" points east, "Rot" will represent a rotation facing east
        force = Rot.RotateVector(force); // Convert the previously calculated LOCAL force into WORLD space. Rotates the force so it points in the direction the attacker -> this actor vector is facing
        locoComp->ApplyRootMotionSourceConstant(HitData.duration, force, HitData.velocityOnFinishMode, HitData.velocityOnFinish, HitData.clampVelocityOnFinish, HitData.strengthOverTime, HitData.bAdditive);
    }
    else locoComp->ApplyRootMotionSourceMoveTo(ownerChar->GetActorLocation(), HitData.moveToLoc, HitData.duration, HitData.bRestrictSpeedToExpected, HitData.velocityOnFinishMode, HitData.velocityOnFinish, HitData.clampVelocityOnFinish);
}

/*float UHitState::CalculateHitAngle(const FAtkHitData& HitData) const
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
}*/

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

    // Bounce height = damage location height + any extra bounce height desired
    float heightDiffAbs = FMath::Abs(ownerLoc.Z - groundBounceData.damagerLoc.Z);
    FVector bounceLoc = ownerLoc + (ownerChar->GetActorUpVector() * (heightDiffAbs + groundBounceData.extraBounceHeight));

    double bounceDist = FVector::Dist(ownerLoc, bounceLoc);

    float duration = FMath::Clamp(bounceDist / 2000.0f, 0.1f, 1.0f); // Clamp bounce duration for combat feel
    FVector force = (bounceLoc - ownerLoc).GetSafeNormal() * (bounceDist / duration);
    locoComp->ApplyRootMotionSourceConstant(duration, force, ERootMotionFinishVelocityMode::ClampVelocity, FVector::ZeroVector, groundBounceData.clampVelocityOnFinish, groundBounceData.strengthOverTime, groundBounceData.bAdditive);
    groundBounceData.Reset(); // Reset gorund bounce data so when landing again, the character doesn't bounce again
}

FGameplayTag UHitState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}