#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "States/Core/CharacterState.h"
#include "States/Movement/Airborne/AirContainerState.h"
#include "States/Movement/Grounded/GroundContainerState.h"
#include "StateMachineComponent.generated.h"

class ILocomotionCmdInterface;
class ICombatCmdInterface;

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
    UPROPERTY() ACharacter* ownerChar = nullptr;

    /** Cached command interfaces (Option B) */
    ILocomotionCmdInterface* iLocomotionCmd = nullptr;
    ICombatCmdInterface* iCombatCmd = nullptr;

    void CacheCommandInterfaces();
    void InitializeMovementMap();
    void InitializeActionMap();

    void ApplyBaselineMovement(bool bForce);
    static bool CanTransition(const UCharacterState*, const UCharacterState*, bool);

    UFUNCTION() void HandleJumpApexReached();
    UFUNCTION() void HandleLanded(const FHitResult& Hit);
    UFUNCTION() void HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode);

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tags")
    FGameplayTagContainer activeStateTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags")
    FGameplayTag airborneTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags")
    FGameplayTag groundedTag;

    /** Current / Previous per layer */
    UPROPERTY(VisibleAnywhere, Transient, Category="Movement")
    UMovementState* currentMovementState = nullptr;

    UPROPERTY(VisibleAnywhere, Transient, Category="Movement")
    UMovementState* previousMovementState = nullptr;

    UPROPERTY(VisibleAnywhere, Transient, Category="Action")
    UActionState* currentActionState = nullptr;

    UPROPERTY(VisibleAnywhere, Transient, Category="Action")
    UActionState* previousActionState = nullptr;

    /** State classes (editable) */
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    TArray<TSubclassOf<UMovementState>> movementStateClasses;

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UMovementState>> movementStateInstances;

    UPROPERTY(EditDefaultsOnly, Category="Action")
    TArray<TSubclassOf<UActionState>> actionStateClasses;

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UActionState>> actionStateInstances;

    /** Defaults */
    UPROPERTY(EditDefaultsOnly, Category="Movement|Defaults", meta=(Tooltip="Set = Blueprint child of Ground Container State"))
    TSubclassOf<UMovementState> defaultGroundMovementClass;

    UPROPERTY(EditDefaultsOnly, Category="Movement|Defaults", meta=(Tooltip="Set = blueprint child of Air Container State"))
    TSubclassOf<UMovementState> defaultAirMovementClass;

    UPROPERTY(EditDefaultsOnly, Category="Action|Defaults", meta=(Tooltip="Set = blueprint child of Action Container State"))
    TSubclassOf<UActionState> defaultActionStateClass;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type) override;

public:
    UStateMachineComponent();

    /* ----------------------Tags--------------------- */

    UFUNCTION(BlueprintCallable, Category="State|Tags")
    const FGameplayTagContainer& GetActiveStateTags() const { return activeStateTags; }

    UFUNCTION(BlueprintCallable, Category="State|Tags")
    void RebuildActiveStateTags();

    // Generic query: looks at the published container
    UFUNCTION(BlueprintCallable, Category="State|Tags")
    bool HasActiveTag(const FGameplayTag& Tag) const;

    // Optional exact versions (handy for "exact state identity")
    UFUNCTION(BlueprintCallable, Category="State|Tags")
    bool HasExactActiveTag(const FGameplayTag& Tag) const;

    // Layer-specific queries: looks at layer state identity
    UFUNCTION(BlueprintCallable, Category="State|Tags")
    bool IsInMovementTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State|Tags")
    bool IsInActionTag(const FGameplayTag& Tag) const;

    /* ---------------- State Changes ---------------- */
    void ChangeState(EStateLayer, UCharacterState*, bool);
    void ChangeMovementState(UMovementState*, bool);
    void ChangeActionState(UActionState*, bool);
    void ClearActionState();

    void RequestAirborneMode(TSubclassOf<class UAirborneModeState> ModeClass);
    UFUNCTION() void ClearAirborneMode();

    void RequestGroundedMode(TSubclassOf<class UGroundedModeState> ModeClass);
    UFUNCTION() void ClearGroundedMode();

    /* ---------------- Queries ---------------- */
    bool IsAirborne() const;
    bool IsGrounded() const;

    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }
    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    // Expose interfaces to states
    ICombatCmdInterface*     GetCombatCommands() const;
    ILocomotionCmdInterface* GetLocomotionCommands() const;

    UActionState* GetActionState(TSubclassOf<UActionState> StateClass) const;
    UActionState* GetActionStateByTag(const FGameplayTag& Tag) const;
    template <typename TState>
    TState* GetActionState(TSubclassOf<TState> StateClass) const
    {
        static_assert(TIsDerivedFrom<TState, UActionState>::IsDerived, "TState must derive from UActionState");

        if (!StateClass) return nullptr;
        return Cast<TState>(GetActionState(TSubclassOf<UActionState>(StateClass.Get())));
    }

    UMovementState* GetMovementState(TSubclassOf<UMovementState> StateClass) const;
    UMovementState* GetMovementStateByTag(const FGameplayTag& Tag) const;
    template <typename TState>
    TState* GetMovementState(TSubclassOf<TState> StateClass) const
    {
        static_assert(TIsDerivedFrom<TState, UMovementState>::IsDerived, "TState must derive from UMovementState");

        if (!StateClass) return nullptr;
        return Cast<TState>(GetMovementState(TSubclassOf<UMovementState>(StateClass.Get())));
    }

    /* ---------------- Unified Requests ---------------- */
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestAttack(const FVector2D& InputVector);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestBlockStart();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestBlockStop();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestDodge(const FVector2D& InputVector);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestJumpStart();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestJumpStop();
    
    void RequestLook(const FVector2D& InputVector);
    
    void RequestMove(const FVector2D& InputVector);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestMoveTo(AActor* Target, const FVector Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f);

    /* ---------------- Animation / AnimInstance forwarding ---------------- */
    void OnAnimNotify(FGameplayTag NotifyTag);
    void OnMontageBlendingOut(UAnimMontage*, bool);
    
    /* -------------------- Combat Forwarding -----------------------*/
    void OnReceiveHit(const struct FAtkHitData& HitData);
};