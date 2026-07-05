#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

struct FAtkHitData;

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
	virtual void AttackDetected(AActor* Attacker) {} // Being targetted for an attack, but the attack hasn't hit yet
	virtual void Countered(AActor* Counteror,const FString& Reason) {} // Parry or Perfect Block has succeeded
	virtual bool IsAlive() const { return false; }
	virtual void ReceiveHit(FAtkHitData& HitData) {} // Attack has hit, doesn't necessarily mean you took damage
};
