#include "GroundLocomotionState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../StateMachineComponent.h"

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

bool UGroundLocomotionState::OnMoveIntent(const FGameplayTag& MoveProfile, AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    Super::OnMoveIntent(MoveProfile, Target, Loc, AcceptanceRadius);

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->SetMoveProfileTag(MoveProfile);
        locoCMD->AddMoveInputScaled(Target, Loc, AcceptanceRadius);
        return true;
    }

    return false;
}
