#include "HitState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Tags/CharacterStateTags.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UHitState::Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    Super::Initialize_Implementation(InSM, InOwner);

    if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
    enemyBrainComp = ownerChar ? ownerChar->FindComponentByClass<UEnemyBrainComponent>() : nullptr;
    enemyController = ownerChar ? Cast<AEnemyController>(ownerChar->GetController()) : nullptr;
}

void UHitState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (locoComp)
    {
        locoComp->AddMoveOverrideTag(OverrideTags::Lock);
        locoComp->AddMoveOverrideTag(OverrideTags::NoJump);
    }
}

void UHitState::ExitState_Implementation()
{
    ExitJuggle();
    if (locoComp)
    {
        locoComp->RemoveMoveOverrideTag(OverrideTags::Lock);
        locoComp->RemoveMoveOverrideTag(OverrideTags::NoJump);
    }

    Super::ExitState_Implementation();
}

void UHitState::EnterJuggle()
{
    if (!moveComp) return;

    UWorld* world = GetWorld();
    if (!world) return;

    FTimerManager& timerManager = world->GetTimerManager();
    if (timerManager.IsTimerActive(TH_Juggle)) timerManager.ClearTimer(TH_Juggle);

    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::MoveStats);
    moveComp->GravityScale = juggleGravity;

    timerManager.SetTimer(TH_Juggle, this, &UHitState::ExitJuggle, gravityRestoreDelay, false);
}

void UHitState::ExitJuggle() { if (locoComp) locoComp->RemoveMoveOverrideTag(OverrideTags::MoveStats); }

void UHitState::OnLanded(const FHitResult& Hit)
{
    if (animInst) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
}

void UHitState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (NotifyTag.MatchesTagExact(StateMachineTags::Grounded) && animInst)
    {
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) animInst->PlayMontageHNS(animInst->GetCurrentActiveMontage(), "HitGround");
    }
}

void UHitState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);

    if (!ownerChar || !animInst || !combatResComp) return;

    if (enemyBrainComp) enemyBrainComp->DeactivateSequence();
    if (locoComp) locoComp->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();
    if (enemyController)
    {
        enemyController->StopMovement();
        enemyController->ClearFocusHNS();
    }

    if (HitData.resolvedReaction == StateReactionTags::Flinch || HitData.resolvedReaction == StateReactionTags::Stagger)
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

        UAnimMontage* hitReaction = (HitData.resolvedReaction == StateReactionTags::Flinch) ? combatResComp->GetHitReactions().flinch : combatResComp->GetHitReactions().stagger;
        animInst->PlayMontageHNS(hitReaction, sectionName);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == StateReactionTags::Air)
    {
        EnterJuggle();
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().air);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == StateReactionTags::Launch)
    {
        EnterJuggle();
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().launch);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == StateReactionTags::Knockback || HitData.resolvedReaction == StateReactionTags::Knockdown)
    {
        ExitJuggle();
        FaceDamageSource(HitData.damager, HitData.hitLoc);

        UAnimMontage* hitReaction = (HitData.resolvedReaction == StateReactionTags::Knockback) ? combatResComp->GetHitReactions().knockBack : combatResComp->GetHitReactions().knockDown;
        animInst->PlayMontageHNS(hitReaction);
        ApplyHitForce(HitData);
    }
    else if (HitData.resolvedReaction == StateReactionTags::BlockHit)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().blockHit);
    }
    else if (HitData.resolvedReaction == StateReactionTags::BlockBreak)
    {
        FaceDamageSource(HitData.damager, HitData.hitLoc);
        animInst->PlayMontageHNS(combatResComp->GetHitReactions().blockBreak);
    }
    else if (HitData.resolvedReaction == StateReactionTags::Countered)
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

FGameplayTag UHitState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}

/*FGameplayTag UHitState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::JumpRelease))         return CharacterActionTags::JumpRelease;
    else if (PlayerAction.MatchesTagExact(CharacterActionTags::LockOnOffStart)) return CharacterActionTags::LockOnOffStart;
    else if (PlayerAction.MatchesTagExact(CharacterActionTags::LookMouse))      return CharacterActionTags::LookMouse;
    else if (PlayerAction.MatchesTagExact(CharacterActionTags::LookStick))      return CharacterActionTags::LookStick;
    return CharacterActionTags::None;
}*/