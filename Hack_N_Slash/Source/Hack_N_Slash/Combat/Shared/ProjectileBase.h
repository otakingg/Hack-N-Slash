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

private:
	float CalculateDamage() const;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FAtkHitData hitData;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float damage = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float penetration = 0.0f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void HandleDamage(AActor* HitActor, const FVector& HitLocation);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* boxComp;

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
