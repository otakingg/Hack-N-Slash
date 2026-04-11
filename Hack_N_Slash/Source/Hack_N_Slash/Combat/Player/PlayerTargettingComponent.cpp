#include "PlayerTargettingComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UPlayerTargettingComponent::UPlayerTargettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UPlayerTargettingComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureReferences();
}

void UPlayerTargettingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!EnsureReferences() || !currentTarget) return;

	FVector currentLocation = owner->GetActorLocation();
	FVector targetLocation = currentTarget->GetActorLocation();

	//Can't lock on to target if they're out of range
	double distance {FVector::Distance(currentLocation, targetLocation)};
	if (distance > lockOnRadius) LockOff();
}

bool UPlayerTargettingComponent::EnsureReferences()
{
    if (!owner) owner = GetOwner();
    if (!owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] Owner isn't valid: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!moveComp) moveComp = owner->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(owner));
        return false;
    }

	if (!camComp) camComp = owner->FindComponentByClass<UCameraComponent>();
	if (!camComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] No CameraComponent on: %s"), *GetNameSafe(owner));
        return false;
    }

    return true;
}

double UPlayerTargettingComponent::GetCameraToTargetAlignment(FVector StartLoc, FVector EndLoc) const
{
	// -1.0 = Directly behind
	// -0.5 = Mostly behind
	// 0.0 = Target is to the side
	// 0.5 = Somewhat facing target
	// 1.0 = Looking directly at target
	FVector dirToTarget = (EndLoc - StartLoc).GetSafeNormal(); // Direction from the player to the enemy
	FVector camFwdVec = camComp->GetForwardVector(); // Forward direction the player camera is facing
	return FVector::DotProduct(camFwdVec, dirToTarget); // How close the player and their camera are to facing the same direction
}

void UPlayerTargettingComponent::ToggleLockOn()
{
	if (bLockedOn) LockOff();
	else LockOn();
}

void UPlayerTargettingComponent::LockOff()
{
	PrimaryComponentTick.bCanEverTick = false;
	bLockedOn = false;
	if (moveComp) moveComp->bOrientRotationToMovement = true;
	//if (currentTarget) IEnemy::Execute_OnDeselect(currentTarget);
	currentTarget = nullptr;
}

void UPlayerTargettingComponent::LockOn()
{
	TArray<AActor*> enemies = GetEnemiesInRadius(lockOnRadius);
	AActor* enemy = FindClosestTarget(enemies);
	if (!enemy) return;

	currentTarget = enemy;
	if (moveComp) moveComp->bOrientRotationToMovement = false;
	bLockedOn = true;
	PrimaryComponentTick.bCanEverTick = true;
	//IEnemy::Execute_OnSelect(currentTarget);
}

TArray<AActor*> UPlayerTargettingComponent::GetEnemiesInRadius(float Radius)
{
	TArray<AActor*> enemies;
    if (!EnsureReferences()) return enemies;

	TArray<FHitResult> outHits;
	FVector startLoc = owner->GetActorLocation();
	TArray<AActor*> ignore = {owner};

	if (bDebug) {bool targetFound = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, startLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false, ignore, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 1.0f);}
	else {bool targetFound = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, startLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false, ignore, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green);}

	for (const FHitResult& hit : outHits)
	{
		AActor* enemy = hit.GetActor();
		if (enemy) enemies.AddUnique(enemy);
	}
	return enemies;
}

AActor* UPlayerTargettingComponent::FindClosestTarget(const TArray<AActor*>& Targets)
{
	if (!EnsureReferences() || Targets.IsEmpty()) return nullptr;

	AActor* closestTarget = nullptr;
	double bestDotProd = -1.0f;

	for (AActor* target : Targets)
	{
		if (!target) continue;
		FHitResult outHit;
		FVector startLoc = camComp->GetComponentLocation();
		FVector endLoc = target->GetActorLocation();
		TArray<AActor*> ignore = {owner};

		//Check for something blocking the player's line of sight to the enemy
		if (bDebug) {UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::ForDuration, outHit, true, FLinearColor::Red, FLinearColor::Green, 1.0f);}
		else {UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green);}
		if (outHit.bBlockingHit) {continue;}

		double camAlignmentToTarget = GetCameraToTargetAlignment(startLoc, endLoc);
		if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Cam Alignment to Target: %f"), camAlignmentToTarget));}

		if (camAlignmentToTarget < 0.0f || camAlignmentToTarget < bestDotProd) continue;
		closestTarget = target;


	}
    return closestTarget;
}