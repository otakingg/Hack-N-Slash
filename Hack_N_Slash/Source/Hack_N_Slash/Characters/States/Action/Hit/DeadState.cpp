#include "DeadState.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
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

void UDeadState::OnLanded(const FHitResult &Hit)
{
    if (!combatResComp) return;
    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
}

void UDeadState::OnAnimNotify(FName NotifyName)
{
    if (NotifyName == "Grounded")
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (bGrounded) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
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
        break;

    case EAttackIntent::Knockdown:
        montage = combatResComp->GetHitReactions().knockDown;
        break;

    case EAttackIntent::Launch:
        montage = combatResComp->GetHitReactions().launch;
        break;
    
    default:
        bool bAirborne = (ownerStateMachineComp && ownerStateMachineComp->IsAirborne()) || (moveComp && moveComp->IsFalling());
        montage = bAirborne ? combatResComp->GetHitReactions().airStagger : combatResComp->GetHitReactions().death;
        break;
    }

    combatResComp->PlayHitReaction(montage);
    ++animationsPlayed;
}