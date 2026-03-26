#include "EnemyBrainModule.h"
#include "../EnemyBrainComponent.h"
#include "../../../Controllers/EnemyController.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Characters/StateMachineComponent.h"

void UEnemyBrainModule::StopMovingHNS()
{
    if (!brain) return;

    AActor* owner = brain->GetOwner();
    if (!owner) return;

    UStateMachineComponent* smComp = brain->GetStateMachine();
    ILocomotionCmdInterface* iLocoCmd = smComp->GetLocomotionCommands();
    
    if (iLocoCmd) iLocoCmd->SetMoveProfileTag(TAG_Move_Profile_Idle); // Not moving, so set Idle
    if (AEnemyController* controller = brain->GetEnemyController()) controller->StopMovement(); // Stop AI Move To
}