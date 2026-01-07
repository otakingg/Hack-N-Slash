// Fill out your copyright notice in the Description page of Project Settings.

#include "RootMovementState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../StateMachineComponent.h"

void URootMovementState::EnterState()
{
    Super::EnterState();
    if (!ownerChar || !moveComp) return;

    // Bind delegates once per active root
    ownerChar->LandedDelegate.AddDynamic(this, &URootMovementState::HandleLanded);
    ownerChar->MovementModeChangedDelegate.AddDynamic(this, &URootMovementState::HandleMovementModeChanged);

    // Init grounded time if we start grounded
    if (moveComp->IsMovingOnGround()) lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

    ApplyBaselineContainer();
}

void URootMovementState::ExitState()
{
    if (ownerChar)
    {
        ownerChar->MovementModeChangedDelegate.RemoveDynamic(this, &URootMovementState::HandleMovementModeChanged);
        ownerChar->LandedDelegate.RemoveDynamic(this, &URootMovementState::HandleLanded);

        if (UWorld* World = ownerChar->GetWorld()) World->GetTimerManager().ClearTimer(TH_JumpBuffer);
    }

    if (activeContainer)
    {
        activeContainer->ExitState();
        activeContainer = nullptr;
    }

    Super::ExitState();
}

bool URootMovementState::OnInputMove(const FVector2D& Move)
{
    inputCtx.move = Move;
    return activeContainer ? activeContainer->OnInputMove(Move) : false;
}

bool URootMovementState::OnInputLook(const FVector2D& Look)
{
    inputCtx.look = Look;
    return activeContainer ? activeContainer->OnInputLook(Look) : false;
}

bool URootMovementState::OnInputJumpPressed()
{
    if (!ownerChar) return false;

    inputCtx.bWantsJump = true;
    inputCtx.jumpPressedTime = ownerChar->GetWorld()->GetTimeSeconds();

    StartJumpBufferWindow();
    TryConsumeBufferedJump();

    // Forward too (some containers/substates may want to react)
    return activeContainer ? activeContainer->OnInputJumpPressed() : false;
}

bool URootMovementState::OnInputJumpReleased() { return activeContainer ? activeContainer->OnInputJumpReleased() : false; }

void URootMovementState::HandleLanded(const FHitResult& Hit)
{
    if (!ownerChar) return;

    lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();

    ApplyBaselineContainer();

    // Consume buffered jump on landing if still valid
    TryConsumeBufferedJump();

    if (activeContainer) activeContainer->OnLanded(Hit);
}

void URootMovementState::HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (!ownerChar || !moveComp) return;

    if (moveComp->IsMovingOnGround())
    {
        lastGroundedTime = ownerChar->GetWorld()->GetTimeSeconds();
        TryConsumeBufferedJump();
    }

    ApplyBaselineContainer();

    if (activeContainer) activeContainer->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void URootMovementState::ApplyBaselineContainer()
{
    if (!moveComp) return;

    if (moveComp->IsFalling())
    {
        if (defaultAirContainerClass) SetActiveContainer(defaultAirContainerClass);
    }
    else
    {
        if (defaultGroundContainerClass) SetActiveContainer(defaultGroundContainerClass);
    }
}

void URootMovementState::SetActiveContainer(TSubclassOf<UMovementState> NewContainerClass)
{
    if (!ownerStateMachineComp) return;

    UClass* DesiredClass = NewContainerClass.Get();
    if (!DesiredClass) return;

    if (DesiredClass->HasAnyClassFlags(CLASS_Abstract))
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetActiveContainer rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeContainer && activeContainer->GetClass() == DesiredClass) return;

    UMovementState* NewContainer = ownerStateMachineComp->GetMovementState(NewContainerClass);
    if (!NewContainer)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SetActiveContainer failed: no instance for %s."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeContainer) activeContainer->ExitState();

    activeContainer = NewContainer;
    activeContainer->Initialize(ownerStateMachineComp, ownerChar);
    activeContainer->EnterState();

    // If we swap into a grounded container and had a buffered jump, try immediately.
    TryConsumeBufferedJump();
}

void URootMovementState::StartJumpBufferWindow()
{
    if (!ownerChar) return;
    UWorld* World = ownerChar->GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(TH_JumpBuffer);
    World->GetTimerManager().SetTimer(
        TH_JumpBuffer,
        this,
        &URootMovementState::ExpireJumpBuffer,
        jumpBufferSeconds,
        false
    );
}

void URootMovementState::ExpireJumpBuffer()
{
    inputCtx.ClearJump();
}

bool URootMovementState::CanUseBufferedJump() const
{
    if (!ownerChar || !moveComp) return false;

    const float Now = ownerChar->GetWorld()->GetTimeSeconds();
    const bool bBuffered = inputCtx.bWantsJump && (Now - inputCtx.jumpPressedTime) <= jumpBufferSeconds;
    const bool bGroundOrCoyote = !moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);

    return bBuffered && bGroundOrCoyote;
}

void URootMovementState::TryConsumeBufferedJump()
{
    if (!ownerChar) return;

    if (CanUseBufferedJump())
    {
        // Later: transition the ACTIVE CONTAINER to a Jump substate, not root.
        // e.g. if (auto* Ground = Cast<UGroundContainerState>(activeContainer)) Ground->RequestGroundedMode(JumpStateClass);

        inputCtx.ClearJump();
    }
}
