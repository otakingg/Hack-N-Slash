#include "AirContainerState.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "AirborneModeState.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"
#include "../../../StateMachineComponent.h"

void UAirContainerState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;
    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    // Baseline: we are airborne
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // Engine mode
        // If you transition from Ground → Air, UE will usually already be Falling, but not always (custom moves, launch, etc.)
        locoCMD->SetMovementModeCmd(MOVE_Falling);

        // Stats-driven tuning via locomotion profile
        locoCMD->SetMoveProfileTag(TAG_Move_Profile_Airborne);

        // Clear grounded-only leftovers (safe reset)
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Root);
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Slow);
        // NOTE: do NOT force-remove NoJump here if you plan midair states that intentionally block jump
        // If you want default double-jump enabled, remove it:
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_NoJump);
    }

    // Select rising/falling mode based on velocity
    const bool bGoingUp = (moveComp->Velocity.Z > 0.f);

    if (bGoingUp && risingModeClass)      SetSubState(risingModeClass);
    else if (fallingModeClass)           SetSubState(fallingModeClass);
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

bool UAirContainerState::OnJumpPressed(const FCommandContext& Ctx)
{
    if (!ownerChar) return false;

    // AIR: choose whether you want buffering bookkeeping or not.
    // If you *don't* want buffering/coyote bookkeeping in air, don't call Super.
    // We'll keep your original behavior: record jump desire/time but no buffer timer.
    inputCtx.bWantsJump = true;
    inputCtx.JumpPressedTime = ownerChar ? ownerChar->GetWorld()->GetTimeSeconds() : -1.f;

    // 1) Substate override (double jump variants, glide flap, air dash, etc.)
    if (activeSubState && activeSubState->OnJumpPressed(Ctx)) return true;

    // 2) Default: "UE double-jump" via locomotion interface
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->JumpPressed();
        return true;
    }

    return false;
}

bool UAirContainerState::OnJumpReleased(const FCommandContext& Ctx)
{
    // Call base for consistency (it does nothing by default currently)
    Super::OnJumpReleased(Ctx);

    // 1) Substate override
    if (activeSubState && activeSubState->OnJumpReleased(Ctx)) return true;

    // 2) Default: preserve variable jump height
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->JumpReleased();
        return true;
    }

    return false;
}

bool UAirContainerState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    inputCtx.Look = Look;

    if (activeSubState && activeSubState->OnLookIntent(Look, Ctx)) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddLookInputScaled(Look, turnRate, lookUpRate);
        return true;
    }

    return false;
}
bool UAirContainerState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    inputCtx.Move = Move;

    if (activeSubState && activeSubState->OnMoveIntent(Move, Ctx)) return true;

    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveInput(Move);
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

void UAirContainerState::ClearAirMode()
{
    if (fallingModeClass) SetSubState(fallingModeClass);
}

void UAirContainerState::SetSubState(TSubclassOf<UAirborneModeState> NewSubStateClass)
{
    if (!ownerStateMachineComp) return;

    UClass* DesiredClass = NewSubStateClass.Get();
    if (!DesiredClass) return;

    if (DesiredClass->HasAnyClassFlags(CLASS_Abstract))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    UAirborneModeState* NewState = ownerStateMachineComp->GetMovementState<UAirborneModeState>(NewSubStateClass);
    if (!NewState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState failed: no instance found for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    const UCharacterState* Prev = activeSubState ? Cast<UCharacterState>(activeSubState) : Cast<UCharacterState>(this);
    if (!NewState->CanEnterState(Prev))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();
    activeSubState = NewState;
    activeSubState->EnterState();
}