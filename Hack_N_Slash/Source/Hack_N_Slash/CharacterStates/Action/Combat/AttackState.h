#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "AttackState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UAttackState : public UActionState
{
	GENERATED_BODY()

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;
    
    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};