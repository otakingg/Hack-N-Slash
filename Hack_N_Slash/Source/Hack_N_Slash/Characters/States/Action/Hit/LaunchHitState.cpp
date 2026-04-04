#include "LaunchHitState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../../StateMachineComponent.h"

void ULaunchHitState::EnterState()
{
    Super::EnterState();

    UWorld* world = ownerChar->GetWorld();
    if (!world) return;

    EnterJuggle();
    world->GetTimerManager().SetTimer(TH_Juggle, this, &ULaunchHitState::ExitJuggle, gravityRestoreDelay, false);
}

void ULaunchHitState::ExitState()
{
    ExitJuggle();
    Super::ExitState();
}

void ULaunchHitState::OnLanded(const FHitResult &Hit)
{
    if (combatResComp) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch, "HitGround");
}

void ULaunchHitState::OnAnimNotify(FName NotifyName)
{
    Super::OnAnimNotify(NotifyName);

    if (NotifyName == "Grounded")
    {
        bool bGrounded = (ownerStateMachineComp && ownerStateMachineComp->IsGrounded()) || (moveComp && moveComp->IsMovingOnGround());
        if (bGrounded) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch, "HitGround");
    }
}

void ULaunchHitState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);

    if (!ownerChar || !combatResComp) return;

    UWorld* world = ownerChar->GetWorld();
    if (world)
    {
        world->GetTimerManager().ClearTimer(TH_Juggle);
        world->GetTimerManager().SetTimer(TH_Juggle, this, &ULaunchHitState::ExitJuggle, gravityRestoreDelay, false);
    }

    FaceDamageSource(HitData.attacker, HitData.hitLoc);

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch);

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}