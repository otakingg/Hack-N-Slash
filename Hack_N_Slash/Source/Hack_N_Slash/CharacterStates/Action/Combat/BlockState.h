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

protected:
    void FaceDamageSource(AActor* Actor, FVector Location);

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) override; // Combat Feedback
};