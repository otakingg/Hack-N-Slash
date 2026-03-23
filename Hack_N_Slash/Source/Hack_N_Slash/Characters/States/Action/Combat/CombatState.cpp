#include "CombatState.h"

bool UCombatState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return true;

    // Let Reaction always interrupt Combat by default
    if (Other->HasStateTag(FGameplayTag::RequestGameplayTag(TEXT("State.Action.Hit")))) return true;
    
    return Other->GetPriority() >= GetPriority();
}