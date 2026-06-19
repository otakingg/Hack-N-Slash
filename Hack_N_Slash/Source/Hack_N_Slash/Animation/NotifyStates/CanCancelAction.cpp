#include "CanCancelAction.h"
#include "../../Combat/Player/PlayerCombatCancelComponent.h"

UCanCancelAction::UCanCancelAction()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void UCanCancelAction::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatCancelComponent* playerCombatCancelComp = owner->FindComponentByClass<UPlayerCombatCancelComponent>();
    if (!playerCombatCancelComp) return;

    playerCombatCancelComp->SetCanCancelCurrentAction(true);
}

void UCanCancelAction::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatCancelComponent* playerCombatCancelComp = owner->FindComponentByClass<UPlayerCombatCancelComponent>();
    if (!playerCombatCancelComp) return;

    playerCombatCancelComp->SetCanCancelCurrentAction(false);
}