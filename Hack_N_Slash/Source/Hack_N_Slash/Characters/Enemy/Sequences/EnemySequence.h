#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
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
	UFUNCTION() void EndCooldown() { bOnCooldown = false; }

protected:
	UPROPERTY() UEnemyBrainComponent* brain = nullptr;

	FTimerHandle TH_Cooldown;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDebug = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName sequenceName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1"))
	int sequenceIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bExecuting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOnCooldown = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float cooldown = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float weight = 1.0f;

	void Initialize(UEnemyBrainComponent* InBrain) { brain = InBrain; }

	UFUNCTION(BlueprintPure, Category = "Sequence")
	UEnemyBrainComponent* GetBrain() const { return brain; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent)
	bool CanExecute() const;
    virtual bool CanExecute_Implementation() const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Execute();
    virtual void Execute_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AdvanceSequence();
	virtual void AdvanceSequence_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Finish();
	virtual void Finish_Implementation()
	{
		if (cooldown > 0.0f)
		{
			bOnCooldown = true;
			if (UWorld* world = GetWorld())
			{
				FTimerManager& timerManager = world->GetTimerManager();
				timerManager.ClearTimer(TH_Cooldown);
				timerManager.SetTimer(TH_Cooldown, this, &UEnemySequence::EndCooldown, cooldown, false);
			}
		}

		sequenceIndex = -1;
		bExecuting = false;
	}
};