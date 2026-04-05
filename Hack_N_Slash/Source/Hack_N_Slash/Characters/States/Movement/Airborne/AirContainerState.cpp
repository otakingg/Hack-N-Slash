#include "AirContainerState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AirborneModeState.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../Player/PlayerCamComponent.h"
#include "../../../StateMachineComponent.h"

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

bool UAirContainerState::OnJumpStartIntent()
{
    if (!ownerChar) return false;

    // Record press + timestamp in base (shared jump buffer/coyote bookkeeping)
    Super::OnJumpStartIntent();

    // If someone accidentally presses jump twice in coyote time, block it
    if (activeSubState && activeSubState->IsA(airJumpStartModeClass)) return true;

    // Coyote consumption: allow FIRST jump shortly after leaving ground
    if (ConsumeBufferedJumpIfValid() && airJumpStartModeClass)
    {
        SetSubState(airJumpStartModeClass);
        return true;
    }

    // Substate override (double jump variants, glide flap, air dash, etc.)
    if (activeSubState && activeSubState->OnJumpStartIntent()) return true;

    // Default UE double-jump (Jump() again)
    ILocomotionCmdInterface* locoCMD = GetLocoCmd();
    if (locoCMD && locoCMD->CanMultiJump())
    {
        locoCMD->JumpPressed();
        ClearAirborneModeDelayed();
        return true;
    }

    return false;
}

bool UAirContainerState::OnJumpStopIntent()
{
    Super::OnJumpStopIntent();

    if (activeSubState && activeSubState->OnJumpStopIntent()) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->JumpReleased();
        return true;
    }

    return false;
}

bool UAirContainerState::OnLookIntent(const FVector2D& Look)
{
    Super::OnLookIntent(Look);

    // Forward to substate (not consumed by container unless substate consumes)
    bool bSubstateConsumed = activeSubState ? activeSubState->OnLookIntent(Look) : false;
    if (bSubstateConsumed) return true;

    if (playerCamComp)
    {
        playerCamComp->AddLookInputScaled(Look);
        return true;
    }
    return false;
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

void UAirContainerState::RequestAirborneMode(TSubclassOf<UAirborneModeState> ModeClass)
{
    if (!ModeClass) return;
    SetSubState(ModeClass);
}

void UAirContainerState::ClearAirborneMode()
{
    // Select rising/falling mode based on velocity
    const bool bGoingUp = (moveComp->Velocity.Z > ZVelEpsilon);

    if (bGoingUp && risingModeClass)      SetSubState(risingModeClass);
    else if (fallingModeClass)           SetSubState(fallingModeClass);
}

void UAirContainerState::SetSubState(TSubclassOf<UAirborneModeState> NewSubStateClass)
{
    if (!ownerStateMachineComp) return;

    UClass* DesiredClass = NewSubStateClass.Get();
    if (!DesiredClass) return;

    if (DesiredClass->HasAnyClassFlags(CLASS_Abstract))
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    UAirborneModeState* NewState = ownerStateMachineComp->GetMovementState<UAirborneModeState>(NewSubStateClass);
    if (!NewState)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    const UCharacterState* Prev = activeSubState ? Cast<UCharacterState>(activeSubState) : Cast<UCharacterState>(this);
    if (!NewState->CanEnterState(Prev))
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();
    activeSubState = NewState;
    activeSubState->EnterState();

    if (ownerStateMachineComp) ownerStateMachineComp->RebuildActiveStateTags();
}