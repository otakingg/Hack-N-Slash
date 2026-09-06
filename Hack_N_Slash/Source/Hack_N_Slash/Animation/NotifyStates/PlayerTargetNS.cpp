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
            break;
    }

    playerTargettingComp->SoftTarget(targetingStyle, playerCombatComp->move, targettingRadius, softHeightCeiling);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target)
    {
        // Rotate in direction of input if holding a direction
        if (bSnapToInputDirectionIfNoTarget && !playerCombatComp->move.IsNearlyZero())
        {
            const FRotator controlRot = ownerChar->GetControlRotation();
            const FRotator yawRot(0.f, controlRot.Yaw, 0.f);

            const FVector forward = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);
            const FVector right   = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

            FVector MoveDir = forward * playerCombatComp->move.Y + right * playerCombatComp->move.X;
            MoveDir.Z = 0.f;
            MoveDir.Normalize();

            ownerChar->SetActorRotation(MoveDir.Rotation());
        }
        return;
    }

    FVector warpLoc;
    FRotator warpRot;
    if (playerTargettingComp->GetLockedOn()) locoComp->CalcWarpLocRot(target, warpLoc, warpRot, offset, maxWarpTranslDistLockOn, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    else locoComp->CalcWarpLocRot(target, warpLoc, warpRot, offset, 0.0f, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, bIgnoreTranslation);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}