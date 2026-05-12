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
struct FAtkHitData;

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
	virtual void BeginPlay() override;

public:
	UEnemyCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BlockStart();
	void BlockStop();

	void ReceieveHit(FAtkHitData& HitData);

	/* Combat Command Interface Functions*/
	//virtual void AttackIntent(const FVector2D& Dir, EPlayerAction PlayerAction) override;
	virtual void BlockStartIntent() override;
	virtual void BlockStopIntent() override;
};
