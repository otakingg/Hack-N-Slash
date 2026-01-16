#include "GroundLocomotionState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../../StateMachineComponent.h"

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
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Ground Loco State: OnLookIntent Entered"));}

    // Option B: delegate to locomotion component
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true; // Consumed (prevents movement layer below, but you're already in movement)
    }
    else if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Ground Loco State: OnLookIntent: Locomotion command interface invalid"));}

    return false;
}

bool UGroundLocomotionState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    Super::OnMoveIntent(Move, Ctx);
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Ground Loco State: OnMoveIntent Entered"));}

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInputScaled(Move, 1.0f);
        return true;
    }
    else if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Ground Loco State: OnMoveIntent: Locomotion command interface invalid"));}

    return false;
}