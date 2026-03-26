// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "PlayerLocomotionComponent.generated.h"

class UCharacterMovementComponent;
class UStatsComponent;

/**
 * Player locomotion driver (Option B)
 * Implements locomotion interface so states can command movement without touching ACharacter directly
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UPlayerLocomotionComponent : public UActorComponent, public ILocomotionCmdInterface
{
    GENERATED_BODY()

private:
    UPROPERTY() ACharacter* ownerChar;
    UPROPERTY() UCharacterMovementComponent* moveComp;
    UPROPERTY() UStatsComponent* statsComp;
    FTimerHandle TH_StopLaunch;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTag activeMoveProfile;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Tags")
    FGameplayTagContainer moveOverrides;

    float defaultGravity = 1.0f;

    bool EnsureOwnerCharacter();
    bool HasOverrideExact(const FGameplayTag& Tag) const;

    void ApplyMovementFromTagsAndStats();
    float ResolveSpeedForProfile(const FGameplayTag& Profile) const;

    // Optional: safe fallback numbers if no StatsComponent is present (prototype/editor testing)
    float FallbackSpeedForProfile(const FGameplayTag& Profile) const;
    float FallbackAcceleration() const { return 2048.f; }
    float FallbackJumpZ() const { return 420.f; }

    UFUNCTION() void StopLaunch();

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug = false;

    UPROPERTY(EditAnywhere, Category="Locomotion|Jump")
    bool bAllowMultiJump = false;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float jumpBufferSeconds = 0.15f;

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Jump", meta=(ClampMin="0.0"))
    float coyoteSeconds = 0.10f;

    UPROPERTY(VisibleAnywhere, Category="Locomotion|Jump")
    float lastGroundedTime = -1000.0f; // Safe default far in past

    UPROPERTY(EditAnywhere, Category = "Locomotion|Juggle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float juggleGravity = 0.2f;

    virtual void BeginPlay() override;

public:
    UPlayerLocomotionComponent();

    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(const FGameplayTag& NewProfile) override;

    virtual void AddMoveOverrideTag(const FGameplayTag& OverrideTag) override;
    virtual void RemoveMoveOverrideTag(const FGameplayTag& OverrideTag) override;

    virtual void RefreshMovement() override;

    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) override;

    /* ---------------- Jump buffering / coyote time ----------------*/
    virtual bool CanMultiJump() const override { return bAllowMultiJump; }
    virtual bool CanUseBufferedJump(bool& bWantsJump, float& JumpPressedTime) const override;
    virtual void MarkGroundedNow() override;
    
    /* ---------------- Movement Actions ------------------------------*/
    virtual void AddMoveInput(const FVector2D& Move) override;

    virtual void JumpPressed() override;
    virtual void JumpReleased() override;
    virtual void LaunchCharacterHNS(FVector Velocity = FVector::ZeroVector, bool OverrideXY = true, bool OverrideZ = true, float TimeToStop = 0.0f, AActor* Actor = nullptr) override;
};