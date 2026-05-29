// Helpful Video: https://youtu.be/ConO4G0j9wI?si=G_yjCu7rzYwv-pa6
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOE_Base.generated.h"

struct FAtkHitData;

UCLASS()
class HACK_N_SLASH_API AAOE_Base : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AOE", meta = (ExposeOnSpawn = true))
	bool bDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AOE", meta = (ExposeOnSpawn = true))
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AOE", meta = (ExposeOnSpawn = true))
	AActor* target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AOE", meta = (ExposeOnSpawn = true))
	float radius = 500.0f;

	virtual void BeginPlay() override;

public:
	AAOE_Base();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AOE")
	void OnAOEOverlap(AActor* Actor);
	virtual void OnAOEOverlap_Implementation(AActor* Actor) {}

	void SetDebug(bool bInDebug) { bDebug = bInDebug; }
	void SetIgnoreSelf(bool bInIgnoreSelf) { bIgnoreSelf = bInIgnoreSelf; }
	void SetRadius(float InRadius) { radius = InRadius; }
	void SetTarget(AActor* InTarget) {  if (InTarget) target = InTarget; }

	UFUNCTION(BlueprintCallable, Category = "AOE")
	void HandleHit(UPARAM(ref) FAtkHitData& HitData, AActor* OverlappedActor);
};