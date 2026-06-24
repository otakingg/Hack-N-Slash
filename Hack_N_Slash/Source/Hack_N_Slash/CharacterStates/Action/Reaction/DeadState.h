#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "DeadState.generated.h"

class AEnemyController;
class UBaseCharAnimInstance;
class UCombatResolutionComponent;
class UEnemyBrainComponent;
class ULocomotionComponent;

/**
 * The character has died
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDeadState : public UActionState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
    UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
    UPROPERTY(Transient) UEnemyBrainComponent* enemyBrainComp = nullptr;
	UPROPERTY(Transient) AEnemyController* enemyController = nullptr;
	UPROPERTY(Transient) ULocomotionComponent* locoComp = nullptr;

	void ApplyHitForce(const FAtkHitData& HitData);
    void FaceDamageSource(AActor* Actor, FVector Location);

public:
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState() override;
    virtual void ExitState() override;

    virtual void OnLanded(const FHitResult& Hit) override; // Movement feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override; // Animation Feedback
    virtual void ReceiveHit(const FAtkHitData& HitData) override; // Combat Feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector = FVector2D::ZeroVector) override;
};
