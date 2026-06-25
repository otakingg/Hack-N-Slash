// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "Player_Base.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHit, const FAtkHitData&, HitData);

class UCombatResolutionComponent;
class UCombatTraceComponent;
class ULocomotionComponent;
class UPlayerCamComponent;
class UPlayerCombatCancelComponent;
class UPlayerCombatComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter, public ICombatInstigator, public IDamageable
{
	GENERATED_BODY()

private:
	FTimerHandle TH_Input_Move;

protected:
	UPROPERTY(EditAnywhere, Category = "Player")
	bool bDebug = false;

	//UPROPERTY(EditAnywhere, Category = "Player|Input", meta = (Tooltip = "The time after recieving an input that the system will wait before executing logic"))
	//float inputRegisterTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Player|Input", meta = (Tooltip = "How long a button has to be held before considered being held by the system"))
	float inputHeldThreshold = 0.1f;

	UPROPERTY(VisibleAnywhere, Category = "Player|Input", meta = (ToolTip = "When was the input started"))
	float heavyStartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Player|Input", meta = (ToolTip = "When was the input started"))
	float lightStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ULocomotionComponent* locoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCamComponent* playerCamComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatCancelComponent* playerCombatCancelComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatComponent* combatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerTargettingComponent* playerTargettingComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerHit OnHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (ToolTip = "Is heavy input held"))
	bool bHeavyHeld = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkHeavy = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (ToolTip = "Is heavy input held"))
	bool bLightHeld = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (ToolTip = "How long this input has been held"))
	float heldTimeAtkLight = 0.0f;

	APlayer_Base();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Player Input")
	void PlayerInput(EPlayerInput PlayerInput, const FVector2D& InputVector);

	void HandleActorDeath(AActor* Actor);

	/* Combat Instigator Interface Functions*/
	virtual AActor* GetCurrentTarget() const override;
	virtual bool GetLockedOn() const override;
	
	/* Damageable Interface Functions*/
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
