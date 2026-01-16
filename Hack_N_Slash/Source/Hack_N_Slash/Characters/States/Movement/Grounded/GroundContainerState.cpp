#include "GroundContainerState.h"
#include "GroundedModeState.h"
#include "../../../StateMachineComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

static ILocomotionCmdInterface* GetLoco(UStateMachineComponent* SM) { return SM ? SM->GetLocomotionCommands() : nullptr; }

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    // If jump was buffered just before landing, execute it now (ground-only)
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        if (jumpStartModeClass) SetSubState(jumpStartModeClass);
        else if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp)) Loco->JumpPressed();
        return;
    }

    // Else always start in default grounded mode
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundContainerState::ExitState()
{
    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }

    Super::ExitState();
}

bool UGroundContainerState::OnJumpPressed(const FCommandContext& Ctx)
{
    // Record buffer/coyote in base (does not consume)
    Super::OnJumpPressed(Ctx);

    if (!ownerChar) return false;

    // 1) Give active substate first right of refusal (climb/wallrun/grind/etc)
    if (activeSubState && activeSubState->OnJumpPressed(Ctx)) return true;

    // 2) Default grounded jump behavior: prefer JumpStart
    if (jumpStartModeClass)
    {
        SetSubState(jumpStartModeClass);
        return true;
    }

    // 3) Fallback: execute jump via locomotion interface
    if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp))
    {
        Loco->JumpPressed();
        return true;
    }

    return false;
}

bool UGroundContainerState::OnJumpReleased(const FCommandContext& Ctx)
{
    Super::OnJumpReleased(Ctx);

    // 1) Let substate override release behavior if needed
    if (activeSubState && activeSubState->OnJumpReleased(Ctx)) return true;

    // 2) Default: preserve variable jump height via locomotion interface
    if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp))
    {
        Loco->JumpReleased();
        return true;
    }

    return false;
}

bool UGroundContainerState::OnLookIntent(const FVector2D& Look, const FCommandContext& Ctx)
{
    // Store inputs at movement layer (useful for animation / steering)
    inputCtx.Look = Look;

    // Forward to substate (not consumed by container unless substate consumes)
    return activeSubState ? activeSubState->OnLookIntent(Look, Ctx) : false;
}

bool UGroundContainerState::OnMoveIntent(const FVector2D& Move, const FCommandContext& Ctx)
{
    inputCtx.Move = Move;
    return activeSubState ? activeSubState->OnMoveIntent(Move, Ctx) : false;
}

void UGroundContainerState::OnLanded(const FHitResult& Hit)
{
    MarkGroundedNow();

    // If buffered jump exists, consume immediately on landing
    if (ownerChar && ConsumeBufferedJumpIfValid())
    {
        if (jumpStartModeClass) SetSubState(jumpStartModeClass);
        else if (ILocomotionCmdInterface* Loco = GetLoco(ownerStateMachineComp)) Loco->JumpPressed();
        return;
    }

    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UGroundContainerState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UGroundContainerState::RequestGroundedMode(TSubclassOf<UGroundedModeState> ModeClass)
{
    if (!ModeClass) return;
    SetSubState(ModeClass);
}

void UGroundContainerState::ClearGroundedMode()
{
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
}

void UGroundContainerState::SetSubState(TSubclassOf<UGroundedModeState> NewSubStateClass)
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

    UGroundedModeState* NewState = ownerStateMachineComp->GetMovementState<UGroundedModeState>(NewSubStateClass);
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