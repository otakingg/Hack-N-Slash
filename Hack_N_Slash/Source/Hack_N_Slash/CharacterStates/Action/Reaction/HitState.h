#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "HitState.generated.h"

class AEnemyController;
class UBaseCharAnimInstance;
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
	UPROPERTY(Transient) UBaseCharAnimInstance* animInst = nullptr;
    UPROPERTY(Transient) UCombatResolutionComponent* combatResComp = nullptr;
    UPROPERTY(Transient) UEnemyBrainComponent* enemyBrainComp = nullptr;
	UPROPERTY(Transient) AEnemyController* enemyController = nullptr;
    
    FTimerHandle TH_Juggle;

    UPROPERTY(EditAnywhere, Category = Hit, meta = (ClampMin = "0.0"))
    float gravityRestoreDelay = 1.0f;

    UPROPERTY(EditAnywhere, Category = Hit, meta = (ClampMin = "0.0"))
    float juggleGravity = 0.2f;

	void ApplyHitForce(const FAtkHitData& HitData);
    float CalculateHitAngle(const FAtkHitData& HitData) const;
    void FaceDamageSource(AActor* Actor, FVector Location);
    void EnterJuggle();
    UFUNCTION() void ExitJuggle();

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    virtual void OnLanded(const FHitResult& Hit) override; // Movement feedback
    virtual void OnAnimNotify_Implementation(FGameplayTag NotifyTag) override; // Animation Feedback
    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) override; // Combat Feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction, const FVector2D& InputVector = FVector2D::ZeroVector) override;
};
