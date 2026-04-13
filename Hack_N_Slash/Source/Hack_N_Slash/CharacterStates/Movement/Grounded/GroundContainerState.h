#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "GroundContainerState.generated.h"

class UGroundedModeState;
struct FCommandContext;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UGroundContainerState : public UMovementState
{
    GENERATED_BODY()

protected:
    UPROPERTY() TObjectPtr<UGroundedModeState> activeSubState = nullptr;

    /** Default grounded behavior (locomotion) */
    UPROPERTY(EditDefaultsOnly, Category="Ground", meta=(Tooltip="Set = Blueprint Child of Ground Locomotion State"))
    TSubclassOf<UGroundedModeState> defaultGroundedModeClass;

    void SetSubState(TSubclassOf<UGroundedModeState> NewSubStateClass);

public:
    /** Request a grounded override (climb, grind, etc.) */
    void RequestGroundedMode(TSubclassOf<UGroundedModeState> ModeClass);

    /** Return to default grounded mode */
    void ClearGroundedMode();

    virtual void EnterState() override;
    virtual void ExitState() override;

    // Tags: publish container + active substate
    virtual void GatherStateTags(FGameplayTagContainer& OutTags) const override;

    // Forward intents/events to substate
    virtual bool OnMoveIntent(const FVector2D& Move) override;
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override;

    virtual void OnLanded(const FHitResult& Hit) override;
    virtual void OnMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;
};