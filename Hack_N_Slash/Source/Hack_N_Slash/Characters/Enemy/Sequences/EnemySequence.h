#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
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

	UPROPERTY(EditDefaultsOnly, Category = "Sequence|Description", meta = (MultiLine = "true"))
	FText description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"), Category = "Sequence")
    float weight = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"), Category = "Sequence")
	int sequenceIndex = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"), Category = "Sequence|Cooldown")
    float cooldown = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	FTimerHandle TH_Cooldown;
	
	UFUNCTION(BlueprintCallable)
	void EndCooldown() { bOnCooldown = false; }

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Description")
    FName sequenceName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence")
	bool bInterruptible = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Sequence|Cooldown")
	bool bOnCooldown = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sequence|Cooldown")
	bool bStartOnCooldown = false;

	void Initialize(UEnemyBrainComponent* InBrain);

	UFUNCTION(BlueprintPure, Category = "Sequence")
	UEnemyBrainComponent* GetBrain() const { return brain; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	float GetScore() const;
	float GetScore_Implementation() { return weight; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Sequence")
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const { return !bOnCooldown; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Execute();
    virtual void Execute_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AdvanceSequence();
	virtual void AdvanceSequence_Implementation() { ++sequenceIndex; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Finish();
	virtual void Finish_Implementation()
	{
		if (cooldown > 0.0f)
		{
			if (UWorld* world = GetWorld())
			{
				bOnCooldown = true;
				FTimerManager& timerManager = world->GetTimerManager();
				timerManager.ClearTimer(TH_Cooldown);
				timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
			}
		}

		sequenceIndex = 1;
		bInterruptible = true;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void HandleAnimNotify(FGameplayTag NotifyTag);
	virtual void HandleAnimNotify_Implementation(FGameplayTag NotifyTag) {}
	
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void AddMoveOverrideTag(const FGameplayTag& Tag);
    
    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category = "Brain")
    void SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration);
};