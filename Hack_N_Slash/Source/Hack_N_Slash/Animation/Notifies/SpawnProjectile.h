#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SpawnProjectile.generated.h"

class AProjectileBase;

UENUM(BlueprintType)
enum class EProjectileRotationMode : uint8
{
	SocketRotation,
	OwnerForward,
	RotToTarget
};

/**
 * Spawns a projectile
 */
UCLASS()
class HACK_N_SLASH_API USpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Projectile|Debug")
	bool bDebug = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ToolTip = "Radius of the debug sphere at the spawn location of the projectile"))
	float debugRadius = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ToolTip = "Number of segments of the debug sphere"))
	float debugSegments = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ToolTip = "Color of the debug sphere"))
	FColor debugColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ClampMin = "0.1", ToolTip = "Duration of the debug sphere"))
	float debugDuration = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Debug", meta = (ClampMin = "0.0", ToolTip = "Thickness of the debug sphere"))
	float debugThickness = 2.0f;

	

	UPROPERTY(EditAnywhere, Category = "Projectile")
	EProjectileRotationMode rotationMode = EProjectileRotationMode::OwnerForward;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bRotationFollowsVelocity = true;
	
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ToolTip = "Projectile velocity will be in the direction of the target regardless of rotation"))
	bool bAimAtTarget = false;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectileBase> projectileClass;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FName spawnSocketName;

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ToolTip = "Local offset from the spawn socket"))
	FVector spawnLocOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ToolTip = "Local offset. Only applies when 'Rotation Follows Velocity' = false"))
	FRotator spawnRotationOffset = FRotator::ZeroRotator;

public:
	USpawnProjectile();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};