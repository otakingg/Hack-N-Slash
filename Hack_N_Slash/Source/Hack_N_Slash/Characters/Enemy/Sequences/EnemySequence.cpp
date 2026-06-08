#include "EnemySequence.h"
#include "../../../Tags/AnimNotifyTags.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
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

float UEnemySequence::GetScore_Implementation() const
{
    if (!brain) return -1.0f;

    float score = baseScore;

    // Aggro
    score *= FMath::Lerp(1.0f, aggroWeight, brain->blackboard.Aggro);

    // Distance
    score *= GetDistanceMultiplier();

    if (bDebug)
    {
        const FString DebugText = FString::Printf(TEXT("%s Score: %.2f"), *GetName(), score);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, DebugText);
        UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);
    }
    return score;
}

float UEnemySequence::GetDistanceMultiplier() const { return distanceScoreCurve ? distanceScoreCurve->GetFloatValue(brain->blackboard.TargetDistance) : 1.0f; }

bool UEnemySequence::CanExecute_Implementation() const { return !bOnCooldown && brain && !brain->blackboard.bStaggered && !brain->blackboard.bForgotTarget && brain->GetOwner(); }

void UEnemySequence::Finish_Implementation()
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
        if (AEnemyController* controller = brain->GetEnemyController()) controller->ClearFocus(EAIFocusPriority::Gameplay);
    }
    else if (NotifyTag.MatchesTagExact(EnemyBrainTags::SetFocus))
    {
        if (!brain) return;
        if (AEnemyController* controller = brain->GetEnemyController()) controller->SetFocus(brain->blackboard.TargetActor);
    }
}

void UEnemySequence::AddMoveOverrideTag(const FGameplayTag& Tag)
{
    if (!brain) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;

    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    if (iLocoCmd) iLocoCmd->AddMoveOverrideTag(Tag);

    //if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Add Move Override Tag"), *GetClass()->GetName()));
}

void UEnemySequence::RemoveMoveOverrideTag(const FGameplayTag &Tag)
{
    if (!brain) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;

    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    if (iLocoCmd) iLocoCmd->RemoveMoveOverrideTag(Tag);

    //if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Remove Move Override Tag"), *GetClass()->GetName()));
}

void UEnemySequence::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!brain) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;

    //if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Set Walk Speed and Acceleration"), *GetClass()->GetName()));
}

void UEnemySequence::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!brain) return;

    UCharacterMovementComponent* moveComp = brain->GetCharacterMovement();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;

    //if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("[%s] Set Fly Speed and Acceleration"), *GetClass()->GetName()));
}

void UEnemySequence::SetMovementMode(EMovementMode NewMode, uint8 CustomMode)
{
    if (!brain) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;

    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    if (iLocoCmd) iLocoCmd->SetMovementModeCmd(NewMode, CustomMode);
}

void UEnemySequence::StopMovementAI()
{
    if (!brain) return;
    
    if (AEnemyController* controller = brain->GetEnemyController()) controller->StopMovement(); // Stop AI Move To
}