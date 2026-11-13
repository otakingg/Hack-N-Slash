#include "SetReactionOverride.h"
#include "../../Combat/Shared/CombatResolutionComponent.h"

USetReactionOverride::USetReactionOverride()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void USetReactionOverride::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* combatResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (combatResComp) combatResComp->SetReactionOverride(reactionOverride);
}

void USetReactionOverride::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* combatResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (combatResComp) combatResComp->SetReactionOverride(EHitReaction::None);
}