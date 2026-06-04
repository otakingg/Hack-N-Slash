#include "DeathState.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/CharAnimInterface.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Structs/FAtkHitData.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UDeathState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    Super::Initialize(InSM, InOwner);
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
}

void UDeathState::EnterState()
{
    Super::EnterState();
    
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->AddMoveOverrideTag(TAG_Move_Override_NoJump);
    }
}

void UDeathState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_NoJump);
    }
    Super::ExitState();
}

void UDeathState::OnLanded(const FHitResult& Hit)
{
    if (!ownerChar) return;

    USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
    ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
    if (!iAnimInst) return;

    if (combatResComp) combatResComp->PlayHitReaction(iAnimInst->GetActiveMontage(), "HitGround");
    ownerChar->SetActorEnableCollision(false);
}

void UDeathState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (!ownerChar) return;

    if (NotifyTag.MatchesTagExact(StateMachineTags::Grounded))
    {
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
        if (!iAnimInst) return;
        
        bool bGrounded = false;
        if (ownerStateMachineComp) bGrounded = ownerStateMachineComp->IsGrounded();
        else if (moveComp) bGrounded = moveComp->IsMovingOnGround();

        if (bGrounded) combatResComp->PlayHitReaction(iAnimInst->GetActiveMontage(), "HitGround");
        ownerChar->SetActorEnableCollision(false);
    }
    else if (NotifyTag.MatchesTagExact(StateMachineTags::DeathFreeze))
    {
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
        if (iAnimInst) iAnimInst->PauseMontageHNS();
    }
}

void UDeathState::ReceiveHit(const FAtkHitData& HitData)
{
    if (!ownerChar || !combatResComp) return;

    ILocomotionCmdInterface* locoCMD = GetLocoCmd();
    if (locoCMD) locoCMD->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();
    if (AAIController* aiController = Cast<AAIController>(ownerChar->GetController())) aiController->StopMovement();
    
    if (HitData.resolvedReaction == ReactionTags::StaggerAir)
    {
        combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else if (HitData.resolvedReaction == ReactionTags::Launch || HitData.resolvedReaction == ReactionTags::Knockback || HitData.resolvedReaction == ReactionTags::Knockdown)
    {
        FaceDamageSource(HitData.attacker, HitData.hitLoc);

        UAnimMontage* hitReaction = nullptr;
        if (HitData.resolvedReaction == ReactionTags::Launch) hitReaction = combatResComp->GetHitReactions().launch;
        else if (HitData.resolvedReaction == ReactionTags::Knockback) hitReaction = combatResComp->GetHitReactions().knockBack;
        else if (HitData.resolvedReaction == ReactionTags::Knockdown) hitReaction = combatResComp->GetHitReactions().knockDown;

        combatResComp->PlayHitReaction(hitReaction);
        if (locoCMD) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
    }
    else
    {
        combatResComp->PlayHitReaction(combatResComp->GetHitReactions().death);
        ownerChar->SetActorEnableCollision(false);
    }
}

void UDeathState::FaceDamageSource(AActor* Actor, FVector Location)
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