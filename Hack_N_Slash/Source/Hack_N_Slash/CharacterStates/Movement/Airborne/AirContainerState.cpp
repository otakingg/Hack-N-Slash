#include "AirContainerState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AirborneModeState.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

static constexpr float ZVelEpsilon = 5.f;

void UAirContainerState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;
    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    // Baseline: we are airborne (container-level baseline)
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // Only force Falling if we are *still grounded*.
        // This avoids stomping future custom air modes (Flying, Gliding, etc).
        if (moveComp->IsMovingOnGround()) locoCMD->SetMovementModeCmd(MOVE_Falling);

        // Stats-driven tuning via locomotion profile
        // Default move profile will be used unless another is passed in throuhg the "OnMoveIntent" functions
        locoCMD->SetMoveProfileTag(TAG_Move_Profile_Falling);
    }

    ClearAirborneMode();
}

void UAirContainerState::ExitState()
{
    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }

    Super::ExitState();
}

void UAirContainerState::GatherStateTags(FGameplayTagContainer& OutTags) const
{
    Super::GatherStateTags(OutTags); // Adds AirContainer's stateTag

    if (activeSubState) activeSubState->GatherStateTags(OutTags); // Adds mode state's tag(s)
}

bool UAirContainerState::OnMoveIntent(const FVector2D& Move)
{
    Super::OnMoveIntent(Move);
    
    if (activeSubState && activeSubState->OnMoveIntent(Move)) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInput(Move);
        return true;
    }

    return false;
}

bool UAirContainerState::OnMoveIntent(AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    Super::OnMoveIntent(Target, Loc, AcceptanceRadius);

    if (activeSubState && activeSubState->OnMoveIntent(Target, Loc, AcceptanceRadius)) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInput(Target, Loc, AcceptanceRadius);
        return true;
    }

    return false;
}

void UAirContainerState::OnJumpApexReached()
{
    if (activeSubState) activeSubState->OnJumpApexReached();
}

void UAirContainerState::OnLanded(const FHitResult& Hit)
{
    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UAirContainerState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UAirContainerState::RequestAirborneMode(const FGameplayTag& StateTag) { SetSubState(StateTag); }

void UAirContainerState::ClearAirborneMode()
{
    // Select rising/falling mode based on velocity
    const bool bGoingUp = (moveComp->Velocity.Z > ZVelEpsilon);

    if (bGoingUp) SetSubState(risingStateTag);
    else          SetSubState(fallingStateTag);
}

void UAirContainerState::SetSubState(const FGameplayTag& StateTag)
{
    if (!ownerStateMachineComp) return;

    UAirborneModeState* newState = Cast<UAirborneModeState>(ownerStateMachineComp->GetMovementStateByTag(StateTag));
    if (!newState)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *StateTag.ToString());
        return;
    }

    const UCharacterState* prev = activeSubState ? Cast<UCharacterState>(activeSubState) : Cast<UCharacterState>(this);
    if (!newState->CanEnterState(prev))
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *StateTag.ToString());
        return;
    }

    if (activeSubState) activeSubState->ExitState();
    activeSubState = newState;
    activeSubState->EnterState();

    if (ownerStateMachineComp) ownerStateMachineComp->RebuildActiveStateTags();
}