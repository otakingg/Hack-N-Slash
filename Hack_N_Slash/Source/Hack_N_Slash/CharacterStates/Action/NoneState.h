#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "NoneState.generated.h"

/**
 * Neutral Action State
 * Any action is permitted in this state
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UNoneState : public UActionState
{
	GENERATED_BODY()	
};