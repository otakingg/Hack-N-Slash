// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "DeathState.generated.h"

class UCombatResolutionComponent;
class UEnemyBrainComponent;

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDeathState : public UActionState
{
	GENERATED_BODY()

protected:
    UPROPERTY(Transient) UCombatResolutionComponent* combatResComp;
    UPROPERTY(Transient) UEnemyBrainComponent* enemyBrainComp = nullptr;

	void FaceDamageSource(AActor* Actor, FVector Location);

public:
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Reactions usually consume player combat intent
    virtual bool OnAttackIntent(const FVector2D& InputVector, EPlayerAction PlayerAction) override { return true; }
    virtual bool OnAttackIntent(const FEnemyAtkData& AtkData) override { return true;}
    virtual bool OnBlockStartIntent() override { return true; }
    virtual bool OnBlockStopIntent() override { return true; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector = FVector2D::ZeroVector) override { return true; }
    virtual bool OnLookMouseIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnLookStickIntent(const FVector2D& InputVector) { return false; }
    virtual bool OnToggleLockOnIntent() { return false; }

    // Locomotion intents are generally consumed in a hit state, but can always override
    virtual bool OnJumpStartIntent() override { return true; }
    virtual bool OnJumpStopIntent() override { return true; }
    virtual bool OnMoveIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return true; }

    // Movement feedback
    virtual void OnLanded(const FHitResult& Hit) override;

    // Animation feedback
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override;

    // Combat Feedback
    virtual void ReceiveHit(const FAtkHitData& HitData) override;	
};