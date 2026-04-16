// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Enemy.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemy : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API IEnemy
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Enemy")
	void OnLockOn();
	virtual void OnLockOn_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Enemy")
	void OnLockOff();
	virtual void OnLockOff_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Enemy")
	void OnSoftLockOn();
	virtual void OnSoftLockOn_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Enemy")
	void OnSoftLockOff();
	virtual void OnSoftLockOff_Implementation() {}
};