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
	UPROPERTY(EditAnywhere, Category = "AOE")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "AOE")
	bool bIgnoreSelf = true;
	
	UPROPERTY(EditAnywhere, Category = "AOE")
	float radius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "AOE")
	TSubclassOf<AAOE_Base> aoeClass;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
