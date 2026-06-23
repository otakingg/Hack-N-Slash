#include "UpdateWarp.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Characters/Shared/LocomotionComponent.h"

UUpdateWarp::UUpdateWarp()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void UUpdateWarp::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    locoComp->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());
    locoComp->UpdateMotionWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}