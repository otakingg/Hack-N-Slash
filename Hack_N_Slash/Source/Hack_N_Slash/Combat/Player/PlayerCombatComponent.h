// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FPlayerAtkData.h"
#include "PlayerCombatComponent.generated.h"

class ICharAnimInterface;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACK_N_SLASH_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	ICharAnimInterface* iCharAnimInst;
	ACharacter* ownerChar;
	UCharacterMovementComponent* moveComp;
	FPlayerAtkData* currentAtkData;

	bool EnsureOwnerCharacter();
	bool IsAtkContextValid(const FPlayerAtkData& AtkData, EPlayerAction PlayerAction, const FVector2D& InputVector) const;
	void PerformAttack(FPlayerAtkData* AtkData);

protected:
	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
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
};
