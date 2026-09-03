#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../CharacterStates/Core/CharacterState.h"
#include "StateMachineComponent.generated.h"

// Handles all the state tranition logic for characters in this game
// Handles player input interperetation

/*********States*********/
// States in this project are represented as UObjects
// This allows each state to easily define what player input mean and character actions are allowed
// It also allows each state to define when it can be entered
// States are divided into layers with the 2 top layers being "Action" & "Movement"
// Action is further divided into "Combat" & "Reaction"
// Combat examples: Attack, Block, dodge, etc.
// Reaction examples: Hit, Dead, etc.

class ULocomotionComponent;
struct FAtkHitData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HACK_N_SLASH_API UStateMachineComponent : public UActorComponent
{
    GENERATED_BODY()

private:
    UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
    UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

    void InitializeActionMap(); // Initialize all the aciton states
    void InitializeMovementMap(); // Initialize all the movement states

    void DecideMovementState(bool bForce); // Decides which movement state to enter
    static bool CanTransition(const UCharacterState* Current, const UCharacterState* Next, bool bForce); // Can this state be entered?

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

    UPROPERTY(EditDefaultsOnly, Category="State Machine|Action")
    TArray<TSubclassOf<UActionState>> actionStateClasses; // All the action classes to instantiate for this actor

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UActionState>> actionStateInstances; // Stores the instantiated action classes for this actor
    
    /** State classes */
    UPROPERTY(EditDefaultsOnly, Category="State Machine|Movement")
    TArray<TSubclassOf<UMovementState>> movementStateClasses; // All the movement classes to instantiate for this actor

    UPROPERTY(Transient)
    TMap<TObjectPtr<UClass>, TObjectPtr<UMovementState>> movementStateInstances; // Stores the instantiated movement classes for this actor

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type) override;

public:
    UStateMachineComponent();

    /* ---------------- State Changes ---------------- */
    bool ChangeMovementState(UMovementState* NewState, bool bForce);
    bool ChangeActionState(UActionState* NewState, bool bForce);

    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void ClearActionState(); // Resets the action state to the default action state (None)

    /* ---------------- Queries ---------------- */
    UMovementState* GetCurrentMovementState() const { return currentMovementState; }
    UMovementState* GetPreviousMovementState() const { return previousMovementState; }
    UActionState* GetCurrentActionState() const { return currentActionState; }
    UActionState* GetPreviousActionState() const { return previousActionState; }

    UActionState* GetActionStateByTag(const FGameplayTag& Tag) const;
    UMovementState* GetMovementStateByTag(const FGameplayTag& Tag) const;

    bool IsAirborne() const { return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tags::StateMachine::Movement::Airborne); }
    bool IsGrounded() const { return currentMovementState && currentMovementState->GetStateTag().MatchesTag(Tags::StateMachine::Movement::Grounded); }

    /* ---------------- Animation Forwarding ---------------- */
    void HandleAnimNotify(FGameplayTag NotifyTag);
    
    /* -------------------- Combat Forwarding -----------------------*/
    void HandleReceiveHit(const FAtkHitData& HitData);
    void HandleCountered(AActor* Counteror, const FString& Reason);

    /* --------------------- PLayer Input Handling ----------------- */
    // Choose an action to do based on the button input and player movement state
    FGameplayTag ResolvePlayerInput(EPlayerInput PlayerInput = EPlayerInput::None, const FVector2D& LookVector = FVector2D::ZeroVector, const FVector2D& MoveVector = FVector2D::ZeroVector);
};