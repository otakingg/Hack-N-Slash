#include "PlayerTarget.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Interfaces/CombatInstigator.h"
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

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(ownerChar);
    if (!iCombatInst) return;

    ULocomotionComponent* locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    UPlayerCombatComponent* playerCombatComp = ownerChar->FindComponentByClass<UPlayerCombatComponent>();
    if (!playerCombatComp) return;

    UPlayerTargettingComponent* playerTargettingComp = ownerChar->FindComponentByClass<UPlayerTargettingComponent>();
    if (!playerTargettingComp) return;

    FVector2D move = bAlignOverDist ? playerCombatComp->move : FVector2D::ZeroVector;

    float targettingRadius = softRadius;
    if (bAlignOverDist && !move.IsNearlyZero()) targettingRadius = freeFlowRadius;

    playerTargettingComp->SoftTarget(move, targettingRadius, softHeightCeiling, bAlignOverDist);

    AActor* target = playerTargettingComp->GetCurrentTarget();
    if (!target)
    {
        if (bSnapToInputDirectionIfNoTarget && !move.IsNearlyZero())
        {
            // Rotate in direction of input if holding a direction
            const FRotator controlRot = ownerChar->GetControlRotation();
            const FRotator yawRot(0.f, controlRot.Yaw, 0.f);

            const FVector forward = UKismetMathLibrary::GetForwardVector(yawRot);
            const FVector right   = UKismetMathLibrary::GetRightVector(yawRot);

            FVector MoveDir = forward * move.Y + right * move.X;
            MoveDir.Z = 0.f;
            MoveDir.Normalize();

            ownerChar->SetActorRotation(MoveDir.Rotation());
        }
        return;
    }

    FVector warpLoc = locoComp->warpLocation;
    FRotator warpRot = locoComp->warpRotation;
    if (bAlignOverDist) locoComp->CalcWarpLocRotFreeFlow(target, warpLoc, warpRot, warpLocOffset, bIgnorePitch, bIgnoreRoll, bIgnoreYaw, move, playerTargettingComp->GetLockedOn());
    else locoComp->CalcWarpLocRot(target, warpLoc, warpRot, warpLocOffset, bIgnorePitch, bIgnoreRoll, bIgnoreYaw);
    locoComp->UpdateWarpData(warpLoc, warpRot);

    if (bDebug) DrawDebugSphere(ownerChar->GetWorld(), warpLoc, 25.0f, 12, FColor::Green, false, 2.f);
}