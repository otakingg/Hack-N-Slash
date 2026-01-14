#include "AirContainerState.h"
#include "GameFramework/Character.h"
#include "AirborneModeState.h"
#include "../../../StateMachineComponent.h"

void UAirContainerState::EnterState()
{
    Super::EnterState();

    // On entry, always start in default air mode
    if (defaultAirModeClass) SetSubState(defaultAirModeClass);
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

bool UAirContainerState::OnInputJumpPressed()
{
    //Super::OnInputJumpPressed();
    // Do this instaed of calling Super() if you don't want air buffering/coyote bookkeeping
    inputCtx.bWantsJump = true;
    inputCtx.jumpPressedTime = ownerChar ? ownerChar->GetWorld()->GetTimeSeconds() : -1.f;

    // 1) Substate override
    if (activeSubState && activeSubState->OnInputJumpPressed()) return true;

    // 2) Default: UE double-jump
    if (!ownerChar) return false;

    ownerChar->Jump();
    return true;
}

bool UAirContainerState::OnInputJumpReleased()
{
    Super::OnInputJumpReleased();

    // 1) Substate override
    if (activeSubState && activeSubState->OnInputJumpReleased()) return true;

    if (!ownerChar) return false;

    ownerChar->StopJumping();
    return true;
}

bool UAirContainerState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return activeSubState ? activeSubState->OnInputLook(Look) : false;
}

bool UAirContainerState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return activeSubState ? activeSubState->OnInputMove(Move) : false;
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
    if (defaultAirModeClass) SetSubState(defaultAirModeClass);
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

    const UCharacterState* prev = activeSubState ? Cast<UCharacterState>(activeSubState) : Cast<UCharacterState>(this);
    if (!NewState->CanEnterState(prev))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: CanEnterState failed (%s)."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState) activeSubState->ExitState();

    activeSubState = NewState;
    activeSubState->EnterState();
}