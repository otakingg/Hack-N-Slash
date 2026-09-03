#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "CombatTraceComponent.generated.h"

// Handles hit trace logic for combat
// The trace is along the "Damageable" channel (ECC_GameTraceChannel1) and ignores the owner actor

struct FSocketTrace;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UCombatTraceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) AActor* owner;
	UPROPERTY(Transient) TArray<AActor*> actorsToIgnore; // Stores actors that have already been hit by the trace so they aren't hit again
	FAtkHitData activeHitData; // Stores Hit Data built by the trace source (EX: Notify)

	void HandleHit(TArray<FHitResult>& Hits, FAtkHitData HitData); // Handles hit trace logic

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	bool bDebug = false;

	virtual void BeginPlay() override;

public:
	UCombatTraceComponent();

	void SetHitData(const FAtkHitData& HitData) { activeHitData = HitData; }

	void ForwardTrace(float Radius, float Distance, FVector Offset); // Performs a forward trace from the owning actor's location
	void SocketTrace(USkeletalMeshComponent* SkeletalMesh, TArray<FSocketTrace> Sockets, float Radius); // Performs a trace along a set of sockets

	void ClearHitActors(); // Clears acotrs to ignore and the active hit data
};