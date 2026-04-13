#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Characters/Player/PlayerCamComponent.h"
#include "../../Combat/Player/PlayerCombatCancelComponent.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"
#include "../../Characters/Shared/StateMachineComponent.h"

/*--------------------------------- UCharacterState ---------------------------------*/

void UCharacterState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    if (bInitialized) return;

    ownerStateMachineComp = InSM;
    ownerChar = InOwner;
    moveComp = ownerChar ? ownerChar->GetCharacterMovement() : nullptr;

    if (ownerStateMachineComp && ownerChar && moveComp) bInitialized = true;
    else UE_LOG(LogTemp, Warning, TEXT("[%s] Initialization failed. StateMachineComp and/or Character is null"), *GetNameSafe(this));
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
    if (ownerStateMachineComp->IsGrounded())
    {
        if (ILocomotionCmdInterface* locoCmd = GetLocoCmd()) locoCmd->MarkGroundedNow();
    }
}

/*--------------------------------- UActionState ---------------------------------*/
void UActionState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize(InSM, InOwner);
    playerCamComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerCamComponent>() : nullptr;
    playerCombatCancelComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerCombatCancelComponent>() : nullptr;
    playerTargettingComp = ownerChar ? ownerChar->FindComponentByClass<UPlayerTargettingComponent>() : nullptr;
}

bool UActionState::OnLookIntent(const FVector2D& InputVector)
{
    if (!playerCamComp) return false;
    playerCamComp->AddLookInputScaled(InputVector);
    return true;
}

bool UActionState::OnToggleLockOnIntent()
{
    if (!playerTargettingComp) return false;
    playerTargettingComp->ToggleLockOn();
    return true;
}

void UActionState::OnAnimNotify(FGameplayTag NotifyTag)
{
    if (NotifyTag.MatchesTagExact(TAG_Notify_StateMachine_ClearActionState)) ownerStateMachineComp->ClearActionState();
}