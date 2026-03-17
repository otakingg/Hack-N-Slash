// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CharAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACK_N_SLASH_API ICharAnimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float PlayMontageHNS(UAnimMontage* Montage = nullptr, float PlayRate = 1.0f, FName Section = NAME_None) { return 0.0f; }
};