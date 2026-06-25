#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "JumpState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UJumpState : public UActionState
{
	GENERATED_BODY()

public:
    virtual void OnJumpApexReached() override; // Movement feedback
};