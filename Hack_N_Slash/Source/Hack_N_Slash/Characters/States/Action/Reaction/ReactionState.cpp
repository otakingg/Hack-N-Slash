#include "ReactionState.h"

bool UReactionState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return true;

    // Same-layer reaction interruption policy defaults to priority-based.
    // Example: allow Critical to interrupt High, etc.
    return Other->GetPriority() > GetPriority();
}