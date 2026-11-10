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

    if (duration > 0.0f)
    {
        if (UWorld* World = MeshComp->GetWorld())
        {
            TWeakObjectPtr<UAnimInstance> weakAnimInst = animInst;
            TWeakObjectPtr<UAnimMontage> weakMontage = montage;

            FTimerHandle TH_Resume;
            World->GetTimerManager().SetTimer(
                TH_Resume,
                [weakAnimInst, weakMontage]()
                {
                    if (weakAnimInst.IsValid() && weakMontage.IsValid()) weakAnimInst->Montage_Resume(weakMontage.Get());
                },
                duration,
                false
            );
        }
    }

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