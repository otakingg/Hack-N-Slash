#include "ModifyPoise.h"
#include "../../Combat/Shared/CombatResolutionComponent.h"

UModifyPoise::UModifyPoise()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::Green;
    #endif
}


void UModifyPoise::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* cmbtResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (!cmbtResComp) return;
    
    switch (modStyle)
    {
    case EPoiseModStyle::Add:
        cmbtResComp->SetPoiseCalc(cmbtResComp->GetPoiseCalc() + additionalPoise);
        break;

    case EPoiseModStyle::Set:
        cmbtResComp->SetPoiseCalc(newPoise);
    
    default:
        break;
    }
}

void UModifyPoise::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    UCombatResolutionComponent* cmbtResComp = owner->FindComponentByClass<UCombatResolutionComponent>();
    if (!cmbtResComp) return;
    
    if (modStyle != EPoiseModStyle::None) cmbtResComp->ResetPoiseCalc();
}