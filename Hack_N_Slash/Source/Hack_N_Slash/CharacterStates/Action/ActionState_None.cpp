#include "ActionState_None.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

void UActionState_None::EnterState()
{
    Super::EnterState();
    if (!ownerChar) return;

    UPlayerCombatComponent* playerCmbtComp = ownerChar->FindComponentByClass<UPlayerCombatComponent>();
    if (!playerCmbtComp) return;

    bool bAirborne = (ownerStateMachineComp && ownerStateMachineComp->IsAirborne()) || (ownerChar->GetCharacterMovement() && ownerChar->GetCharacterMovement()->IsFalling());

    // Entering the none state in the air after having attacked in the air means the aerial combo chain has ended so reset the ability to atk in the air
    if (playerCmbtComp->GetHasAirAttacked() && bAirborne) playerCmbtComp->SetCanAirAtk(false);
}

bool UActionState_None::OnAttackIntent(const FVector2D& InputVector, EPlayerAction PlayerAction)
{
    if (ICombatCmdInterface* iCombatCmd = GetCombatCmd())
    {
        iCombatCmd->AttackIntent(InputVector, PlayerAction);
        return true;
    }
    return false;
}

bool UActionState_None::OnBlockStartIntent()
{
    if (ICombatCmdInterface* iCombatCmd = GetCombatCmd())
    {
        iCombatCmd->BlockStartIntent();
        return true;
    }
    return false;
}

bool UActionState_None::OnDodgeIntent(const FVector2D& InputVector)
{
    if (ICombatCmdInterface* iCombatCmd = GetCombatCmd())
    {
        iCombatCmd->DodgeIntent(InputVector);
        return true;
    }
    return false;
}

bool UActionState_None::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();

    if (!ownerChar) return false;
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStart();
    else ownerChar->Jump();
    return true;
}

bool UActionState_None::OnJumpStopIntent()
{
    if (!ownerChar) return false;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->JumpStop();
    else ownerChar->StopJumping();
    return true;
}