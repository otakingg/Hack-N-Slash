// Fill out your copyright notice in the Description page of Project Settings.
#include "Player_Base.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Characters/StateMachineComponent.h"

APlayer_Base::APlayer_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	movementComp = GetCharacterMovement();
	stateMachineComp = CreateDefaultSubobject<UStateMachineComponent>(TEXT("State Machine"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	movementComp->bOrientRotationToMovement = true; //Set this to false when locked on and strafing is desired
	movementComp->bUseControllerDesiredRotation = false;
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	camComp = FindComponentByClass<UCameraComponent>();
	if (IsValid(camComp)) {camComp->bUsePawnControlRotation = false;}
	
	springArmComp = FindComponentByClass<USpringArmComponent>();
}

void APlayer_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayer_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/************************************Protected Functions************************************/
void APlayer_Base::Input_AttackHeavy(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->OnInputAttackPressed(InputVector);
}

void APlayer_Base::Input_AttackLight(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->OnInputAttackPressed(InputVector);
}

void APlayer_Base::Input_BlockDodge(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->OnInputBlockDodgePressed(InputVector);
}

void APlayer_Base::Input_JumpPressed()
{
	if (stateMachineComp) stateMachineComp->OnInputJumpPressed();
}

void APlayer_Base::Input_JumpReleased()
{
	if (stateMachineComp) stateMachineComp->OnInputJumpReleased();
}

void APlayer_Base::Input_Look(const FVector2D &InputVector)
{
	if (stateMachineComp) stateMachineComp->OnInputLook(InputVector);
}

void APlayer_Base::Input_Move(const FVector2D& InputVector)
{
	if (stateMachineComp) stateMachineComp->OnInputMove(InputVector);
}
/************************************Protected Functions************************************/
/************************************Public Functions************************************/
/************************************Public Functions************************************/