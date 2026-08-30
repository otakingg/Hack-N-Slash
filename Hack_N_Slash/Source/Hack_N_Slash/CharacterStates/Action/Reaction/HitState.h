#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "HitState.generated.h"

class UCombatResolutionComponent;
class UEnemyBrainComponent;

USTRUCT(BlueprintType)
struct FGroundBounceData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) AActor* damager = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector damagerLoc = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FVector bounceLocOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float bounceSpeed = 1000.0f;
};

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
    UPROPERTY(VisibleAnywhere) FGroundBounceData groundBounceData;

	void ApplyHitForce(const FAtkHitData& HitData);
    float CalculateHitAngle(const FAtkHitData& HitData) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    void FaceDamageSource(AActor* Actor, FVector Location);

    bool CanBounceGround() const;
    void BounceGround();

    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void HandleBlockBreak(const FAtkHitData& HitData);
    virtual void HandleBlockBreak_Implementation(const FAtkHitData& HitData) {}

public:
    /* ---------------- Lifecycle ---------------- */
    virtual void Initialize_Implementation(UStateMachineComponent* InSM, ACharacter* InOwner) override;
    virtual void EnterState_Implementation() override;
    virtual void ExitState_Implementation() override;

    // Movement feedback
    virtual void OnJumpApexReached_Implementation() override;
    virtual void OnLanded(const FHitResult& Hit) override;

    virtual void OnAnimNotify_Implementation(FGameplayTag NotifyTag) override; // Animation Feedback
    virtual void ReceiveHit_Implementation(const FAtkHitData& HitData) override; // Combat Feedback

    // Player only: Action Management
    virtual FGameplayTag ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction) override;
};
