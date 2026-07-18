#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FPlayerAtkData.h"
#include "GameFramework/RootMotionSource.h"
#include "PlayerCombatComponent.generated.h"

class ICombatInstigator;
class UBaseCharAnimInstance;
class UCharacterMovementComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UPlayerInputComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBlock, const FAtkHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBlockBreak, const FAtkHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectBlock, const FAtkHitData&, HitData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY(Transient) UPlayerInputComponent* inputComp = nullptr;
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY(Transient) UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY(Transient) UCombatTraceComponent* traceComp = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;
	ICombatInstigator* iCmbtInst = nullptr;

	FPlayerAtkData* currentAtkData = nullptr;
	UAnimMontage* currentDodgeMont = nullptr;

	FTimerHandle TH_BlockRegenDelay; // After block breaks, will have to wait before the block starts regenerating
	FTimerHandle TH_BlockRegen; // Block hits will reduce back down to 0 over a period of time
	FTimerHandle TH_Dodge;

	bool EnsureReferences();

	void SnapToInputDirection(const FVector2D& InputDir);

    bool IsAtkContextValid(const FPlayerAtkData& AtkData, const FGameplayTag& CharacterAction, const FVector2D& Move) const;
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Move, bool bBuffer = false);
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION() void EndDodge(UAsyncRootMovement* RootMotion);

	UFUNCTION() void StartRegenBlockCount();
	UFUNCTION() void RegenBlockCount();

	UFUNCTION() void HandleLanded(const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

	/* -------------------- Attack -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UDataTable* activeAtkDT = nullptr;

	/* -------------------- Block -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block")
	UAnimMontage* activeBlockMontage = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block", meta = (ToolTip = "Which action is causing the block attempt? Example of this beiong useful: Open perfect block window on 'Block Start', but not 'Block Trigger'"))
	FGameplayTag blockActionInput;

	UPROPERTY(EditAnywhere, Category = "Combat|Block")
	bool bCanBlockSuperArmor = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block")
	int32 blockCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "How long after your block is broken before you can block again and it starts regenerating"))
	float blockRegenDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "Your current block count will reduce by 1 every 'this' seconds"))
	float blockRegenRate = 1.0f;

	/* -------------------- Dodge -----------------------*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* airDodgeMont = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontBack = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontFwd = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* groundDodgeMontRight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	int32 maxAirDodges = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	int32 airDodgeCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float distance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	float duration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Should the dodge be additive or override the character's existing velocity"))
    bool bIsAdditive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Behavior of the dodge velocity over time"))
    UCurveFloat* strengthOverTime = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Velocity to set after movement finishes in SetVelocity mode. Ignored otherwise"))
    FVector setVelocityOnFinish = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ToolTip = "Clamp value to use after movement finishes in Clamp mode. Ignored otherwise"))
    float clampVelocityOnFinish = 0.0f;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/* -------------------- Block -----------------------*/
	UPROPERTY(BlueprintAssignable)
	FOnPlayerBlock OnBlock;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerBlockBreak OnBlockBreak;
	
	UPROPERTY(BlueprintAssignable)
	FOnPerfectBlock OnPerfectBlock;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Block")
	int32 maxBlockHits = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	bool bBlockBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	bool bPerfectBlockUnlocked = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	bool bPerfectBlockWindow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	float perfectBlockWindow = 0.13f;

	UPlayerCombatComponent();

	/* -------------------- Attack -----------------------*/
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ClearAtkData() { currentAtkData = nullptr; }
	
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
	void ReceieveHit(FAtkHitData& HitData); // Handles blocking

	/* ----------------- Intents ---------------*/
	void Attack(const FGameplayTag& ActionTag, const FVector2D& Move, bool bBuffer = false);
	void BlockStart(bool bBuffer = false);
	void BlockHold(bool bBuffer = false);
	void BlockStop();
	void Dodge(const FVector2D& Move = FVector2D::ZeroVector, bool bBuffer = false);
};