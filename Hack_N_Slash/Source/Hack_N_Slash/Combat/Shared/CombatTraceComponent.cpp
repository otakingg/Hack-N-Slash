#include "CombatTraceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "../../Interfaces/Damageable.h"
#include "../../Structs/FSocketTrace.h"

UCombatTraceComponent::UCombatTraceComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	owner = GetOwner();
}

void UCombatTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

void UCombatTraceComponent::DistanceTrace(float Radius, float Distance, FVector Offset)
{
	if (!owner) return;

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
	if (!owner) return;

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

		HitData.hitImpactNormal = hit.ImpactNormal;
		HitData.hitImpactPoint = hit.ImpactPoint;
        HitData.hitLoc = hit.Location;

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