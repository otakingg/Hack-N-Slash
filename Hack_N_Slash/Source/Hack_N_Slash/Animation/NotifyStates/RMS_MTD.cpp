#include "RMS_MTD.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Characters/Shared/LocomotionComponent.h"

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

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    // Get Warp Info
    FVector warpLoc;
    FRotator warpRot;
    locoComp->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());

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
    locoComp->ApplyRootMotionSourceMoveToDynamic(startLoc, warpLoc, duration, bRestrictSpeedToExpected);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}

void URMS_MTD::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;


    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    // Update Warp Info if target is valid
    UAsyncRootMovement* asyncRootMovement = locoComp->GetActiveRootMotionSource();
    if (!asyncRootMovement || !asyncRootMovement->IsActive()) return;

    FVector warpLoc;
    FRotator warpRot;
    locoComp->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());

    asyncRootMovement->UpdateMoveToDynamicTargetLocation(warpLoc);
    FRotator currentRot = owner->GetActorRotation();
    FRotator desiredRot = FMath::RInterpTo(currentRot, warpRot, FrameDeltaTime, roationSpeed);
    owner->SetActorRotation(desiredRot);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}