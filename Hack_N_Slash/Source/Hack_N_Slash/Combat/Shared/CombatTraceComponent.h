#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "CombatTraceComponent.generated.h"

class UCombatResolutionComponent;
class UStatsComponent;
struct FSocketTrace;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UCombatTraceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) AActor* owner;
	UPROPERTY(Transient) TArray<AActor*> actorsToIgnore;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp;
	UPROPERTY(Transient) UStatsComponent* statsComp;
	FAtkHitData activeHitData;

	void HandleHit(TArray<FHitResult>& Hits, FAtkHitData HitData);

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	bool bDebug = false;

	virtual void BeginPlay() override;

public:
	UCombatTraceComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BuildHitData(FAtkHitData HitData);

	void DistanceTrace(float Radius, float Distance, FVector Offset);
	void SocketTrace(USkeletalMeshComponent* SkeletalMesh, TArray<FSocketTrace> Sockets, float Radius);

	void ClearHitActors();
};
