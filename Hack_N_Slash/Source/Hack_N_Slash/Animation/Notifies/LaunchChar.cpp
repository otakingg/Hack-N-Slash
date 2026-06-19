#include "LaunchChar.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

ULaunchChar::ULaunchChar()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

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