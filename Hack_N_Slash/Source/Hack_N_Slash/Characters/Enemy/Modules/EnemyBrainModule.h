#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "../../Structs/FAtkHitData.h"
#include "../../Tags/LocomotionTags.h"
#include "Navigation/PathFollowingComponent.h"
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

UENUM(BlueprintType)
enum class EBrainState : uint8
{
    Active,
    Inactive
};

UCLASS(Abstract, Blueprintable)
class HACK_N_SLASH_API UEnemyBrainModule : public UObject
{
    GENERATED_BODY()

protected:
    UPROPERTY() UEnemyBrainComponent* brain = nullptr;

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain")
    EBrainPriority priority = EBrainPriority::Medium;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Brain")
    EBrainState moduleState = EBrainState::Inactive;

    /** Friendly name for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain")
    FName moduleName = NAME_None;

    /** Initialize instance (called by Brain) */
    void Initialize(UEnemyBrainComponent* InBrain) {brain = InBrain;}

    /** Query whether module wants control right now */
	UFUNCTION(BlueprintNativeEvent)
	bool CanStart(const FString& Reason) const;
    bool CanStart_Implementation(const FString& Reason) const { return false; }

    /** Return true if this module can be interrupted by Other (default: compare priority) */
    UFUNCTION(BlueprintNativeEvent)
    bool CanBeInterruptedBy(UEnemyBrainModule* Other) const;
    virtual bool CanBeInterruptedBy_Implementation(UEnemyBrainModule* Other) const
    {
        if (!Other) return false;
        return static_cast<int>(Other->priority) > static_cast<int>(priority);
    }

    /** Called when module is granted control */
	UFUNCTION(BlueprintNativeEvent)
    void OnEnter();
	virtual void OnEnter_Implementation() {moduleState = EBrainState::Active;}

    /** Called when module loses control */
	UFUNCTION(BlueprintNativeEvent)
    void OnExit();
	virtual void OnExit_Implementation() {moduleState = EBrainState::Inactive;}
    
    /** Override in BP */
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
    void HandleAnimNotify(FName NotifyName);
	virtual void HandleAnimNotify_Implementation(FName NotifyName) {}

	UFUNCTION(BlueprintNativeEvent)
    void HandleReceiveHit(const FAtkHitData& HitData);
	virtual void HandleReceiveHit_Implementation(const FAtkHitData& HitData) {}

    UFUNCTION(BlueprintNativeEvent)
    void HandleTargettedForAttack();
    virtual void HandleTargettedForAttack_Implementation() {}

    /** Convenience: helper to access brain/blackboard/state machine */
    UFUNCTION(BlueprintPure, Category="Brain")
    UEnemyBrainComponent* GetBrain() const { return brain; }

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetWalkSpeedAndAcceleration(float WalkSpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void SetFlySpeedAndAcceleration(float FlySpeed, float Acceleration);

    UFUNCTION(BlueprintCallable, Category="Brain")
    void StopMovingHNS();

    UFUNCTION(BlueprintCallable, Category="Brain")
    void AddMoveOverrideTag(const FGameplayTag& Tag);
};