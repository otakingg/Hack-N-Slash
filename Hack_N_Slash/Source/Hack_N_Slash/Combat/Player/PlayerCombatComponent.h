// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/CombatCmdInterface.h"
#include "../../Structs/FPlayerAtkData.h"
#include "PlayerCombatComponent.generated.h"

class ICharAnimInterface;
class UCharacterMovementComponent;
class UCombatTraceComponent;
class UPlayerTargettingComponent;
class UStateMachineComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent, public ICombatCmdInterface
{
	GENERATED_BODY()

private:
	ICharAnimInterface* iCharAnimInst = nullptr;
	UPROPERTY() ACharacter* ownerChar = nullptr;
	UPROPERTY() UCharacterMovementComponent* moveComp = nullptr;
	UPROPERTY() UPlayerTargettingComponent* playerTargettingComp = nullptr;
	UPROPERTY() UStateMachineComponent* stateMachineComp = nullptr;
	UPROPERTY() UCombatTraceComponent* traceComp = nullptr;
	FPlayerAtkData* currentAtkData = nullptr;

	bool Ensurereferences();
    bool IsAtkContextValid(const FPlayerAtkData &AtkData, EPlayerAction PlayerAction, const FVector2D &InputVector) const;
    void PerformAttack(FPlayerAtkData* AtkData, const FVector2D& Dir);
	UFUNCTION() void OnAttackMontageEnded(UAnimMontage* montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	
	UPROPERTY(EditDefaultsOnly)
	UDataTable* activeAtkDT = nullptr;

	virtual void BeginPlay() override;

public:
	UPlayerCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//void BlockStart();
	//void BlockStop();
	void AttackHeavyStart(const FVector2D& InputVector);
	void AttackLightStart(const FVector2D& InputVector);

	void ClearAtkData();
	FPlayerAtkData* GetCurrentAtkData() const;

	/* Combat Command Interface Functions*/
	virtual void AttackIntent(const FVector2D& Dir, EPlayerAction PlayerAction) override;
};
