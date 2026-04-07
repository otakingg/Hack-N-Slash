#include "CanCancelActionNS.h"
#include "../../Combat/Player/PlayerCombatCancelComponent.h"

void UCanCancelActionNS::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!IsValid(MeshComp)) return;

    AActor* owner = MeshComp->GetOwner();
    if (!IsValid(owner)) return;

    playerCombatCancelComp = owner->FindComponentByClass<UPlayerCombatCancelComponent>();
    if (!IsValid(playerCombatCancelComp)) {return;}

    playerCombatCancelComp->SetCanCancelCurrentAction(true);
}

void UCanCancelActionNS::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (playerCombatCancelComp) playerCombatCancelComp->SetCanCancelCurrentAction(false);
}