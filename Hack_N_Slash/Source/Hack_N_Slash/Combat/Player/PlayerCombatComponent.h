// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Structs/FPlayerAtkData.h"
#include "GameFramework/RootMotionSource.h"
#include "PlayerCombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerfectBlock);

class ICharAnimInterface;
class UCharacterMovementComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;
struct FAtkHitData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent, public ICombatCmdInterface
{
	GENERATED_BODY()

private:
	ICharAnimInterface* iCharAnimInst = nullptr;
	UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY(Transient) ACharacter* ownerChar = nullptr;
	UPROPERTY(Transient) UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY(Transient) UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY(Transient) UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY(Transient) UCombatTraceComponent* traceComp = nullptr;
	FPlayerAtkData* currentAtkData = nullptr;

	FTimerHandle TH_BlockRegenDelay; // After block breaks, will have to wait before the block starts regenerating
	FTimerHandle TH_BlockRegen; // Block hits will reduce back down to 0 over a period of time
	FTimerHandle TH_Dodge;

	bool EnsureReferences();

	FVector GetInputWorldDirRelativeToCamOrTarget(const FVector2D& InputVector, FVector& OutLocalForward, FVector& OutLocalRight, AActor* Target = nullptr) const;
    EStickMotion GetStickMotionFromWorldDir(const FVector& WorldDir, const FVector& LocalForward, const FVector& LocalRight) const;
    EStickMotion GetWorldDirRelativeToPlayerFacing(const FVector& WorldDir) const;

	void SnapToInputDirection(const FVector2D& InputDir);

    bool IsAtkContextValid(const FPlayerAtkData &AtkData, EPlayerAction PlayerAction, const FVector2D &InputVector) const;
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Dir);
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION() void EndDodge();

	UFUNCTION() void StartRegenBlockCount();
	UFUNCTION() void RegenBlockCount();

	UFUNCTION() void HandleLanded(const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Attack")
	bool bHasAirAttacked = false;

	UPROPERTY(EditAnywhere, Category = "Combat|Attack")
	bool bCanAirAtk = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UDataTable* activeAtkDT = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat|Block")
	bool bCanBlockSuperArmor = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block")
	int16 maxBlockHits = 5;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block")
	int16 blockCount = 0;

	UPROPERTY(EditAnywhere, Category = "Combat|Block")
	bool bBlockBroken = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "How long after your block is broken before you can block again and it starts regenerating"))
	float blockRegenDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Block", meta = (Tooltip = "Your current block count will reduce by 1 every 'this' seconds"))
	float blockRegenRate = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Block")
	bool bPerfectBlockWindow = false;

	UPROPERTY(EditAnywhere, Category = "Combat|Block")
	float perfectBlockWindow = 0.13f;

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

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
	int16 maxAirDodges = 1;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Dodge")
	int16 airDodgeCount = 0;

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
	UPROPERTY(BlueprintAssignable)
	FOnPerfectBlock OnPerfectBlock;

	UPlayerCombatComponent();

	void AttackHeavyStart(const FVector2D& InputVector);
	void AttackLightStart(const FVector2D& InputVector);

	void ClearAtkData();
	FPlayerAtkData* GetCurrentAtkData() const;
	bool GetHasAirAttacked() const { return bHasAirAttacked; }
	void SetCanAirAtk(bool bCanAirAttack)  { bCanAirAtk = bCanAirAttack; }

	void SetMaxBlockHits(int16 MaxBlockHits) { maxBlockHits = MaxBlockHits; }
	void SetPerfectBlockWindow(bool bOpen) { bPerfectBlockWindow = bOpen; }
	float GetPerfectBlockWindowTime() const { return perfectBlockWindow; }

	void ReceieveHit(FAtkHitData& HitData); // Handles blocking

	/* Combat Command Interface Functions*/
	virtual void AttackIntent(const FVector2D& Dir, EPlayerAction PlayerAction) override;
	virtual void BlockStartIntent() override;
	virtual void BlockStopIntent() override;
	virtual void DodgeIntent(const FVector2D& Dir = FVector2D::ZeroVector) override;
};
