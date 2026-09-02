// Helpful Video: https://youtu.be/hkQ9bEwpfV8?si=Duu4bDhCBd4R4zKx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interfaces/Damageable.h"
#include "../../Structs/FAtkHitData.h"
#include "ProjectileBase.generated.h"

class UBoxComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;

UCLASS()
class HACK_N_SLASH_API AProjectileBase : public AActor, public IDamageable
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ExposeOnSpawn = true))
	bool bDebug = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ToolTip = "Was this projectile countered?"))
	bool bCountered = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ExposeOnSpawn = true))
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ExposeOnSpawn = true))
	AActor* target = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	USoundBase* spawnSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	USoundBase* destroyedSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    UNiagaraSystem* destroyedVFX = nullptr;

    //--------------------------------
    // Tags
    //--------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Tags", meta = (Categories = "Attack.Motion."))
	FGameplayTag attackMotionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Tags", meta = (Categories = "Attack.Type."))
	FGameplayTag attackTypeTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Tags", meta = (Categories = "Element."))
    TArray<FGameplayTag> elementTags;

    //--------------------------------
    // Special
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Special")
    EAttackIntent attackIntent = EAttackIntent::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Special", meta = (ToolTip = "Can this attack break through super armor"))
    bool bArmorBreaker = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Special", meta = (ToolTip = "The attack following a parry or perfect block"))
    bool bIsCounterFollowUp = false;

    //--------------------------------
    // Numbers
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Numbers", meta = (ClampMin = 0, ClampMax = 1, Tooltip = "How much this attack aggros the target"))
    float aggroBuildup = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Numbers", meta = (ClampMin = 0))
    float damage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Numbers", meta = (ClampMin = 0))
    float penetration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Numbers", meta = (ClampMin = 0, ToolTip = "Exact poise used for reaction calculation"))
    int poise = 0;

    //--------------------------------
    // Knockback
    //--------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (ToolTip = "Should this add to existing forces or override them?"))
    bool bAdditive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (ToolTip = "How the knockback force will behave over time"))
    UCurveFloat* strengthOverTime = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (ToolTip = "Local-space knockback direction. Will be normalized so only direction matters"))
    FVector localDir = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "Distance the victim will be moved"))
    float distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (ClampMin = "0.0", ToolTip = "How long it'll take for the victim to cover the distance"))
    float duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback")
    ERootMotionFinishVelocityMode velocityOnFinishMode = ERootMotionFinishVelocityMode::SetVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::SetVelocity", EditConditionHides))
    FVector velocityOnFinish = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Knockback", meta = (EditCondition = "VelocityOnFinishMode == ERootMotionFinishVelocityMode::ClampVelocity", EditConditionHides))
    float clampVelocityOnFinish = 0.0f;

    //--------------------------------
    // Feedback
    //--------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Feedback")
    USoundBase* hitSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Hit Data|Feedback")
    UNiagaraSystem* hitVFX = nullptr;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void HandleDamage(AActor* HitActor, const FHitResult& HitResult);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* projectileMovComp;

	AProjectileBase();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Activate();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void AimAtTarget();

	void SetDebug(bool bDebugMode) { bDebug = bDebugMode; }
	void SetIgnoreSelf(bool bIgnore) { bIgnoreSelf = bIgnore; }
	void SetTarget(AActor* InTarget) { target = InTarget; }
	void SetRotationFollowsVelocity(bool bRotFollowsVelocity);

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void CounteredHelper(AActor* Counteror, const FString& Reason);
	virtual void CounteredHelper_Implementation(AActor* Counteror, const FString& Reason) {}

	/* Damageable Interface Functions*/
	virtual void Countered(AActor* Counteror, const FString& Reason) override { CounteredHelper(Counteror, Reason); } // Parry or Perfect Block has succeeded
};
