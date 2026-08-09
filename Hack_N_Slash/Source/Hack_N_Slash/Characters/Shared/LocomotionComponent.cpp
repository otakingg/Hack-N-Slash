#include "LocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Interfaces/CombatInstigator.h"
#include "../../Controllers/EnemyController.h"
#include "../Player/PlayerInputComponent.h"
#include "../Shared/StateMachineComponent.h"

ULocomotionComponent::ULocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULocomotionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureReferences()) return;

    moveComp->GravityScale = gravity;
    RefreshMovementStats();
}

void ULocomotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld()) world->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

bool ULocomotionComponent::EnsureReferences()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ULocomotionComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ULocomotionComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

	if (!iCmbtInst) iCmbtInst = Cast<ICombatInstigator>(ownerChar);
	if (!iCmbtInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULocomotionComponent] Owner does not implement ICombatInstigator: %s"), *GetNameSafe(ownerChar));
		return false;
	}

    if (!stateMachineComp) stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
	if (!enemyController) enemyController = ownerChar->GetController<AEnemyController>();
    if (!motionWarpComp) motionWarpComp = ownerChar->FindComponentByClass<UMotionWarpingComponent>();
    if (!inputComp) inputComp = ownerChar->FindComponentByClass<UPlayerInputComponent>();
    if (!animInst) if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());

    return true;
}

/* ---------------- Movement Tuning ---------------- */
void ULocomotionComponent::RefreshMovementStats()
{
    if (!EnsureReferences() || !iCmbtInst || iCmbtInst->HasTag(Tags::Status::MoveStatsOverride, true)) return;

    moveComp->GravityScale = gravity;

    if (iCmbtInst->HasTag(Tags::StateMachine::Movement::Climb, true))
    {
        /* code */
    }
    else if (iCmbtInst->HasTag(Tags::StateMachine::Movement::Fall, true))
    {
        // Air control
        moveComp->AirControl = fallingAirControl;
        moveComp->AirControlBoostMultiplier = fallingAirControlBoostMult;
        moveComp->AirControlBoostVelocityThreshold = fallingAirControlBoostVeloctiyThreshold;

        // Braking behavior
        moveComp->BrakingDecelerationFalling = fallingBrakingDecelleration;

        // Friction
        moveComp->FallingLateralFriction = fallingLateralFriction;

        // Rotation
        moveComp->RotationRate = fallingRotationRate;
    }
    else if (iCmbtInst->HasTag(Tags::StateMachine::Movement::Fly, true))
    {
        moveComp->BrakingDecelerationFlying = flyingBrakingDecelleration;
        moveComp->RotationRate = flyingRotationRate;
    }
    else if (iCmbtInst->HasTag(Tags::StateMachine::Movement::Grind, true))
    {
        /* code */
    }
    else if (iCmbtInst->HasTag(Tags::StateMachine::Movement::Walk, true))
    {
        moveComp->BrakingDecelerationWalking = groundBrakingDecelleration;
        moveComp->GroundFriction = groundFriction;
        moveComp->RotationRate = groundRotationRate;
    }
    else if (iCmbtInst->HasTag(Tags::StateMachine::Movement::WallRun, true))
    {
        /* code */
    }
}

/* ---------------- Coyote Time ----------------*/
void ULocomotionComponent::UpdateLastGroundedTime()
{
    if (!EnsureReferences()) return;

    UWorld* world = GetWorld();
    if (!world) return;

    lastGroundedTime = world->GetTimeSeconds();

    if (bDebug && GEngine)
    {
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("%s: MarkGroundedNow"), *ClassName));
    }
}

bool ULocomotionComponent::CanCoyoteJump()
{
    UWorld* world = GetWorld();
    if (!world || !EnsureReferences()) return false;

    const float now = world->GetTimeSeconds();

    // By definition, coyote jump happens when airborne
    bool bAirborne = false;
    if (iCmbtInst) bAirborne = iCmbtInst->IsAirborne();
    else bAirborne = moveComp->IsFalling();

    // "Coyote" window: how recently we were grounded
    const bool bCoyote = (now - lastGroundedTime) <= coyoteSeconds;
    if (!bCoyote && bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Coyote time expired"));

    return bCoyote && bAirborne;
}

/* ---------------- Movement Actions ------------------------------*/
void ULocomotionComponent::Move(const FVector2D& Move)
{
    if (!EnsureReferences()) return;

    TArray<FGameplayTag> invalidTags = {Tags::Status::ActionBlocked::Move, Tags::Status::MovementLocked};
    if (iCmbtInst->HasAnyTag(invalidTags)) return;

    if (iCmbtInst->HasTag(Tags::Status::ActionCancelableBy::Move))
    {
        if (stateMachineComp) stateMachineComp->ClearActionState();

        if (animInst) animInst->Montage_Stop(0.25f);
        else ownerChar->StopAnimMontage();
    }

    FRotator ControlRot = ownerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll  = 0.f;

    const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
    const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

    ownerChar->AddMovementInput(Right,   Move.X);
    ownerChar->AddMovementInput(Forward, Move.Y);
}

void ULocomotionComponent::MoveTo(AActor* Target, const FVector Loc, const float AcceptanceRadius)
{   
    if (!EnsureReferences() || !enemyController) return;

    TArray<FGameplayTag> invalidTags = {Tags::Status::ActionBlocked::Move, Tags::Status::MovementLocked};
    if (iCmbtInst->HasAnyTag(invalidTags)) return;


    if (iCmbtInst->HasTag(Tags::Status::ActionCancelableBy::Move))
    {
        if (stateMachineComp) stateMachineComp->ClearActionState();

        if (animInst) animInst->Montage_Stop(0.25f);
        else ownerChar->StopAnimMontage();
    }

	if (Target) enemyController->MoveToActorHNS(Target, AcceptanceRadius);
	else enemyController->MoveToLocationHNS(Loc, AcceptanceRadius);
}

void ULocomotionComponent::JumpStart(bool bBuffer)
{
    if (!EnsureReferences() || (ownerChar->JumpCurrentCount >= ownerChar->JumpMaxCount)) return;

    if (stateMachineComp)
    {
        if (!stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(Tags::StateMachine::Action::Combat::Jump), false))
        {
            if (inputComp && !bBuffer) inputComp->SetActionBuffer(Tags::PlayerAction::JumpStart); // Only set a new buffer if this function isn't being called by a buffer
            return;
        }
        else if (inputComp) inputComp->ClearActionBuffer(); // Performing this action, so clear any buffered aciton if it exists
    }
    else
    {
        TArray<FGameplayTag> invalidTags = {Tags::Status::ActionBlocked::Jump, Tags::Status::MovementLocked};
        if (iCmbtInst->HasAnyTag(invalidTags)) return;
    }

    if (bDebug && GEngine)
    {
        // Success / action executed
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("%s: Jumping"), *ClassName));
    }

    if (iCmbtInst->HasTag(Tags::Status::ActionCancelableBy::Jump))
    {
        if (stateMachineComp) stateMachineComp->ClearActionState();

        if (animInst) animInst->Montage_Stop(0.25f);
        else ownerChar->StopAnimMontage();
    }

    moveComp->bNotifyApex = true;
    ownerChar->Jump();
    
    //if (animInst && ownerChar->JumpCurrentCount > 0 && doubleJumpMontage) animInst->PlayMontageHNS(doubleJumpMontage);

    if (CanCoyoteJump())
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Coyote Jumping"));
        --ownerChar->JumpCurrentCount;
    }
}

void ULocomotionComponent::JumpStop()
{
    if (!EnsureReferences()) return;
    ownerChar->StopJumping();
}

void ULocomotionComponent::LaunchCharacterHNS(FVector Velocity, bool OverrideXY, bool OverrideZ, float TimeToStop, AActor* Actor)
{
    if (Velocity == FVector::ZeroVector || !EnsureReferences() || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return;

	if (IsValid(Actor)) //If actor is valid, get buffered with respect to them
	{
		/*If the victim is above you, the “away” vector tilts upward
		{800, 0, 0} may give them upward knockback if they’re floating higher than the attacker
		This makes knockbacks feel more “physical” (closer to Smash Bros style), but can mess with combo control — sometimes enemies fly up or down in ways you didn’t intend.*/
		/*FVector dir = (GetActorLocation() - opponent->GetActorLocation()).GetSafeNormal();
		FRotator launchRot = dir.Rotation();
		FVector finalLaunch = launchRot.RotateVector(buffer);*/
		
		/*Calculating the direction like this means vertical movement only comes from velocity.Z
		Great for consistent hit-stun and aerial juggling — you control exactly how much “lift” happens
		This is how DMC5 and similar stylish games keep aerial combos predictable*/
        FVector dir = (ownerChar->GetActorLocation() - Actor->GetActorLocation()); //Get direction from actor to self
        dir.Z = 0.0f; //Flatten to XY
        dir = dir.GetSafeNormal(); //Use "GetSafeNormal" because if characters overlap perfectly, Normalize() can produce NaNs, which can corrupt velocity

        //Rotating the velocity is good design because it means designers can design knockback in local space. It'll be automatically transformed to world-space
		FRotator launchRot = dir.Rotation(); //Build a rotation where X points along that direction
		Velocity = launchRot.RotateVector(Velocity); //Transform the velocity from local-space into world-space
	}
	else //Else buffer with respect to self
	{
		FVector fwdVel = ownerChar->GetActorForwardVector() * Velocity.X;
		FVector sideVel = ownerChar->GetActorRightVector() * Velocity.Y;
		FVector vertVel = ownerChar->GetActorUpVector() * Velocity.Z;
		Velocity = fwdVel + sideVel + vertVel;
	}

    if (Velocity.Z > 5.0f) moveComp->bNotifyApex = true;
	ownerChar->LaunchCharacter(Velocity, OverrideXY, OverrideZ);

    UWorld* world {ownerChar->GetWorld()};
    if (!world) return;

	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(world, TH_StopMovement)) UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(world, TH_StopMovement);
	if (TimeToStop > 0.0f) world->GetTimerManager().SetTimer(TH_StopMovement, moveComp, &UCharacterMovementComponent::StopMovementImmediately, TimeToStop, false);
}

void ULocomotionComponent::GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, bool bLockedOn) const
{
	if (!ownerChar || !Target) return;

	FVector ownerLoc = ownerChar->GetActorLocation();
	FVector targetLoc = Target->GetActorLocation();
	double distance = FVector::Dist(ownerLoc, targetLoc);

    // Decides whether to warp translation and/or rotation
    bool bWarpRotation = !bLockedOn;
    bool bWarpTranslation = (WarpOffset > 0.0f) && (distance > WarpOffset);

    // Calculates potential warp location
    FVector dirVec = ownerLoc - targetLoc;
    FVector dirVecNorm = UKismetMathLibrary::Normal(dirVec);
    WarpLoc = bWarpTranslation ? (dirVecNorm * WarpOffset) + targetLoc : ownerLoc;

    // Calculates potential warp rotation
    if (bWarpRotation)
    {
        WarpRot = UKismetMathLibrary::FindLookAtRotation(ownerLoc, targetLoc);
        WarpRot.Pitch = 0.0f;
        WarpRot.Roll = 0.0f;
    }
    else WarpRot = ownerChar->GetActorRotation();
}

void ULocomotionComponent::GetWarpingLocRotFreeFlow(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FVector2D& InputDir, bool bLockedOn) const
{
	if (!ownerChar || !Target) return;

	FVector ownerLoc = ownerChar->GetActorLocation();
	FVector targetLoc = Target->GetActorLocation();
	double distance = FVector::Dist(ownerLoc, targetLoc);

    // Decides whether to warp translation and/or rotation
    bool bWarpRotation = !bLockedOn;
    bool bWarpTranslation = (WarpOffset > 0.0f) && (distance > WarpOffset) && !InputDir.IsNearlyZero() && !bLockedOn;

    // Calculates potential warp location
    FVector dirVec = ownerLoc - targetLoc;
    FVector dirVecNorm = UKismetMathLibrary::Normal(dirVec);
    WarpLoc = bWarpTranslation ? (dirVecNorm * WarpOffset) + targetLoc : ownerLoc;

    // Calculates potential warp rotation
    if (bWarpRotation)
    {
        WarpRot = UKismetMathLibrary::FindLookAtRotation(ownerLoc, targetLoc);
        WarpRot.Pitch = 0.0f;
        WarpRot.Roll = 0.0f;
    }
    else WarpRot = ownerChar->GetActorRotation();
}

void ULocomotionComponent::UpdateMotionWarpData(const FVector& DesiredLoc, const FRotator& DesiredRot)
{
    if (!EnsureReferences() || !motionWarpComp) return;

    if (DesiredRot == ownerChar->GetActorRotation()) motionWarpComp->RemoveWarpTarget(TEXT("Target_Rot"));
    else motionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("Target_Rot"), DesiredLoc, DesiredRot);

    if (DesiredLoc == ownerChar->GetActorLocation()) motionWarpComp->RemoveWarpTarget(TEXT("Target_Transl"));
    else motionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("Target_Transl"), DesiredLoc, DesiredRot);
}

void ULocomotionComponent::ClearMotionWarpData() { if (motionWarpComp) motionWarpComp->RemoveAllWarpTargets(); }

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceConstant(float Duration, FVector Force, FVector VelocityOnFinish, float ClampVelocityOnFinish, ERootMotionFinishVelocityMode VelocityOnFinishMode, UCurveFloat* StrengthOverTime, bool bAdditive)
{
    if (Force.IsNearlyZero() || Duration <= 0.0f || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return nullptr;

    UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_ConstantForce(
        ownerChar,
        moveComp,
        Force,
        Duration,
        bAdditive,
        StrengthOverTime,
        VelocityOnFinishMode,
        VelocityOnFinish,
        ClampVelocityOnFinish
    );

    if (tempRootMovement)
    {
        if (bAdditive)
        {
            asyncRootMotionsAdditive.Add(tempRootMovement);
            tempRootMovement->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
            tempRootMovement->Activate();
            return tempRootMovement;
        }
        else
        {
            ClearRootMotionSource(asyncRootMotionOverride);
            asyncRootMotionOverride = tempRootMovement;
            asyncRootMotionOverride->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
            asyncRootMotionOverride->Activate();
            return asyncRootMotionOverride;
        }
    }
    else return nullptr;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceJump(FVector Direction, float Distance, float Height, float Duration, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish)
{
    if (Distance <= 0.0f || Duration <= 0.0f || Height <= 0.0f || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return nullptr;

    UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_JumpForce(
        ownerChar,
        moveComp,
        Direction,
        Distance,
        Height,
        Duration,
        VelocityOnFinishMode,
        SetVelocityOnFinish,
        ClampVelocityOnFinish
    );

    if (tempRootMovement)
    {
        asyncRootMotionsAdditive.Add(tempRootMovement);
        tempRootMovement->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
        tempRootMovement->Activate();
        return tempRootMovement;
    }
    else return nullptr;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceMoveTo(FVector StartLoc, FVector TargetLoc, float Duration, bool bRestrictSpeedToExpected)
{
    if (Duration <= 0.0f || StartLoc.Equals(TargetLoc) || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return nullptr;

    UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_MoveTo(
        ownerChar,
        moveComp,
        StartLoc,
        TargetLoc,
        Duration,
        bRestrictSpeedToExpected
    );

    if (tempRootMovement)
    {
        ClearRootMotionSource(asyncRootMotionOverride);
        asyncRootMotionOverride = tempRootMovement;
        asyncRootMotionOverride->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
        asyncRootMotionOverride->Activate();
        return asyncRootMotionOverride;
    }
    return nullptr;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceMoveToDynamic(FVector StartLoc, FVector InitTargetLoc, float Duration, bool bRestrictSpeedToExpected)
{
    if (Duration <= 0.0f || StartLoc.Equals(InitTargetLoc) || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return nullptr;

    UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_MoveToDynamic(
        ownerChar,
        moveComp,
        StartLoc,
        InitTargetLoc,
        Duration,
        bRestrictSpeedToExpected
    );

    if (tempRootMovement)
    {
        ClearRootMotionSource(asyncRootMotionOverride);
        asyncRootMotionOverride = tempRootMovement;
        asyncRootMotionOverride->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
        asyncRootMotionOverride->Activate();
        return asyncRootMotionOverride;
    }
    return nullptr;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceRadial(FVector Origin, float Radius, float Strength, float Duration, bool bIsPush, UCurveFloat* StrengthOverTime)
{
    if (Radius <= 0.0f || Strength <= 0.0f || Duration <= 0.0f || iCmbtInst->HasTag(Tags::Status::MovementLocked)) return nullptr;

    UAsyncRootMovement* tempRootMovement = UAsyncRootMovement::AsyncRootMovement_RadialForce(
        ownerChar,
        moveComp,
        Origin,
        Radius,
        Strength,
        Duration,
        bIsPush,
        StrengthOverTime
    );
    
    if (tempRootMovement)
    {
        asyncRootMotionsAdditive.Add(tempRootMovement);
        tempRootMovement->OnComplete.AddDynamic(this, &ULocomotionComponent::OnRootMotionComplete);
        tempRootMovement->Activate();
        return tempRootMovement;
    }
    else return nullptr;
}

void ULocomotionComponent::OnRootMotionComplete(UAsyncRootMovement* RootMotion)
{
    if (RootMotion == asyncRootMotionOverride) asyncRootMotionOverride = nullptr;
    else
    {
        for (UAsyncRootMovement* rootMove : asyncRootMotionsAdditive)
        {
            if (RootMotion == rootMove)
            {
                UAsyncRootMovement* temp = rootMove;
                asyncRootMotionsAdditive.Remove(rootMove);
                temp = nullptr;
            }
        }
    }
}

void ULocomotionComponent::ClearRootMotionSource(UAsyncRootMovement* RootMotion)
{
    if (!RootMotion) return;
    else if (RootMotion == asyncRootMotionOverride)
    {
        RootMotion->Cancel();
        asyncRootMotionOverride = nullptr;
    }
    else
    {
        for (UAsyncRootMovement* rootMove : asyncRootMotionsAdditive)
        {
            UAsyncRootMovement* temp = rootMove;
            asyncRootMotionsAdditive.Remove(rootMove);
            temp->Cancel();
            temp = nullptr;
        }
    }
}

void ULocomotionComponent::ClearAllRootMotionSources()
{
    if (asyncRootMotionOverride)
    {
        asyncRootMotionOverride->Cancel();
        asyncRootMotionOverride = nullptr;
    }

    for (UAsyncRootMovement* rootMove : asyncRootMotionsAdditive)
    {
        if (!rootMove) continue;
        rootMove->Cancel();
        rootMove = nullptr;
    }
    asyncRootMotionsAdditive.Empty();
}

void ULocomotionComponent::AddRootMotionSource(UAsyncRootMovement* RootMotion, bool bAdditive)
{
    if (!RootMotion) return;
    else if (bAdditive) asyncRootMotionsAdditive.Add(RootMotion);
    else
    {
        ClearRootMotionSource(asyncRootMotionOverride);
        asyncRootMotionOverride = RootMotion;
    }
}