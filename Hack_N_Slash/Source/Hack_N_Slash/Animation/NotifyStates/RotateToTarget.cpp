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

    ULocomotionComponent* locoComp = owner->FindComponentByClass<ULocomotionComponent>();
    if (!locoComp) return;

    ICombatInstigator* iCombatInst = Cast<ICombatInstigator>(owner);
    if (!iCombatInst) return;

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FRotator currentRot = owner->GetActorRotation();
    FRotator desiredRot = locoComp->warpRotation;

    float calcSpeed = 50.0f;
    calcSpeed = speed > 0 ? speed : FVector::Dist(owner->GetActorLocation(), locoComp->warpLocation) / duration;

    FRotator newRotation = FMath::RInterpTo(currentRot, desiredRot, FrameDeltaTime, speed);
    owner->SetActorRotation(newRotation);
}