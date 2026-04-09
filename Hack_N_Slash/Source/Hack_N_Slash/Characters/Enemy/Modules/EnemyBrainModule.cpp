#include "EnemyBrainModule.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/Shared/StateMachineComponent.h"

void UEnemyBrainModule::SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UCharacterMovementComponent* moveComp = owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp) return;

    moveComp->MaxWalkSpeed = WalkSpeed;
    moveComp->MaxAcceleration = Acceleration;
}

void UEnemyBrainModule::SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UCharacterMovementComponent* moveComp = owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp) return;

    moveComp->MaxFlySpeed = FlySpeed;
    moveComp->MaxAcceleration = Acceleration;
}


void UEnemyBrainModule::StopMovingHNS()
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;
    
    if (AEnemyController* controller = brain->GetEnemyController()) controller->StopMovement(); // Stop AI Move To
}

void UEnemyBrainModule::AddMoveOverrideTag(const FGameplayTag& Tag)
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    if (!smComp) return;

    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    if (iLocoCmd) iLocoCmd->AddMoveOverrideTag(Tag);
}
