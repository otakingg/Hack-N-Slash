#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTargettingComponent.generated.h"

class UCameraComponent;
class UCharacterMovementComponent;
class ULocomotionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerTargettingComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) ACharacter* ownerChar;
	UPROPERTY(Transient) UCameraComponent* camComp = nullptr;
	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

	bool EnsureReferences();
	AActor* FindBestTargetToLeft(const TArray<AActor*>& Targets);
	AActor* FindBestTargetToRight(const TArray<AActor*>& Targets);

protected:
	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Targeting")
	AActor* currentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Targeting|Hard Target")
	bool bLockedOn = false;

	UPROPERTY(EditAnywhere, Category = "Targeting|Soft Target", meta = (ClampMin = "-1.0", ClampMax = "1.0"), meta = (ToolTip = "Soft-lock targets need a dot product >= this number. Will only soft-lock a target within reasonable direction of your left-stick movement or camera facing direction"))
	float softTargetAlignmentTolerance = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Soft Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum height difference a target can be to be soft-targettable"))
	double softTargetHeight = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Soft Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be soft-targettable, Free Flow attacks use the free flow radius"))
	float softTargetRadius = 750.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Soft Target", meta = (ClampMin = "0.0"), meta = (ToolTip = "Maximum radius a target can be to be free-flowable. Make sure this is > soft lock radius"))
	float ffTargetRadius = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Hard Target", meta = (ClampMin = "0.0"))
	float hardTargetRadius = 2000.0f;

	virtual void BeginPlay() override;

public:	
	UPlayerTargettingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* GetCurrentTarget() const { return currentTarget; }
	bool GetLockedOn() const { return bLockedOn; }

	void SoftTarget(const FVector2D& InputDir);
	void ToggleLockOn();
	void LockOff();
	bool LockOnBasedOnYaw(float Yaw);
	TArray<AActor*> GetEnemiesInRadius(float Radius); // Get all enemies within lock on raidus
	AActor* FindBestTarget(const TArray<AActor*>& Targets); // Pick best target
	double GetCameraToTargetAlignment(FVector StartLoc, FVector EndLoc) const; // How much is the camera pointing toward the target?
	double GetDirToTargetAlignment2D(AActor* Target, FVector2D Dir) const;

	void ClearCurrentTarget();
};