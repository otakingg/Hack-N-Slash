#include "DeadState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../../Interfaces/CharAnimInterface.h"
#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../StateMachineComponent.h"

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
    else if (mont == combatResComp->GetHitReactions().deathKnockBack) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathKnockBack, "HitGround");
    else if (mont == combatResComp->GetHitReactions().deathKnockDown) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathKnockDown, "HitGround");
    else if (mont == combatResComp->GetHitReactions().deathLaunch) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathLaunch, "HitGround");
    else if (mont == combatResComp->GetHitReactions().deathAirStagger) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathAirStagger, "HitGround");

    if (ownerChar) ownerChar->SetActorEnableCollision(false);
}

void UDeadState::OnAnimNotify(FName NotifyName)
{
    if (!ownerChar) return;

    if (NotifyName == "Grounded" && combatResComp)
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (!bGrounded) return;
    
        USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh();
        ICharAnimInterface* iAnimInst = Cast<ICharAnimInterface>(skeletalMeshComp->GetAnimInstance());
        if (!iAnimInst) return;
        
        UAnimMontage* mont = iAnimInst->GetActiveMontage();
        if (!mont) return;
        else if (mont == combatResComp->GetHitReactions().deathKnockBack) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathKnockBack, "HitGround");
        else if (mont == combatResComp->GetHitReactions().deathKnockDown) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathKnockDown, "HitGround");
        else if (mont == combatResComp->GetHitReactions().deathLaunch) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathLaunch, "HitGround");
        else if (mont == combatResComp->GetHitReactions().deathAirStagger) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().deathAirStagger, "HitGround");  

        ownerChar->SetActorEnableCollision(false);
    }
}

void UDeadState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);
    
    if (!ownerChar || !combatResComp || animationsPlayed > 0) return;

    switch (HitData.attackIntent)
    {
    case EAttackIntent::Knockback:
        montage = combatResComp->GetHitReactions().deathKnockBack;
        FaceActorOrLocation(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;

    case EAttackIntent::Knockdown:
        montage = combatResComp->GetHitReactions().deathKnockDown;
        FaceActorOrLocation(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;

    case EAttackIntent::Launch:
        montage = combatResComp->GetHitReactions().deathLaunch;
        FaceActorOrLocation(HitData.attacker, HitData.hitLoc);
        if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
        break;
    
    default:
        bool bAirborne = (ownerStateMachineComp && ownerStateMachineComp->IsAirborne()) || (moveComp && moveComp->IsFalling());
        if (bAirborne)
        {
            montage = combatResComp->GetHitReactions().deathAirStagger;
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