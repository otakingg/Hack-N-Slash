#include "AOE_Base.h"
#include "../Interfaces/Damageable.h"
#include "../Structs/FAtkHitData.h"

AAOE_Base::AAOE_Base()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAOE_Base::BeginPlay()
{
	Super::BeginPlay();
}

void AAOE_Base::HandleHit(UPARAM(ref) FAtkHitData& HitData, AActor* OverlappedActor)
{
	if (IDamageable* damageable = Cast<IDamageable>(OverlappedActor)) damageable->ReceiveHit(HitData);
}