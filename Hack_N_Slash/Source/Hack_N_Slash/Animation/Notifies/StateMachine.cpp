#include "StateMachine.h"
#include "../../Characters/Shared/StateMachineComponent.h"

UStateMachine::UStateMachine()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UStateMachine::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UStateMachineComponent* sm = owner->FindComponentByClass<UStateMachineComponent>())
    {
        for (const FGameplayTag& tag : notifyTags) sm->HandleAnimNotify(tag);
    }
}