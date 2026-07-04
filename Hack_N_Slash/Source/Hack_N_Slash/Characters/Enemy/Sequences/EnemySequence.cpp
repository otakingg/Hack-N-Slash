#include "EnemySequence.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../Tags/AnimNotifyTags.h"
#include "../../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../../Interfaces/CombatInstigator.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Characters/Shared/LocomotionComponent.h"

void UEnemySequence::Initialize_Implementation(UEnemyBrainComponent* InBrain)
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
    if (!brain || !brain->GetOwner() || !brain->GetCharacterMovement() || !brain->GetMesh() || !brain->GetCapsule() || bOnCooldown || brain->blackboard.bForgotTarget) return false;

    return (!requiredActionState.IsValid() || HasTag(requiredActionState)) && (!requiredMovementState.IsValid() || HasTag(requiredMovementState));
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
    UWorld* world = GetWorld();
    if (!world || lastSequenceTime < 0.0f) return stalenessCurve ? stalenessCurve->GetFloatValue(-1.0f) : 1.0f;

    float timeSinceLastSequence = world->GetTimeSeconds() - lastSequenceTime;
    return stalenessCurve ? stalenessCurve->GetFloatValue(timeSinceLastSequence) : 1.0f;
}

void UEnemySequence::Finish_Implementation() { FinishHelper(); }

bool UEnemySequence::IsActive() const { return brain && brain->GetActiveSequence() == this; }

void UEnemySequence::FinishHelper()
{
    if (!brain) return;

    UWorld* world = GetWorld();
    if (!world) return;

    lastSequenceTime = world->GetTimeSeconds();
    brain->RemoveActiveSequence();
    sequenceIndex = 1;
    bInterruptible = false;

    if (cooldown > 0.0f)
    {
        bOnCooldown = true;
        FTimerManager& timerManager = world->GetTimerManager();
        timerManager.ClearAllTimersForObject(this);
        timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
    }
}

void UEnemySequence::Abort_Implementation() { AbortHelper(); }

void UEnemySequence::AbortHelper()
{
    if (!brain) return;

    sequenceIndex = 1;
    bInterruptible = false;

    if (AEnemyController* controller = brain->GetEnemyController())
    {
        controller->StopMovement();
        controller->ClearFocusHNS();
    }

    SetMovementMode(EMovementMode::MOVE_Walking);

    if (UBaseCharAnimInstance* animInst = brain->GetAnimInstance()) animInst->Montage_Stop(0.25f);
    
    if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
}

float UEnemySequence::GetTargetDistance() const
{
    if (!brain || !brain->blackboard.TargetActor) return -1.0f;

    AActor* owner = brain->GetOwner();
    if (!owner) return -1.0f;

    FVector startLoc = owner->GetActorLocation();
    FVector endLoc = brain->blackboard.TargetActor->GetActorLocation();
    return FVector::Dist(startLoc, endLoc);
}

void UEnemySequence::HandleAnimNotify_Implementation(const FGameplayTag& NotifyTag)
{
    if (!IsActive()) return;
    else if (NotifyTag.MatchesTagExact(EnemyBrainTags::AdvanceSequence)) AdvanceSequence();
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

void UEnemySequence::AddTag(const FGameplayTag& Tag)
{
    if (!IsActive()) return;
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) iCmbtInst->AddTag(Tag);
}

void UEnemySequence::RemoveTag(const FGameplayTag& Tag)
{
    if (!IsActive()) return;
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) iCmbtInst->RemoveTag(Tag);
}

bool UEnemySequence::HasTag(const FGameplayTag& Tag, bool bExact) const
{
    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(brain->GetOwner())) return iCmbtInst->HasTag(Tag, bExact);
    else return false;
}

void UEnemySequence::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!IsActive()) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!IsActive()) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetMovementMode(EMovementMode NewMode, uint8 CustomMode)
{
    if (!IsActive()) return;
    if (UCharacterMovementComponent* moveComp = brain->GetCharacterMovement()) moveComp->SetMovementMode(NewMode, CustomMode);
}