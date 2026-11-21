#include "BPFL_Combat.h"
#include "../Interfaces/Combatant.h"
#include "../Interfaces/Damageable.h"

void UBPFL_Combat::AddTagToActor(AActor* Actor, const FGameplayTag Tag)
{
	ICombatant* iCombatant = Cast<ICombatant>(Actor);
	if (iCombatant) iCombatant->AddTag(Tag);
}

void UBPFL_Combat::AddTagsToActor(AActor* Actor, const TArray<FGameplayTag> Tags)
{
	ICombatant* iCombatant = Cast<ICombatant>(Actor);
	if (iCombatant) iCombatant->AddTags(Tags);
}

void UBPFL_Combat::RemoveTagFromActor(AActor* Actor, const FGameplayTag Tag)
{
	ICombatant* iCombatant = Cast<ICombatant>(Actor);
	if (iCombatant) iCombatant->RemoveTag(Tag);
}

void UBPFL_Combat::RemoveTagsFromActor(AActor* Actor, const TArray<FGameplayTag> Tags)
{
	ICombatant* iCombatant = Cast<ICombatant>(Actor);
	if (iCombatant) iCombatant->RemoveTags(Tags);
}

bool UBPFL_Combat::ActorHasTag(const AActor* Actor, const FGameplayTag Tag, bool bExact)
{
	const ICombatant* iCombatant = Cast<ICombatant>(Actor);
	return iCombatant && iCombatant->HasTag(Tag, bExact);
}

bool UBPFL_Combat::ActorHasAnyTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact)
{
	const ICombatant* iCombatant = Cast<ICombatant>(Actor);
	return iCombatant && iCombatant->HasAnyTags(Tags, bExact);
}

bool UBPFL_Combat::ActorHasAllTags(const AActor* Actor, const TArray<FGameplayTag> Tags, bool bExact)
{
	const ICombatant* iCombatant = Cast<ICombatant>(Actor);
	return iCombatant && iCombatant->HasAllTags(Tags, bExact);
}

int32 UBPFL_Combat::GetActorTeamID(const AActor* Actor)
{
	const ICombatant* iCombatant = Cast<ICombatant>(Actor);
	return iCombatant ? iCombatant->GetTeamID() : -1;
}

void UBPFL_Combat::Counter(AActor* CounteringActor, AActor* CounteredActor, const FString& Reason)
{
	IDamageable* iDmg = Cast<IDamageable>(CounteredActor);
	if (iDmg) iDmg->Countered(CounteringActor, Reason);
}

bool UBPFL_Combat::IsActorAlive(const AActor* Actor)
{
    const IDamageable* iDmg = Cast<IDamageable>(Actor);
    return iDmg && iDmg->IsAlive();
}