#include "PlayerTargettingComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include "../../Interfaces/Damageable.h"
#include "../../Interfaces/Enemy.h"
#include "../../Characters/Shared/LocomotionComponent.h"

UPlayerTargettingComponent::UPlayerTargettingComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerTargettingComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	EnsureReferences();
}

void UPlayerTargettingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!EnsureReferences()) return;

	if (!currentTarget) // Can happen, for example, if an enemy is destroyed while locked onto them
	{
		LockOff();
		return;
	}

	FVector currentLocation = ownerChar->GetActorLocation();
	FVector targetLocation = currentTarget->GetActorLocation();

	//Can't lock on to target if they're out of range
	double distance = FVector::Distance(currentLocation, targetLocation);
	if (distance > hardTargetRadius) LockOff();
}

bool UPlayerTargettingComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] Owner isn't an 'ACharacter': %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!moveComp) moveComp = ownerChar->FindComponentByClass<UCharacterMovementComponent>();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!camComp) camComp = ownerChar->FindComponentByClass<UCameraComponent>();
	if (!camComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerTargettingComponent] No CameraComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!locoComp) locoComp = ownerChar->FindComponentByClass<ULocomotionComponent>();

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

double UPlayerTargettingComponent::GetDirToTargetAlignment2D(AActor* Target, FVector2D Dir) const
{
	// Formula for rotation in the Z-direciton of you're input: (CR^z * Input Direciton)
	// Get the dot product of that and the line between the target and the player to see how close they are to pointing in the same direciton
	// (CR^z * Input Direciton) DOT (Enemy Loc - Player Loc)
	// Use the normals of the 2 lines as we only care about their directions. So we can warp to a target closer to our input even if they're further away than another target
	FRotator playerCR = ownerChar->GetControlRotation(); // Player control rotation
	FRotator playerCRY = FRotator(0.0f, playerCR.Yaw, 0.0f); // Player yaw (z) control rotation
	FVector playerCRYFwdVec = UKismetMathLibrary::GetForwardVector(playerCRY); // Player control rotation yaw (z) forward vec
	FVector playerCRYRVec = UKismetMathLibrary::GetRightVector(playerCRY); // Player control rotation yaw (z) right vec
	FVector worldDirection = playerCRYFwdVec * Dir.Y + playerCRYRVec * Dir.X;
	FVector worldDirectionNorm = UKismetMathLibrary::Normal(worldDirection);

	FVector playerLoc = ownerChar->GetActorLocation();
	FVector targetLoc = Target->GetActorLocation();
	FVector dirToTarget = targetLoc - playerLoc;
	FVector dirToTargetNorm = UKismetMathLibrary::Normal(dirToTarget);

	return FVector::DotProduct(worldDirectionNorm, dirToTargetNorm);
}

void UPlayerTargettingComponent::SoftTarget(const FVector2D& Move, float TargettingRadius, float TargetHeightCeiling, bool bAlignmentOverDist)
{
	if (!EnsureReferences() || !locoComp || bLockedOn) return;

	// Clear pevious data
	locoComp->ClearWarpData();
	ClearCurrentTarget();

	FVector ownerLoc = ownerChar->GetActorLocation();

	TArray<AActor*> Targets = GetEnemiesInRadius(TargettingRadius);
	float bestDProduct = -1.0f;
	float bestDistance = -1.0f;
	AActor* bestTarget = nullptr;
	for (AActor* target : Targets)
	{
		if (!target) continue;

		// Make sure the target is within soft lock height
		FVector targetLoc = target->GetActorLocation();
		double height = FMath::Abs((targetLoc - ownerLoc).Z);
		if (height > TargetHeightCeiling) continue;

		// Make sure nothing is blocking the player's line of sight to the target
		FHitResult outHit;
		TArray<AActor*> ignore = {ownerChar};
		if (bDebug) UKismetSystemLibrary::SphereTraceSingle(GetWorld(), ownerLoc, targetLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::ForDuration, outHit, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
		else UKismetSystemLibrary::SphereTraceSingle(GetWorld(), ownerLoc, targetLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green);
		if (!outHit.bBlockingHit || outHit.GetActor() != target) continue;

		if (bAlignmentOverDist)
		{
			// Choose the best target based on either input direction or camera facing direction alignment with the enemy
			double dProduct = 0.0f;
			if (Move.IsNearlyZero()) dProduct = GetCameraToTargetAlignment(ownerLoc, targetLoc);
			else dProduct = GetDirToTargetAlignment2D(target, Move);

			//if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Target DProd: %f"), dProduct));}
			
			if (dProduct >= softTargetAlignmentTolerance && dProduct > bestDProduct)
			{
				bestDProduct = dProduct;
				bestTarget = target;
			}
		}
		else
		{
			float distance = FVector::Distance(ownerLoc, targetLoc);
			//if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Target Distance: %f"), distance));}
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestTarget = target;
			}
		}
	}

	if (bDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Best DProd: %f"), bestDProduct));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Best Distance: %f"), bestDistance));
	}

	if (bestTarget != currentTarget && (bestDProduct != -1.0f || bestDistance != -1.0f))
	{
		currentTarget = bestTarget;
		IEnemy::Execute_OnSoftLockOn(currentTarget);
	}
}

void UPlayerTargettingComponent::ToggleLockOn()
{
	if (bLockedOn) LockOff();
	else if (LockOnBasedOnYaw(0.0f))
	{
		//if (moveComp) moveComp->bOrientRotationToMovement = false;
		bLockedOn = true;
		SetComponentTickEnabled(true);
	}
}

void UPlayerTargettingComponent::LockOff()
{
	SetComponentTickEnabled(false);
	bLockedOn = false;
	//if (moveComp) moveComp->bOrientRotationToMovement = true;
	if (currentTarget)
	{
		IEnemy::Execute_OnLockOff(currentTarget);
		IEnemy::Execute_OnSoftLockOff(currentTarget);
	}
	currentTarget = nullptr;
}

bool UPlayerTargettingComponent::LockOnBasedOnYaw(float Yaw)
{
	TArray<AActor*> enemies = GetEnemiesInRadius(hardTargetRadius);
	AActor* enemy = nullptr;
	if (Yaw < 0.0f) enemy = FindBestTargetToLeft(enemies);
	else if (Yaw > 0.0f) enemy = FindBestTargetToRight(enemies);
	else enemy = FindBestTarget(enemies);

	if (!enemy) return false;
	if (currentTarget)
	{
		IEnemy::Execute_OnSoftLockOff(currentTarget);
		IEnemy::Execute_OnLockOff(currentTarget);
	}
	currentTarget = enemy;
	IEnemy::Execute_OnLockOn(currentTarget);
	return true;
}

TArray<AActor*> UPlayerTargettingComponent::GetEnemiesInRadius(float Radius)
{
	TArray<AActor*> enemies;
    if (!EnsureReferences()) return enemies;

	TArray<FHitResult> outHits;
	FVector startLoc = ownerChar->GetActorLocation();
	TArray<AActor*> ignore = {ownerChar};

	if (bDebug) UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, startLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false, ignore, EDrawDebugTrace::ForDuration, outHits, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
	else UKismetSystemLibrary::SphereTraceMulti(GetWorld(), startLoc, startLoc, Radius, UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false, ignore, EDrawDebugTrace::None, outHits, true, FLinearColor::Red, FLinearColor::Green);

	for (const FHitResult& hit : outHits)
	{
		AActor* enemy = hit.GetActor();

		if (IDamageable* iDmgbl = Cast<IDamageable>(enemy)) if (!iDmgbl->IsAlive()) continue;
		if (enemy) enemies.AddUnique(enemy);
	}
	return enemies;
}

AActor* UPlayerTargettingComponent::FindBestTarget(const TArray<AActor*>& Targets)
{
	if (!EnsureReferences() || Targets.IsEmpty()) return nullptr;

	AActor* bestTarget = nullptr;
	double bestDotProd = -1.0f;

	for (AActor* target : Targets)
	{
		if (!target) continue;
		FHitResult outHit;
		FVector startLoc = camComp->GetComponentLocation();
		FVector endLoc = target->GetActorLocation();
		TArray<AActor*> ignore = {ownerChar};

		// Check for something blocking the player's line of sight to the enemy
		if (bDebug) UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::ForDuration, outHit, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
		else UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green);
		if (!outHit.bBlockingHit || outHit.GetActor() != target) continue;

		// Make sure the camera aligns closely with the enemy
		double camAlignmentToTarget = GetCameraToTargetAlignment(startLoc, endLoc);
		if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Cam Alignment to Target: %f"), camAlignmentToTarget));

		if (camAlignmentToTarget < 0.8f || camAlignmentToTarget < bestDotProd) continue;
		bestTarget = target;
		bestDotProd = camAlignmentToTarget;
	}
    return bestTarget;
}

AActor* UPlayerTargettingComponent::FindBestTargetToLeft(const TArray<AActor*>& Targets)
{
    if (!EnsureReferences() || Targets.IsEmpty()) return nullptr;

    AActor* bestTarget = nullptr;
    double bestAlignment = -1.0;

    const FVector startLoc = camComp->GetComponentLocation();
    const FVector camRight = camComp->GetRightVector();

    for (AActor* target : Targets)
    {
        if (!target || target == currentTarget) continue;

        const FVector endLoc = target->GetActorLocation();
        TArray<AActor*> ignore = {ownerChar};
        FHitResult outHit;

        if (bDebug) UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::ForDuration,outHit, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
        else UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green);
        if (!outHit.bBlockingHit || outHit.GetActor() != target) continue;

        const FVector dirToTarget = (endLoc - startLoc).GetSafeNormal(); // Direction from camera to target
        const double sideDot = FVector::DotProduct(camRight, dirToTarget); // Negative = left, positive = right
        if (sideDot >= 0.0) continue; // Keep only targets on the LEFT

        const double camAlignmentToTarget = FVector::DotProduct(camComp->GetForwardVector(), dirToTarget);
        if (camAlignmentToTarget < 0.8f || camAlignmentToTarget < bestAlignment) continue; // Reject targets behind the camera or worse than current best

        bestTarget = target;
        bestAlignment = camAlignmentToTarget;
    }
	if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Cam Alignment to Target: %f"), bestAlignment));

    return bestTarget;
}

AActor* UPlayerTargettingComponent::FindBestTargetToRight(const TArray<AActor*>& Targets)
{
    if (!EnsureReferences() || Targets.IsEmpty()) return nullptr;

    AActor* bestTarget = nullptr;
    double bestAlignment = -1.0;

    const FVector startLoc = camComp->GetComponentLocation();
    const FVector camRight = camComp->GetRightVector();

    for (AActor* target : Targets)
    {
        if (!target || target == currentTarget) continue;

        const FVector endLoc = target->GetActorLocation();
        TArray<AActor*> ignore = {ownerChar};
        FHitResult outHit;

        if (bDebug) UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::ForDuration,outHit, true, FLinearColor::Red, FLinearColor::Green, 1.0f);
        else UKismetSystemLibrary::SphereTraceSingle(GetWorld(), startLoc, endLoc, 20.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ignore, EDrawDebugTrace::None, outHit, true, FLinearColor::Red, FLinearColor::Green);
        if (!outHit.bBlockingHit || outHit.GetActor() != target) continue;

        const FVector dirToTarget = (endLoc - startLoc).GetSafeNormal(); // Direction from camera to target
        const double sideDot = FVector::DotProduct(camRight, dirToTarget); // Negative = left, positive = right
        if (sideDot <= 0.0) continue; // Keep only targets on the RIGHT

        const double camAlignmentToTarget = FVector::DotProduct(camComp->GetForwardVector(), dirToTarget);
        if (camAlignmentToTarget < 0.8f || camAlignmentToTarget < bestAlignment) continue; // Reject targets behind the camera or worse than current best

        bestTarget = target;
        bestAlignment = camAlignmentToTarget;
    }
	if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("Cam Alignment to Target: %f"), bestAlignment));

    return bestTarget;
}

void UPlayerTargettingComponent::ClearCurrentTarget()
{
	if (bLockedOn) return;
	
	if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[PlayerTargettingComp] Clearing Current Target"));
	if (currentTarget) IEnemy::Execute_OnSoftLockOff(currentTarget);
	currentTarget = nullptr;
}