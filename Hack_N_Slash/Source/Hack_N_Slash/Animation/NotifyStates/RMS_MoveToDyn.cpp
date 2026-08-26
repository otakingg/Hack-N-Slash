#include "RMS_MoveToDyn.h"
#include "../../Characters/Shared/LocomotionComponent.h"

URMS_MoveToDyn::URMS_MoveToDyn()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void URMS_MoveToDyn::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    const FVector startLoc = owner->GetActorLocation();

    float moveToDuration = 0.0f;
    if (duration > 0.0f) moveToDuration = duration;
    else
    {
        const float calcDistance = FVector::Dist(startLoc, locoComp->warpLocation);
        moveToDuration = FMath::Clamp(calcDistance / speed, 0.1f, 0.5f);
    }

    locoComp->ApplyRootMotionSourceMoveToDynamic(startLoc, locoComp->warpLocation, moveToDuration, bRestrictSpeedToExpected);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), locoComp->warpLocation, 25.0f, 12, FColor::Green, false, 2.0f);
}

void URMS_MoveToDyn::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    UAsyncRootMovement* asyncRootMove = locoComp->GetActiveRootMotionOverrideSource();
    if (!asyncRootMove) return;

    asyncRootMove->UpdateMoveToDynamicTargetLocation(locoComp->warpLocation);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), locoComp->warpLocation, 25.0f, 12, FColor::Green, false, 2.0f);
}