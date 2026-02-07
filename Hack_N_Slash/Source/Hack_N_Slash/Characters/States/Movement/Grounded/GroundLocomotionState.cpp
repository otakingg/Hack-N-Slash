#include "GroundLocomotionState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../StateMachineComponent.h"

void UGroundLocomotionState::EnterState()
{
    Super::EnterState();

    if (!ownerChar || !moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UGroundLocomotionState] EnterState: ownerChar or moveComp is null."));
        return;
    }

    // Rotation policy (state-owned behavior)
    moveComp->bOrientRotationToMovement = bOrientRotationToMovement;
    moveComp->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
    moveComp->RotationRate = rotationRate;

    // Braking behavior
    moveComp->BrakingDecelerationWalking = brakingDecelerationWalking;

    // Friction
    moveComp->GroundFriction = groundFriction;

    // Movement profile (stats-driven numbers applied by locomotion component)
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->SetMoveProfileTag(TAG_Move_Profile_Ground_Jog);
}

void UGroundLocomotionState::ExitState() { Super::ExitState(); }

bool UGroundLocomotionState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    // Keep recording in base inputCtx (useful for animation, camera, etc.)
    Super::OnLookIntent(Look, Ctx);

    // Delegate to locomotion component
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true; // Consumed (prevents movement layer below, but you're already in movement)
    }

    return false;
}

bool UGroundLocomotionState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    Super::OnMoveIntent(Move, Ctx);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInputScaled(Move);
        return true;
    }

    return false;
}