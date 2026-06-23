#include "LaunchChar.h"
#include "../../Characters/Shared/LocomotionComponent.h"

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

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (locoComp) locoComp->LaunchCharacterHNS(velocity, bOverrideXY, bOverrideZ, timeToStop);
}