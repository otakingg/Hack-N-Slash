#include "CombatState.h"
#include "../../../Tags/CharacterStateTagNamespaces.h"

bool UCombatState::CanBeInterruptedBy(const UCharacterState *Other) const
{
    if (!Other) return true;

    // Let Reaction always interrupt Combat by default
    if (Other->HasStateTag(HitTags::Hit)) return true;
    
    return Other->GetPriority() >= GetPriority();
}