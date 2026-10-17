#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "WalkingState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UWalkingState : public UMovementState
{
	GENERATED_BODY()

public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* CurrentState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;
};
