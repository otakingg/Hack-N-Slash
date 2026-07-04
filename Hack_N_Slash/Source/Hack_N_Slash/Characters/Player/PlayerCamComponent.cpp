#include "PlayerCamComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Combat/Player/PlayerTargettingComponent.h"

UPlayerCamComponent::UPlayerCamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!EnsureReferences()) return;

	normalSpringLength = springArmComp->TargetArmLength;
	normalSpringSocketOffset = springArmComp->SocketOffset;
}

void UPlayerCamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!EnsureReferences()) return;

	if (playerTargettingComp && playerTargettingComp->GetLockedOn()) UpdateLockOnCam(DeltaTime);
	else UpdateLockOffCam(DeltaTime);
}

bool UPlayerCamComponent::EnsureReferences()
{
	if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
	if (!ownerChar) return false;

	if (!controller) controller = ownerChar->GetController();
	if (!controller) return false;

	if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
	if (!moveComp) return false;

	if (!camComp) camComp = ownerChar->FindComponentByClass<UCameraComponent>();
	if (!camComp) return false;

	if (!springArmComp) springArmComp = ownerChar->FindComponentByClass<USpringArmComponent>();
	if (!springArmComp) return false;

	if (!playerTargettingComp) playerTargettingComp = ownerChar->FindComponentByClass<UPlayerTargettingComponent>();
	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(ownerChar);

	return true;
}

bool UPlayerCamComponent::IsGrounded() const
{
	if (iCmbtInst) return iCmbtInst->IsGrounded();
	else if (moveComp) return moveComp->IsMovingOnGround();
	else return false;
}

FVector UPlayerCamComponent::GetActorFocusPoint(const AActor* Actor, float HeightOffset) const
{
	if (!Actor) return FVector::ZeroVector;
	return Actor->GetActorLocation() + FVector(0.0f, 0.0f, HeightOffset);
}

void UPlayerCamComponent::UpdateLockOnCam(float DeltaTime)
{
	if (!playerTargettingComp) return;

	AActor* target = playerTargettingComp->GetCurrentTarget();
	if (!target)
	{
		playerTargettingComp->LockOff();
		return;
	}

	const bool bGrounded = IsGrounded();

	const FVector playerLoc = ownerChar->GetActorLocation();
	const FVector targetLoc = target->GetActorLocation();

	// Player facing
	/*FRotator desiredActorRot = UKismetMathLibrary::FindLookAtRotation(playerLoc, targetLoc);
	desiredActorRot.Pitch = 0.0f;
	desiredActorRot.Roll = 0.0f;

	const FRotator newActorRot = FMath::RInterpTo(ownerChar->GetActorRotation(), desiredActorRot, DeltaTime, speedRot);
	ownerChar->SetActorRotation(newActorRot);*/

	// Camera focus solve
	const FVector playerFocus = GetActorFocusPoint(ownerChar, playerFocusHeight);
	const FVector targetFocus = GetActorFocusPoint(target, targetFocusHeight);
	const FVector focusPoint = FMath::Lerp(playerFocus, targetFocus, focusBiasToTarget);

	FRotator desiredCamRot = UKismetMathLibrary::FindLookAtRotation(camComp->GetComponentLocation(), focusPoint);
	desiredCamRot.Roll = 0.0f;

    // Prevents ugly extreme angles
    // Keeps camera readable in vertical combat
    desiredCamRot.Pitch = FMath::ClampAngle(desiredCamRot.Pitch, -20.0f, 35.0f);

	const FRotator newCamRot = FMath::RInterpTo(controller->GetControlRotation(), desiredCamRot, DeltaTime, speedRot);

	controller->SetControlRotation(newCamRot);

	// Zoom
	const float distance = FVector::Dist(playerLoc, targetLoc);
	const float desiredArmLength = FMath::Clamp(distance * lockOnZoomScale, springZoomMinLockOn, springZoomMaxLockOn);
	springArmComp->TargetArmLength = FMath::FInterpTo(springArmComp->TargetArmLength, desiredArmLength, DeltaTime, speedZoom);

	// Framing offset
	const FVector desiredOffset = bGrounded ? groundSpringOffsetLockOn : airSpringOffsetLockOn;
    springArmComp->SocketOffset = FMath::VInterpTo(springArmComp->SocketOffset, desiredOffset, DeltaTime, speedTrans);
}

void UPlayerCamComponent::UpdateLockOffCam(float DeltaTime)
{
	springArmComp->SocketOffset = FMath::VInterpTo(springArmComp->SocketOffset, normalSpringSocketOffset, DeltaTime, speedTrans);
	springArmComp->TargetArmLength = FMath::FInterpTo(springArmComp->TargetArmLength, normalSpringLength, DeltaTime, speedZoom);
}

void UPlayerCamComponent::AddLookMouseInput(const FVector2D &Look)
{
	if (!EnsureReferences()) return;
	else if (playerTargettingComp && playerTargettingComp->GetLockedOn()) return;
	else
	{
		UWorld* world = ownerChar->GetWorld();
		if (!world) return;

		const float DT = world->GetDeltaSeconds();

		ownerChar->AddControllerYawInput(Look.X * turnRate * DT);
		ownerChar->AddControllerPitchInput(Look.Y * lookUpRate * DT);
	}
}

void UPlayerCamComponent::AddLookStickInput(const FVector2D &Look)
{
	if (!EnsureReferences()) return;
	else if (playerTargettingComp && playerTargettingComp->GetLockedOn()) playerTargettingComp->LockOnBasedOnYaw(Look.X);
	else
	{
		UWorld* world = ownerChar->GetWorld();
		if (!world) return;

		const float DT = world->GetDeltaSeconds();

		ownerChar->AddControllerYawInput(Look.X * turnRate * DT);
		ownerChar->AddControllerPitchInput(Look.Y * lookUpRate * DT);
	}
}