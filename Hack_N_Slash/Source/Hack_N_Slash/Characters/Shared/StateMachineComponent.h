#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../CharacterStates/Core/CharacterState.h"
#include "StateMachineComponent.generated.h"

class ULocomotionComponent;
struct FAtkHitData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

    void InitializeMovementMap();
    void InitializeActionMap();

    void DecideMovementState(bool bForce);
    static bool CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce);

    UFUNCTION() void HandleJumpApexReached();
    UFUNCTION() void HandleLanded(const FHitResult& Hit);
    UFUNCTION() void HandleMovementModeChanged(ACharacter* InCharacter, EMovementMode PrevMovementMode, uint8 PrevCustomMode);

protected:
    UPROPERTY(EditAnywhere, Category = "State Machine")
    bool bDebug = false;

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

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type) override;

public:
    UStateMachineComponent();

    /* ---------------- State Changes ---------------- */
    bool ChangeMovementState(UMovementState* NewState, bool bForce);
    bool ChangeActionState(UActionState* NewState, bool bForce);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ClearActionState();

    /* ---------------- Queries ---------------- */
    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }
    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    UActionState* GetActionStateByTag(const FGameplayTag& Tag) const;
    UMovementState* GetMovementStateByTag(const FGameplayTag& Tag) const;

    /* ---------------- Animation Forwarding ---------------- */
    void HandleAnimNotify(FGameplayTag NotifyTag);
    
    /* -------------------- Combat Forwarding -----------------------*/
    void HandleReceiveHit(const FAtkHitData& HitData);
    void HandleCountered(AActor* Counteror, const FString& Reason);

    /* --------------------- PLayer Input Handling ----------------- */
    // Choose an action to do based on the button input and player movement state
    FGameplayTag ResolvePlayerInput(EPlayerInput PlayerInput = EPlayerInput::None, const FVector2D& InputVector = FVector2D::ZeroVector);
};