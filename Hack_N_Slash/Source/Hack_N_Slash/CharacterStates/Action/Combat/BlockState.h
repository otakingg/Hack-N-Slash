#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "BlockState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBlockState : public UActionState
{
	GENERATED_BODY()

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;
};
