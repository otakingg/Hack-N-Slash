// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Enemy.h"
#include "../../Interfaces/Targetable.h"
#include "EnemyBase.generated.h"

class APlayer_Base;
class UCapsuleComponent;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UCharacterMovementComponent;
class UEnemyBrainComponent;
class ULocomotionComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API AEnemyBase : public ACharacter, public ICombatInstigator, public IDamageable, public IEnemy, public ITargetable
{
	GENERATED_BODY()

private:
	UPROPERTY() UCapsuleComponent* capsuleComp;
	UPROPERTY() UCharacterMovementComponent* moveComp;
	UPROPERTY() APlayer_Base* player = nullptr;
	void PlayFlinchAnim(const FAtkHitData& HitData);

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
	ULocomotionComponent* locoComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStatsComponent* statsComp;

	virtual void BeginPlay() override; // Called when the game starts or when spawned

public:
	AEnemyBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Called to bind functionality to input

	/* Combat Instigator Interface Functions*/
	virtual int GetPowerLevel() const override;
	virtual int GetPowerLevelMax() const override;

	/* Damageable Interface Functions*/
	virtual bool IsAlive() const override;
	virtual void ReceiveHit(FAtkHitData& HitData) override;
};
