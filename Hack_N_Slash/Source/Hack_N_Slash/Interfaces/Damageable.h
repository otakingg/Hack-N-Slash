// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../Structs/FAtkHitData.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API IDamageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsAlive() const { return false; }
	virtual void AttackDetected() {} // Being targetted for an attack, but the attack hasn't hit yet

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable", meta = (Tooltip = "Attack has hit, doesn't necessarily mean you took damage"))
	void ReceiveHit(UPARAM(ref) FAtkHitData& HitData);
};
