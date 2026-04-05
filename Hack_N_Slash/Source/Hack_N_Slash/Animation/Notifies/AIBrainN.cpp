#include "AIBrainN.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"

void UAIBrainN::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UEnemyBrainComponent* brain = owner->FindComponentByClass<UEnemyBrainComponent>())
    {
        for (const FGameplayTag& tag : notifyTags) brain->HandleAnimNotify(tag);
    }
}