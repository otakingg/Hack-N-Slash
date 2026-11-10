#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PauseMont.generated.h"

/**
 * Pauses the current montage
 */
UCLASS()
class HACK_N_SLASH_API UPauseMont : public UAnimNotify
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Pause")
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category = "Pause", meta = (ToolTip = "0 means pause indefinitely"))
    float duration = 0.0f;

public:
    UPauseMont();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
