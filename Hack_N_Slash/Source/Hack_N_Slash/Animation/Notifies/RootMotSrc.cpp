#include "RootMotSrc.h"
#include "../../Characters/Shared/LocomotionComponent.h"

URootMotSrc::URootMotSrc()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void URootMotSrc::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    switch (SourceType)
    {
        case ERootMotionType::Constant:
            HandleConstant(owner, locoComp);
            break;
        
        case ERootMotionType::Jump:
            HandleJump(owner, locoComp);
            break;

        case ERootMotionType::MoveTo:
            HandleMoveTo(owner, locoComp);
            break;

        case ERootMotionType::Radial:
            HandleRadial(owner, locoComp);
            break;

        default:
            break;
    }
}

void URootMotSrc::HandleConstant(AActor* Owner, ULocomotionComponent* LocoComp)
{
    if (duration <= 0.0f) return;

    FVector calcDirection = localDir.IsNearlyZero() ? FVector::ZeroVector : Owner->GetActorRotation().RotateVector(localDir.GetSafeNormal());

    FVector force = calcDirection * (distance / duration);
    
    LocoComp->ApplyRootMotionSourceConstant(duration, force, velocityOnFinishMode, velocityOnFinish, clampVelocityOnFinish, strengthOverTime, bAdditive);
}

void URootMotSrc::HandleJump(AActor* Owner, ULocomotionComponent* LocoComp)
{
    if (duration <= 0.0f) return;

    FVector calcDirection = localDir.IsNearlyZero() ? FVector::ZeroVector : Owner->GetActorRotation().RotateVector(localDir.GetSafeNormal());

    LocoComp->ApplyRootMotionSourceJump(calcDirection, distance, height, duration, velocityOnFinishMode, velocityOnFinish, clampVelocityOnFinish);
}

void URootMotSrc::HandleMoveTo(AActor* Owner, ULocomotionComponent* LocoComp)
{
    if (duration <= 0.0f) return;
    
    const FVector startLoc = Owner->GetActorLocation();
    LocoComp->ApplyRootMotionSourceMoveTo(startLoc, LocoComp->warpLocation, duration, bRestrictSpeedToExpected, velocityOnFinishMode, velocityOnFinish, clampVelocityOnFinish);

    if (bDebug) DrawDebugSphere(Owner->GetWorld(), LocoComp->warpLocation, 25.0f, 12, FColor::Green, false, 2.0f);
}

void URootMotSrc::HandleRadial(AActor* Owner, ULocomotionComponent* LocoComp)
{
    if (duration <= 0.0f) return;

    FVector origin = Owner->GetActorLocation();

    const bool bIsPush = strength >= 0.0f;

    LocoComp->ApplyRootMotionSourceRadial(origin, radius, FMath::Abs(strength), duration, bIsPush, strengthOverTime);

    if (bDebug)
    {
        DrawDebugSphere(Owner->GetWorld(), origin, radius, 16, FColor::Red, false, 2.0f);
        DrawDebugDirectionalArrow(
            Owner->GetWorld(),
            origin,
            origin + FVector(0,0,100),
            50.0f,
            bIsPush ? FColor::Blue : FColor::Purple,
            false,
            2.0f
        );
    }
}