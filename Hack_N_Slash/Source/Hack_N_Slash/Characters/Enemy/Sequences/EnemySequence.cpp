#include "EnemySequence.h"
#include "../EnemyBrainComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UEnemySequence::Initialize(UEnemyBrainComponent* InBrain)
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

    // Random variation
    // Final score gets slightly varied by at most 10%
    // Feels less robotic and predictable, more organic and varied
    // Restricted to at most 10% because AI should still mostly choose good decisions
    // A smaller vairance means randomness only matters when scores are close, GOOD!
    score *= FMath::FRandRange(0.9f, 1.1f);

    return score;
}

void UEnemySequence::AddMoveOverrideTag(const FGameplayTag& Tag)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;

    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    if (iLocoCmd) iLocoCmd->AddMoveOverrideTag(Tag);
}

void UEnemySequence::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UCharacterMovementComponent* moveComp = owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemySequence::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UCharacterMovementComponent* moveComp = owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;
}