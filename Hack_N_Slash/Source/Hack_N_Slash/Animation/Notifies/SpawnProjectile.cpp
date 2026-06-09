#include "SpawnProjectile.h"
#include "Components/CapsuleComponent.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Combat/Shared/ProjectileBase.h"

void USpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;

    UWorld* world = GetWorld();
	if (!world) return;

    AActor* owner = MeshComp->GetOwner();
    if (!owner) return;

    APawn* pawn = Cast<APawn>(owner);
    if (!pawn) return;

    UCapsuleComponent* capsule = owner->FindComponentByClass<UCapsuleComponent>();
    if (!capsule) return;

    if (!MeshComp->DoesSocketExist(spawnSocketName))
    {
        if (bDebugMode && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Socket Doesn't Exist"));
        return;
    }

	FTransform socketTransform = MeshComp->GetSocketTransform(spawnSocketName);
	FVector spawnLocation = socketTransform.GetLocation() + spawnOffset;
	FRotator spawnRotation = socketTransform.GetRotation().Rotator();

    FActorSpawnParameters spawnParams;
    spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    spawnParams.Instigator = pawn;
    spawnParams.Owner = owner;

	//Spawn the projectile
    AProjectileBase* projectile = world->SpawnActor<AProjectileBase>(projectileClass, spawnLocation, spawnRotation, spawnParams);
    if (!projectile)
    {
        if (bDebugMode && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Spawn Projectile: Failed"));
        return;
    }

    capsule->IgnoreActorWhenMoving(projectile, true);

    if (ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(owner))
    {
        if (AActor* target = iCmbtInst->GetCurrentTarget()) projectile->SetTarget(target);
    }

    projectile->Activate();
}