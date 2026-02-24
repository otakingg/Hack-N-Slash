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
	UPROPERTY() AEnemyController* controller {nullptr};
    UPROPERTY() ACharacter* ownerChar {nullptr};
    UPROPERTY() UCharacterMovementComponent* moveComp {nullptr};
    UPROPERTY() UStatsComponent* statsComp {nullptr};

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
    bool bDebug {false};

    UPROPERTY(EditAnywhere, Category="Locomotion|Jump")
    bool bAllowMultiJump {false};

    /** --- Tuning --- */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float jumpBufferSeconds {0.15f};

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float coyoteSeconds {0.10f};

    /** --- Jump buffer + coyote (shared) --- */
    UPROPERTY(VisibleAnywhere, Category="Locomotion|Jump")
    float lastGroundedTime {-1000.f}; // Safe default far in past

	virtual void BeginPlay() override;

public:
	UEnemyLocomotionComponent();
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(FGameplayTag NewProfile) override;

    virtual void AddMoveOverrideTag(FGameplayTag OverrideTag) override;
    virtual void RemoveMoveOverrideTag(FGameplayTag OverrideTag) override;

    virtual void RefreshMovement() override;

    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) override;

    /* ---------------- Jump buffering / coyote time ----------------*/
    virtual bool CanMultiJump() const override { return bAllowMultiJump; }
    virtual bool CanUseBufferedJump(bool& bWantsJump, float& JumpPressedTime) const override;
    virtual void MarkGroundedNow() override;
    
    /* ---------------- Movement Actions ------------------------------*/
	virtual void AddMoveInputScaled(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f, float Scale = 1.0f) override;
    virtual void LaunchUp(float JumpZ) override;
};
