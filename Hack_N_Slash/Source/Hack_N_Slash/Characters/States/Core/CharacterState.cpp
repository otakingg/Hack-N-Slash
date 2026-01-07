#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/
void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    if (bInitialized) return;
    bInitialized = true;
    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

/*--------------------------------- UMovementState ---------------------------------*/
void UMovementState::EnterState() { moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr; }

void UMovementState::ExitState()
{
    // Base movement state does not own timers or delegates anymore.
}

bool UMovementState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return false;
}

bool UMovementState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return false;
}

bool UMovementState::OnInputJumpPressed()
{
    if (!ownerChar) return false;

    inputCtx.bWantsJump = true;
    inputCtx.jumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();
    return false;
}

bool UMovementState::OnInputJumpReleased() { return false; }
/*--------------------------------- UActionState ---------------------------------*/

bool UActionState::OnInputJumpPressed()
{
    return false;
}

bool UActionState::OnInputJumpReleased()
{
    return false;
}

bool UActionState::OnInputLook(const FVector2D &InputVector)
{
    return false;
}

bool UActionState::OnInputMove(const FVector2D &Move)
{
    return false;
}