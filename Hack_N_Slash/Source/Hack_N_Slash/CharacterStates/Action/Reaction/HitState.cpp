#include "HitState.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UHitState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    Super::Initialize(InSM, InOwner);
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
}

void UHitState::EnterState()
{
    Super::EnterState();
    
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveOverrideTag(OverrideTags::Lock);
        locoCMD->AddMoveOverrideTag(OverrideTags::NoJump);
    }
}

void UHitState::ExitState()
{
    ExitJuggle();
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->RemoveMoveOverrideTag(OverrideTags::Lock);
        locoCMD->RemoveMoveOverrideTag(OverrideTags::NoJump);
    }
    Super::ExitState();
}

void UHitState::EnterJuggle()
{
    if (!moveComp) return;

    UWorld* world = GetWorld();
    if (!world) return;

    FTimerManager& timerManager = world->GetTimerManager();
    if (timerManager.IsTimerActive(TH_Juggle)) timerManager.ClearTimer(TH_Juggle);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->AddMoveOverrideTag(OverrideTags::MoveStats);
    moveComp->GravityScale = juggleGravity;

    timerManager.SetTimer(TH_Juggle, this, &UHitState::ExitJuggle, gravityRestoreDelay, false);
}

void UHitState::ExitJuggle()
{
    if (!moveComp) return;
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(OverrideTags::MoveStats);
}

void UHitState::OnLanded(const FHitResult& Hit)
{
    if (!ownerChar || !combatResComp) return;

    USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
    UAnimInstance* animInst = skeletalMeshComp->GetAnimInstance();
    if (!animInst) return;

    combatResComp->PlayHitReaction(animInst->GetCurrentActiveMontage(), "HitGround");
}

void UHitState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (!ownerChar || !combatResComp) return;

    if (NotifyTag.MatchesTagExact(StateMachineTags::Grounded))
    {
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        UAnimInstance* animInst = skeletalMeshComp->GetAnimInstance();
        if (!animInst) return;
        
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) combatResComp->PlayHitReaction(animInst->GetCurrentActiveMontage(), "HitGround");
    }
}

void UHitState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);

    if (!ownerChar || !combatResComp) return;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();
    if (locoCMD) locoCMD->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();
    if (AAIController* aiController = Cast<AAIController>(ownerChar->GetController())) aiController->StopMovement();

    if (HitData.resolvedReaction == ReactionTags::Flinch || HitData.resolvedReaction == ReactionTags::Stagger)
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

        UAnimMontage* hitReaction = (HitData.resolvedReaction == ReactionTags::Flinch) ? combatResComp->GetHitReactions().flinch : combatResComp->GetHitReactions().stagger;
        combatResComp->PlayHitReaction(hitReaction, sectionName);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else if (HitData.resolvedReaction == ReactionTags::StaggerAir)
    {
        EnterJuggle();
        combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else if (HitData.resolvedReaction == ReactionTags::Launch)
    {
        EnterJuggle();
        FaceDamageSource(HitData.attacker, HitData.hitLoc);
        combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else if (HitData.resolvedReaction == ReactionTags::Knockback || HitData.resolvedReaction == ReactionTags::Knockdown)
    {
        ExitJuggle();
        FaceDamageSource(HitData.attacker, HitData.hitLoc);

        UAnimMontage* hitReaction = (HitData.resolvedReaction == ReactionTags::Knockback) ? combatResComp->GetHitReactions().knockBack : combatResComp->GetHitReactions().knockDown;
        combatResComp->PlayHitReaction(hitReaction);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else if (HitData.resolvedReaction == ReactionTags::BlockHit) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().blockHit);
    else if (HitData.resolvedReaction == ReactionTags::BlockBreak) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().blockBreak);
    else if (HitData.resolvedReaction == ReactionTags::Countered) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().countered);
    else return;
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

    // ✅ Use ONLY yaw rotation (ignores mesh weirdness)
    FRotator YawRot = ownerChar->GetActorRotation();
    YawRot.Pitch = 0.f;
    YawRot.Roll = 0.f;

    FVector Forward = YawRot.Vector(); // Clean forward
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
        ownerChar->SetActorRotation(desiredRot);
    }
    else
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), Location);
        ownerChar->SetActorRotation(desiredRot);
    }
}