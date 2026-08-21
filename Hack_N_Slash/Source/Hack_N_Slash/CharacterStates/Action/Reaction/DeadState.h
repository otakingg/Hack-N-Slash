#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "DeadState.generated.h"

class UCombatResolutionComponent;
class UEnemyBrainComponent;

/**
 * The character has died
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDeadState : public UActionState
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
    UPROPERTY(Transient) UEnemyBrainComponent* enemyBrainComp = nullptr;

	void ApplyHitForce(const FAtkHitData& HitData);
    void FaceDamageSource(AActor* Actor, FVector Location);

public:
    /* ---------------- Transition Rules ---------------- */
    virtual bool CanEnterState_Implementation(const UCharacterState* CurrentState) const override;

    /* ---------------- Lifecycle ---------------- */
    virtual void Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    virtual void OnLanded(const FHitResult& Hit) override; // Movement feedback

    virtual void OnAnimNotify_Implementation(FGameplayTag NotifyTag) override; // Animation Feedback
    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) override; // Combat Feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};
