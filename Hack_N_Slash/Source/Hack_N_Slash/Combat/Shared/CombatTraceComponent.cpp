#include "CombatTraceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "../../Interfaces/Damageable.h"
#include "../../Structs/FHitBoxShape.h"

UCombatTraceComponent::UCombatTraceComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCombatTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	owner = GetOwner();
	iDmgble = Cast<IDamageable>(owner);
}

void UCombatTraceComponent::Trace(USkeletalMeshComponent* SkeletalMesh, TArray<FHitBoxShape> HitBoxes, bool bAddToIgnore)
{
	if (!owner || !SkeletalMesh) return;
	
	for (const FHitBoxShape& hitBox : HitBoxes)
	{
		if (hitBox.shape == EHitBoxShapeType::ActorDirection)
		{
			TraceActorDirection(hitBox.radius, hitBox.length, hitBox.localOffset, hitBox.axis, bAddToIgnore);
		}
		else if (hitBox.shape == EHitBoxShapeType::BoneToBone)
		{
			TraceBoneToBone(SkeletalMesh, hitBox.startBone, hitBox.endBone, hitBox.radius, hitBox.extraLength, bAddToIgnore);
		}
		else if (hitBox.shape == EHitBoxShapeType::SingleBone)
		{
			TraceSingleBone(SkeletalMesh, hitBox.startBone, hitBox.radius, hitBox.length, hitBox.localOffset, hitBox.axis, bAddToIgnore);
		}
	}

}

void UCombatTraceComponent::TraceActorDirection(float Radius, float Length, FVector LocalOffset, EAxis::Type Axis, bool bAddToIgnore)
{
	TArray<FHitResult> outHits;
	FVector startLoc = owner->GetActorLocation() + owner->GetActorRotation().RotateVector(LocalOffset);
	FVector direction = FVector::ZeroVector;

	switch (Axis)
	{
	case EAxis::X:
		direction = owner->GetActorForwardVector();
		break;

	case EAxis::Y:
		direction = owner->GetActorRightVector();
		break;

	case EAxis::Z:
		direction = owner->GetActorUpVector();
		break;

	default:
		return;
	}

	FVector endLoc = startLoc + (direction * Length);
	TArray<AActor*> ignoredActors {owner};
	if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
	else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

	if (outHits.Num() <= 0) return;
	HandleHit(outHits, activeHitData, bAddToIgnore);
}

void UCombatTraceComponent::TraceBoneToBone(USkeletalMeshComponent* SkeletalMesh, FName SocketStart, FName SocketEnd, float Radius, float ExtraLength, bool bAddToIgnore)
{
	TArray<FHitResult> outHits;
	FVector startLoc = SkeletalMesh->GetSocketLocation(SocketStart);
	FVector endLoc = SkeletalMesh->GetSocketLocation(SocketEnd);

	// Add the extra length
	FVector direction = (endLoc - startLoc).GetSafeNormal();
	endLoc += (direction * ExtraLength);

	TArray<AActor*> ignoredActors {owner};
	if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
	else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

	if (outHits.Num() <= 0) return;
	HandleHit(outHits, activeHitData, bAddToIgnore);
}

void UCombatTraceComponent::TraceSingleBone(USkeletalMeshComponent* SkeletalMesh, FName Socket, float Radius, float Length, FVector LocalOffset, EAxis::Type Axis, bool bAddToIgnore)
{
	TArray<FHitResult> outHits;

    FTransform socketTransform = SkeletalMesh->GetSocketTransform(Socket); // Get the bone's world transform
    FVector startLoc = socketTransform.GetLocation() + socketTransform.GetRotation().RotateVector(LocalOffset); // Apply local offset relative to the bone's rotation

    FVector direction = FVector::ZeroVector;

    switch (Axis)
    {
    case EAxis::X:
        direction = socketTransform.GetUnitAxis(EAxis::X);
        break;

    case EAxis::Y:
        direction = socketTransform.GetUnitAxis(EAxis::Y);
        break;

    case EAxis::Z:
        direction = socketTransform.GetUnitAxis(EAxis::Z);
        break;

	default:
        return;
    }

    FVector endLoc = startLoc + (direction * Length);

	TArray<AActor*> ignoredActors {owner};
	if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);
	else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, endLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false, ignoredActors, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green, 0.5f);

	if (outHits.Num() <= 0) return;
	HandleHit(outHits, activeHitData, bAddToIgnore);
}

void UCombatTraceComponent::HandleHit(const TArray<FHitResult>& Hits, FAtkHitData HitData, bool bAddToIgnore)
{
	for (const FHitResult& hit : Hits) //Loop through each actor hit by the trace
	{
		AActor* hitActor = hit.GetActor(); //Get the actor
		if (actorsToIgnore.Contains(hitActor)) continue; //If this actor already took damage from this trace, skip them
		if (bAddToIgnore) actorsToIgnore.AddUnique(hitActor); // Add them to the list of actors to ignore for this trace

		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[CombatTraceComp] Hitting Actor"));
		HitData.hitImpactNormal = hit.ImpactNormal;
		HitData.hitImpactPoint = hit.ImpactPoint;
        HitData.hitLoc = hit.Location;

		IDamageable* iDmgbleVictim = Cast<IDamageable>(hitActor); // Cast to "Damageable" interface
		if (iDmgbleVictim)
		{
			iDmgbleVictim->ReceiveHit(HitData); // Apply damage via custom damage system
			if (iDmgble) iDmgble->NotifyHitResult(HitData); // Pass the hit result to the attacker
		}
		else UGameplayStatics::ApplyDamage(hitActor, HitData.dmg, owner->GetInstigatorController(), owner, UDamageType::StaticClass()); // Apply damage via Unreal's damage system
	}
}

void UCombatTraceComponent::ClearHitActors()
{
	activeHitData = FAtkHitData::FAtkHitData();
	actorsToIgnore.Empty();
}