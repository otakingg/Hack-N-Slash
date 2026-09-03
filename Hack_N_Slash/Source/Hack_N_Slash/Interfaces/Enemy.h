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
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
	void OnLockOn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
	void OnLockOff();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
	void OnSoftLockOn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
	void OnSoftLockOff();

	virtual bool HasSuperArmor() const { return false; }
};