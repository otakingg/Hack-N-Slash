#include "AOE_Base.h"
#include "Kismet/GameplayStatics.h"
#include "../Interfaces/Damageable.h"

AAOE_Base::AAOE_Base()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAOE_Base::BeginPlay()
{
	Super::BeginPlay();
}

void AAOE_Base::HandleHit(AActor* OverlappedActor, const FVector& HitLocation)
{
	if (!OverlappedActor) return;
	
	hitData.attacker = GetInstigator(); // The instigator is the one who caused the attack
	hitData.damager = this; // The damager is the direct dealer of damage, which in this case is the projectile itself
	hitData.hitLoc = HitLocation;
	hitData.dmgHP = damage;
	hitData.penetration = penetration;

	if (IDamageable* damageable = Cast<IDamageable>(OverlappedActor)) damageable->ReceiveHit(hitData);
	else UGameplayStatics::ApplyDamage(OverlappedActor, hitData.dmgHP, GetInstigatorController(), this, UDamageType::StaticClass());
}