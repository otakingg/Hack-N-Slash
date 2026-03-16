// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTraceComponent.generated.h"

struct FAtkHitData;
struct FSocketTrace;
class UStatsComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UCombatTraceComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AActor* owner;
	TArray<AActor*> actorsToIgnore;
	UStatsComponent* statsComp;

	void HandleHit(TArray<FHitResult>& Hits, FAtkHitData& HitData);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};

	virtual void BeginPlay() override;

public:
	UCombatTraceComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SocketTrace(USkeletalMeshComponent* SkeletalMesh, TArray<FSocketTrace> Sockets, float Radius, FAtkHitData& HitData);
	void Trace(float Radius, FVector Distance, FVector Offset, FAtkHitData& HitData);

	void ResetAttack();
};
