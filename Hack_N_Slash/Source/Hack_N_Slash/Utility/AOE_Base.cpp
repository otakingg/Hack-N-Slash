#include "AOE_Base.h"
#include "Kismet/GameplayStatics.h"
#include "../Interfaces/Damageable.h"
#include "../../Structs/FAtkHitData.h"

AAOE_Base::AAOE_Base() { PrimaryActorTick.bCanEverTick = false; }

void AAOE_Base::BeginPlay() { Super::BeginPlay(); }

void AAOE_Base::HandleHit(AActor* OverlappedActor, FAtkHitData HitData)
{
	if (!OverlappedActor) return;
    
	if (IDamageable* damageable = Cast<IDamageable>(OverlappedActor)) damageable->ReceiveHit(HitData);
	else UGameplayStatics::ApplyDamage(OverlappedActor, HitData.dmg, GetInstigatorController(), this, UDamageType::StaticClass());
}