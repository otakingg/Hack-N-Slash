#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "States/Core/CharacterState.h" // Contains UMovementState / UActionState
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
    UPROPERTY()
    ACharacter* ownerChar {nullptr};

    void InitializeMovementMap();
    void InitializeActionMap();

    static bool CanTransition(const UCharacterState*, const UCharacterState*, bool);

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug {false};

    /** Current / Previous per layer (strongly-typed) */
    UPROPERTY(VisibleAnywhere, Category="Movement")
    UMovementState* currentMovementState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Movement")
    UMovementState* previousMovementState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Action")
    UActionState* currentActionState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Action")
    UActionState* previousActionState {nullptr};

    //Pick which movement state classes exist (editable)
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    TArray<TSubclassOf<UMovementState>> movementStateClasses;

    // Runtime instances only (NOT editable / NOT serialized)
    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UMovementState>> movementStateInstances;

    //Pick which action state classes exist (editable)
    UPROPERTY(EditDefaultsOnly, Category="Action")
    TArray<TSubclassOf<UActionState>> actionStateClasses;

    // Runtime instances only (NOT editable / NOT serialized)
    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UActionState>> actionStateInstances;
    
    /** Optional defaults (strongly-typed) */
    UPROPERTY(EditDefaultsOnly, Category="Movement", meta = (Tooltip = "Set = RootMovementState or Blueprint child of it"))
    TSubclassOf<UMovementState> defaultMovementStateClass;

    UPROPERTY(EditDefaultsOnly, Category="Action")
    TSubclassOf<UActionState> defaultActionStateClass;

    virtual void BeginPlay() override;

public:
    UStateMachineComponent();

    /* ---------------- State Changes ---------------- */
    // Convenience layer-based versions (rarely needed, but useful for generic code)
    void ChangeState(EStateLayer, UCharacterState*, bool);
    void ChangeMovementState(UMovementState*, bool);
    void ChangeActionState(UActionState*, bool);

    /* ---------------- Queries ---------------- */

    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }

    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    UActionState* GetActionState(TSubclassOf<UActionState> StateClass);
    template<typename TState>
    TState* GetActionState(TSubclassOf<TState> StateClass) const
    {
        static_assert(TIsDerivedFrom<TState, UActionState>::IsDerived, "TState must derive from UActionState");

        if (!StateClass) return nullptr;
        return Cast<TState>(GetActionState(TSubclassOf<UActionState>(StateClass.Get())));
    }

    UMovementState* GetMovementState(TSubclassOf<UMovementState> StateClass);
    template<typename TState>
    TState* GetMovementState(TSubclassOf<TState> StateClass) const
    {
        static_assert(TIsDerivedFrom<TState, UMovementState>::IsDerived, "TState must derive from UMovementState");

        if (!StateClass) return nullptr;
        return Cast<TState>(GetMovementState(TSubclassOf<UMovementState>(StateClass.Get())));
    }

    bool IsInMovementTag(FGameplayTag Tag) const;
    bool IsInActionTag(FGameplayTag Tag) const;
    bool IsInAnyTag(FGameplayTag Tag) const;

    /* ---------------- Event Forwarding ---------------- */
    /* Called by Character / AnimInstance */
    void OnInputAttackPressed(const FVector2D& InputVector);
    void OnInputBlockDodgePressed(const FVector2D& InputVector);
    virtual void OnInputJumpPressed();
    virtual void OnInputJumpReleased();
    virtual void OnInputLook(const FVector2D& InputVector);
    virtual void OnInputMove(const FVector2D& InputVector);

    void OnAnimNotify(FName);
    void OnMontageBlendingOut(UAnimMontage*, bool);
};