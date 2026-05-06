#include "CanCancelActionNS.h"
#include "../../Combat/Player/PlayerCombatCancelComponent.h"

void UCanCancelActionNS::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    playerCombatCancelComp = owner->FindComponentByClass<UPlayerCombatCancelComponent>();
    if (!playerCombatCancelComp) return;

    playerCombatCancelComp->SetCanCancelCurrentAction(true);
}

void UCanCancelActionNS::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp || !playerCombatCancelComp) return;

    playerCombatCancelComp->SetCanCancelCurrentAction(false);
}