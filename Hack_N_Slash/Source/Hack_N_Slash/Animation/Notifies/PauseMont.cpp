#include "PauseMont.h"

UPauseMont::UPauseMont()
{
    #if WITH_EDITORONLY_DATA
        NotifyColor = FColor::White;
    #endif
}

void UPauseMont::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UAnimInstance* animInst = MeshComp->GetAnimInstance();
    if (!animInst) return;

    UAnimMontage* montage = animInst->GetCurrentActiveMontage();
    if (!montage) return;

    animInst->Montage_Pause(montage);

    const bool bIsActive = animInst->Montage_IsActive(montage);
    const bool bIsPlaying = animInst->Montage_IsPlaying(montage);

    if (bDebug && GEngine)
    {
        const FString Message = FString::Printf(
            TEXT("After pause: Active=%s, Playing=%s"),
            bIsActive ? TEXT("true") : TEXT("false"),
            bIsPlaying ? TEXT("true") : TEXT("false")
        );

        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, Message);
    }
}