#include "PlayerTargetNS.h"
#include "GameFramework/Character.h"

#include "../../Characters/Shared/LocomotionComponent.h"
#include "../../Combat/Player/PlayerCombatComponent.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"

UPlayerTargetNS::UPlayerTargetNS()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Purple;
    #endif
}

void UPlayerTargetNS::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation, float FrameDeltaTime, const FAnimNotifyEventReference &EventReference)
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
            // Rotate in direction of input
            if (bSnapToInputDirectionIfNoTarget) SnapToMoveInputDirection(ownerChar, playerCombatComp->move);
            return;
    }

    playerTargettingComp->SoftTarget(targetingStyle, playerCombatComp->move, targettingRadius, softHeightCeiling);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target)
    {
        // Rotate in direction of input
        if (bSnapToInputDirectionIfNoTarget) SnapToMoveInputDirection(ownerChar, playerCombatComp->move);
        return;
    }

    if (velocityThreshold > 0)
    {
        UMovementComponent* targetMoveComp = target->FindComponentByClass<UMovementComponent>();
        if (targetMoveComp && targetMoveComp->Velocity.Size() > velocityThreshold) return;
    }

    FVector warpLoc;
    FRotator warpRot;
    if (playerTargettingComp->GetLockedOn())
    {
        float translDistLimit = bLimitTranslDistLockOn ? softRadius : 0;
        locoComp->CalcWarpLocRot(target->GetActorLocation(), warpLoc, warpRot, offsetDistance, offsetVertical, translDistLimit, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    }
    else locoComp->CalcWarpLocRot(target->GetActorLocation(), warpLoc, warpRot, offsetDistance, offsetVertical, 0.0f, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug)
    {
        if (ownerChar->GetActorLocation().Equals(warpLoc, 10.0f)) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Red, false, 2.0f);
        else                                                      DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.0f);
    }
}

void UPlayerTargetNS::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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
            return;
    }

    playerTargettingComp->SoftTarget(targetingStyle, playerCombatComp->move, targettingRadius, softHeightCeiling);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target) return;

    if (velocityThreshold > 0)
    {
        UMovementComponent* targetMoveComp = target->FindComponentByClass<UMovementComponent>();
        if (targetMoveComp && targetMoveComp->Velocity.Size() > velocityThreshold) return;
    }

    FVector warpLoc;
    FRotator warpRot;
    if (playerTargettingComp->GetLockedOn())
    {
        float translDistLimit = bLimitTranslDistLockOn ? softRadius : 0;
        locoComp->CalcWarpLocRot(target->GetActorLocation(), warpLoc, warpRot, offsetDistance, offsetVertical, translDistLimit, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    }
    else locoComp->CalcWarpLocRot(target->GetActorLocation(), warpLoc, warpRot, offsetDistance, offsetVertical, 0.0f, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug)
    {
        if (ownerChar->GetActorLocation().Equals(warpLoc, 10.0f)) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Red, false, 2.0f);
        else                                                      DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.0f);
    }
}

void UPlayerTargetNS::SnapToMoveInputDirection(ACharacter* Owner, FVector2D Input)
{
    if (!Owner || Input.IsNearlyZero()) return;

    // Rotate in direction of input
    const FRotator controlRot = Owner->GetControlRotation();
    const FRotator yawRot(0.f, controlRot.Yaw, 0.f);

    const FVector forward = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);
    const FVector right   = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

    FVector worldDir = forward * Input.Y + right * Input.X;
    worldDir.Z = 0.f;
    worldDir.Normalize();

    Owner->SetActorRotation(worldDir.Rotation());
}