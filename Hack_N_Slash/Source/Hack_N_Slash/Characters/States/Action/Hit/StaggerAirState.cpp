#include "StaggerAirState.h"
#include "GameFramework/Character.h"
#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void UStaggerAirState::EnterState()
{
    Super::EnterState();

    UWorld* world = ownerChar->GetWorld();
    if (!world) return;

    EnterJuggle();
    world->GetTimerManager().SetTimer(TH_Juggle, this, &UStaggerAirState::ExitJuggle, gravityRestoreDelay, false);
}

void UStaggerAirState::ExitState()
{
    ExitJuggle();
    Super::ExitState();
}

void UStaggerAirState::OnLanded(const FHitResult& Hit)
{
    if (combatResComp) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().launch, "HitGround");
}

void UStaggerAirState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);

    if (!ownerChar || !combatResComp) return;

    UWorld* world = ownerChar->GetWorld();
    if (world)
    {
        world->GetTimerManager().ClearTimer(TH_Juggle);
        world->GetTimerManager().SetTimer(TH_Juggle, this, &UStaggerAirState::ExitJuggle, gravityRestoreDelay, false);
    }

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger);

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}