#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "JumpState.generated.h"

/**
 * The character is jumping
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UJumpState : public UActionState
{
	GENERATED_BODY()

public:
    virtual void OnJumpApexReached_Implementation() override; // Movement feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};