#include "AIBrain.h"
#include "AIBrain.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"

UAIBrain::UAIBrain()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UAIBrain::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UEnemyBrainComponent* brain = owner->FindComponentByClass<UEnemyBrainComponent>())
    {
        for (const FGameplayTag& tag : notifyTags) brain->HandleAnimNotify(tag);
    }
}