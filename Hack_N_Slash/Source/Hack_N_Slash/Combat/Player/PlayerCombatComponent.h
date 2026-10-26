#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Enums/EChakraNature.h"
#include "../../Structs/FPlayerAtkData.h"
#include "GameFramework/RootMotionSource.h"
#include "PlayerCombatComponent.generated.h"

// This class handles all the player combat functionality
// Attacking, Blocking, Dodging, etc.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChakraUpdate, float, NewPercent, float, PrevPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChakraNatureChange, EChakraNature, NewNature);

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
	
	FTimerHandle TH_ActionsBlockedAtk; // Handles removing the "actionBlockedTagsAtk" tags
	FTimerHandle TH_BlockPerfect; // Timer handle for disabling perfect block window
	FTimerHandle TH_BlockRegenDelay; // After block breaks, will have to wait before the block starts regenerating
	FTimerHandle TH_BlockRegen; // Block hits will reduce back down to 0 over a period of time

	bool EnsureReferences();

    bool IsAtkContextValid(const FPlayerAtkData& AtkData, const FGameplayTag& PlayerAction, const FVector2D& Move) const; // Context-based attack selection
	bool HasHigherAtkPriority(FPlayerAtkData* CurrentChoice, FPlayerAtkData* EvaluatingChoice) const; // Does the attack we're looking at have > prioirty than the current choice
	FPlayerAtkData* GetPotentialAtkData(const FGameplayTag& ActionTag, const FVector2D& Move); // Searches the active attack data table for valid attacks
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Move); // Actually performs the attack (Plays the montage, sets the current attack data, etc.)

	void HandlePerfectBlock(FAtkHitData& HitData);
	UFUNCTION() void StartRegenBlockCount();
	UFUNCTION() void RegenBlockCount();

	UFUNCTION() void EndDodge(UAsyncRootMovement* RootMotion); // Handles functionality for when the dodge ends

	UFUNCTION() void HandleLanded(const FHitResult& Hit); // Handles functionality for when the player lands on the gorund

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

	/* -------------------- Chakra -----------------------*/
	UPROPERTY(VisibleAnywhere, Category = "Combat|Chakra")
	EChakraNature chakraNature = EChakraNature::None;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0))
	int32 chakraMax = 100;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0))
	int32 chakraMeter = 0;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0.01f, ClampMax = 1, ToolTip = "What chakra % will regen every 'regen rate' seconds when charging"))
	float chakraRegenPercent = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0.1f, ToolTip = "How often charging replenishes your chakra"))
	float chakraRegenRate = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Chakra")
	float chakraRegenTimeStamp = -1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "Percent of chakra regenerated on successfull attack hits"))
	float chakraRefundAtk = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "Percent of chakra regenerated on successfull perfect blocks"))
	float chakraRefundPerfectBlock = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Combat|Chakra")
	UAnimMontage* regenChakraMont = nullptr;

	/* -------------------- Rage -----------------------*/
	UPROPERTY(VisibleAnywhere, Category = "Combat|Rage")
	bool bRageMode = false;

	/* -------------------- Attack -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack", meta = (ToolTip = "Actions that attacking will temporarily block. Still add them via 'AddTags' notify state. This is just a quick safeguard"))
	TArray<FGameplayTag> actionBlockedTagsAtk;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Attack", meta = (ToolTip = "Has the player sucessfully waited before attacking again. Used for attacks that require a slight input delay. Will be opened by the anim notify system"))
	bool bAtkDelayWindow = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Attack")
	UDataTable* activeAtkDT = nullptr; // The data table to search for attack selection

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UDataTable* atkDTRage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UDataTable* atkDTRegular = nullptr;

	/* -------------------- Block -----------------------*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Block", meta = (ClampMin = 0, ToolTip = "Max # of hits the player can block before their guard is borken"))
	int32 maxBlockHits = 5;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block", meta = (ClampMin = 0, ToolTip = "Number of hits the player has blocked. Will dcrease over time"))
	int32 blockCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Block", meta = (ToolTip = "Can't block anymore. Will slowly reset after 'block regen delay' seconds"))
	bool bBlockBroken = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (ClampMin = 0.01, Tooltip = "How long after blocking a hit before it starts regenerating"))
	float blockRegenDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (ClampMin = 0.01, Tooltip = "Your current block count will reduce by 1 every 'this' seconds"))
	float blockRegenRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Block", meta = (ToolTip = "Is the perfect block window open?"))
	bool bPerfectBlockWindow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block", meta = (ClampMin = 0.01, ToolTip = "Duration of the perfect block window"))
	float perfectBlockWindow = 0.13f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (ToolTip = " This name will be searched for in the active attack data table. Will be executed when a perfect block occurs"))
	FName perfectBlockAtkDataID;

	UPROPERTY(EditAnywhere, Category = "Combat|Block", meta = (ToolTip = "Can the player block attacks that normally break armor"))
	bool bCanBlockArmorBreaker = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (ToolTip = "The montage that will play when blocking. Include a START, LOOP, & End sections in the montage"))
	UAnimMontage* activeBlockMontage = nullptr;

	/* -------------------- Dodge -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* airDodgeMont = nullptr; // The montage to play for dodging in the air

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMont = nullptr; // The montage to play for dodging on the ground

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge", meta = (ClampMin = 0, ToolTip = "Max # of dodges the player can do in the air"))
	int32 maxAirDodges = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Dodge", meta = (ClampMin = 0))
	int32 airDodgeCount = 0; // Number of dodges the player has done in the air

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = 0))
	float distance = 600.0f; // Dodge distance

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = 0))
	float duration = 0.2f; // Dodge duration

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Should the dodge be additive or override the character's existing velocity"))
    bool bIsAdditive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Behavior of the dodge velocity over time"))
    UCurveFloat* strengthOverTime = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides, ToolTip = "Velocity to set after movement finishes in SetVelocity mode. Ignored otherwise"))
    FVector setVelocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides, ClampMin = 0, ToolTip = "Clamp value to use after movement finishes in Clamp mode. Ignored otherwise"))
    float clampVelocityOnFinish = 0.0f;

	virtual void BeginPlay() override;
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (ToolTip = "Holds the move input of the last attack. Used by anim notifies for player targetting"))
	FVector2D move = FVector2D::ZeroVector;

	UPlayerCombatComponent();

	/* -------------------- Chakra -----------------------*/
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnChakraUpdate OnChakraUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnChakraNatureChange OnChakraNatureChange;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ChangeChakraNature(EChakraNature Nature);

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetMaxChakra() const { return chakraMax; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	int32 GetChakraMeter() const { return chakraMeter; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetChakraPercentage() const;

	void NotifyHitResult(const FAtkHitData& HitData);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetMaxChakra(int32 Num);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UpdateChakraMeterPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UpdateChakraMeterWhole(int32 Num);

	/* -------------------- Rage -----------------------*/
	void ToggleRageMode()
	{
		if (bRageMode) ExitRageMode();
		else EnterRageMode();
	}

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnterRageMode()
	{
		if (!atkDTRage) return;
		bRageMode = true;
		activeAtkDT = atkDTRage;
	}

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExitRageMode()
	{
		if (!atkDTRegular) return;
		bRageMode = false;
		activeAtkDT = atkDTRegular;
	}

	/* -------------------- Attack -----------------------*/
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearAtkData(); // Call this in the player Attack State's "Exit" function

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearTargetingData(); // Call this in the player Attack State's "Exit" function

	// The active data table will be searched if one is not provided
	FPlayerAtkData* GetAtkData(const FName& ID, const FString& Reason, UDataTable* DataTable = nullptr) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	FPlayerAtkData GetAtkData_BP(const FName& ID, const FString& Reason, UDataTable* DataTable = nullptr) const;

	void SetAtkDelayWindow(bool bflag) { bAtkDelayWindow = bflag; }

	/* -------------------- Block -----------------------*/
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClosePerfectBlockWindow();

	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* GetBlockMontage() const { return activeBlockMontage; }

	/* -------------------- Dodge -----------------------*/
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAnimMontage* GetCurrentDodgeMontage() const { return currentDodgeMont; }

	/* -------------------- Event Handling -----------------------*/
	void ReceieveHit(FAtkHitData& HitData); // Handles addtional functionality the player wants when processing a hit

	/* ----------------- Intents ---------------*/
	// NOTE: The "buffer" variable lets the system know if an action is being requested by the player directly or the input buffer system

	void Attack(const FGameplayTag& ActionTag, const FVector2D& Move, bool bBuffer = false);
	void BlockTrigger();
	void BlockStart();
	void BlockStop(); // Block Stop can't be buffered
	void Dodge(const FVector2D& Move = FVector2D::ZeroVector, bool bBuffer = false);
	void RegenChakraTrigger();
	void RegenChakraStop();
};