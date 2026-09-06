#include "OpenAtkDelayWindow.h"
#include "../../Combat/Player/PlayerCombatComponent.h"

UOpenAtkDelayWindow::UOpenAtkDelayWindow()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}

void UOpenAtkDelayWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UPlayerCombatComponent* playerCombatComp = owner->FindComponentByClass<UPlayerCombatComponent>();
	if (playerCombatComp) playerCombatComp->bAtkDelayWindow = true;
}