#include "ResetCombo.h"
#include "../../Combat/Player/PlayerCombatComponent.h"

UResetCombo::UResetCombo()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Red;
    #endif
}

void UResetCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatComponent* playerCombatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	if (playerCombatComp) playerCombatComp->ResetCombo();
}