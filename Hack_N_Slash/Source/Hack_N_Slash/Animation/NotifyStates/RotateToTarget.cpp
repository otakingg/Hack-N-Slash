#include "RotateToTarget.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Interfaces/CombatInstigator.h"

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

    // Get Target
    AActor* target = iCombatInst->GetCurrentTarget();
    if (!target) return;

    FRotator currentRot = owner->GetActorRotation();
    FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(owner->GetActorLocation(), target->GetActorLocation());
    if (!bRotPitch) desiredRot.Pitch = 0.0f;
    if (!bRotRoll) desiredRot.Roll = 0.0f;

    FRotator NewRotation = FMath::RInterpTo(currentRot, desiredRot, FrameDeltaTime, roationSpeed);
    owner->SetActorRotation(NewRotation);
}