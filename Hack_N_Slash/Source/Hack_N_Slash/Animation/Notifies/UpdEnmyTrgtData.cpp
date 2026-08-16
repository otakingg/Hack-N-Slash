#include "UpdEnmyTrgtData.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"

UUpdEnmyTrgtData::UUpdEnmyTrgtData()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void UUpdEnmyTrgtData::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UEnemyBrainComponent* brainComp = owner->FindComponentByClass<UEnemyBrainComponent>();
    if (!brainComp) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    AActor* target = brainComp->blackboard.TargetActor;
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    locoComp->CalcWarpLocRot(target, warpLoc, warpRot, warpLocOffset, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, brainComp->blackboard.bLockedOn);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}