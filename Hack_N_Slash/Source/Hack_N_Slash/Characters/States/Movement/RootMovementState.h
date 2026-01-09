// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Core/CharacterState.h"
#include "RootMovementState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API URootMovementState : public UMovementState
{
    GENERATED_BODY()

private:
    // Jump buffer + coyote live at ROOT
    float lastGroundedTime {1000.f};
    FTimerHandle TH_JumpBuffer;

    void StartJumpBufferWindow();
    UFUNCTION() void ExpireJumpBuffer();

    bool CanUseBufferedJump() const;
    void TryConsumeBufferedJump();

    void ApplyBaselineContainer(); // choose ground vs air

    // Delegate handlers (bind THESE, not virtual UFUNCTIONs)
    UFUNCTION()
    void HandleLanded(const FHitResult& Hit);
    
    UFUNCTION()
    void HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode);

protected:
    /** --- Tuning --- */
    UPROPERTY(EditDefaultsOnly, Category="Movement|Tuning")
    float jumpBufferSeconds {0.15f};

    UPROPERTY(EditDefaultsOnly, Category="Movement|Tuning")
    float coyoteSeconds {0.10f};

    /** --- Container classes --- */
    UPROPERTY(EditDefaultsOnly, Category="Movement|Containers", meta=(Tooltip="Set = child of UGroundContainerState"))
    TSubclassOf<UMovementState> defaultGroundContainerClass;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Containers", meta=(Tooltip="Set = child of UAirContainerState"))
    TSubclassOf<UMovementState> defaultAirContainerClass;

    /** Active container (ground or air) */
    UPROPERTY()
    TObjectPtr<UMovementState> activeContainer {nullptr};

    void SetActiveContainer(TSubclassOf<UMovementState> NewContainerClass);

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // ROOT captures jump buffer/coyote + forwards input to container
    virtual bool OnInputMove(const FVector2D& Move) override;
    virtual bool OnInputLook(const FVector2D& Look) override;
    virtual bool OnInputJumpPressed() override;
    virtual bool OnInputJumpReleased() override;
};