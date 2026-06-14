// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "LocomotionComponent.generated.h"

class AEnemyController;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class UMotionWarpingComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API ULocomotionComponent : public UActorComponent, public ILocomotionCmdInterface
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) AEnemyController* controller = nullptr;
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) UMotionWarpingComponent* motionWarpComp = nullptr;
    UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
    UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
    UPROPERTY(Transient) UAsyncRootMovement* activeAsyncRootMotion = nullptr;

    FTimerHandle TH_ClearAirborne;
    FTimerHandle TH_StopMovement;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTag activeMoveProfile;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTagContainer moveOverrides;

    bool EnsureReferences();
    bool HasOverrideExact(const FGameplayTag& Tag) const;

    void ApplyMovementFromTagsAndStats();
    float ResolveSpeedForProfile(const FGameplayTag& Profile) const;

    // Safe fallback numbers if no StatsComponent is present
    float FallbackSpeedForProfile(const FGameplayTag& Profile) const;
    float FallbackAcceleration() const { return 2048.f; }
    float FallbackJumpZ() const { return 420.f; }

protected:
    UPROPERTY(EditAnywhere, Category="Locomotion")
    bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float coyoteSeconds = 0.10f;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Jump")
    float lastGroundedTime = -1000.0f; // Safe default far in past

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump")
    UAnimMontage* jumpMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump")
    UAnimMontage* doubleJumpMontage = nullptr;


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

    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(const FGameplayTag& NewProfile) override;

    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    virtual void AddMoveOverrideTag(const FGameplayTag& OverrideTag) override;
    
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
    virtual void RemoveMoveOverrideTag(const FGameplayTag& OverrideTag) override;

    virtual void RefreshMovement() override;

    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) override;

    /* ---------------- Jump buffering / coyote time ----------------*/
    virtual bool CanCoyoteJump() override;
    virtual void MarkGroundedNow() override;
    
    /* ---------------- Movement Actions ------------------------------*/
    virtual void AddMoveInput(const FVector2D& Move) override;
    UFUNCTION(BlueprintCallable, Category = "Locomotion")
	virtual void AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f) override;

    virtual void JumpStart() override;
    virtual void JumpStop() override;
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) override;

	/* ---------------- Warping ------------------------------*/
    virtual void GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, bool bLockedOn = false) override;
	virtual void GetWarpingLocRotFreeFlow(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FVector2D& InputDir = FVector2D::ZeroVector, bool bLockedOn = false) override;
	virtual void UpdateMotionWarpData(const FVector& DesiredLoc, const FRotator& DesiredRot) override;
	virtual void ClearMotionWarpData() override;
    virtual UAsyncRootMovement* ApplyRootMotionSourceConstant(float Duration, FVector Force, FVector VelocityOnFinish = FVector::ZeroVector, float ClampVelocityOnFinish = 0.0f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity, UCurveFloat* StrengthOverTime = nullptr, bool bAdditive = false) override;
    virtual UAsyncRootMovement* ApplyRootMotionSourceJump(FVector Direction, float Distance = 600.0f, float Height = 300.0f, float Duration = 0.6f,
        ERootMotionFinishVelocityMode VelocityOnFinishMode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector SetVelocityOnFinish = FVector::ZeroVector, float ClampVelocityOnFinish = 0.0f) override;
    virtual UAsyncRootMovement* ApplyRootMotionSourceMoveTo(FVector StartLoc, FVector TargetLoc, float Duration, bool bRestrictSpeedToExpected = true) override;
    virtual UAsyncRootMovement* ApplyRootMotionSourceMoveToDynamic(FVector StartLoc, FVector InitTargetLoc, float Duration, bool bRestrictSpeedToExpected = true) override;
    virtual UAsyncRootMovement* ApplyRootMotionSourceRadial(FVector Origin, float Radius, float Strength, float Duration, bool bIsPush = true, UCurveFloat* StrengthOverTime = nullptr) override;
    virtual void ClearRootMotionSource() override;
    virtual UAsyncRootMovement* GetActiveRootMotionSource() const override { return activeAsyncRootMotion; }
};
