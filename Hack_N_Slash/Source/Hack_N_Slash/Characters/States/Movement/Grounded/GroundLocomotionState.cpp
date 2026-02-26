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

    // Default Movement profile (stats-driven numbers applied by locomotion component)
    // Will be used if none was passed in throuhg the "OnMoveIntent" functions
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->SetMoveProfileTag(TAG_Move_Profile_Ground_Jog);
}

void UGroundLocomotionState::ExitState() { Super::ExitState(); }

bool UGroundLocomotionState::OnLookIntent(const FVector2D& Look)
{
    // Keep recording in base inputCtx (useful for animation, camera, etc.)
    Super::OnLookIntent(Look);

    // Delegate to locomotion component
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true;
    }

    return false;
}

bool UGroundLocomotionState::OnMoveIntent(const FVector2D& Move)
{
    Super::OnMoveIntent(Move);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInputScaled(Move);
        return true;
    }

    return false;
}

bool UGroundLocomotionState::OnMoveIntent(AActor* Target, const FVector& Loc, const FGameplayTag& MoveProfile, float AcceptanceRadius)
{
    Super::OnMoveIntent(Target, Loc, MoveProfile, AcceptanceRadius);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->SetMoveProfileTag(MoveProfile);
        locoCMD->AddMoveInputScaled(Target, Loc, AcceptanceRadius);
        return true;
    }

    return false;
}
