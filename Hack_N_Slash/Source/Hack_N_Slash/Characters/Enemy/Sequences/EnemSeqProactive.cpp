#include "EnemSeqProactive.h"
#include "../EnemyBrainComponent.h"

bool UEnemSeqProactive::CanExecute_Implementation() const
{
    if (!brain || !brain->GetOwner() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return HasTag(validMovementState) && !HasAnyTags(invalidSequenceTags);
}

float UEnemSeqProactive::GetScore_Implementation() const
{
    if (!brain) return -1.0f;

    float score = baseScore;
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

float UEnemSeqProactive::GetDistanceMultiplier() const { return distanceCurve ? distanceCurve->GetFloatValue(brain->blackboard.TargetDistance) : 1.0f; }
float UEnemSeqProactive::GetStalenessMultiplier() const
{
    if (!stalenessCurve)                 return 1.0f;
    else if (lastSequenceTime < 0.0f)    return stalenessCurve->GetFloatValue(-1.0f); // If the sequence never happened before, use -1.0f as the input
    else if (UWorld* world = GetWorld()) return stalenessCurve->GetFloatValue(world->GetTimeSeconds() - lastSequenceTime);
    else                                 return 1.0f;
}