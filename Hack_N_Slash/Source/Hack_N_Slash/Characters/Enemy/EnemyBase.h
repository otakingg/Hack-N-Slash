// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Targetable.h"
#include "EnemyBase.generated.h"

UCLASS()
class HACK_N_SLASH_API AEnemyBase : public ACharacter, public ICombatInstigator, public IDamageable, public ITargetable
{
	GENERATED_BODY()

private:
	class UCapsuleComponent* capsuleComp;
	//class AEnemyCrowdAIController* controller;
	class UCharacterMovementComponent* moveComp;

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UEnemyBrainComponent* brainComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStateMachineComponent* stateMachineComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStatsComponent* statsComp;

	virtual void BeginPlay() override;

public:
	AEnemyBase();
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
