#include "ChakraRegenState.h"
#include "../../../Interfaces/CombatInstigator.h"

bool UChakraRegenState::CanEnterState_Implementation(const UCharacterState* CurrentState) const
{
    return Super::CanEnterState_Implementation(CurrentState) && iCmbtInst &&
    !iCmbtInst->HasTag(Tags::Status::ActionBlocked::RegenChakra) && iCmbtInst->HasTag(Tags::StateMachine::Movement::Walk);
}

void UChakraRegenState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Move);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::RegenChakra);
    }
}

void UChakraRegenState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Move);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::RegenChakra);
    }

    Super::ExitState_Implementation();
}

FGameplayTag UChakraRegenState::ResolvePlayerAction_Implementation(const FGameplayTag &PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}
