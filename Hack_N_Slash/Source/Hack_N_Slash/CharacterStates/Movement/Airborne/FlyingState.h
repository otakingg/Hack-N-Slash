#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "FlyingState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UFlyingState : public UMovementState
{
	GENERATED_BODY()
	
public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* PreviousState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
};