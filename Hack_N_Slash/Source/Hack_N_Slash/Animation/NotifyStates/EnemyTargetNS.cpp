#include "EnemyTargetNS.h"
#include "../../Characters/Enemy/EnemyBrainComponent.h"
#include "../../Characters/Shared/LocomotionComponent.h"

UEnemyTargetNS::UEnemyTargetNS()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void UEnemyTargetNS::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UEnemyBrainComponent* brainComp = owner->FindComponentByClass<UEnemyBrainComponent>();
    if (!brainComp) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    AActor* target = brainComp->blackboard.TargetActor;
    if (!target)
    {
        locoComp->ClearWarpData();
        return;
    }

    FVector ownerLoc = owner->GetActorLocation();
    FVector targetLoc = target->GetActorLocation();

    if (maxTranslDist > 0.0f && FVector::Dist(ownerLoc, targetLoc) > maxTranslDist)
    {
        locoComp->ClearWarpData();
        return;
    }

    if (heightCeiling > 0.0f && FMath::Abs(targetLoc.Z - ownerLoc.Z) > heightCeiling)
    {
        locoComp->ClearWarpData();
        return;
    }

    FVector warpLoc;
    FRotator warpRot;
    locoComp->CalcWarpLocRot(targetLoc, warpLoc, warpRot, offsetDistance, offsetVertical, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug)
    {
        UWorld* world = owner->GetWorld();
        if (!world) return;

        if (ownerLoc.Equals(warpLoc, 10.0f)) DrawDebugSphere(world, warpLoc, 25.0f, 12, FColor::Red, false, 2.0f);
        else                                 DrawDebugSphere(world, warpLoc, 25.0f, 12, FColor::Green, false, 2.0f);
    }
}

void UEnemyTargetNS::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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

    FVector ownerLoc = owner->GetActorLocation();
    FVector targetLoc = target->GetActorLocation();

    if (maxTranslDist > 0.0f && FVector::Dist(ownerLoc, targetLoc) > maxTranslDist) return;

    if (heightCeiling > 0.0f && FMath::Abs(targetLoc.Z - ownerLoc.Z) > heightCeiling) return;

    FVector warpLoc;
    FRotator warpRot;
    locoComp->CalcWarpLocRot(targetLoc, warpLoc, warpRot, offsetDistance, offsetVertical, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug)
    {
        UWorld* world = owner->GetWorld();
        if (!world) return;

        if (ownerLoc.Equals(warpLoc, 10.0f)) DrawDebugSphere(world, warpLoc, 25.0f, 12, FColor::Red, false, 2.0f);
        else                                 DrawDebugSphere(world, warpLoc, 25.0f, 12, FColor::Green, false, 2.0f);
    }
}