// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "EnemyCombatComponent.generated.h"

class ICharAnimInterface;
class UCombatResolutionComponent;
class UCombatTraceComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UEnemyCombatComponent : public UActorComponent, public ICombatCmdInterface
{
	GENERATED_BODY()

private:
	ICharAnimInterface* iCharAnimInst = nullptr;
	UPROPERTY() ACharacter* ownerChar = nullptr;
	UPROPERTY() UCombatResolutionComponent* combatResComp = nullptr;
	UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY() UCombatTraceComponent* traceComp = nullptr;

	bool EnsureReferences();

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bDebug = false;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UEnemyCombatComponent();

    UFUNCTION(BlueprintNativeEvent, Category = "Enemy Combat")
	void ReceieveHit(FAtkHitData& HitData);
	void ReceieveHit_Implementation(FAtkHitData& HitData) {}

	/* Combat Command Interface Functions*/
	virtual void AttackIntent(const FEnemyAtkData& AtkData) override;
	virtual void BlockStartIntent() override;
	virtual void BlockStopIntent() override;
};