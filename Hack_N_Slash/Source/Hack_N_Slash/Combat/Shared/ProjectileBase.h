// Helpful Video: https://youtu.be/hkQ9bEwpfV8?si=Duu4bDhCBd4R4zKx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Structs/FAtkHitData.h"
#include "ProjectileBase.generated.h"

class UBoxComponent;
class UNiagaraSystem;
class UProjectileMovementComponent;

UCLASS()
class HACK_N_SLASH_API AProjectileBase : public AActor
{
	GENERATED_BODY()

private:
	float CalculateDamage() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FVector impactScale {1.0f, 1.0f, 1.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	USoundBase* impactSFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	UNiagaraSystem* impactVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	AActor* target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FAtkHitData hitData;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float damage = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float penetration = 0.0f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void HandleDamage(AActor* HitActor, FVector HitLocation);

public:	
	AProjectileBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* boxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* projectileMovComp;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void Activate();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetTarget(AActor* InTarget);
};
