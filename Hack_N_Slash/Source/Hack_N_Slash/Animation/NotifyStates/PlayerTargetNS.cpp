#include "PlayerTargetNS.h"
#include "GameFramework/Character.h"

#include "../../Interfaces/CombatInstigator.h"
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

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(ownerChar);
    if (!iCombatInst) return;

    ULocomotionComponent* locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    UPlayerCombatComponent* playerCombatComp = ownerChar->FindComponentByClass<UPlayerCombatComponent>();
    if (!playerCombatComp) return;

    UPlayerTargettingComponent* playerTargettingComp = ownerChar->FindComponentByClass<UPlayerTargettingComponent>();
    if (!playerTargettingComp) return;

    FVector2D move = bFreeFlow ? playerCombatComp->move : FVector2D::ZeroVector;

    float targettingRadius = softRadius;
    if (bFreeFlow && !move.IsNearlyZero()) targettingRadius = freeFlowRadius;

    playerTargettingComp->SoftTarget(move, targettingRadius, softHeightCeiling, bFreeFlow);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target)
    {
        if (bSnapToInputDirectionIfNoTarget && !move.IsNearlyZero())
        {
            // Rotate in direction of input if holding a direction
            const FRotator ControlRot = ownerChar->GetControlRotation();
            const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

            const FVector forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
            const FVector right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

            FVector MoveDir = forward * move.Y + right * move.X;
            MoveDir.Z = 0.f;
            MoveDir.Normalize();

            ownerChar->SetActorRotation(MoveDir.Rotation());
        }
        return;
    }

    FVector warpLoc = locoComp->warpLocation;
    FRotator warpRot = locoComp->warpRotation;
    if (bFreeFlow) locoComp->CalcWarpLocRotFreeFlow(target, warpLoc, warpRot, warpLocOffset, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, move, playerTargettingComp->GetLockedOn());
    else locoComp->CalcWarpLocRot(target, warpLoc, warpRot, warpLocOffset, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, playerTargettingComp->GetLockedOn());
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}