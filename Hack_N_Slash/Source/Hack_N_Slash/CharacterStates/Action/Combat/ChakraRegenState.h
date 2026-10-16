#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "ChakraRegenState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UChakraRegenState : public UActionState
{
	GENERATED_BODY()

public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* CurrentState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};