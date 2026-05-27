#include "AOE_Damage_Base.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Interfaces/Damageable.h"
#include "../../Characters/Shared/StatsComponent.h"

float AAOE_Damage_Base::CalculateDamage() const
{
	AActor* owner = GetOwner();
	if (!owner) return damage;

	UStatsComponent* statsComp = owner->FindComponentByClass<UStatsComponent>();
	if (!statsComp) return damage;

	float critRate = statsComp->GetStat(EStat::CritRate);
	if (critRate > 0.0f && UKismetMathLibrary::RandomFloatInRange(0.f, 1.f) <= critRate) return damage * statsComp->GetStat(EStat::CritDmg);
	else return damage;
}

void AAOE_Damage_Base::HandleDamage(AActor* HitActor)
{
	if (!HitActor) return;

	hitData.attacker = GetInstigator();
	hitData.hitLoc = HitActor->GetActorLocation();
	hitData.dmgHP = CalculateDamage();
	hitData.penetration = penetration;

	IDamageable* iDamageable = Cast<IDamageable>(HitActor);
	if (iDamageable) iDamageable->ReceiveHit(hitData);
	else UGameplayStatics::ApplyDamage(HitActor, hitData.dmgHP, GetInstigatorController(), this, UDamageType::StaticClass());
	//else UGameplayStatics::ApplyDamage(HitActor, hitData.dmgHP, GetInstigatorController(), GetInstigator, UDamageType::StaticClass());
}