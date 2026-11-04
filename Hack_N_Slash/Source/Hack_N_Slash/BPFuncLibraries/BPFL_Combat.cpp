#include "BPFL_Combat.h"
#include "../Interfaces/CombatInstigator.h"
#include "../Interfaces/Damageable.h"

bool UBPFL_Combat::ActorHasTag(const AActor* Actor, const FGameplayTag Tag, bool bExact)
{
	const ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	return iCmbtInst && iCmbtInst->HasTag(Tag, bExact);
}

bool UBPFL_Combat::ActorHasAnyTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact)
{
	const ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	return iCmbtInst && iCmbtInst->HasAnyTags(Tags, bExact);
}

bool UBPFL_Combat::ActorHasAllTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact)
{
	const ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	return iCmbtInst && iCmbtInst->HasAllTags(Tags, bExact);
}

int32 UBPFL_Combat::GetActorTeamID(const AActor* Actor)
{
	const ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	return iCmbtInst ? iCmbtInst->GetTeamID() : -1;
}

bool UBPFL_Combat::IsActorAlive(const AActor* Actor)
{
    const IDamageable* iDmg = Cast<IDamageable>(Actor);
    return iDmg && iDmg->IsAlive();
}
