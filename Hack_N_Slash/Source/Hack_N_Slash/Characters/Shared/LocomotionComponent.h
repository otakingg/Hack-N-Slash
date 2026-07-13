#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AsyncRootMovement.h"
#include "GameFramework/RootMotionSource.h"
#include "LocomotionComponent.generated.h"

class AEnemyController;
class ICombatInstigator;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class UMotionWarpingComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API ULocomotionComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) AEnemyController* enemyController = nullptr;
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UMotionWarpingComponent* motionWarpComp = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
    UPROPERTY(Transient) UAsyncRootMovement* asyncRootMotionOverride = nullptr;
    UPROPERTY(Transient) TArray<UAsyncRootMovement*> asyncRootMotionsAdditive;
    ICombatInstigator* iCmbtInst = nullptr;

    FTimerHandle TH_ClearAirborne;
    FTimerHandle TH_StopMovement;

    bool EnsureReferences();

protected:
    UPROPERTY(EditAnywhere, Category="Locomotion")
    bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float coyoteSeconds = 0.10f;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Jump")
    float lastGroundedTime = -1000.0f; // Safe default far in past


    /*UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump")
    UAnimMontage* doubleJumpMontage = nullptr;*/


    UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0"))
    float gravity = 1.5f;

    

    UPROPERTY(EditAnywhere, Category = "Locomotion|Ground", meta = (ClampMin = "0.0"))
    float groundFriction = 10.0f;
 
    UPROPERTY(EditAnywhere, Category = "Locomotion|Ground", meta = (ClampMin = "0.0"))
    float groundBrakingDecelleration = 5200.0f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Ground")
    FRotator groundRotationRate = FRotator(0.f, 720.0f, 0.0f);



    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling", meta = (ClampMin = "0.0"))
    float fallingAirControl = 0.28;
 
    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling", meta = (ClampMin = "0.0"))
    float fallingAirControlBoostMult = 1.15f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling", meta = (ClampMin = "0.0"))
    float fallingAirControlBoostVeloctiyThreshold = 1.15f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling", meta = (ClampMin = "0.0"))
    float fallingBrakingDecelleration = 320.0f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling", meta = (ClampMin = "0.0"))
    float fallingLateralFriction = 0.22f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Falling")
    FRotator fallingRotationRate = FRotator(0.f, 720.0f, 0.0f);



    UPROPERTY(EditAnywhere, Category = "Locomotion|Flying", meta = (ClampMin = "0.0"))
    float flyingBrakingDecelleration = 320.0f;

    UPROPERTY(EditAnywhere, Category = "Locomotion|Flying")
    FRotator flyingRotationRate = FRotator(0.f, 720.0f, 0.0f);

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    ULocomotionComponent();

    /* ---------------- Coyote Time ----------------*/
    void UpdateLastGroundedTime();
    bool CanCoyoteJump();

    /* ---------------- Tag-driven Tuning ---------------- */
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void RefreshMovementStats();
    
    /* ---------------- Movement Actions ------------------------------*/
    void Move(const FVector2D& Move);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void MoveTo(AActor* Target, const FVector Loc = FVector::ZeroVector, const float AcceptanceRadius = 50.0f);

    void JumpStart();
    void JumpStop();
    void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr);

	/* ---------------- Warping ------------------------------*/
    UFUNCTION(BlueprintPure, Category = "Locomotion")
    void GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, bool bLockedOn = false) const;
	void GetWarpingLocRotFreeFlow(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FVector2D& InputDir = FVector2D::ZeroVector, bool bLockedOn = false) const;
	void UpdateMotionWarpData(const FVector& DesiredLoc, const FRotator& DesiredRot);
	void ClearMotionWarpData();

    UAsyncRootMovement* ApplyRootMotionSourceConstant(float Duration, FVector Force, FVector VelocityOnFinish = FVector::ZeroVector, float ClampVelocityOnFinish = 0.0f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity, UCurveFloat* StrengthOverTime = nullptr, bool bAdditive = false);
    
    UAsyncRootMovement* ApplyRootMotionSourceJump(FVector Direction, float Distance = 600.0f, float Height = 300.0f, float Duration = 0.6f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector SetVelocityOnFinish = FVector::ZeroVector, float ClampVelocityOnFinish = 0.0f);
    
    UAsyncRootMovement* ApplyRootMotionSourceMoveTo(FVector StartLoc, FVector TargetLoc, float Duration, bool bRestrictSpeedToExpected = true);
    UAsyncRootMovement* ApplyRootMotionSourceMoveToDynamic(FVector StartLoc, FVector InitTargetLoc, float Duration, bool bRestrictSpeedToExpected = true);
    UAsyncRootMovement* ApplyRootMotionSourceRadial(FVector Origin, float Radius, float Strength, float Duration, bool bIsPush = true, UCurveFloat* StrengthOverTime = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void OnRootMotionComplete(UAsyncRootMovement* RootMotion = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void ClearRootMotionSource(UAsyncRootMovement* RootMotion);

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void ClearAllRootMotionSources();

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    UAsyncRootMovement* GetActiveRootMotionOverrideSource() const { return asyncRootMotionOverride; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    TArray<UAsyncRootMovement*> GetActiveRootMotionAdditiveSources() const { return asyncRootMotionsAdditive; }

    // Blueprint Helpers
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    void AddRootMotionSource(UAsyncRootMovement* RootMotion, bool bAdditive = false);
};
