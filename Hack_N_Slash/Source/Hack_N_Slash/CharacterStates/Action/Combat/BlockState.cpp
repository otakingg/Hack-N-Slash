#include "BlockState.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Tags/LocomotionTags.h"

void UBlockState::EnterState_Implementation()
{
    Super::EnterState_Implementation();
    if (locoComp) locoComp->AddMoveOverrideTag(OverrideTags::Lock);
}

void UBlockState::ExitState_Implementation()
{
    if (locoComp) locoComp->RemoveMoveOverrideTag(OverrideTags::Lock);
    Super::ExitState_Implementation();
}