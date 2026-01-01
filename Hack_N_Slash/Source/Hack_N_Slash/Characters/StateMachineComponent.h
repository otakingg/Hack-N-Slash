#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "States/CharacterState.h" // Contains UMovementState / UActionState
#include "StateMachineComponent.generated.h"

UENUM(BlueprintType)
enum class EStateLayer : uint8
{
    Movement UMETA(DisplayName="Movement"),
    Action   UMETA(DisplayName="Action")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    /** Current / Previous per layer (strongly-typed) */
    UPROPERTY()
    UMovementState* currentMovementState {nullptr};

    UPROPERTY()
    UMovementState* previousMovementState {nullptr};

    UPROPERTY()
    UActionState* currentActionState {nullptr};

    UPROPERTY()
    UActionState* previousActionState {nullptr};

    void InitializeMovementMap();
    void InitializeActionMap();

    static bool CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce);

protected:
    /**
     * One persistent instance per state class per layer.
     * Populate in editor with: IdleMoveState → nullptr, FallState → nullptr, LightAttackState → nullptr, KnockdownState → nullptr, etc.
     */
    UPROPERTY(EditDefaultsOnly, Instanced, Category="States")
    TMap<TSubclassOf<UMovementState>, UMovementState*> movementStateInstances;

    UPROPERTY(EditDefaultsOnly, Instanced, Category="States")
    TMap<TSubclassOf<UActionState>, UActionState*> actionStateInstances;

    /** Optional defaults (strongly-typed) */
    UPROPERTY(EditDefaultsOnly, Category="Defaults")
    TSubclassOf<UMovementState> defaultMovementStateClass;

    UPROPERTY(EditDefaultsOnly, Category="Defaults")
    TSubclassOf<UActionState> defaultActionStateClass;

    virtual void BeginPlay() override;

public:
    UStateMachineComponent();

    /* ---------------- State Changes ---------------- */

    void ChangeMovementState(UMovementState* NewState, bool bForce = false);
    void ChangeActionState(UActionState* NewState, bool bForce = false);

    // Convenience layer-based versions (rarely needed, but useful for generic code)
    void ChangeState(EStateLayer Layer, UCharacterState* NewState, bool bForce = false);

    /* ---------------- Queries ---------------- */

    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }

    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    bool IsInMovementTag(FGameplayTag Tag) const;
    bool IsInActionTag(FGameplayTag Tag) const;
    bool IsInAnyTag(FGameplayTag Tag) const;

    /* ---------------- Event Forwarding ---------------- */
    /* Called by Character / AnimInstance */
    void OnInputAttackPressed();
    void OnInputBlockDodgePressed();
    virtual void OnInputJumpPressed();
    virtual void OnInputJumpReleased();
    virtual void OnInputMoveStarted();
    virtual void OnInputMoveStopped();

    void OnLanded(const FHitResult& Hit);
    void OnMovementModeChanged(EMovementMode PrevMode, uint8 PrevCustomMode);

    void OnAnimNotify(FName NotifyName);
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};