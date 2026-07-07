#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "../../Interfaces/Damageable.h"
#include "../../Characters/Shared/StatsComponent.h"

AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
	hitData.dmgHP = CalculateDamage();
	hitData.penetration = penetration;

	if (IDamageable* iDmgble = Cast<IDamageable>(HitActor)) iDmgble->ReceiveHit(hitData);
	else UGameplayStatics::ApplyDamage(HitActor, hitData.dmgHP, GetInstigatorController(), this, UDamageType::StaticClass());
}

float AProjectileBase::CalculateDamage() const
{
	AActor* owner = GetOwner();
	if (!owner) return damage;

	UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
	if (!statsComp) return damage;

	float critRate = statsComp->GetStat(EStat::CritRate);
	if (critRate > 0.0f && UKismetMathLibrary::RandomFloatInRange(0.f, 1.f) <= critRate) return damage * statsComp->GetStat(EStat::CritDmg);
	else return damage;
}
