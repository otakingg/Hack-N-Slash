#include "SpawnProjectile.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Combat/Shared/ProjectileBase.h"

/*void USpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !projectileClass) return;

	AActor* owner = MeshComp->GetOwner();
	if (!owner) return;

	APawn* pawn = Cast<APawn>(owner);
	if (!pawn) return;

	UCapsuleComponent* capsule = owner->FindComponentByClass<UCapsuleComponent>();
	if (!capsule) return;

	UWorld* world = owner->GetWorld();
	if (!world) return;

	if (!MeshComp->DoesSocketExist(spawnSocketName))
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Socket Doesn't Exist"));
		return;
	}

	AActor* target = nullptr;
	if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner)) target = iCmbtInst->GetCurrentTarget();

	FTransform socketTransform = MeshComp->GetSocketTransform(spawnSocketName);
	FVector spawnLocation = socketTransform.GetLocation() + socketTransform.TransformVector(spawnLocOffset);
	FRotator baseRotation = FRotator::ZeroRotator;

	switch (rotationMode)
	{
	case EProjectileRotationMode::SocketRotation:
		baseRotation = socketTransform.GetRotation().Rotator();
		break;

	case EProjectileRotationMode::OwnerForward:
		baseRotation = owner->GetActorForwardVector().Rotation();
		break;

	case EProjectileRotationMode::RotToTarget:
		if (target)
		{
			const FVector dir = (target->GetActorLocation() - spawnLocation).GetSafeNormal();
			baseRotation = dir.Rotation();
		}
		else baseRotation = socketTransform.GetRotation().Rotator();
		break;
	}

	FRotator spawnRotation = (baseRotation.Quaternion() * spawnRotationOffset.Quaternion()).Rotator();

	if (bDebug)
	{
		DrawDebugSphere(world, spawnLocation, debugRadius, debugSegments, debugColor, false, debugDuration, (uint8)0U, debugThickness);
		DrawDebugCoordinateSystem(world, spawnLocation, spawnRotation, 100.f, false, debugDuration, 0, 2.f);
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParams.Instigator = pawn;
	spawnParams.Owner = owner;

	AProjectileBase* projectile = world->SpawnActor<AProjectileBase>(projectileClass, spawnLocation, spawnRotation, spawnParams);
	if (!projectile)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Failed"));
		return;
	}

    capsule->IgnoreActorWhenMoving(projectile, bIgnoreSelf);
	projectile->SetDebug(bDebug);
	projectile->SetIgnoreSelf(bIgnoreSelf);
    projectile->SetTarget(target);
    projectile->SetRotationFollowsVelocity(bRotationFollowsVelocity);
    if (bAimAtTarget) projectile->AimAtTarget();

	projectile->Activate();
}*/

void USpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !projectileClass) return;

	AActor* owner = MeshComp->GetOwner();
	if (!owner) return;

	APawn* pawn = Cast<APawn>(owner);
	if (!pawn) return;

	UCapsuleComponent* capsule = owner->FindComponentByClass<UCapsuleComponent>();
	if (!capsule) return;

	UWorld* world = owner->GetWorld();
	if (!world) return;

	if (!MeshComp->DoesSocketExist(spawnSocketName))
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Socket Doesn't Exist"));
		return;
	}

	AActor* target = nullptr;
	if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner)) target = iCmbtInst->GetCurrentTarget();

	FTransform socketTransform = MeshComp->GetSocketTransform(spawnSocketName);
	FVector spawnLocation = socketTransform.GetLocation() + socketTransform.TransformVector(spawnLocOffset);
	FRotator spawnRotation = FRotator::ZeroRotator;

	switch (rotationMode)
	{
	case EProjectileRotationMode::SocketRotation:
		spawnRotation = socketTransform.GetRotation().Rotator();
		break;

	case EProjectileRotationMode::OwnerForward:
		spawnRotation = owner->GetActorForwardVector().Rotation();
		break;

	case EProjectileRotationMode::RotToTarget:
		if (target)
		{
			const FVector dir = (target->GetActorLocation() - spawnLocation).GetSafeNormal();
			spawnRotation = dir.Rotation();
		}
		else spawnRotation = owner->GetActorForwardVector().Rotation();
		break;
	}

	spawnRotation = (spawnRotation.Quaternion() * spawnRotationOffset.Quaternion()).Rotator();

	if (bDebug)
	{
		DrawDebugSphere(world, spawnLocation, debugRadius, debugSegments, debugColor, false, debugDuration, (uint8)0U, debugThickness);
		DrawDebugCoordinateSystem(world, spawnLocation, spawnRotation, 100.f, false, debugDuration, 0, 2.f);
	}
    
    AProjectileBase* projectile = world->SpawnActorDeferred<AProjectileBase>(projectileClass, FTransform(spawnRotation, spawnLocation), owner, pawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!projectile)
	{
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Failed"));
		return;
	}

    capsule->IgnoreActorWhenMoving(projectile, bIgnoreSelf);
	projectile->SetDebug(bDebug);
	projectile->SetIgnoreSelf(bIgnoreSelf);
    projectile->SetTarget(target);
    projectile->SetRotationFollowsVelocity(bRotationFollowsVelocity);
    if (bAimAtTarget) projectile->AimAtTarget();

    UGameplayStatics::FinishSpawningActor(projectile, FTransform(spawnRotation, spawnLocation));

	projectile->Activate();
}