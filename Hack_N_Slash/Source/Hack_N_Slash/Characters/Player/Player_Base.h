// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/PlayerInt.h"
#include "../../Interfaces/Targetable.h"
#include "Player_Base.generated.h"

class UCameraComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UCharacterMovementComponent;
class UPlayerCamComponent;
class UPlayerCombatCancelComponent;
class UPlayerCombatComponent;
class UPlayerLocomotionComponent;
class UPlayerTargettingComponent;
class USpringArmComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API APlayer_Base : public ACharacter, public IPlayerInt, public ICombatInstigator, public IDamageable, public ITargetable
{
	GENERATED_BODY()

private:
	UPROPERTY() UCameraComponent* camComp;
	UPROPERTY() UCharacterMovementComponent* moveComp;
	UPROPERTY() USpringArmComponent* springArmComp;

	void PlayFlinchAnim(const FAtkHitData& HitData);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCamComponent* playerCamComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatCancelComponent* playerCombatCancelComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerCombatComponent* playerCombatComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerLocomotionComponent* playerLocoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerTargettingComponent* playerTargettingComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;
	
	virtual void BeginPlay() override;

public:
	APlayer_Base();
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Inputs */
	UFUNCTION(BlueprintCallable)
    void Input_Started_AttackHeavy(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Started_AttackLight(const FVector2D& InputVector);
	
	UFUNCTION(BlueprintCallable)
    void Input_Started_BlockDodge(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Released_BlockDodge();

	UFUNCTION(BlueprintCallable)
    void Input_Started_Jump();

	UFUNCTION(BlueprintCallable)
    void Input_Released_Jump();

	UFUNCTION(BlueprintCallable)
    void Input_Triggered_Look_Mouse(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Triggered_Look_Stick(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Started_Move(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Triggered_Move(const FVector2D& InputVector);

	UFUNCTION(BlueprintCallable)
    void Input_Started_ToggleLockOn();

	/* Combat Instigator Interface Functions*/
	virtual AActor* GetCurrentTarget() const override;
	virtual int GetPowerLevel() const override;
	virtual int GetPowerLevelMax() const override;

	/* Damageable Interface Functions*/
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
