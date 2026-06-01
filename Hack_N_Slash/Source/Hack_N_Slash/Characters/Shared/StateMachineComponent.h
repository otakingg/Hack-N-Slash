#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "../../CharacterStates/Core/CharacterState.h"
#include "../../CharacterStates/Movement/Airborne/AirContainerState.h"
#include "../../CharacterStates/Movement/Grounded/GroundContainerState.h"
#include "../../Enums/EPlayerAction.h"
#include "StateMachineComponent.generated.h"

class ILocomotionCmdInterface;
class ICombatCmdInterface;
struct FAtkHitData;
struct FEnemyAtkData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
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
    UPROPERTY(EditAnywhere, Category = "State Machine")
    bool bDebug = false;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="State Machine|Tags")
    FGameplayTagContainer activeStateTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State Machine|Tags")
    FGameplayTag airborneTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="State Machine|Tags")
    FGameplayTag groundedTag;

    /** Current / Previous per layer */
    UPROPERTY(Transient, VisibleAnywhere, Category="State Machine|Movement")
    UMovementState* currentMovementState = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, Category="State Machine|Movement")
    UMovementState* previousMovementState = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, Category="State Machine|Action")
    UActionState* currentActionState = nullptr;

    UPROPERTY(Transient, VisibleAnywhere, Category="State Machine|Action")
    UActionState* previousActionState = nullptr;

    /** State classes */
    UPROPERTY(EditDefaultsOnly, Category="State Machine|Movement")
    TArray<TSubclassOf<UMovementState>> movementStateClasses;

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UMovementState>> movementStateInstances;

    UPROPERTY(EditDefaultsOnly, Category="State Machine|Action")
    TArray<TSubclassOf<UActionState>> actionStateClasses;

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UActionState>> actionStateInstances;

    /** Defaults */
    UPROPERTY(EditDefaultsOnly, Category="State Machine|Movement", meta = (Tooltip = "Set = Tag of Ground Container"))
    FGameplayTag defaultGroundMovementTag;

    UPROPERTY(EditDefaultsOnly, Category="State Machine|Movement", meta = (Tooltip = "Set = Tag of Airborne Container"))
    FGameplayTag defaultAirborneMovementTag;

    UPROPERTY(EditDefaultsOnly, Category="State Machine|Action", meta = (Tooltip = "Set = Tag of None Action State"))
    FGameplayTag defaultActionTag;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type) override;

public:
    UStateMachineComponent();

    /* ----------------------Tags--------------------- */
    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    const FGameplayTagContainer& GetActiveStateTags() const { return activeStateTags; }

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    void RebuildActiveStateTags();

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool HasActiveTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool HasExactActiveTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool IsInMovementTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool IsInExactMovementTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool IsInActionTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category="State Machine|Tags")
    bool IsInExactActionTag(const FGameplayTag& Tag) const;

    bool IsAirborne() const;
    bool IsGrounded() const;

    /* ---------------- State Changes ---------------- */
    void ChangeMovementState(UMovementState*, bool);
    void ChangeActionState(UActionState*, bool);
    void ClearActionState();

    void RequestAirborneMode(const FGameplayTag& StateTag);
    UFUNCTION() void ClearAirborneMode();

    void RequestGroundedMode(const FGameplayTag& StateTag);
    UFUNCTION() void ClearGroundedMode();

    /* ---------------- Queries ---------------- */
    ICombatCmdInterface*     GetCombatCommands() const;
    ILocomotionCmdInterface* GetLocomotionCommands() const;
    
    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }
    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    UActionState* GetActionStateByTag(const FGameplayTag& Tag) const;
    UMovementState* GetMovementStateByTag(const FGameplayTag& Tag) const;


    /* ---------------- Unified Requests ---------------- */
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestAttackPlayer(const FVector2D& InputVector, EPlayerAction PlayerAction);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestAttackEnemy(const FEnemyAtkData& AtkData);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestBlockStart();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestBlockStop();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestDodge(const FVector2D& InputVector = FVector2D::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestJumpStart();

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestJumpStop();
    
    void RequestLookMouse(const FVector2D& InputVector);
    void RequestLookStick(const FVector2D& InputVector);
    
    void RequestMove(const FVector2D& InputVector);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void RequestMoveTo(AActor* Target, const FVector Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f);

    void RequestToggleLockOn();

    /* ---------------- Animation / AnimInstance forwarding ---------------- */
    void OnAnimNotify(FGameplayTag NotifyTag);
    UFUNCTION() void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    
    /* -------------------- Combat Forwarding -----------------------*/
    void OnReceiveHit(const FAtkHitData& HitData);
    void OnCountered(AActor* Counteror, const FString& Reason);
};