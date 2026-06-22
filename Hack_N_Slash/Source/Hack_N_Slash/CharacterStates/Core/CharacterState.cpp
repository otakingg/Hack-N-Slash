#include "CharacterState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../Tags/AnimNotifyTags.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
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
    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
}

ICombatCmdInterface* UCharacterState::GetCombatCmd() const { return ownerStateMachineComp ? ownerStateMachineComp->GetCombatCommands() : nullptr; }
ILocomotionCmdInterface* UCharacterState::GetLocoCmd() const { return ownerStateMachineComp ? ownerStateMachineComp->GetLocomotionCommands() : nullptr; }

/*--------------------------------- UMovementState ---------------------------------*/
/*--------------------------------- UActionState ---------------------------------*/

void UActionState::OnAnimNotify(FGameplayTag NotifyTag)
{
    if (NotifyTag.MatchesTagExact(StateMachineTags::ClearActionState))
    {
        ownerStateMachineComp->ClearActionState();
        //if (playerCombatComp) playerCombatComp->ClearAtkData();
    }
}
