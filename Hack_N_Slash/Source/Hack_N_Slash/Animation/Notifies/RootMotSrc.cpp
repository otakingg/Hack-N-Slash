#include "RootMotSrc.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

void URootMotSrc::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
            HandleConstant(owner, iLocoCmd);
            break;
        
        case ERootMotionType::Jump:
            HandleJump(owner, iLocoCmd);
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

void URootMotSrc::HandleConstant(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
{
    if (!Owner || !iLocoCmd) return;

    FVector calcDirection = direction.IsNearlyZero() ? Owner->GetActorForwardVector() : direction.GetSafeNormal();
    FVector force = calcDirection * (distance / duration);
    
    iLocoCmd->ApplyRootMotionSourceConstant(duration, force, velocityOnFinish, clampVelocityOnFinish, velocityOnFinishMode, strengthOverTime, bAdditive);
}

void URootMotSrc::HandleJump(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
{
    if (!Owner || !iLocoCmd) return;

    FVector calcDirection = direction.IsNearlyZero() ? Owner->GetActorForwardVector() : direction.GetSafeNormal();

    UAsyncRootMovement* asyncRM = iLocoCmd->ApplyRootMotionSourceJump(calcDirection, distance, height, duration, velocityOnFinishMode, velocityOnFinish, clampVelocityOnFinish);
    if (asyncRM && asyncRM->IsActive())
    {
        calcDirection.Z = 0;
        Owner->SetActorRotation(calcDirection.Rotation());
    }
}

void URootMotSrc::HandleMoveTo(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
{
    if (!Owner || !iLocoCmd) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(Owner);
    if (!iCombatInst) return;

    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FVector warpLoc;
    FRotator warpRot;
    iLocoCmd->GetWarpingLocRot(target, warpLoc, warpRot, offset, iCombatInst->GetLockedOn());

    const FVector startLoc = Owner->GetActorLocation();

    float moveToDuration = 0.0f;
    if (duration > 0.0f) moveToDuration = duration;
    else
    {
        const float calcDistance = FVector::Dist(startLoc, warpLoc);
        moveToDuration = FMath::Clamp(calcDistance / 2500.f, 0.1f, 0.5f);
    }

    UAsyncRootMovement* asyncRM = iLocoCmd->ApplyRootMotionSourceMoveTo(startLoc, warpLoc, moveToDuration, bRestrictSpeedToExpected);
    if (asyncRM && asyncRM->IsActive()) Owner->SetActorRotation(warpRot);

    if (bDebug) DrawDebugSphere(Owner->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}

void URootMotSrc::HandleRadial(AActor* Owner, ILocomotionCmdInterface* iLocoCmd)
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