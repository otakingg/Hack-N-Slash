// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/States/Core/CharacterState.h"
#include "../../Structs/FAtkHitData.h"
#include "HitState.generated.h"

/**
 * Action-Hit base:
 * - External force states (stagger, knockdown, getup, death,, etc.)
 * - Usually higher priority and more restrictive about being interrupted
 */
UCLASS(Abstract)
class HACK_N_SLASH_API UHitState : public UActionState
{
    GENERATED_BODY()

protected:
    FTimerHandle TH_Juggle;
    class UCombatResolutionComponent* combatResComp;

    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"), Category = Hit)
    float gravityRestoreDelay = 1.0f;

    UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"), Category = Hit)
    float juggleGravity = 0.2f;

    float CalculateHitAngle(const FAtkHitData& HitData) const;
    void FaceDamageSource(AActor* Actor, FVector Location);
    void EnterJuggle();
    UFUNCTION() void ExitJuggle();

public:
    virtual void Initialize(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Reactions should not be easily interrupted unless the incoming reaction is stronger
    virtual EStatePriority GetPriority() const override { return EStatePriority::High; }

    // Reactions usually consume player combat intent
    // Movement still can run unless you make a specific reaction consume it
    virtual bool OnAttackIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnBlockStartIntent() override { return true; }
    virtual bool OnBlockStopIntent() override { return true; }
    virtual bool OnDodgeIntent(const FVector2D& InputVector) override { return true; }

    // Locomotion intents are generally consumed in a hit state, but can always override
    virtual bool OnJumpPressed() override { return true; }
    virtual bool OnJumpReleased() override { return true; }
    virtual bool OnLookIntent(const FVector2D& InputVector) override { return false; }
    virtual bool OnMoveIntent(const FVector2D& InputVector) override { return true; }
    virtual bool OnMoveIntent(AActor* Target, const FVector& Loc = FVector::ZeroVector, float AcceptanceRadius = 50.0f) override { return true; }

    // Combat Feedback
    virtual void ReceiveHit(const struct FAtkHitData& HitData) override;
};