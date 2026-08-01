#include "EnemSeqProactive.h"
#include "../EnemyBrainComponent.h"

bool UEnemSeqProactive::CanExecute_Implementation() const
{
    if (!brain || !brain->GetOwner() || !brain->GetCharacterMovement() || !brain->GetMesh() || !brain->GetCapsule() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return HasTag(validMovementState) && !HasAnyTag(invalidSequenceTags);
}

float UEnemSeqProactive::GetScore_Implementation() const
{
    if (!brain) return -1.0f;

    float score = baseScore;

    score *= GetAggroMultiplier();     // Aggro
    score *= GetAtkTimeMultiplier();   // Atk Time
    score *= GetDistanceMultiplier();  // Distance
    score *= GetStalenessMultiplier(); // Staleness

    if (bDebug)
    {
        const FString DebugText = FString::Printf(TEXT("%s Score: %.2f"), *GetName(), score);

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, DebugText);
        UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);
    }
    return score;
}

float UEnemSeqProactive::GetAggroMultiplier() const { return aggroCurve ? aggroCurve->GetFloatValue(brain->blackboard.Aggro) : 1.0f; }
float UEnemSeqProactive::GetAtkTimeMultiplier() const
{
    UWorld* world = GetWorld();
    if (!world || brain->blackboard.LastAttackTime < 0.0f) return timeSinceLastAtkCurve ? timeSinceLastAtkCurve->GetFloatValue(-1.0f) : 1.0f;

    float timeSinceLastAtk = world->GetTimeSeconds() - brain->blackboard.LastAttackTime;
    return timeSinceLastAtkCurve ? timeSinceLastAtkCurve->GetFloatValue(timeSinceLastAtk) : 1.0f;
}
float UEnemSeqProactive::GetDistanceMultiplier() const { return distanceCurve ? distanceCurve->GetFloatValue(brain->blackboard.TargetDistance) : 1.0f; }