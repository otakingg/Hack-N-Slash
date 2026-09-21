#include "IsImmuneReaction.h"
#include "../../Combat/Shared/CombatResolutionComponent.h"

UIsImmuneReaction::UIsImmuneReaction()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void UIsImmuneReaction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* combatResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (combatResComp) combatResComp->SetImmunity(true);
}

void UIsImmuneReaction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* combatResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (combatResComp) combatResComp->SetImmunity(false);
}