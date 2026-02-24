#pragma once

#include "CoreMinimal.h"
#include "../../Core/CharacterState.h"
#include "AirContainerState.generated.h"

class UAirborneModeState;
struct FCommandContext;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAirContainerState : public UMovementState
{
    GENERATED_BODY()

protected:
    UPROPERTY()
    TObjectPtr<UAirborneModeState> activeSubState {nullptr};

    /** Default airborne behavior */
    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint child of air falling mode"))
    TSubclassOf<UAirborneModeState> fallingModeClass;

    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint child of air rising mode"))
    TSubclassOf<UAirborneModeState> risingModeClass;

    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint Child of Air Jump Start State"))
    TSubclassOf<UAirborneModeState> airJumpStartModeClass;

    /** Braking */
    UPROPERTY(EditDefaultsOnly, Category="Air|Braking", meta=(ClampMin="0.0"))
    float brakingDecelerationFalling {0.0f};

    /** Air Control */
    UPROPERTY(EditDefaultsOnly, Category="Air|Air Control", meta=(ClampMin="0.0"))
    float airControl {0.05f};

    UPROPERTY(EditDefaultsOnly, Category="Air|Air Control", meta=(ClampMin="0.0"))
    float airControlBoostMult {2.0f};

    UPROPERTY(EditDefaultsOnly, Category="Air|Air Control", meta=(ClampMin="0.0"))
    float airControlBoostVelocityThreshold {25.0f};

    /** Friction */
    UPROPERTY(EditDefaultsOnly, Category="Air|Friction", meta=(ClampMin="0.0"))
    float fallingLateralFriction {0.0f};

    void SetSubState(TSubclassOf<UAirborneModeState> NewSubStateClass);

public:
    /** Optional airborne overrides (glide, hover, etc.) */
    void RequestAirborneMode(TSubclassOf<UAirborneModeState> ModeClass);

    /** Return to default air mode */
    UFUNCTION () void ClearAirborneMode();

    virtual void EnterState() override;
    virtual void ExitState() override;

    // Tags: publish container + active substate
    virtual void GatherStateTags(FGameplayTagContainer& OutTags) const override;

    // Forward intents/events to substate
    virtual bool OnJumpPressed() override;
    virtual bool OnJumpReleased() override;
    virtual bool OnLookIntent(const FVector2D& Look) override;
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc, float AcceptanceRadius = 50.0f) override;

    virtual void OnJumpApexReached() override;
    virtual void OnLanded(const FHitResult& Hit) override;
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
};