#include "UpdateMW.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void UUpdateMW::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    ILocomotionCmdInterface* iLocoCmd = nullptr;
    TArray<UActorComponent*> locoComps = owner->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (locoComps.Num() > 0) iLocoCmd = Cast<ILocomotionCmdInterface>(locoComps[0]);
    if (!iLocoCmd) return;

    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, "UpdateMWNotifyState");
    iLocoCmd->UpdateMotionWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}
