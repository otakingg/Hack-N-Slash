#include "CharacterState.h"

void UCharacterState::Initialize(UStateMachineComponent* InSM, ACharacter* InOwner)
{
    ownerStateMachine = InSM;
    ownerCharacter = InOwner;
}

bool UCharacterState::CanEnterState(const UCharacterState* PreviousState) const { return true; }
bool UCharacterState::CanExitState() const { return true; }
bool UCharacterState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}

EStatePriority UCharacterState::GetPriority() const { return EStatePriority::Medium; }
FGameplayTag UCharacterState::GetStateTag() const { return FGameplayTag(); }