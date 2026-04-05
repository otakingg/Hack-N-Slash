#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Player/PlayerCamComponent.h"
#include "../../../Combat/Player/PlayerCombatComponent.h"
#include "../../StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/

void UCharacterState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;
    playerCamComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerCamComponent>() : nullptr;

    if (ownerStateMachineComp && ownerChar) bInitialized = true;
    else if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. StateMachineComp and/or Character is null"), *GetNameSafe(this));
}

void UCharacterState::EnterState()
{
    if (bDebug)
    {
        const FString ClassName = GetNameSafe(this);

        UE_LOG(LogTemp, Log, TEXT("%s: EnterState"), *ClassName);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: EnterState"), *ClassName));
    }
}

void UCharacterState::ExitState()
{
    if (bDebug)
    {
        const FString ClassName = GetNameSafe(this);

        UE_LOG(LogTemp, Log, TEXT("%s: ExitState"), *ClassName);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s: ExitState"), *ClassName));
    }
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState* Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

ILocomotionCmdInterface* UCharacterState::GetLocoCmd() const { return ownerStateMachineComp ? ownerStateMachineComp->GetLocomotionCommands() : nullptr; }
/*--------------------------------- UMovementState ---------------------------------*/
void UMovementState::EnterState()
{
    Super::EnterState();

    // Initialize grounded time if we enter while grounded
    if (ownerChar && moveComp && moveComp->IsMovingOnGround())
    {
        if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    }
}

void UMovementState::ClearAirborneModeDelayed()
{
    UWorld* world = ownerChar->GetWorld();
    if (!world) return;

    FTimerManager& TimerManager = world->GetTimerManager();
    if (TimerManager.IsTimerActive(TH_ClearAirborne)) TimerManager.ClearTimer(TH_ClearAirborne);
    TimerManager.SetTimer(TH_ClearAirborne, ownerStateMachineComp, &UStateMachineComponent::ClearAirborneMode, 0.1f,false);
}

bool UMovementState::OnJumpStartIntent()
{
    Super::OnJumpStartIntent();
    if (!ownerChar) return false;

    if (UWorld* World = ownerChar->GetWorld())
    {
        // Record intent + timestamp (buffer/coyote are checked by consumers later)
        inputCtx.bWantsJump = true;
        inputCtx.JumpPressedTime = World->GetTimeSeconds();
    }

    return false; // not consumed; containers/modes decide what to do
}

bool UMovementState::OnJumpStopIntent()
{
    Super::OnJumpStopIntent();
    return false;
}

bool UMovementState::OnLookIntent(const FVector2D& Look)
{
    inputCtx.Look = Look;
    return false;
}

bool UMovementState::OnMoveIntent(const FVector2D& Move)
{
    inputCtx.Move = Move;
    return false;
}

bool UMovementState::OnMoveIntent(AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    if (bDebug) UE_LOG(LogTemp, Log, TEXT("[%s] OnMoveToIntent: Entered"), *GetNameSafe(this));
    return false;
}

bool UMovementState::ConsumeBufferedJumpIfValid()
{
    ILocomotionCmdInterface* locoCmd = GetLocoCmd();
    if (!locoCmd) return false;

    if (!locoCmd->CanUseBufferedJump(inputCtx.bWantsJump, inputCtx.JumpPressedTime)) return false;
    inputCtx.ClearJump();
    return true;
}

/*--------------------------------- UActionState ---------------------------------*/
void UActionState::OnAnimNotify(FGameplayTag NotifyTag)
{
    if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_ClearActionState) && ownerStateMachineComp)
    {
        if (!ownerChar) return;
        if (UPlayerCombatComponent* playerCmbtComp = ownerChar->FindComponentByClass<UPlayerCombatComponent>()) playerCmbtComp->ClearAtkData();
        
        UActionState* NoneState = ownerStateMachineComp->GetActionState(noneStateClass);
        //ownerStateMachineComp->ChangeActionState(NoneState, false);
        ownerStateMachineComp->ChangeActionState(NoneState, true);
    }
}
