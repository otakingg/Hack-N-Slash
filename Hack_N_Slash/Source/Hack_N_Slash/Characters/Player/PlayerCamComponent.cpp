#include "PlayerCamComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"

#include "../../Interfaces/CombatInstigator.h"
#include "../../Interfaces/PlayerInt.h"
#include "../Shared/StateMachineComponent.h"

// Sets default values for this component's properties
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
    if (!owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

	if (!controller) controller = owner->GetController();
	if (!controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] Owner controller isn't valid: %s"), *GetNameSafe(GetOwner()));
		return false;
	}

    if (!moveComp) moveComp = owner->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(owner));
        return false;
    }

    if (!springArmComp) springArmComp = owner->FindComponentByClass<USpringArmComponent>();
    if (!springArmComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] No SpringArmComponent on: %s"), *GetNameSafe(owner));
        return false;
    }

    if (!camComp) camComp = owner->FindComponentByClass<UCameraComponent>();
    if (!camComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] No CameraComponent on: %s"), *GetNameSafe(owner));
        return false;
    }

	if (!iPlayer) iPlayer = Cast<IPlayerInt>(owner);
	if (!iPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UPlayerCamComponent] %s doesn't implement 'Player' interface"), *GetNameSafe(owner));
		return false;
	}

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(owner);
	if (!stateMachineComp) stateMachineComp = owner->FindComponentByClass<UStateMachineComponent>();

    return true;
}

void UPlayerCamComponent::UpdateLockOnCam(float DeltaTime)
{
	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(owner);
	if (!iCmbtInst) return;

	AActor* target = iCmbtInst->GetCurrentTarget();
	if (!target) return;

	bool bGrounded = (stateMachineComp && stateMachineComp->IsGrounded()) || moveComp->IsMovingOnGround();
	FVector currentLoc = owner->GetActorLocation();
	FVector targetLoc = target->GetActorLocation();
	
	/**********************************************************PLAYER*********************************************************************/
	FRotator newActorRot = UKismetMathLibrary::FindLookAtRotation(currentLoc, targetLoc);
	newActorRot.Pitch = 0.0f;
	owner->SetActorRotation(FMath::RInterpTo(owner->GetActorRotation(), newActorRot, DeltaTime, speedRot));
	/**********************************************************PLAYER*********************************************************************/

	/**********************************************************CAMERA*********************************************************************/
    //Midpoint between player and target
    FVector midPoint = (currentLoc + targetLoc) * 0.5f;

    //Blend toward enemy: mostly enemy, some midpoint for framing
    FVector lookAtTarget = FMath::Lerp(targetLoc, midPoint, midPointBias);

    //Ignore vertical swing if you want less jitter on uneven ground
    //lookAtTarget.Z = targetLoc.Z;

    //Desired rotation to look at midpoint
    FRotator targetRot = UKismetMathLibrary::FindLookAtRotation(camComp->GetComponentLocation(), lookAtTarget);

    //Smooth interpolation of camera boom rotation
    FRotator newCamRot = FMath::RInterpTo(controller->GetControlRotation(), targetRot, DeltaTime, speedRot);
    controller->SetControlRotation(newCamRot);

    //Ground vs Air pitch offset
	float targetPitchOffset = bGrounded ? groundSpringPitchOffsetLockOn : airSpringPitchOffsetLockOn;
    FRotator controlRot = controller->GetControlRotation();
    controlRot.Pitch = FMath::FInterpTo(controlRot.Pitch, targetPitchOffset, DeltaTime, speedRot);
    controller->SetControlRotation(controlRot);

    //Adjust arm length based on distance
    double distance = FVector::Dist(currentLoc, targetLoc);
    float desiredArmLength = FMath::Clamp(distance * 0.6f, springZoomMinLockOn, springZoomMaxLockOn);
    springArmComp->TargetArmLength = FMath::FInterpTo(springArmComp->TargetArmLength, desiredArmLength, DeltaTime, speedZoom);

    //Adjust socket offset
    FVector desiredOffset = bGrounded? groundSpringOffsetLockOn : airSpringOffsetLockOn;
    springArmComp->SocketOffset = FMath::VInterpTo(springArmComp->SocketOffset, desiredOffset, DeltaTime, speedTrans);
}

void UPlayerCamComponent::UpdateLockOffCam(float DeltaTime)
{
	//Interp the spring arm offset to the normal offset
	springArmComp->SocketOffset = UKismetMathLibrary::VInterpTo_Constant(springArmComp->SocketOffset, normalSpringSocketOffset, DeltaTime, speedTrans);

	//Interp the spring arm length to the normal length
	springArmComp->TargetArmLength = UKismetMathLibrary::FInterpTo_Constant(springArmComp->TargetArmLength, normalSpringLength, DeltaTime, speedZoom);
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