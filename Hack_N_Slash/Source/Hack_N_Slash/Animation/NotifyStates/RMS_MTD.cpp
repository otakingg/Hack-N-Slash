#include "RMS_MTD.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

URMS_MTD::URMS_MTD()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void URMS_MTD::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
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

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    // Get Warp Info
    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());

    const FVector startLoc = owner->GetActorLocation();

    // Calculated Root Motion Source Duration
    float moveToDuration = 0.0f;
    if (duration > 0.0f) moveToDuration = duration;
    else
    {
        const float calcDistance = FVector::Dist(startLoc, warpLoc);
        moveToDuration = FMath::Clamp(calcDistance / 2500.f, 0.1f, 0.5f);
    }

    // Apply Root Motion Source
    iLocoCmd->ApplyRootMotionSourceMoveToDynamic(startLoc, warpLoc, duration, bRestrictSpeedToExpected);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}

void URMS_MTD::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    // Update Warp Info if target is valid
    UAsyncRootMovement* asyncRootMovement = iLocoCmd->GetActiveRootMotionSource();
    if (!asyncRootMovement || !asyncRootMovement->IsActive()) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());

    asyncRootMovement->UpdateMoveToDynamicTargetLocation(warpLoc);
    FRotator currentRot = owner->GetActorRotation();
    FRotator desiredRot = FMath::RInterpTo(currentRot, warpRot, FrameDeltaTime, roationSpeed);
    owner->SetActorRotation(desiredRot);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}