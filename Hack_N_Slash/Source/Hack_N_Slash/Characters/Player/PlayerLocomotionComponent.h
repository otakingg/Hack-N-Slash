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

    virtual void BeginPlay() override;

public:
    UPlayerLocomotionComponent();

    /* ---------------- Tag-driven tuning ---------------- */
    virtual void SetMoveProfileTag(FGameplayTag NewProfile) override;

    virtual void AddMoveOverrideTag(FGameplayTag OverrideTag) override;
    virtual void RemoveMoveOverrideTag(FGameplayTag OverrideTag) override;

    virtual void RefreshMovement() override;

    /* ---------------- Engine movement mode ---------------- */
    virtual void SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode = 0) override;

    /* ---------------- ILocomotionCmdInterface ---------------- */
    virtual void AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate) override;
    virtual void AddMoveInputScaled(const FVector2D& Move, float Scale = 1.0f) override;

    virtual void JumpPressed() override;
    virtual void JumpReleased() override;
    virtual void LaunchUp(float JumpZ) override;
};