#include "GroundContainerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GroundedModeState.h"
#include "../../../Tags/CharacterStateTagNamespaces.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Tags/LocomotionTags.h"
#include "../../../Characters/Player/PlayerCamComponent.h"
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
    if (defaultGroundedModeClass) SetSubState(defaultGroundedModeClass);
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

bool UGroundContainerState::OnLookIntent(const FVector2D& Look)
{
    // Store inputs at movement layer (useful for animation / steering)
    Super::OnLookIntent(Look);

    // Forward to substate
    bool bSubstateConsumed = activeSubState ? activeSubState->OnLookIntent(Look) : false;
    if (bSubstateConsumed) return true;

    if (playerCamComp)
    {
        playerCamComp->AddLookInputScaled(Look);
        return true;
    }
    return false;
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
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] SetSubState rejected: %s is abstract."), *GetNameSafe(this), *GetNameSafe(DesiredClass));
        return;
    }

    if (activeSubState && activeSubState->GetClass() == DesiredClass) return;

    UGroundedModeState* NewState = ownerStateMachineComp->GetMovementState<UGroundedModeState>(NewSubStateClass);
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