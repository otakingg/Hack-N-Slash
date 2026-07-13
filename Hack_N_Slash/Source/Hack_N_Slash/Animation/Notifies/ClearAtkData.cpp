#include "ClearAtkData.h"
#include "../../Combat/Player/PlayerCombatComponent.h"

UClearAtkData::UClearAtkData()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UClearAtkData::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatComponent* playerCombatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	if (!playerCombatComp) return;

    playerCombatComp->ClearAtkData();
}