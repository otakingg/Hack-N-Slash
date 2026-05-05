#include "RMS_MTD.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void URMS_MTD::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    owner = MeshComp->GetOwner();
    if (!owner) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    iLocoCmd = nullptr;
    TArray<UActorComponent*> locoComps = owner->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (locoComps.Num() > 0) iLocoCmd = Cast<ILocomotionCmdInterface>(locoComps[0]);
    if (!iLocoCmd) return;

    target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, "RootMotionNotifyState");

    const FVector startLoc = owner->GetActorLocation();
    const float distance = FVector::Dist(startLoc, warpLoc);
    const float duration = FMath::Clamp(distance / 2500.f, 0.1f, 0.5f);

    iLocoCmd->ApplyRootMotionSourceMoveToDynamic(startLoc, warpLoc, duration, bRestrictSpeedToExpected);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}

void URMS_MTD::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp || !owner || !target || !iLocoCmd) return;

    UAsyncRootMovement* asyncRootMovement = iLocoCmd->GetActiveRootMotionSource();
    if (!asyncRootMovement) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, "RootMotionNotifyState");

    asyncRootMovement->UpdateMoveToDynamicTargetLocation(warpLoc);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}