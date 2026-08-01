#include "EnemSeqReactive.h"
#include "../EnemyBrainComponent.h"

bool UEnemSeqReactive::CanExecute_Implementation(const FAtkHitData& HitData) const
{
    if (!brain || !brain->GetOwner() || !brain->GetCharacterMovement() || !brain->GetMesh() || !brain->GetCapsule() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return HasTag(validMovementState) && !HasAnyTag(invalidSequenceTags);
}