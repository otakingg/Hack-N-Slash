#include "CounteredState.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"

void UCounteredState::EnterState()
{
    Super::EnterState();

    if (!ownerChar || !combatResComp) return;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->ClearRootMotionSource();
    if (moveComp) moveComp->StopMovementImmediately();
    if (AAIController* aiController = Cast<AAIController>(ownerChar->GetController())) aiController->StopMovement();

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().countered);
}