#include "AOE_Base.h"
#include "Kismet/GameplayStatics.h"
#include "../Interfaces/Damageable.h"
#include "../../Structs/FAtkHitData.h"

AAOE_Base::AAOE_Base() { PrimaryActorTick.bCanEverTick = false; }

void AAOE_Base::BeginPlay() { Super::BeginPlay(); }

void AAOE_Base::HandleHit(AActor* OverlappedActor, FAtkHitData HitData)
{
	if (!OverlappedActor) return;
    
	if (IDamageable* iDmgble = Cast<IDamageable>(OverlappedActor))
	{
		iDmgble->ReceiveHit(HitData); // Apply damage via custom damage system
		if (IDamageable* iDmgbleInst = Cast<IDamageable>(GetInstigator())) iDmgbleInst->NotifyHitResult(HitData); // Pass the hit result to the attacker
	}
	else UGameplayStatics::ApplyDamage(OverlappedActor, HitData.dmg, GetInstigatorController(), this, UDamageType::StaticClass());
}