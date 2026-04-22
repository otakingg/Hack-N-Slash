#include "RootMotSrcN.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void URootMotSrcN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ILocomotionCmdInterface* iLocoCmd = nullptr;
    TArray<UActorComponent*> locoComps = owner->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (locoComps.Num() > 0) iLocoCmd = Cast<ILocomotionCmdInterface>(locoComps[0]);
    if (!iLocoCmd) return;

    switch(sourceType)
    {
        case ERootMotionType::MoveTo:
        {
            ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner);
            if (!iCmbtInst) break;

            AActor* target = iCmbtInst->GetCurrentTarget();
            if (!target) break;

            FVector warpLoc;
            FRotator warpRot;
            iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset);

            FVector playerLoc = owner->GetActorLocation();
            FVector targetLoc = target->GetActorLocation();

            float dist = FVector::Dist(playerLoc, warpLoc);
            float duration = FMath::Clamp(dist / 2500.f, 0.1f, 0.5f);

            TSharedPtr<FRootMotionSource_MoveToForce> moveToForce = MakeShared<FRootMotionSource_MoveToForce>();
            moveToForce->InstanceName = FName("MoveToTarget");
            moveToForce->AccumulateMode = ERootMotionAccumulateMode::Override;
            moveToForce->StartLocation = playerLoc;
            moveToForce->TargetLocation = warpLoc;
            moveToForce->Duration = duration;
            moveToForce->Priority = 5;
            moveToForce->bRestrictSpeedToExpected = false;
            if (iLocoCmd->ApplyRootMotionSource(*moveToForce)) owner->SetActorRotation(warpRot);
        }
        default:
            break;
    }
}