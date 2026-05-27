#include "AOE_Base.h"

AAOE_Base::AAOE_Base()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAOE_Base::BeginPlay()
{
	Super::BeginPlay();
}