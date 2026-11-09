#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FAtkHitData.h"
#include "CombatTraceComponent.generated.h"

// Handles hit trace logic for combat
// The trace is along the "Damageable" channel (ECC_GameTraceChannel1) and ignores the owner actor

class IDamageable;
struct FHitBoxShape;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UCombatTraceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) AActor* owner;
	UPROPERTY(Transient) TArray<AActor*> actorsToIgnore; // Stores actors that have already been hit by the trace so they aren't hit again
	IDamageable* iDmgble = nullptr;
	FAtkHitData activeHitData; // Stores Hit Data built by the trace source (EX: Notify)

	void TraceActorDirection(float Radius, float Length, FVector LocalOffset, EAxis::Type Axis, bool bAddToIgnore);
	void TraceBoneToBone(USkeletalMeshComponent* SkeletalMesh, FName SocketStart, FName SocketEnd, float Radius, float ExtraLength, bool bAddToIgnore);
	void TraceSingleBone(USkeletalMeshComponent* SkeletalMesh, FName Socket, float Radius, float Length, FVector LocalOffset, EAxis::Type Axis, bool bAddToIgnore);
	void HandleHit(const TArray<FHitResult>& Hits, FAtkHitData HitData, bool bAddToIgnore); // Handles hit trace logic

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	bool bDebug = false;

	virtual void BeginPlay() override;

public:
	UCombatTraceComponent();

	void SetHitData(const FAtkHitData& HitData) { activeHitData = HitData; }
	void Trace(USkeletalMeshComponent* SkeletalMesh, TArray<FHitBoxShape> HitBoxes, bool bAddToIgnore);
	void ClearHitActors(); // Clears acotrs to ignore and the active hit data
};