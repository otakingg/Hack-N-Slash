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
class UEnemyBrainComponent;
class UEnemyCombatComponent;
class ULocomotionComponent;
class UStateMachineComponent;
class UStatsComponent;

UCLASS()
class HACK_N_SLASH_API AEnemyBase : public ACharacter, public ICombatInstigator, public IDamageable, public IEnemy, public ITargetable
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	APlayer_Base* player = nullptr;

protected:
	UPROPERTY(EditAnywhere, Category = "Enemy")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyBrainComponent* brainComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UEnemyCombatComponent* combatComp;

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
	virtual void Tick(float DeltaTime) override;

public:
	AEnemyBase();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // Called to bind functionality to input

	/* Combat Instigator Interface Functions*/
	virtual AActor* GetCurrentTarget() const override;
	virtual int GetPoise() const override;

	/* Damageable Interface Functions*/
	virtual bool IsAlive() const override;
	virtual void ReceiveHit_Implementation(FAtkHitData& HitData) override;
};
