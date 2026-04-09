#include "DeadState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/CharAnimInterface.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UDeadState::EnterState()
{
    Super::EnterState();
    animationsPlayed = 0;
}

void UDeadState::ExitState()
{
    animationsPlayed = 0;
    Super::ExitState();
}

void UDeadState::OnLanded(const FHitResult& Hit)
{
    if (!combatResComp) return;

    USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
    ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
    if (!iAnimInst) return;

    UAnimMontage* mont = iAnimInst->GetActiveMontage();
    if (!mont) return;
    else if (mont == combatResComp->GetHitReactions().knockBack) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
    else if (mont == combatResComp->GetHitReactions().knockDown) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockDown, "HitGround");
    else if (mont == combatResComp->GetHitReactions().launch) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch, "HitGround");
    else if (mont == combatResComp->GetHitReactions().airStagger) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger, "HitGround");

    if (ownerChar) ownerChar->SetActorEnableCollision(false);
}

void UDeadState::OnAnimNotify(FGameplayTag NotifyTag)
{
    if (!ownerChar) return;

    if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_Grounded) && combatResComp)
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (!bGrounded) return;
    
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
        if (!iAnimInst) return;
        
        UAnimMontage* mont = iAnimInst->GetActiveMontage();
        if (!mont) return;
        else if (mont == combatResComp->GetHitReactions().knockBack) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
        else if (mont == combatResComp->GetHitReactions().knockDown) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockDown, "HitGround");
        else if (mont == combatResComp->GetHitReactions().launch) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch, "HitGround");
        else if (mont == combatResComp->GetHitReactions().airStagger) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger, "HitGround");

        ownerChar->SetActorEnableCollision(false);
    }
    else if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_DeathFreeze))
    {
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
        if (iAnimInst) iAnimInst->PauseMontageHNS();
    }
}

void UDeadState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);
    
    if (!ownerChar || !combatResComp || animationsPlayed > 0) return;

    switch (HitData.attackIntent)
    {
    case EAttackIntent::Knockback:
        montage = combatResComp->GetHitReactions().knockBack;
        FaceDamageSource(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;

    case EAttackIntent::Knockdown:
        montage = combatResComp->GetHitReactions().knockDown;
        FaceDamageSource(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;

    case EAttackIntent::Launch:
        montage = combatResComp->GetHitReactions().launch;
        FaceDamageSource(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;
    
    default:
        bool bAirborne = (ownerStateMachineComp && ownerStateMachineComp->IsAirborne()) || (moveComp && moveComp->IsFalling());
        if (bAirborne)
        {
            montage = combatResComp->GetHitReactions().airStagger;
            if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        }
        else
        {
            montage = combatResComp->GetHitReactions().death;
            ownerChar->SetActorEnableCollision(false);
        }
        break;
    }

    combatResComp->PlayHitReaction(montage);
    ++animationsPlayed;
}