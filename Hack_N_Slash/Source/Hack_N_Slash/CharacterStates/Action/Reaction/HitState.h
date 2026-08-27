#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "HitState.generated.h"

class UCombatResolutionComponent;
class UEnemyBrainComponent;

/**
 * This state is for when a hit reaction is being played
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UHitState : public UActionState
{
	GENERATED_BODY()
	
protected:
    UPROPERTY(Transient, BlueprintReadOnly) UCombatResolutionComponent* combatResComp = nullptr;
    UPROPERTY(Transient, BlueprintReadOnly) UEnemyBrainComponent* enemyBrainComp = nullptr;

	void ApplyHitForce(const FAtkHitData& HitData);
    float CalculateHitAngle(const FAtkHitData& HitData) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    void FaceDamageSource(AActor* Actor, FVector Location);

    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void HandleBlockBreak(const FAtkHitData& HitData);
    virtual void HandleBlockBreak_Implementation(const FAtkHitData& HitData) {}

public:
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
