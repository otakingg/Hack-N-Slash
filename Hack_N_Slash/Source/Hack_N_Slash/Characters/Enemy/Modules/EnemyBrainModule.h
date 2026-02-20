#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnemyBrainModule.generated.h"

class UEnemyBrainComponent;

UENUM(BlueprintType)
enum class EBrainPriority : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3
};

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemyBrainModule : public UObject
{
    GENERATED_BODY()

protected:
    UPROPERTY() UEnemyBrainComponent* brain {nullptr};

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain")
    EBrainPriority priority = EBrainPriority::Medium;

    /** If >0 module keeps exclusive control for this many seconds unless it voluntarily yields */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain")
    float claimDuration = 0.f;

    /** Friendly name for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain")
    FName moduleName = NAME_None;

    /** Initialize instance (called by Brain) */
    void Initialize(UEnemyBrainComponent* InBrain) {brain = InBrain;}

    /** Query whether module wants control right now */
	UFUNCTION(BlueprintNativeEvent)
	bool CanStart(const FString& Reason);
    bool CanStart_Implementation(const FString& Reason) { return false; }

    /** Called when module is granted control */
	UFUNCTION(BlueprintNativeEvent)
    void OnEnter();
	void OnEnter_Implementation() {}

    /** Called when module loses control */
	UFUNCTION(BlueprintNativeEvent)
    void OnExit();
	void OnExit_Implementation() {}

    /** Override in BP */
	UFUNCTION(BlueprintNativeEvent)
    void HandleSensedSight(AActor* Seen);
	void HandleSensedSight_Implementation(AActor* Seen) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleLostSight(AActor* Lost);
	void HandleLostSight_Implementation(AActor* Lost) {}
	
	UFUNCTION(BlueprintNativeEvent)
	void HandleSensedSound(AActor* Heard, const FVector& Origin);
	void HandleSensedSound_Implementation(AActor* Heard, const FVector& Origin) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleSensedDamage(AActor* Source);
	void HandleSensedDamage_Implementation(AActor* Source) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleEQSFinished(const FEnvQueryResult& Result);
	void HandleEQSFinished_Implementation(const FEnvQueryResult& Result) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleMoveCompleted(bool bSuccess);
	void HandleMoveCompleted_Implementation(bool bSuccess) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleAnimNotify(FName NotifyName);
	void HandleAnimNotify_Implementation(FName NotifyName) {}

    /** Convenience: helper to access brain/blackboard/state machine */
    UFUNCTION(BlueprintCallable, Category="Brain")
    UEnemyBrainComponent* GetBrain() const { return brain; }
};