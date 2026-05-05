#include "RootMotSrcN.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void URootMotSrcN::Notify(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, const FAnimNotifyEventReference &EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ILocomotionCmdInterface* iLocoCmd = nullptr;
    TArray<UActorComponent*> locoComps = owner->GetComponentsByInterface(ULocomotionCmdInterface::StaticClass());
    if (locoComps.Num() > 0) iLocoCmd = Cast<ILocomotionCmdInterface>(locoComps[0]);
    if (!iLocoCmd) return;

    switch (SourceType)
    {
        case ERootMotionType::Constant:
            HandleConstant(iLocoCmd);
            break;

        case ERootMotionType::MoveTo:
            HandleMoveTo(owner, iLocoCmd);
            break;

        case ERootMotionType::Radial:
            HandleRadial(owner, iLocoCmd);
            break;

        default:
            break;
    }
}

void URootMotSrcN::HandleConstant(ILocomotionCmdInterface* iLocoCmd)
{
    if (!iLocoCmd) return;
    iLocoCmd->ApplyRootMotionSourceConstant(duration, force, velocityOnFinish, clampVelocityOnFinish, velocityOnFinishMode, strengthOverTime, bAdditive);
}

void URootMotSrcN::HandleMoveTo(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
{
    if (!Owner || !iLocoCmd) return;

    ICombatInstigator* combatInst = Cast<ICombatInstigator>(Owner);
    if (!combatInst) return;

    AActor* target = combatInst->GetCurrentTarget();
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, "RootMotionNotify");

    const FVector startLoc = Owner->GetActorLocation();
    const float distance = FVector::Dist(startLoc, warpLoc);
    const float calcDuration = FMath::Clamp(distance / 2500.f, 0.1f, 0.5f);

    iLocoCmd->ApplyRootMotionSourceMoveTo(startLoc, warpLoc, calcDuration, bRestrictSpeedToExpected);

    if (bDebug) DrawDebugSphere(Owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}

void URootMotSrcN::HandleRadial(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
{
    if (!Owner || !iLocoCmd) return;

    FVector origin = Owner->GetActorLocation();

    const bool bIsPush = strength >= 0.0f;

    iLocoCmd->ApplyRootMotionSourceRadial(origin, radius, FMath::Abs(strength), duration, bIsPush, strengthOverTime);

    if (bDebug)
    {
        DrawDebugSphere(Owner->GetWorld(), origin, radius, 16, FColor::Red, false, 2.0f);
        DrawDebugDirectionalArrow(
            Owner->GetWorld(),
            origin,
            origin + FVector(0,0,100),
            50.0f,
            bIsPush ? FColor::Red : FColor::Purple,
            false,
            2.0f
        );
    }
}