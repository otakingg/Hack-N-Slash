#include "LaunchChar.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void ULaunchChar::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ILocomotionCmdInterface* iLocoCmd = nullptr;
    TArray<UActorComponent*> locoComps = owner->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (locoComps.Num() > 0) iLocoCmd = Cast<ILocomotionCmdInterface>(locoComps[0]);

    if (iLocoCmd) iLocoCmd->LaunchCharacterHNS(velocity, bOverrideXY, bOverrideZ, timeToStop);
}