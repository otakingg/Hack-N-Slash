#include "CharacterState.h"

void UCharacterState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    ownerStateMachine = InSM;
    ownerCharacter = InOwner;
}

bool UCharacterState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return false;
    return Other->GetPriority() >= GetPriority();
}