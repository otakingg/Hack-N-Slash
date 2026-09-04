#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemySequence.generated.h"

// This class represents the actions and enemy can perform
// In the blueprint for this class you can make the enemy do whatever you want
// When a sequence finishes performing its 1st action you can finish the sequence or move onto another action by using "Advance Sequence" & "Sequence Index"

class UEnemyBrainComponent;

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemySequence : public UObject
{
    GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence")
	bool bDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (ToolTip = "Tags the enemy can't have to perform this sequence"))
	TArray<FGameplayTag> invalidSequenceTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence", meta = (Categories = "State.Movement."))
	FGameplayTag validMovementState; // The movement state required to perform this sequence

	UPROPERTY(Transient, BlueprintReadWrite)
    UEnemyBrainComponent* brain = nullptr; // Reference to the enemy brain component

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Description")
    FName sequenceName;

	UPROPERTY(EditDefaultsOnly, Category = "Sequence|Description", meta = (MultiLine = "true"))
	FText description;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence", meta = (ToolTip = "The last time this sequence was performed"))
	float lastSequenceTime = -1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Score", meta = (ClampMin = "0.0", ToolTip = "How much does the enemy want to perform this sequence"))
    float baseScore = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence", meta = (ClampMin = "1"))
	int32 sequenceIndex = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown", meta = (ClampMin = "0.0"))
    float cooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	FTimerHandle TH_Cooldown;
	
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void EndCooldown() { bOnCooldown = false; }

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool IsActive() const;

	// Helper functions
	UFUNCTION(BlueprintPure, Category = "Sequence")
	float GetTargetDistance() const;

    UFUNCTION(BlueprintCallable, Category = "Sequence")
    void AddTag(const FGameplayTag& Tag);
    
    UFUNCTION(BlueprintCallable, Category = "Sequence")
    void RemoveTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool HasTag(const FGameplayTag& Tag, bool bExact = false) const;

	UFUNCTION(BlueprintPure, Category = "Sequence")
	bool HasAnyTag(const TArray<FGameplayTag>& Tags, bool bExact = false) const;
    
    UFUNCTION(BlueprintCallable, Category = "Sequence")
    void SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Sequence")
    void SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration);

	UFUNCTION(BlueprintCallable, Category = "Sequence")
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

	UFUNCTION(BlueprintPure, Category = "Sequence")
	UEnemyBrainComponent* GetBrain() const { return brain; }

	FName GetSeqName() const { return sequenceName; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Execute();
    virtual void Execute_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AdvanceSequence();
	virtual void AdvanceSequence_Implementation() { Finish(); }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Finish(); // Finished normally
	virtual void Finish_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Abort(); // Was interrupted
	virtual void Abort_Implementation();

    /** Event Handlers */
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
	void HandleReceiveHitPost(const FAtkHitData& HitData);
	virtual void HandleReceiveHitPost_Implementation(const FAtkHitData& HitData) {}
};