#include "EnemSeqReactive.h"
#include "../EnemyBrainComponent.h"

bool UEnemSeqReactive::CanExecute_Implementation(const FAtkHitData& HitData) const
{
    if (!brain || !brain->GetOwner() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return HasTag(validMovementState) && !HasAnyTags(invalidSequenceTags);
}

bool UEnemSeqReactive::RollChance() const
{
    float totalRecentHitsModifier = recentHitsModifier * brain->blackboard.RecentHits; // Each recent hit increases the chance by "recentHitsModifier";
    float totalChance = FMath::Clamp(chance + totalRecentHitsModifier, 0.0f, 1.0f); // Clamp the chance between 0 & 1

    if (bDebug)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Rolling chance for sequence %s: Base chance = %.2f, Recent hits modifier = %.2f, Total chance = %.2f"), *sequenceName.ToString(), chance, totalRecentHitsModifier, totalChance));
        UE_LOG(LogTemp, Warning, TEXT("Rolling chance for sequence %s: Base chance = %.2f, Recent hits modifier = %.2f, Total chance = %.2f"), *sequenceName.ToString(), chance, totalRecentHitsModifier, totalChance);
    }

    if (totalChance == 0.0f) return false;
    else if (totalChance == 1.0f) return true;
    else return FMath::RandRange(0.0f, 1.0f) <= totalChance;
}

bool UEnemSeqReactive::IsFacingActor(AActor* Actor, float Tolerance) const
{
    if (!brain || !Actor) return false;

    AActor* owner = brain->GetOwner();
    if (!owner) return false;

    Tolerance = FMath::Clamp(Tolerance, -1.0f, 1.0f);

    FVector ownerLoc = owner->GetActorLocation();
    FVector targetLoc = Actor->GetActorLocation();
    FVector dirToTarget = (targetLoc - ownerLoc).GetSafeNormal();
    FVector ownerForward = owner->GetActorForwardVector();

    return FVector::DotProduct(dirToTarget, ownerForward) >= Tolerance;
}