#include "CombatState.h"

bool UCombatState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return true;

    // Let Reaction always interrupt Combat by default.
    // (If you later want super-armor, you override this in a specific combat state.)
    if (Other->HasStateTag(FGameplayTag::RequestGameplayTag(TEXT("State.Action.Hit")))) return true;

    // Same-layer combat interruption policy defaults to priority-based.
    // Example: allow High to interrupt Medium, etc.
    return Other->GetPriority() >= GetPriority();
}