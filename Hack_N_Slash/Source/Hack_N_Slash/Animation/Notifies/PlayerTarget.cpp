#include "PlayerTarget.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"

UPlayerTarget::UPlayerTarget()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Purple;
    #endif
}

void UPlayerTarget::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    ACharacter* ownerChar = MeshComp->GetOwner<ACharacter>();
    if (!ownerChar) return;

    ULocomotionComponent* locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    UPlayerCombatComponent* playerCombatComp = ownerChar->FindComponentByClass<UPlayerCombatComponent>();
    if (!playerCombatComp) return;

    UPlayerTargettingComponent* playerTargettingComp = ownerChar->FindComponentByClass<UPlayerTargettingComponent>();
    if (!playerTargettingComp) return;

    float targettingRadius = 0.0f;
    switch (targetingStyle)
    {
        case ETargetingStyle::AlignCam:
            targettingRadius = softRadius;
            break;

        case ETargetingStyle::AlignMove:
            targettingRadius = freeFlowRadius;
            break;

        case ETargetingStyle::AlignMoveOrCam:
        case ETargetingStyle::AlignMoveOrDist:
            targettingRadius = playerCombatComp->move.IsNearlyZero() ? softRadius : freeFlowRadius;
            break;

        case ETargetingStyle::Dist:
            targettingRadius = softRadius;
            break;
        
        default:
            if (bSnapToInputDirectionIfNoTarget) SnapToMoveInputDirection(ownerChar, playerCombatComp->move);
            return;
    }

    playerTargettingComp->SoftTarget(targetingStyle, playerCombatComp->move, targettingRadius, softHeightCeiling);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target)
    {
        locoComp->ClearWarpData();
        if (bSnapToInputDirectionIfNoTarget) SnapToMoveInputDirection(ownerChar, playerCombatComp->move);
        return;
    }

    FVector targetLoc = target->GetActorLocation();
    if (playerTargettingComp->GetLockedOn() && bLimitTranslDistLockOn && FVector::Dist(ownerChar->GetActorLocation(), targetLoc) > softRadius)
    {
        locoComp->ClearWarpData();
        return;
    }

    FVector predLoc = targetLoc;
    if (predictionTime > 0)
    {
        if (UMovementComponent* targetMoveComp = target->FindComponentByClass<UMovementComponent>())
        {
            predLoc += (targetMoveComp->Velocity * predictionTime);
        }
    }

    FVector warpLoc;
    FRotator warpRot;
    locoComp->CalcWarpLocRot(predLoc, warpLoc, warpRot, offsetDistance, offsetVertical, 0.0f, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug)
    {
        if (ownerChar->GetActorLocation().Equals(warpLoc, 10.0f)) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Red, false, 2.0f);
        else                                                      DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.0f);
    }
}

void UPlayerTarget::SnapToMoveInputDirection(ACharacter* Owner, FVector2D Input)
{
    if (!Owner || Input.IsNearlyZero()) return;

    // Rotate in direction of input
    const FRotator controlRot = Owner->GetControlRotation();
    const FRotator yawRot(0.f, controlRot.Yaw, 0.f);

    const FVector forward = UKismetMathLibrary::GetForwardVector(yawRot);
    const FVector right   = UKismetMathLibrary::GetRightVector(yawRot);

    FVector worldDir = forward * Input.Y + right * Input.X;
    worldDir.Z = 0.f;
    worldDir.Normalize();

    Owner->SetActorRotation(worldDir.Rotation());
}