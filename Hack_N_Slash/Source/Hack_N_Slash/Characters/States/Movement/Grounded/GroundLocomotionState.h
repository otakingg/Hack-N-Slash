// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GroundedModeState.h"
#include "GroundLocomotionState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundLocomotionState : public UGroundedModeState
{
    GENERATED_BODY()

protected:
    /** If true, character rotates to movement direction. */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    bool bOrientRotationToMovement {true};

    /** If true (and not orient-to-movement), rotation follows controller desired rotation. */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    bool bUseControllerDesiredRotation {false};

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Rotation")
    FRotator rotationRate {FRotator(0.f, 0.0f, 360.f)};

    /** Basic walk tuning */
    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Tuning", meta = (ClampMin = "0.0"))
    float maxWalkSpeed {600.f};

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Tuning", meta = (ClampMin = "0.0"))
    float maxAcceleration {2048.f};

    UPROPERTY(EditDefaultsOnly, Category="Locomotion|Tuning", meta = (ClampMin = "0.0"))
    float brakingDecelerationWalking {2048.f};

	/** Camera */
	UPROPERTY(EditDefaultsOnly, Category = Camera, meta = (ClampMin = "0.0"))
	float lookUpRate {45.f};

	UPROPERTY(EditDefaultsOnly, Category = Camera, meta = (ClampMin = "0.0"))
	float turnRate {45.f};

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    /** Enhanced Input Move vector (-1..1 X/Y). Returns true when consumed. */
	virtual bool OnInputLook(const FVector2D& Look) override;
    virtual bool OnInputMove(const FVector2D& Move) override;
};