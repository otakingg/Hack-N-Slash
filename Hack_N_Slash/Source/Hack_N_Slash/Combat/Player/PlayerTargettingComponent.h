#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enums/ETargetingStyle.h"
#include "PlayerTargettingComponent.generated.h"

// Handles Player Targetting Logic
// Hard Target = Lock On (Standard single-target lock on)
// Soft Target = Target without camera locking onto them
// Free flow Target = Soft Target, just at a greater range

class UCameraComponent;
class UCharacterMovementComponent;
class ULocomotionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerTargettingComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UCameraComponent* camComp = nullptr;
	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

	bool EnsureReferences();
	AActor* FindBestTargetToLeft(const TArray<AActor*>& Targets); // Finding a lock-on target to the left of the current one
	AActor* FindBestTargetToRight(const TArray<AActor*>& Targets); // Finding a lock-on target to the right of the current one

protected:
	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Targeting")
	AActor* currentTarget = nullptr; // Holds current Soft or Hard Target 

	UPROPERTY(VisibleAnywhere, Category = "Targeting|Hard Target")
	bool bLockedOn = false;

	// Used to get a reasonable soft target via directional alignment
	// The direction could be anything (Move stick direction, camera direction, etc.)
	UPROPERTY(EditAnywhere, Category = "Targeting|Soft Target", meta = (ClampMin = "-1.0", ClampMax = "1.0"), meta = (ToolTip = "Soft-lock targets need a dot product >= this number. Will only soft-lock a target within reasonable direction of your left-stick movement or camera facing direction"))
	float targetAlignmentTolerance = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Hard Target", meta = (ClampMin = "0.0"))
	float hardTargetRadius = 2000.0f; // Lock-on radius

	virtual void BeginPlay() override;

public:	
	UPlayerTargettingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* GetCurrentTarget() const { return currentTarget; }
	bool GetLockedOn() const { return bLockedOn; }

	void SoftTarget(ETargetingStyle TargetingStyle, const FVector2D& Move, float TargettingRadius = 1000.0f, float TargetHeightCeiling = 150.0f);
	void ToggleLockOn();
	void LockOff();
	bool LockOnBasedOnYaw(float Yaw);
	TArray<AActor*> GetEnemiesInRadius(float Radius); // Get all enemies within given radius
	AActor* FindBestTarget(const TArray<AActor*>& Targets); // Lock-on to best target
	double GetCameraToTargetAlignment(FVector StartLoc, FVector EndLoc) const; // How much is the camera pointing toward the target?
	double GetDirToTargetAlignment2D(AActor* Target, FVector2D Dir) const; // How much is the direction pointing toward the target?

	void ClearCurrentTarget();
};