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
    UPROPERTY() TObjectPtr<UAirborneModeState> activeSubState = nullptr;

    /** Default airborne behavior */
    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint child of air falling mode"))
    TSubclassOf<UAirborneModeState> fallingModeClass;

    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint child of air rising mode"))
    TSubclassOf<UAirborneModeState> risingModeClass;

    UPROPERTY(EditDefaultsOnly, Category="Air", meta=(Tooltip="Set = Blueprint Child of Air Jump Start State"))
    TSubclassOf<UAirborneModeState> airJumpStartModeClass;

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
    virtual bool OnJumpStartIntent() override;
    virtual bool OnJumpStopIntent() override;
    virtual bool OnLookIntent(const FVector2D& Look) override;
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override;

    virtual void OnJumpApexReached() override;
    virtual void OnLanded(const FHitResult& Hit) override;
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
};