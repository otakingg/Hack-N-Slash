#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "BlockState.generated.h"

/**
 * The character is blocking
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UBlockState : public UActionState
{
	GENERATED_BODY()

protected:
    UFUNCTION(BlueprintCallable, Category = "State")
    void FaceDamageSource(AActor* Actor, FVector Location);

public:
    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) override; // Combat Feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};