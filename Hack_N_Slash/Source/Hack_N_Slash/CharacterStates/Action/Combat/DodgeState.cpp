#include "DodgeState.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/LocomotionTags.h"

void UDodgeState::EnterState_Implementation()
{
    Super::EnterState_Implementation();
    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::Lock);
    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::NoJump);
}

void UDodgeState::ExitState_Implementation()
{
    if (locoComp) locoComp->RemoveMoveOverrideTag(OverrideTags::Lock);
    if (locoComp) locoComp->RemoveMoveOverrideTag(OverrideTags::NoJump);
    Super::ExitState_Implementation();
}

FGameplayTag UDodgeState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector)
{
    if (PlayerAction.MatchesTagExact(CharacterActionTags::BlockRelease)) return CharacterActionTags::None;
    else return PlayerAction;
}