// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "EnemyLocomotionComponent.generated.h"

class AEnemyController;
class UCharacterMovementComponent;
class UStatsComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UEnemyLocomotionComponent : public UActorComponent, public ILocomotionCmdInterface
{
	GENERATED_BODY()

private:
	UPROPERTY() AEnemyController* controller = nullptr;
    UPROPERTY() ACharacter* ownerChar = nullptr;
    UPROPERTY() UCharacterMovementComponent* moveComp = nullptr;
    //UPROPERTY() UStatsComponent* statsComp = nullptr;
    int32 rootMotionSourceID = INDEX_NONE;
    
    FTimerHandle TH_StopMovement;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTag activeMoveProfile;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTagContainer moveOverrides;

    bool EnsureOwnerCharacter();
    bool HasOverrideExact(const FGameplayTag& Tag) const;

    void ApplyMovementFromTagsAndStats();
    float ResolveSpeedForProfile(const FGameplayTag& Profile) const;

    // Optional: safe fallback numbers if no StatsComponent is present (prototype/editor testing)
    float FallbackSpeedForProfile(const FGameplayTag& Profile) const;
    float FallbackAcceleration() const { return 2048.f; }
    float FallbackJumpZ() const { return 420.f; }

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float coyoteSeconds = 0.10f;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Jump")
    float lastGroundedTime = -1000.0f; // Safe default far in past



    UPROPERTY(EditAnywhere, Category = "Locomotion", meta = (ClampMin = "0.0"))
    float gravity = 1.7f;



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

public:
	UEnemyLocomotionComponent();

    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(const FGameplayTag& NewProfile) override;

    virtual void AddMoveOverrideTag(const FGameplayTag& OverrideTag) override;
    virtual void RemoveMoveOverrideTag(const FGameplayTag& OverrideTag) override;

    virtual void RefreshMovement() override;

    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) override;

    /* ---------------- Jump buffering / coyote time ----------------*/
    virtual void MarkGroundedNow() override;
    
    /* ---------------- Movement Actions ------------------------------*/
	virtual void AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f) override;
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) override;
    virtual void ApplyRootMotionSource(const FRootMotionSource& RootMotionSrc) override;
    virtual void ClearRootMotionSource() override;
};
