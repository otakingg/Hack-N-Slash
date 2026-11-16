#include "EnemyBrain.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"

UEnemyBrain::UEnemyBrain()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UEnemyBrain::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    if (UEnemyBrainComponent* brain = owner->FindComponentByClass<UEnemyBrainComponent>())
    {
        for (const FGameplayTag& tag : notifyTags) brain->HandleAnimNotify(tag);
    }
}