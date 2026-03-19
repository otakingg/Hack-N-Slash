#include "HitState.h"
#include "GameFramework/Character.h"
#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../../../Characters/StateMachineComponent.h"

void UHitState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize(InSM, InOwner);
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
}

void UHitState::OnAnimNotify(FName NotifyName)
{
    if (NotifyName == "ClearActionState" && ownerStateMachineComp)
    {
        UActionState* NoneState = ownerStateMachineComp->GetActionState(noneStateClass);
        ownerStateMachineComp->ChangeActionState(NoneState, false);
        //ownerStateMachineComp->ChangeActionState(NoneState, true);
    }
}