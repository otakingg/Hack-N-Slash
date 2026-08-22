#include "CombatTraceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "CombatResolutionComponent.h"
#include "../../Interfaces/Damageable.h"
#include "../../Structs/FSocketTrace.h"
#include "../../Characters/Shared/StatsComponent.h"

UCombatTraceComponent::UCombatTraceComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	owner = GetOwner();
	combatResComp = owner ? owner->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
	statsComp = owner ? owner->FindComponentByClass<UStatsComponent>() : nullptr;
}

void UCombatTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

void UCombatTraceComponent::BuildHitData(FAtkHitData HitData)
{
	if (!owner || !statsComp) return;

	activeHitData = HitData;

	activeHitData.dmg = statsComp->GetStat(EStat::Strength) * activeHitData.dmgMult;
	activeHitData.penetration = statsComp->GetStat(EStat::Penetration);
	activeHitData.poiseFinal = combatResComp ? combatResComp->GetPoise() + activeHitData.poisePlus : 0;

	float critRate = statsComp->GetStat(EStat::CritRate);
	if (critRate > 0.0f && UKismetMathLibrary::RandomFloatInRange(0.f, 1.f) <= critRate) activeHitData.dmg *= statsComp->GetStat(EStat::CritDmg);
}

void UCombatTraceComponent::DistanceTrace(float Radius, float Distance, FVector Offset)
{
	if (!owner || !statsComp) return;

	// Trace
	TArray<FHitResult> outHits;
	FVector startLoc = owner->GetActorLocation() + Offset;
	FVector endLoc = startLoc + owner->GetActorForwardVector() * Distance;
	TArray<AActor*> ignoredActors {owner}; //Ignore self

	if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
	else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

	if (outHits.Num() <= 0) return;
	HandleHit(outHits, activeHitData);
}

void UCombatTraceComponent::SocketTrace(USkeletalMeshComponent* SkeletalMesh, TArray<FSocketTrace> Sockets, float Radius)
{
	if (!owner || !statsComp) return;

	// Trace
	TArray<FHitResult> allHits;
	for (const FSocketTrace& socket : Sockets) //Performs a trace for each socket pair
	{
		TArray<FHitResult> outHits; //Array of hit results from each weapon
		FVector startLoc = SkeletalMesh->GetSocketLocation(socket.socketStart);
		FVector endLoc = SkeletalMesh->GetSocketLocation(socket.socketEnd);
		
		TArray<AActor*> ignoredActors {owner}; //Ignore self
		if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
		else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

		for (const FHitResult& hitResult : outHits) {allHits.Add(hitResult);}
	}

	if (allHits.Num() <= 0) return;
	HandleHit(allHits, activeHitData);
}

void UCombatTraceComponent::HandleHit(TArray<FHitResult>& Hits, FAtkHitData HitData)
{
	for (const FHitResult& hit : Hits) //Loop through each actor hit by the trace
	{
		AActor* hitActor = hit.GetActor(); //Get the actor
		if (actorsToIgnore.Contains(hitActor)) continue; //If this actor already took damage from this trace, skip them

		IDamageable* iDmgble = Cast<IDamageable>(hitActor);

        HitData.hitLoc = hit.ImpactPoint;

		if (iDmgble) iDmgble->ReceiveHit(HitData);
		else UGameplayStatics::ApplyDamage(hitActor, HitData.dmg, owner->GetInstigatorController(), owner, UDamageType::StaticClass());
		actorsToIgnore.AddUnique(hitActor); //Now that damage was applied to this actor, add them to the list of actors to ignore for this trace
	}
}

void UCombatTraceComponent::ClearHitActors()
{
	actorsToIgnore.Empty();
	activeHitData = FAtkHitData::FAtkHitData();
}