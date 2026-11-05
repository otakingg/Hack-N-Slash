#include "BPFL_Combat.h"
#include "../Interfaces/CombatInstigator.h"
#include "../Interfaces/Damageable.h"

void UBPFL_Combat::AddTagToActor(AActor* Actor, const FGameplayTag Tag)
{
	ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	if (iCmbtInst) iCmbtInst->AddTag(Tag);
}

void UBPFL_Combat::AddTagsToActor(AActor* Actor, const TArray<FGameplayTag> Tags)
{
	ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	if (iCmbtInst) iCmbtInst->AddTags(Tags);
}

void UBPFL_Combat::RemoveTagFromActor(AActor* Actor, const FGameplayTag Tag)
{
	ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	if (iCmbtInst) iCmbtInst->RemoveTag(Tag);
}

void UBPFL_Combat::RemoveTagsFromActor(AActor* Actor, const TArray<FGameplayTag> Tags)
{
	ICombatInstigator* iCmbtInst = Cast<ICombatInstigator>(Actor);
	if (iCmbtInst) iCmbtInst->RemoveTags(Tags);
}

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
