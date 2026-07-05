#include "DodgeState.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../../../Characters/Shared/LocomotionComponent.h"

void UDodgeState::EnterState_Implementation()
{
    Super::EnterState_Implementation();

    if (iCmbtInst)
    {
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Attack);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Block);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Dodge);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Jump);
        iCmbtInst->AddTag(MyTags::Status::ActionBlocked::Move);
    }
}

void UDodgeState::ExitState_Implementation()
{
    if (iCmbtInst)
    {
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Attack);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Block);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Dodge);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Jump);
        iCmbtInst->RemoveTag(MyTags::Status::ActionBlocked::Move);
    }

    Super::ExitState_Implementation();
}

FGameplayTag UDodgeState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(MyTags::PlayerAction::BlockRelease)) return MyTags::PlayerAction::None;
    else return PlayerAction;
}