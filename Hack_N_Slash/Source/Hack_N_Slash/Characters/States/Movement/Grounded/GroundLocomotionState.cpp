#include "GroundLocomotionState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../StateMachineComponent.h"

// Option B interface
#include "../../Interfaces/LocomotionCmdInterface.h"

static ILocomotionCmdInterface* GetLoco(UStateMachineComponent* SM) { return SM ? SM->GetLocomotionCommands() : nullptr; }

void UGroundLocomotionState::EnterState()
{
    Super::EnterState();

    if (!ownerChar || !moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UGroundLocomotionState] EnterState: ownerChar or moveComp is null."));
        return;
    }

    // Rotation policy
    moveComp->bOrientRotationToMovement = bOrientRotationToMovement;
    moveComp->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
    moveComp->RotationRate = rotationRate;

    // Basic tuning
    moveComp->MaxWalkSpeed = maxWalkSpeed;
    moveComp->MaxAcceleration = maxAcceleration;
    moveComp->BrakingDecelerationWalking = brakingDecelerationWalking;
}

void UGroundLocomotionState::ExitState() { Super::ExitState(); }

bool UGroundLocomotionState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    // Keep recording in base inputCtx (useful for animation, camera, etc.)
    Super::OnLookIntent(Look, Ctx);

    // Option B: delegate to locomotion component
    if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp))
    {
        Loco->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true; // Consumed (prevents movement layer below, but you're already in movement)
    }

    return false;
}

bool UGroundLocomotionState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    Super::OnMoveIntent(Move, Ctx);

    if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp))
    {
        Loco->AddMoveInputScaled(Move, 1.0f);
        return true;
    }

    return false;
}