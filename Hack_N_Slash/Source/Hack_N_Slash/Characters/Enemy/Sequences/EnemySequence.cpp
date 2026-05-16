#include "EnemySequence.h"
#include "../EnemyBrainComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

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