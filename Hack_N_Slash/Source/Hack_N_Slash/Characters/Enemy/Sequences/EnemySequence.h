#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemySequence.generated.h"

class UEnemyBrainComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemySequence : public UObject
{
    GENERATED_BODY()

private:
	UPROPERTY(Transient)
    UEnemyBrainComponent* brain = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
	bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Description")
    FName sequenceName;

	UPROPERTY(EditDefaultsOnly, Category = "Sequence|Description", meta = (MultiLine = "true"))
	FText description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ClampMin = "0.0"))
    float baseScore = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ClampMin = "0.0", Tooltip = "Higher weight = want high aggro"))
    float aggroWeight = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence", meta = (ClampMin = "1"))
	int sequenceIndex = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown", meta = (ClampMin = "0.0"))
    float cooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	FTimerHandle TH_Cooldown;
	
	UFUNCTION(BlueprintCallable)
	void EndCooldown() { bOnCooldown = false; }

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence")
	bool bInterruptible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	bool bOnCooldown = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown")
	bool bStartOnCooldown = false;

	void Initialize(UEnemyBrainComponent* InBrain);

	UFUNCTION(BlueprintPure, Category = "Sequence")
	UEnemyBrainComponent* GetBrain() const { return brain; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore() const;
	virtual float GetScore_Implementation() const;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Execute();
    virtual void Execute_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AdvanceSequence();
	virtual void AdvanceSequence_Implementation() { ++sequenceIndex; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Finish();
	virtual void Finish_Implementation();

    /** Event Handlers */
    // Override in BP
	UFUNCTION(BlueprintNativeEvent)
    void HandleSensedSight(AActor* Seen);
	virtual void HandleSensedSight_Implementation(AActor* Seen) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleLostSight(AActor* Lost);
	virtual void HandleLostSight_Implementation(AActor* Lost) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleForgetSeenTarget(AActor* Forgot);
	virtual void HandleForgetSeenTarget_Implementation(AActor* Forgot) {}
	
	UFUNCTION(BlueprintNativeEvent)
	void HandleSensedSound(AActor* Heard, const FVector& Origin);
	virtual void HandleSensedSound_Implementation(AActor* Heard, const FVector& Origin) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleEQSFinished(const FEnvQueryResult& Result);
	virtual void HandleEQSFinished_Implementation(const FEnvQueryResult& Result) {}

    UFUNCTION(BlueprintNativeEvent)
    void HandleMoveCompleted(int32 RequestID, EPathFollowingResult::Type Result);
    virtual void HandleMoveCompleted_Implementation(int32 RequestID, EPathFollowingResult::Type Result) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleAnimNotify(FGameplayTag NotifyTag);
	virtual void HandleAnimNotify_Implementation(FGameplayTag NotifyTag);

    UFUNCTION(BlueprintNativeEvent)
    void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    virtual void HandleMontageBlendingOut_Implementation(UAnimMontage* Montage, bool bInterrupted) {}

    UFUNCTION(BlueprintNativeEvent)
    void HandleAttackDetected();
    virtual void HandleAttackDetected_Implementation() {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleReceiveHitPre(UPARAM(ref) FAtkHitData& HitData);
	virtual void HandleReceiveHitPre_Implementation(FAtkHitData& HitData) {}
    
	UFUNCTION(BlueprintNativeEvent)
    void HandleReceiveHitPost(UPARAM(ref) FAtkHitData& HitData);
	virtual void HandleReceiveHitPost_Implementation(FAtkHitData& HitData) {}

	UFUNCTION(BlueprintNativeEvent)
	void OnCountered(AActor* Counteror, const FString& Reason);
	virtual void OnCountered_Implementation(AActor* Counteror, const FString& Reason) {};
	
	// Helpers
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void AddMoveOverrideTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "Brain")
	void RemoveMoveOverrideTag(const FGameplayTag& Tag);
    
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration);

	UFUNCTION(BlueprintCallable, Category = "Brain")
	void SetMovementMode(EMovementMode NewMode, uint8 CustomMode = 0);

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void StopMovementAI();
};