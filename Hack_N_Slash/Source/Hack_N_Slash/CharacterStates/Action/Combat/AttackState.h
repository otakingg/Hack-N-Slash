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

protected:
	bool bSetAirAtkStats = false;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Air Atk Move Stats")
	float airAtkGravity = 0.0f;

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override;

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector = FVector2D::ZeroVector) override;
};