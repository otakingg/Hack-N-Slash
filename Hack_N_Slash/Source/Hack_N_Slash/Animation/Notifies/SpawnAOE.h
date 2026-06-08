#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SpawnAOE.generated.h"

class AAOE_Base;

/**
 * 
 */
UCLASS()
class HACK_N_SLASH_API USpawnAOE : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Notify")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Notify")
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, Category = "Notify", meta = (ToolTip = "If true, spawn at self instead of trying to spawn on your potential target"))
	bool bSpawnAtSelf = true;

	UPROPERTY(EditAnywhere, Category = "Notify")
	float radius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Notify")
	TSubclassOf<AAOE_Base> aoeClass;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
