#include "KnockbackState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UKnockbackState::OnLanded(const FHitResult &Hit)
{
    if (combatResComp) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
}

void UKnockbackState::OnAnimNotify(FGameplayTag NotifyTag)
{
    Super::OnAnimNotify(NotifyTag);

    if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_Grounded))
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (bGrounded) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack, "HitGround");
    }
}

void UKnockbackState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);

    if (!ownerChar || !combatResComp) return;

    UWorld* world = ownerChar->GetWorld();
    if (!world) return;

    FaceDamageSource(HitData.attacker, HitData.hitLoc);

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockBack);

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}