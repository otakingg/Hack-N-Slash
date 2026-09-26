#pragma once

#include "CoreMinimal.h"
#include "CharacterStates/Core/CharacterState.h"
#include "GameFramework/RootMotionSource.h"
#include "HitState.generated.h"

class UCombatResolutionComponent;
class UEnemyBrainComponent;

USTRUCT(BlueprintType)
struct FGroundBounceData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) AActor* damager = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector damagerLoc = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FRotator damagerRot = FRotator::ZeroRotator;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float extraBounceHeight = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bAdditive = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) UCurveFloat* strengthOverTime = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float clampVelocityOnFinish = 0;

    void Reset()
    {
        damager = nullptr;
        damagerLoc = FVector::ZeroVector;
        damagerRot = FRotator::ZeroRotator;
        extraBounceHeight = 0.0f;
        bAdditive = false;
        strengthOverTime = nullptr;
        clampVelocityOnFinish = 0;
    }
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGroundBounceData groundBounceData;

	void ApplyHitForce(const FAtkHitData& HitData);
    //float CalculateHitAngle(const FAtkHitData& HitData) const;

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
