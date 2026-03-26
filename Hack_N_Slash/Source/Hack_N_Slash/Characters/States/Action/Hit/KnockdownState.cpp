#include "KnockdownState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../../StateMachineComponent.h"

void UKnockdownState::OnLanded(const FHitResult &Hit)
{
    if (!combatResComp) return;
    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockDown, "HitGround");
}

void UKnockdownState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (NotifyName == "Grounded")
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (bGrounded) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockDown, "HitGround");
    }
}

void UKnockdownState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);
    
    if (!ownerChar || !combatResComp) return;

    UWorld* world = ownerChar->GetWorld();
    if (!world) return;

    if (HitData.attacker)
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), HitData.attacker->GetActorLocation());
        ownerChar->SetActorRotation(desiredRot);
    }
    else
    {
        FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(ownerChar->GetActorLocation(), HitData.hitLoc);
        ownerChar->SetActorRotation(desiredRot);
    }

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().knockDown);

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}
