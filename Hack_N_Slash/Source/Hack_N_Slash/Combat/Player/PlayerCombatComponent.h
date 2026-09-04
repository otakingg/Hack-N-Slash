#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FPlayerAtkData.h"
#include "GameFramework/RootMotionSource.h"
#include "PlayerCombatComponent.generated.h"

// This class handles all the player combat functionality
// Attacking, Blocking, Dodging, etc.

class ICombatInstigator;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UPlayerInputComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;
struct FAtkHitData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	ICombatInstigator* iCmbtInst = nullptr;
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY(Transient) UPlayerInputComponent* inputComp = nullptr;
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY(Transient) UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY(Transient) UCombatTraceComponent* traceComp = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

	FPlayerAtkData* currentAtkData = nullptr; // Stores the current attack data the player has
	UAnimMontage* currentDodgeMont = nullptr; // Stores the current dodge montage playing

	FTimerHandle TH_BlockRegenDelay; // After block breaks, will have to wait before the block starts regenerating
	FTimerHandle TH_BlockRegen; // Block hits will reduce back down to 0 over a period of time
	FTimerHandle TH_Dodge; // Timer handle for ending the dodge

	bool EnsureReferences();

	// Checks wether the provided attack data is valid (Can this attack happen)
	// Context-based attack selection
    bool IsAtkContextValid(const FPlayerAtkData& AtkData, const FGameplayTag& CharacterAction, const FVector2D& Move) const;
	FPlayerAtkData* GetPotentialAtkData(const FGameplayTag& ActionTag, const FVector2D& Move); // Searches the active attack data table for valid attacks
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Move); // Actually performs the attack (Plays the montage, sets the current attack data, etc.)
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted); // Handles functionality for when an attack finishes or gets interrupted

	UFUNCTION() void EndDodge(UAsyncRootMovement* RootMotion); // Handles functionality for when the dodge ends

	UFUNCTION() void StartRegenBlockCount();
	UFUNCTION() void RegenBlockCount();

	UFUNCTION() void HandleLanded(const FHitResult& Hit); // Handles functionality for when the player lands on the gorund

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

	/* -------------------- Attack -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UDataTable* activeAtkDT = nullptr; // The data table to search for attack selection

	/* -------------------- Block -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block")
	UAnimMontage* activeBlockMontage = nullptr; // The montage to play when blocking

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block", meta = (ToolTip = "Which action is causing the block attempt? Example of this being useful: Open perfect block window on 'Block Start', but not 'Block Trigger'"))
	FGameplayTag blockAction; // The action is causing blocking; Block Start? Block Held? 

	UPROPERTY(EditAnywhere, Category = "Combat|Block")
	bool bCanBlockArmorBreaker = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block")
	int32 blockCount = 0; // The number of blocked hits the system registered for the player. Will decrease over time

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "How long after your block is broken before you can block again and it starts regenerating"))
	float blockRegenDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "Your current block count will reduce by 1 every 'this' seconds"))
	float blockRegenRate = 1.0f;

	/* -------------------- Dodge -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* airDodgeMont = nullptr; // The montage to play for dodging in the air

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMont = nullptr; // The montage to play for dodging on the ground

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	int32 maxAirDodges = 1; // Number of dodges the player can do in the air

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	int32 airDodgeCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float distance = 600.0f; // Dodge distance

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float duration = 0.2f; // Dodge duration

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Should the dodge be additive or override the character's existing velocity"))
    bool bIsAdditive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Behavior of the dodge velocity over time"))
    UCurveFloat* strengthOverTime = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", ToolTip = "Velocity to set after movement finishes in SetVelocity mode. Ignored otherwise"))
    FVector setVelocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", ToolTip = "Clamp value to use after movement finishes in Clamp mode. Ignored otherwise"))
    float clampVelocityOnFinish = 0.0f;

	virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	FVector2D move = FVector2D::ZeroVector; // Holds the move input. Used by anim notifies for player targetting

	/* -------------------- Block -----------------------*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Block")
	int32 maxBlockHits = 5; // Maximum number of hits the player can recieve before their block is broken

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	bool bBlockBroken = false; // Can't block when your block is broken. Will reset after "blockRegenDelay" seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	bool bPerfectBlockUnlocked = true; // Is perfect block unlocked?

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	bool bPerfectBlockWindow = false; // Is the perfect block window open

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	float perfectBlockWindow = 0.13f; // Duration of the perfect block window

	UPlayerCombatComponent();

	/* -------------------- Attack -----------------------*/
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearAtkData();
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	FPlayerAtkData GetCurrentAtkDataStruct() const { return currentAtkData ? *currentAtkData : FPlayerAtkData::FPlayerAtkData(); }
	FPlayerAtkData* GetCurrentAtkData() const { return currentAtkData; }

	/* -------------------- Block -----------------------*/
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* GetBlockMontage() const { return activeBlockMontage; }
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanPerfectBlock() const;

	/* -------------------- Dodge -----------------------*/
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* GetCurrentDodgeMontage() const { return currentDodgeMont; }

	/* -------------------- Event Handling -----------------------*/
	void ReceieveHit(FAtkHitData& HitData); // Handles addtional functionality the player wants when processing a hit

	/* ----------------- Intents ---------------*/
	// NOTE: The "buffer" variable lets the system know if an action is being requested by the player directly or the input buffer system

	void Attack(const FGameplayTag& ActionTag, const FVector2D& Move, bool bBuffer = false);
	void BlockStart(bool bBuffer = false);
	void BlockHold(bool bBuffer = false);
	void BlockStop(); // Block Stop can't be buffered
	void Dodge(const FVector2D& Move = FVector2D::ZeroVector, bool bBuffer = false);
};