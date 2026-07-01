#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemySequence.generated.h"

//class UCurveFloat;
class UEnemyBrainComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemySequence : public UObject
{
    GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
	bool bDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (Categories = "State.Action."))
	FGameplayTag requiredActionState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (Categories = "State.Movement."))
	FGameplayTag requiredMovementState;

	UPROPERTY(Transient, BlueprintReadWrite)
    UEnemyBrainComponent* brain = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Description")
    FName sequenceName;

	UPROPERTY(EditDefaultsOnly, Category = "Sequence|Description", meta = (MultiLine = "true"))
	FText description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ClampMin = "0.0"))
    float baseScore = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does the number of times this sequence has happened consecutively affect the score. If left empty, won't affect score"))
	UCurveFloat* stalenessCurve = nullptr;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does aggro affect this sequence. If left empty, won't affect score"))
	UCurveFloat* aggroCurve = nullptr;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on target distance. If left empty, won't affect score"))
	UCurveFloat* distanceCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ToolTip = "How does this enemy want to perform this sequence based on how recent their last atk was. If left empty, won't affect score"))
	UCurveFloat* timeSinceLastAtkCurve = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence", meta = (ClampMin = "1"))
	int32 sequenceIndex = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown", meta = (ClampMin = "0.0"))
    float cooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	FTimerHandle TH_Cooldown;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetStalenessMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetAggroMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetDistanceMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetAtkTimeMultiplier() const;
	
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void EndCooldown() { bOnCooldown = false; }

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool IsActive() const;

	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void FinishHelper();

	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void AbortHelper();

	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetTargetDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void AddOverrideTag(const FGameplayTag& OverrideTag);
    
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void RemoveOverrideTag(const FGameplayTag& OverrideTag);
    
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration);

	UFUNCTION(BlueprintCallable, Category = "Brain")
	void SetMovementMode(EMovementMode NewMode, uint8 CustomMode = 0);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence")
	bool bInterruptible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	bool bOnCooldown = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown")
	bool bStartOnCooldown = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown", meta = (ToolTip = "Should this sequence always happen off cooldown?"))
	bool bForceOffCooldown = false;

	UFUNCTION(BlueprintNativeEvent, Category = "Sequence")
	void Initialize(UEnemyBrainComponent* InBrain);
	void Initialize_Implementation(UEnemyBrainComponent* InBrain);

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	UEnemyBrainComponent* GetBrain() const { return brain; }

	FName GetSeqName() const { return sequenceName; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore() const;
	virtual float GetScore_Implementation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Execute();
    virtual void Execute_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AdvanceSequence();
	virtual void AdvanceSequence_Implementation() { ++sequenceIndex; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Finish();
	virtual void Finish_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Abort();
	virtual void Abort_Implementation();

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
    void HandleAnimNotify(const FGameplayTag& NotifyTag);
	virtual void HandleAnimNotify_Implementation(const FGameplayTag& NotifyTag);

    UFUNCTION(BlueprintNativeEvent)
    void HandleAttackDetected(AActor* Attacker);
    virtual void HandleAttackDetected_Implementation(AActor* Attacker) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleReceiveHitPre(UPARAM(ref) FAtkHitData& HitData);
	virtual void HandleReceiveHitPre_Implementation(FAtkHitData& HitData) {}
    
	UFUNCTION(BlueprintNativeEvent)
    void HandleReceiveHitPost(UPARAM(ref) FAtkHitData& HitData);
	virtual void HandleReceiveHitPost_Implementation(FAtkHitData& HitData) {}

	UFUNCTION(BlueprintNativeEvent)
	void OnCountered(AActor* Counteror, const FString& Reason);
	virtual void OnCountered_Implementation(AActor* Counteror, const FString& Reason) {};
};