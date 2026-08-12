#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../../Interfaces/Damageable.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	projectileMovComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	if (projectileMovComp) projectileMovComp->bAutoActivate = false;
}

void AProjectileBase::BeginPlay() { Super::BeginPlay(); }

void AProjectileBase::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AProjectileBase::Activate() { if (projectileMovComp) projectileMovComp->Activate(); }

void AProjectileBase::AimAtTarget()
{
	if (!target || !projectileMovComp) return;

	FVector actorLoc = GetActorLocation();
	FVector targetLoc = target->GetActorLocation();

	FVector dir = UKismetMathLibrary::GetDirectionUnitVector(actorLoc, targetLoc);
	projectileMovComp->Velocity = dir * projectileMovComp->InitialSpeed; // New Velocity
}

void AProjectileBase::SetRotationFollowsVelocity(bool bRotFollowsVelocity) { if (projectileMovComp) projectileMovComp->bRotationFollowsVelocity = bRotFollowsVelocity; }

void AProjectileBase::HandleDamage(AActor* HitActor, const FVector& HitLocation)
{
	if (!HitActor) return;
	
	hitData.attacker = GetInstigator(); // The instigator is the one who caused the attack
	hitData.damager = this; // The damager is the direct dealer of damage, which in this case is the projectile itself
	hitData.hitLoc = HitLocation;
	hitData.dmg = damage;
	hitData.penetration = penetration;

	if (IDamageable* iDmgble = Cast<IDamageable>(HitActor)) iDmgble->ReceiveHit(hitData);
	else UGameplayStatics::ApplyDamage(HitActor, hitData.dmg, GetInstigatorController(), this, UDamageType::StaticClass());
}