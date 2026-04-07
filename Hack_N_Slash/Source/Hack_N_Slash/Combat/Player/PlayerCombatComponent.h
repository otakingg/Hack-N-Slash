// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Structs/FAtkData.h"
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
	FAtkData* currentAtkData;

	bool EnsureOwnerCharacter();
	bool IsAtkContextValid(const FAtkData& AtkData, EPlayerAction PlayerAction, const FVector2D& InputVector) const;
	void PerformAttack(FAtkData* AtkData);

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
	FAtkData* GetCurrentAtkData() const;
};
