#include "JumpState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UJumpState::OnJumpApexReached() { ownerStateMachineComp->ClearActionState(); }