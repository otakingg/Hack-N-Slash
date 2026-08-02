#include "EnemSeqReactive.h"
#include "../EnemyBrainComponent.h"

bool UEnemSeqReactive::CanExecute_Implementation(const FAtkHitData& HitData) const
{
    if (!brain || !brain->GetOwner() || !brain->GetCharacterMovement() || !brain->GetMesh() || !brain->GetCapsule() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return HasTag(validMovementState) && !HasAnyTag(invalidSequenceTags);
}

bool UEnemSeqReactive::IsFacingTarget(AActor* Target, float Tolerance) const
{
    if (!brain || !Target) return false;

    AActor* owner = brain->GetOwner();
    if (!owner) return false;

    Tolerance = FMath::Clamp(Tolerance, -1.0f, 1.0f);

    FVector ownerLoc = owner->GetActorLocation();
    FVector targetLoc = Target->GetActorLocation();
    FVector dirToTarget = (targetLoc - ownerLoc).GetSafeNormal();
    FVector ownerForward = owner->GetActorForwardVector();

    return FVector::DotProduct(dirToTarget, ownerForward) >= Tolerance;
}