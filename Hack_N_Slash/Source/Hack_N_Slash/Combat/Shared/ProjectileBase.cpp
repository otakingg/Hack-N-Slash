#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../../Interfaces/Damageable.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	projectileMovComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	if (projectileMovComp) projectileMovComp->bAutoActivate = false; // Disable because we want to set parameters 1st
}

void AProjectileBase::BeginPlay() { Super::BeginPlay(); }

void AProjectileBase::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AProjectileBase::Activate() { if (projectileMovComp) projectileMovComp->Activate(); }

void AProjectileBase::AimAtTarget()
{
	if (!target || !projectileMovComp) return;

	FVector actorLoc = GetActorLocation();
	FVector targetLoc = target->GetActorLocation();

    FVector dirToTarget = targetLoc - actorLoc;
	projectileMovComp->Velocity = dirToTarget * projectileMovComp->InitialSpeed; // New Velocity
}

void AProjectileBase::SetRotationFollowsVelocity(bool bRotFollowsVelocity) { if (projectileMovComp) projectileMovComp->bRotationFollowsVelocity = bRotFollowsVelocity; }

void AProjectileBase::HandleDamage(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor) return;

    FAtkHitData hitData = FAtkHitData::FAtkHitData(); // Create hit data

    // Source
	hitData.attacker = GetInstigator(); // The instigator is the one who caused the attack
	hitData.damager = this; // The damager is the direct dealer of damage, which in this case is the projectile itself

    // Tags
	hitData.attackMotionTag = attackMotionTag;
	hitData.attackTypeTag = attackTypeTag;
    hitData.elementTags = elementTags;

    // Special Properties
    hitData.attackIntent = attackIntent;
    hitData.bArmorBreaker = bArmorBreaker;
    hitData.bIsCounterFollowUp = bIsCounterFollowUp;

    // Numbers
    hitData.aggroBuildup = aggroBuildup;
	hitData.dmg = damage;
	hitData.penetration = penetration;
	hitData.poise = poise;

    // Knockback
    hitData.bAdditive = bAdditive;
    hitData.localDir = localDir;
    hitData.distance = distance;
    hitData.duration = duration;
    hitData.velocityOnFinishMode = velocityOnFinishMode;
    hitData.velocityOnFinish = velocityOnFinish;
    hitData.clampVelocityOnFinish = clampVelocityOnFinish;
    hitData.strengthOverTime = strengthOverTime;

    // Feedback
    hitData.hitImpactNormal = HitResult.ImpactNormal;
    hitData.hitImpactPoint = HitResult.ImpactPoint;
	hitData.hitLoc = HitResult.Location;
    hitData.hitSFX = hitSFX;
    hitData.hitVFX = hitVFX;

	if (IDamageable* iDmgble = Cast<IDamageable>(HitActor)) iDmgble->ReceiveHit(hitData); // Apply damage via custom damage system
	else UGameplayStatics::ApplyDamage(HitActor, hitData.dmg, GetInstigatorController(), this, UDamageType::StaticClass()); // Apply damage via Unreal's damage system
}