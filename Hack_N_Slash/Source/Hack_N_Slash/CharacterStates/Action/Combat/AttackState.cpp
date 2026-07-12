#include "AttackState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Interfaces/CombatInstigator.h"

void UAttackState::EnterState_Implementation()
{
    Super::EnterState_Implementation();
    if (iCmbtInst)
    {
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->AddTag(Tags::Status::ActionBlocked::Move);
    }
}

void UAttackState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Attack);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Block);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Dodge);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Jump);
        iCmbtInst->RemoveTag(Tags::Status::ActionBlocked::Move);
    }
    Super::ExitState_Implementation();
}

FGameplayTag UAttackState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}