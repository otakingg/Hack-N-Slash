#include "RotateToTarget.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Characters/Shared/LocomotionComponent.h"

URotateToTarget::URotateToTarget()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor(0, 255, 255);
    #endif
}

void URotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FRotator currentRot = owner->GetActorRotation();
    //FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(owner->GetActorLocation(), target->GetActorLocation());
    //desiredRot.Pitch = 0.0f;
    //desiredRot.Roll = 0.0f;

    FVector warpLoc;
    FRotator warpRot;
    locoComp->GetWarpingLocRot(target, warpLoc, warpRot, 0, iCombatInst->GetLockedOn());
    locoComp->UpdateMotionWarpData(warpLoc, warpRot);

    FRotator NewRotation = FMath::RInterpTo(currentRot, warpRot, FrameDeltaTime, roationSpeed);
    owner->SetActorRotation(NewRotation);
}