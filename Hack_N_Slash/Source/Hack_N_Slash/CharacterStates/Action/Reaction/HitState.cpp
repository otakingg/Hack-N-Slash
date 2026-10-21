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
    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (moveComp) moveComp->StopMovementImmediately();
}

void UHitState::ExitState_Implementation()
{
    groundBounceData.Reset();
    Super::ExitState_Implementation();
}

void UHitState::OnJumpApexReached_Implementation()
{
    if (animInst)
    {
        UAnimMontage* montage = animInst->GetCurrentActiveMontage();
        if (montage)
        {
            animInst->Montage_JumpToSection("Apex", montage);
            animInst->Montage_Resume(montage); // Just in case the montage needs it
        }
    }
}

void UHitState::OnLanded(const FHitResult& Hit)
{
    if (animInst)
    {
        // Try to Ground Bounce, else just land
        if (CanBounceGround() && animInst->PlayMontageHNS(combatResComp->GetHitReactions().bounceGround)) BounceGround();
        else if (UAnimMontage* montage = animInst->GetCurrentActiveMontage())
        {
            animInst->Montage_JumpToSection("Land", montage);
            animInst->Montage_Resume(montage);
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
            else if (UAnimMontage* montage = animInst->GetCurrentActiveMontage())
            {
                animInst->Montage_JumpToSection("Land", montage);
                animInst->Montage_Resume(montage);
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
            if (UAnimMontage* montage = animInst->GetCurrentActiveMontage())
            {
                animInst->Montage_JumpToSection("Land", montage);
                animInst->Montage_Resume(montage);
            }
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

    UAnimMontage* hitReaction = nullptr;

    if (HitData.resolvedReaction == EHitReaction::Air)                     hitReaction = combatResComp->GetHitReactions().air;
    else if (HitData.resolvedReaction == EHitReaction::BlockBreak)
    {
        animInst->Montage_JumpToSection("Break", animInst->GetCurrentActiveMontage());
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        return;
    }
    else if (HitData.resolvedReaction == EHitReaction::BounceGround)
    {
        hitReaction = combatResComp->GetHitReactions().launchDown; // Play the knockdown anim, then play the ground bounce anim when actually hitting the ground
        groundBounceData.damager = HitData.damager;
        groundBounceData.damagerLoc = HitData.damager ? HitData.damager->GetActorLocation() : HitData.hitLoc;
        groundBounceData.damagerRot = HitData.damager ? HitData.damager->GetActorRotation() : FRotator::ZeroRotator;
        groundBounceData.extraBounceHeight = HitData.gbExtraBounceHeight;
        groundBounceData.bAdditive = HitData.bGBAdditive;
        groundBounceData.strengthOverTime = HitData.gbSOT;
        groundBounceData.clampVelocityOnFinish = HitData.gbCVOF;
    }
    else if (HitData.resolvedReaction == EHitReaction::Countered)          hitReaction = combatResComp->GetHitReactions().countered;
    else if (HitData.resolvedReaction == EHitReaction::LaunchDown)         hitReaction = combatResComp->GetHitReactions().launchDown;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdHard)      hitReaction = combatResComp->GetHitReactions().launchFwdHard;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdSoftBack)  hitReaction = combatResComp->GetHitReactions().launchFwdSoftBack;
    else if (HitData.resolvedReaction == EHitReaction::LaunchFwdSoftFront) hitReaction = combatResComp->GetHitReactions().launchFwdSoftFront;
    else if (HitData.resolvedReaction == EHitReaction::LaunchUp)           hitReaction = combatResComp->GetHitReactions().launchUp;
    else if (HitData.resolvedReaction == EHitReaction::StaggerBack)        hitReaction = combatResComp->GetHitReactions().staggerBack;
    else if (HitData.resolvedReaction == EHitReaction::StaggerDownFront1)  hitReaction = combatResComp->GetHitReactions().staggerDownFront1;
    else if (HitData.resolvedReaction == EHitReaction::StaggerDownFront2)  hitReaction = combatResComp->GetHitReactions().staggerDownFront2;
    else if (HitData.resolvedReaction == EHitReaction::StaggerDownFront3)  hitReaction = combatResComp->GetHitReactions().staggerDownFront3;
    else if (HitData.resolvedReaction == EHitReaction::StaggerUpFront1)    hitReaction = combatResComp->GetHitReactions().staggerUpFront1;
    else if (HitData.resolvedReaction == EHitReaction::StaggerUpFront2)    hitReaction = combatResComp->GetHitReactions().staggerUpFront2;
    else if (HitData.resolvedReaction == EHitReaction::StaggerUpFront3)    hitReaction = combatResComp->GetHitReactions().staggerUpFront3;
    else
    {
        animInst->Montage_Stop(0.25f);
        ownerStateMachineComp->ClearActionState();
        return;
    }

    if (animInst->PlayMontageHNS(hitReaction))
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc); // Always snap to hit direction, THIS IS A HACK-N-SLASH GAME!!!
        ApplyHitForce(HitData);
    }
    else ownerStateMachineComp->ClearActionState();
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

void UHitState::ApplyHitForce(const FAtkHitData& HitData)
{
    if (!locoComp || !ownerChar) return;

    if (HitData.knockBackType == EKnockbackType::Constant)
    {
        FVector velocity = HitData.localDir * (HitData.distance / HitData.duration);

        // Calculate the direction from the hit location to this actor
        // Flatten hit direction to XY plane. Won't be pushed upward/downward because of the relative height difference between the owner and hit location
        // Normalize because we only care about the direction, not the distance
        FVector dir = HitData.damager ? ownerChar->GetActorLocation() - HitData.damager->GetActorLocation() : ownerChar->GetActorLocation() - HitData.hitLoc;
        dir.Z = 0.0f;
        dir = dir.GetSafeNormal();

        FRotator Rot = dir.Rotation(); // Convert the direction vector into a rotation. EX: If "dir" points east, "Rot" will represent a rotation facing east
        velocity = Rot.RotateVector(velocity); // Convert the previously calculated LOCAL velocity into WORLD spaceg
        locoComp->ApplyRootMotionSourceConstant(HitData.duration, velocity, HitData.velocityOnFinishMode, HitData.velocityOnFinish, HitData.clampVelocityOnFinish, HitData.strengthOverTime, HitData.bAdditive);
    }
    else locoComp->ApplyRootMotionSourceMoveTo(ownerChar->GetActorLocation(), HitData.moveToLoc, HitData.duration, HitData.bRestrictSpeedToExpected, HitData.velocityOnFinishMode, HitData.velocityOnFinish, HitData.clampVelocityOnFinish);
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
    FGameplayTagContainer doNothingTags;
    doNothingTags.AddTag(Tags::PlayerAction::BlockRelease);
    doNothingTags.AddTag(Tags::PlayerAction::RegenChakraRelease);

    if (PlayerAction.MatchesAnyExact(doNothingTags)) return Tags::PlayerAction::None;
    else return PlayerAction;
}