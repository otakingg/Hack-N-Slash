// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTargettingComponent.generated.h"

class UCameraComponent;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerTargettingComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AActor* owner;
	AActor* currentTarget = nullptr;
	UCameraComponent* camComp = nullptr;
	UCharacterMovementComponent* moveComp = nullptr;

	bool EnsureReferences();
	void LockOff();
	void LockOn();

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Target|Lock On")
	bool bLockedOn = false;

	UPROPERTY(EditAnywhere, Category = "Target|Free Flow", meta = (ClampMin = "-1.0", ClampMax = "1.0"), meta = (ToolTip = "Restricts free flow targets to needing a dot product >= this number. Will only ever free flow to a target within reasonable direction of your left-stick movement"))
	float ffToleranceDot = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Target|Free Flow", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum height difference a target can be to be free-flowable"))
	double ffToleranceHeight = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Target|Free Flow", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be free-flowable"))
	float ffToleranceRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Target|Lock On", meta = (ClampMin = "0.0"))
	float lockOnRadius = 1000.0f;

	virtual void BeginPlay() override;

public:	
	UPlayerTargettingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* GetCurrentTarget() const { return currentTarget; }
	bool GetLockedOn() const { return bLockedOn; }

	void ToggleLockOn();
	TArray<AActor*> GetEnemiesInRadius(float Radius); // Get all enemies within lock on raidus
	AActor* FindClosestTarget(const TArray<AActor*>& Targets); // Pick best target
	double GetCameraToTargetAlignment(FVector StartLoc, FVector EndLoc) const; // How much is the camera pointing toward the target?
};