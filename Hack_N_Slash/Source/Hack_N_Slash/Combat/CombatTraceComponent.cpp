#include "CombatTraceComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "../Interfaces/Damageable.h"
#include "../Structs/FAtkHitData.h"
#include "../Structs/FSocketTrace.h"
#include "../Characters/StatsComponent.h"

UCombatTraceComponent::UCombatTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	owner = GetOwner();
	statsComp = owner ? owner->FindComponentByClass<UStatsComponent>() : nullptr;
}

void UCombatTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatTraceComponent::SocketTrace(USkeletalMeshComponent* SkeletalMesh, TArray<FSocketTrace> Sockets, float Radius, FAtkHitData& HitData)
{
	if (!owner) owner = GetOwner();
	if (!owner) return;

	if (!statsComp) statsComp = owner->FindComponentByClass<UStatsComponent>();
	if (!statsComp) return;

	TArray<FHitResult> allHits;
	for (const FSocketTrace& socket : Sockets) //Performs a trace for each socket pair
	{
		TArray<FHitResult> outHits; //Array of hit results from each weapon
		FVector startLoc {SkeletalMesh->GetSocketLocation(socket.socketStart)};
		FVector endLoc {SkeletalMesh->GetSocketLocation(socket.socketEnd)};
		
		TArray<AActor*> ignoredActors {owner}; //Ignore self
		if (bDebug) {UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);}
		else {UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);}

		for (const FHitResult& hitResult : outHits) {allHits.Add(hitResult);}
	}

	if (allHits.Num() <= 0) {return;}
	HandleHit(allHits, HitData);
}

void UCombatTraceComponent::Trace(float Radius, FVector Distance, FVector Offset, FAtkHitData& HitData)
{
	if (!owner) owner = GetOwner();
	if (!owner) return;

	if (!statsComp) statsComp = owner->FindComponentByClass<UStatsComponent>();
	if (!statsComp) return;

	TArray<FHitResult> outHits;
	FVector startLoc = owner->GetActorLocation() + Offset;
	FVector endLoc = Distance * owner->GetActorForwardVector() + startLoc;
	TArray<AActor*> ignoredActors {owner}; //Ignore self

	if (bDebug) {UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);}
	else {UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);}

	if (outHits.Num() <= 0) {return;}
	HandleHit(outHits, HitData);
}

void UCombatTraceComponent::HandleHit(TArray<FHitResult>& Hits, FAtkHitData& HitData)
{
	for (const FHitResult& hit : Hits) //Loop through each actor hit by the trace
	{
		AActor* hitActor {hit.GetActor()}; //Get the actor
		if (actorsToIgnore.Contains(hitActor)) {continue;} //If this actor already took damage from this trace, skip them

		IDamageable* iDmgbleHitActor {Cast<IDamageable>(hitActor)};
		if (!iDmgbleHitActor) continue; // If this actor isn't damageable, skip them

        HitData.attacker = owner;
        HitData.hitLoc = hit.ImpactPoint;
        HitData.hitDir = (hit.ImpactPoint - owner->GetActorLocation()).GetSafeNormal();

        //Base attack power (NO defense yet)
        HitData.dmgHP = statsComp->GetStat(EStat::Strength) * HitData.dmgHPMult;
		HitData.penetration = statsComp->GetStat(EStat::Penetration);
	
		float critRate = statsComp->GetStat(EStat::CritRate);
        if (critRate > 0.0f && UKismetMathLibrary::RandomFloatInRange(0.f, 1.f) <= critRate) {HitData.dmgHP *= statsComp->GetStat(EStat::CritDmg);}

		iDmgbleHitActor->ReceiveHit(HitData);
		actorsToIgnore.AddUnique(hitActor); //Now that damage was applied to this actor, add them to the list of actors to ignore for this trace
	}
}

void UCombatTraceComponent::ResetAttack() { actorsToIgnore.Empty(); }