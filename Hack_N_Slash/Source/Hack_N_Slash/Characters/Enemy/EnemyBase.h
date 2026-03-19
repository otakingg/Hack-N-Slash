// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Targetable.h"
#include "EnemyBase.generated.h"

class UCapsuleComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UCharacterMovementComponent;
class UEnemyBrainComponent;
class UEnemyLocomotionComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API AEnemyBase : public ACharacter, public ICombatInstigator, public IDamageable, public ITargetable
{
	GENERATED_BODY()

private:
	UCapsuleComponent* capsuleComp;
	//class AEnemyCrowdAIController* controller;
	UCharacterMovementComponent* moveComp;
	void PlayFlinchAnim(FVector Direction);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyBrainComponent* brainComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatResolutionComponent* combatResComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCombatTraceComponent* combatTraceComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyLocomotionComponent* enemyLocomotionComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;

	UPROPERTY(EditAnywhere, meta = (ClampMin="0", ClampMax = "2"))
	int powerLevel = 0;

	virtual void BeginPlay() override; // Called when the game starts or when spawned

public:
	AEnemyBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Called to bind functionality to input

	/* Combat Instigator Interface Functions*/
	virtual int GetPowerLevel() const override {return powerLevel;}

	/* Damageable Interface Functions*/
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
