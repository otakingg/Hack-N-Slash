#include "GroundContainerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GroundedModeState.h"
#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UGroundContainerState::EnterState()
{
    Super::EnterState();

    if (!ownerChar) return;
    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return;

    // Baseline: we are grounded
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        // Only force Falling if we are *still falling*.
        // This avoids stomping future custom ground modes (Grinding, Climbing, etc).
        if (moveComp->IsFalling()) locoCMD->SetMovementModeCmd(MOVE_Walking);
        locoCMD->SetMoveProfileTag(TAG_Move_Profile_Grounded);
    }

    // Else always start in default grounded mode
    SetSubState(defaultGroundedStateTag);
}

void UGroundContainerState::ExitState()
{
    if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    if (activeSubState)
    {
        activeSubState->ExitState();
        activeSubState = nullptr;
    }

    Super::ExitState();
}

void UGroundContainerState::GatherStateTags(FGameplayTagContainer& OutTags) const
{
    Super::GatherStateTags(OutTags); // Adds GroundContainer's stateTag

    if (activeSubState) activeSubState->GatherStateTags(OutTags); // Adds mode state's tag(s)
}

bool UGroundContainerState::OnMoveIntent(const FVector2D& Move)
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

bool UGroundContainerState::OnMoveIntent(AActor* Target, const FVector& Loc, float AcceptanceRadius)
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

void UGroundContainerState::OnLanded(const FHitResult& Hit)
{
    if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    if (activeSubState) activeSubState->OnLanded(Hit);
}

void UGroundContainerState::OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
    if (activeSubState) activeSubState->OnMovementModeChanged(InCharacter, PrevMovementMode, PrevCustomMode);
}

void UGroundContainerState::RequestGroundedMode(const FGameplayTag& StateTag) { SetSubState(StateTag); }

void UGroundContainerState::ClearGroundedMode() { SetSubState(defaultGroundedStateTag); }

void UGroundContainerState::SetSubState(const FGameplayTag& StateTag)
{
    if (!ownerStateMachineComp) return;

    UGroundedModeState* newState = Cast<UGroundedModeState>(ownerStateMachineComp->GetMovementStateByTag(StateTag));
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