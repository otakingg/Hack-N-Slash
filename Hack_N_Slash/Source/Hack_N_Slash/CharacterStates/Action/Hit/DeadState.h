#pragma once

#include "CoreMinimal.h"
#include "HitState.h"
#include "DeadState.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UDeadState : public UHitState
{
	GENERATED_BODY()

private:
	int16 animationsPlayed = 0;
	UAnimMontage* montage = nullptr;

public:
    virtual void EnterState() override;
    virtual void ExitState() override;

    // Reactions should not be easily interrupted unless the incoming reaction is stronger
    virtual EStatePriority GetPriority() const override { return EStatePriority::Critical; }

    // Action Intents
    virtual bool OnToggleLockOnIntent() { return true; }
    virtual bool OnLookIntent(const FVector2D& InputVector) override { return true; }

    // Movement Feedback
    virtual void OnLanded(const FHitResult& Hit) override;

    // Animation Feedback (Action + some Movement like TurnInPlace may care)
    virtual void OnAnimNotify(FGameplayTag NotifyTag) override;
    
	/* ------------------ Combat Feedback ---------------------- */
	virtual void ReceiveHit(const FAtkHitData& HitData) override;
};
