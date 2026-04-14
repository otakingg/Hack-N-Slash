#include "PlayerCamComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/PlayerInt.h"
#include "../Shared/StateMachineComponent.h"

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

	if (iPlayer->GetLockedOn()) UpdateLockOnCam(DeltaTime);
	else UpdateLockOffCam(DeltaTime);
}

bool UPlayerCamComponent::EnsureReferences()
{
	if (!owner) owner = Cast<ACharacter>(GetOwner());
	if (!owner) return false;

	if (!controller) controller = owner->GetController();
	if (!controller) return false;

	if (!moveComp) moveComp = owner->GetCharacterMovement();
	if (!moveComp) return false;

	if (!camComp) camComp = owner->FindComponentByClass<UCameraComponent>();
	if (!camComp) return false;

	if (!springArmComp) springArmComp = owner->FindComponentByClass<USpringArmComponent>();
	if (!springArmComp) return false;

	if (!stateMachineComp) stateMachineComp = owner->FindComponentByClass<UStateMachineComponent>();

	if (!iPlayer) iPlayer = Cast<IPlayerInt>(owner);
	if (!iPlayer) return false;

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(owner);

	return true;
}

bool UPlayerCamComponent::IsGrounded() const
{
	const bool bStateGrounded = stateMachineComp && stateMachineComp->IsGrounded();
	const bool bMoveGrounded = moveComp && moveComp->IsMovingOnGround();
	return bStateGrounded || bMoveGrounded;
}

FVector UPlayerCamComponent::GetActorFocusPoint(const AActor* Actor, float HeightOffset) const
{
	if (!Actor) return FVector::ZeroVector;
	return Actor->GetActorLocation() + FVector(0.0f, 0.0f, HeightOffset);
}

void UPlayerCamComponent::UpdateLockOnCam(float DeltaTime)
{
    if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(owner);
	if (!iCmbtInst) return;

	AActor* target = iCmbtInst->GetCurrentTarget();
	if (!target) return;

	const bool bGrounded = IsGrounded();

	const FVector playerLoc = owner->GetActorLocation();
	const FVector targetLoc = target->GetActorLocation();

	// Player facing
	{
		FRotator desiredActorRot = UKismetMathLibrary::FindLookAtRotation(playerLoc, targetLoc);
		desiredActorRot.Pitch = 0.0f;
		desiredActorRot.Roll = 0.0f;

		const FRotator newActorRot = FMath::RInterpTo(owner->GetActorRotation(), desiredActorRot, DeltaTime, speedRot);
		owner->SetActorRotation(newActorRot);
	}

	// Camera focus solve
	const FVector playerFocus = GetActorFocusPoint(owner, playerFocusHeight);
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

void UPlayerCamComponent::AddLookInputScaled(const FVector2D& Look)
{
	if (!EnsureReferences() || iPlayer->GetLockedOn()) return;

	UWorld* world = owner->GetWorld();
	if (!world) return;

	const float DT = world->GetDeltaSeconds();

	owner->AddControllerYawInput(Look.X * turnRate * DT);
	owner->AddControllerPitchInput(Look.Y * lookUpRate * DT);
}