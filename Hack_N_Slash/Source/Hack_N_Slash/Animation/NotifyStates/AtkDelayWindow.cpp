#include "AtkDelayWindow.h"
#include "../../Combat/Player/PlayerCombatComponent.h"

UAtkDelayWindow::UAtkDelayWindow()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void UAtkDelayWindow::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatComponent* playerCombatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	if (playerCombatComp) playerCombatComp->SetAtkDelayWindow(true);
}

void UAtkDelayWindow::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatComponent* playerCombatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	if (playerCombatComp) playerCombatComp->SetAtkDelayWindow(false);
}
