#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "States/Core/CharacterState.h"
#include "States/Movement/Airborne/AirContainerState.h"
#include "StateMachineComponent.generated.h"

class ILocomotionCmdInterface;
class ICombatCmdInterface;

UENUM(BlueprintType)
enum class EStateLayer : uint8
{
    Movement UMETA(DisplayName="Movement"),
    Action   UMETA(DisplayName="Action")
};

UENUM(BlueprintType)
enum class ECommandSource : uint8
{
    Player,
    AI,
    Script,
    Network
};

USTRUCT(BlueprintType)
struct FCommandContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECommandSource Source = ECommandSource::Player;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UObject> Instigator = nullptr;

    // Optional debug / ordering
    UPROPERTY(BlueprintReadWrite)
    float TimestampSeconds = 0.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY()
    ACharacter* ownerChar {nullptr};

    /** Cached command interfaces (Option B) */
    ILocomotionCmdInterface* LocomotionCmd;
    ICombatCmdInterface* CombatCmd;

    void InitializeMovementMap();
    void InitializeActionMap();
    void CacheCommandInterfaces();

    void ApplyBaselineMovement(bool bForce);
    static bool CanTransition(const UCharacterState*, const UCharacterState*, bool);

    UFUNCTION()
    void HandleJumpApexReached();

    UFUNCTION()
    void HandleLanded(const FHitResult& Hit);

    UFUNCTION()
    void HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode);

protected:
    UPROPERTY(EditAnywhere)
    bool bDebug {false};

    /** Current / Previous per layer */
    UPROPERTY(VisibleAnywhere, Category="Movement")
    UMovementState* currentMovementState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Movement")
    UMovementState* previousMovementState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Action")
    UActionState* currentActionState {nullptr};

    UPROPERTY(VisibleAnywhere, Category="Action")
    UActionState* previousActionState {nullptr};

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

    /* ---------------- State Changes ---------------- */
    void ChangeState(EStateLayer, UCharacterState*, bool);
    void ChangeMovementState(UMovementState*, bool);
    void ChangeActionState(UActionState*, bool);

    void RequestAirMode(TSubclassOf<class UAirborneModeState> ModeClass);
    void ClearAirMode();

    /* ---------------- Queries ---------------- */
    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }
    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    // New: expose interfaces to states
    ILocomotionCmdInterface* GetLocomotionCommands() const;
    ICombatCmdInterface*     GetCombatCommands() const;

    UActionState* GetActionState(TSubclassOf<UActionState> StateClass) const;
    template<typename TState>
    TState* GetActionState(TSubclassOf<TState> StateClass) const
    {
        static_assert(TIsDerivedFrom<TState, UActionState>::IsDerived, "TState must derive from UActionState");

        if (!StateClass) return nullptr;
        return Cast<TState>(GetActionState(TSubclassOf<UActionState>(StateClass.Get())));
    }

    UMovementState* GetMovementState(TSubclassOf<UMovementState> StateClass) const;
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

    /* ---------------- Unified Requests (NEW) ---------------- */
    void RequestAttack(const FVector2D& InputVector, const FCommandContext& Ctx);

    void RequestJumpPressed(const FCommandContext& Ctx);
    void RequestJumpReleased(const FCommandContext& Ctx);

    void RequestLook(const FVector2D& InputVector, const FCommandContext& Ctx);
    void RequestMove(const FVector2D& InputVector, const FCommandContext& Ctx);

    /* ---------------- Compatibility: Player Input Adapters ---------------- */
    void OnInputAttackPressed(const FVector2D& InputVector);

    void OnInputJumpPressed();
    void OnInputJumpReleased();
    void OnInputLook(const FVector2D& InputVector);
    void OnInputMove(const FVector2D& InputVector);

    /* ---------------- Animation / AnimInstance forwarding ---------------- */
    void OnAnimNotify(FName);
    void OnMontageBlendingOut(UAnimMontage*, bool);
};