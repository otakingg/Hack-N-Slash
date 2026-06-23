#include "EnemySequence.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Characters/Shared/LocomotionComponent.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UEnemySequence::Initialize_Implementation(UEnemyBrainComponent *InBrain)
{
    brain = InBrain;

    if (bStartOnCooldown && cooldown > 0.0f)
    {
        if (UWorld* world = GetWorld())
        {
            bOnCooldown = true;
            world->GetTimerManager().SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
        }
    }
}

bool UEnemySequence::CanExecute_Implementation() const
{
    if (!brain || !bOnCooldown || brain->blackboard.bForgotTarget || !brain->GetOwner()) return false;

    UStateMachineComponent* stateMachineComp = brain->GetStateMachineComp();
    if (!stateMachineComp) return true;

    bool bActionTagMatch = !requiredActionState.IsValid() || stateMachineComp->HasExactActiveTag(requiredActionState);
    bool bMovementTagMatch = !requiredMovementState.IsValid() || stateMachineComp->HasExactActiveTag(requiredMovementState);
    return bActionTagMatch && bMovementTagMatch;
}

float UEnemySequence::GetScore_Implementation() const
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

float UEnemySequence::GetAggroMultiplier() const { return aggroCurve ? aggroCurve->GetFloatValue(brain->blackboard.Aggro) : 1.0f; }
float UEnemySequence::GetAtkTimeMultiplier() const
{
    UWorld* world = GetWorld();
    if (!world || brain->blackboard.LastAttackTime < 0.0f) return timeSinceLastAtkCurve ? timeSinceLastAtkCurve->GetFloatValue(-1.0f) : 1.0f;

    float timeSinceLastAtk = world->GetTimeSeconds() - brain->blackboard.LastAttackTime;
    return timeSinceLastAtkCurve ? timeSinceLastAtkCurve->GetFloatValue(timeSinceLastAtk) : 1.0f;
}
float UEnemySequence::GetDistanceMultiplier() const { return distanceCurve ? distanceCurve->GetFloatValue(brain->blackboard.TargetDistance) : 1.0f; }
float UEnemySequence::GetStalenessMultiplier() const
{
    if (!stalenessCurve) return 1.0f;

    const bool bSameAsPrevious = brain->prevSequenceName == sequenceName;

    // Potential means how many consecutive uses would this sequence be if it happened again
    const int32 potentialConsecutiveUses = bSameAsPrevious ? brain->blackboard.ConsecutiveSequenceUses + 1 : 0;

    return stalenessCurve->GetFloatValue(potentialConsecutiveUses);
}

void UEnemySequence::Finish_Implementation() { FinishHelper(); }

void UEnemySequence::FinishHelper()
{
    if (cooldown > 0.0f)
    {
        if (UWorld* world = GetWorld())
        {
            bOnCooldown = true;
            FTimerManager& timerManager = world->GetTimerManager();
            timerManager.ClearAllTimersForObject(this);
            timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
        }
    }

    sequenceIndex = 1;
    bInterruptible = false;
    if (brain) brain->RemoveActiveSequence();
}

void UEnemySequence::HandleAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    if (NotifyTag.MatchesTagExact(EnemyBrainTags::AdvanceSequence)) AdvanceSequence();
    else if (NotifyTag == EnemyBrainTags::ClearFocus)
    {
        if (!brain) return;
        if (AEnemyController* controller = brain->GetEnemyController()) controller->ClearFocusHNS();
    }
    else if (NotifyTag.MatchesTagExact(EnemyBrainTags::SetFocus))
    {
        if (!brain) return;
        if (AEnemyController* controller = brain->GetEnemyController()) controller->SetFocusHNS(brain->blackboard.TargetActor);
    }
}

void UEnemySequence::AddMoveOverrideTag(const FGameplayTag& Tag)
{
    if (!brain) return;
    if (ULocomotionComponent* locoComp = brain->GetLocoMotionComp()) locoComp->AddMoveOverrideTag(Tag); 
}

void UEnemySequence::RemoveMoveOverrideTag(const FGameplayTag &Tag)
{
    if (!brain) return;
    if (ULocomotionComponent* locoComp = brain->GetLocoMotionComp()) locoComp->RemoveMoveOverrideTag(Tag); 
}

void UEnemySequence::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!brain) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!brain) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetMovementMode(EMovementMode NewMode, uint8 CustomMode)
{
    if (!brain) return;
    if (UCharacterMovementComponent* moveComp = brain->GetCharacterMovement()) moveComp->SetMovementMode(NewMode, CustomMode);
}

void UEnemySequence::StopMovementAI()
{
    if (!brain) return;
    if (AEnemyController* controller = brain->GetEnemyController()) controller->StopMovement(); // Stop AI Move To
}