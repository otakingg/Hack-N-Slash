#include "LocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

#include "../../Animation/AnimInstances/BaseCharAnimInstance.h"
#include "../../Tags/CharacterStateTagNamespaces.h"
#include "../../Controllers/EnemyController.h"
#include "../../Tags/LocomotionTags.h"
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
    activeMoveProfile = ProfileTags::Grounded; // Safe default
    ApplyMovementFromTagsAndStats();
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

    if (!animInst)
    {
        if (USkeletalMeshComponent* skeletalMeshComp = ownerChar->GetMesh()) animInst = Cast<UBaseCharAnimInstance>(skeletalMeshComp->GetAnimInstance());
    }
    if (!stateMachineComp) stateMachineComp = ownerChar->FindComponentByClass<UStateMachineComponent>();
	if (!controller) controller = ownerChar->GetController<AEnemyController>();
    if (!motionWarpComp) motionWarpComp = ownerChar->FindComponentByClass<UMotionWarpingComponent>();

    return true;
}

bool ULocomotionComponent::HasOverrideExact(const FGameplayTag& Tag) const { return Tag.IsValid() && moveOverrides.HasTagExact(Tag); }

void ULocomotionComponent::ApplyMovementFromTagsAndStats()
{
    if (!EnsureReferences() || HasOverrideExact(OverrideTags::MoveStats) || !activeMoveProfile.IsValid()) return;

    moveComp->GravityScale = gravity;

    if (activeMoveProfile.MatchesTagExact(ProfileTags::Grounded))
    {
        moveComp->BrakingDecelerationWalking = groundBrakingDecelleration;
        moveComp->GroundFriction = groundFriction;
        moveComp->RotationRate = groundRotationRate;
    }
    else if (activeMoveProfile.MatchesTagExact(ProfileTags::Grind))
    {
        /* code */
    }
    else if (activeMoveProfile.MatchesTagExact(ProfileTags::Climb))
    {
        /* code */
    }
    else if (activeMoveProfile.MatchesTagExact(ProfileTags::WallRun))
    {
        /* code */
    }
    else if (activeMoveProfile.MatchesTagExact(ProfileTags::Falling))
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
    else if (activeMoveProfile.MatchesTagExact(ProfileTags::Fly))
    {
        moveComp->BrakingDecelerationFlying = flyingBrakingDecelleration;
        moveComp->RotationRate = flyingRotationRate;
    }
}

/***************************************** Locomotion Command Interface *****************************************/
void ULocomotionComponent::SetMoveProfileTag(const FGameplayTag& NewProfile)
{
    if (!NewProfile.IsValid() || activeMoveProfile == NewProfile) return;

    activeMoveProfile = NewProfile;
    ApplyMovementFromTagsAndStats();
}

void ULocomotionComponent::AddMoveOverrideTag(const FGameplayTag& OverrideTag)
{
    if (!OverrideTag.IsValid() || moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.AddTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void ULocomotionComponent::RemoveMoveOverrideTag(const FGameplayTag& OverrideTag)
{
    if (!OverrideTag.IsValid() || !moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.RemoveTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void ULocomotionComponent::RefreshMovement() { ApplyMovementFromTagsAndStats(); }

void ULocomotionComponent::SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode)
{
    if (!EnsureReferences()) return;
    moveComp->SetMovementMode(NewMode, CustomMode);
}

bool ULocomotionComponent::CanCoyoteJump()
{
    UWorld* World = ownerChar->GetWorld();
    if (!World || !EnsureReferences()) return false;

    const float Now = World->GetTimeSeconds();

    // By definition, coyote jump happens when airborne
    bool bAirborne = false;
    if (stateMachineComp) bAirborne = stateMachineComp->IsAirborne();
    else bAirborne = moveComp->IsFalling();

    // "Coyote" window: how recently we were grounded
    const bool bCoyote = (Now - lastGroundedTime) <= coyoteSeconds;
    if (!bCoyote && bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Coyote time expired.\nNow = %f\nLast Ground = %f\nCoyote Seconds = %f"), Now, lastGroundedTime, coyoteSeconds));

    return bCoyote && bAirborne;
}

void ULocomotionComponent::MarkGroundedNow()
{
    if (bDebug && GEngine)
    {
        // Informational
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("%s: MarkGroundedNow"), *ClassName));
    }

    if (!ownerChar) return;
    if (UWorld* World = ownerChar->GetWorld()) lastGroundedTime = World->GetTimeSeconds();
}

void ULocomotionComponent::AddMoveInput(const FVector2D& Move)
{
    if (!EnsureReferences() || HasOverrideExact(OverrideTags::Lock)) return;

    FRotator ControlRot = ownerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll  = 0.f;

    const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
    const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

    ownerChar->AddMovementInput(Right,   Move.X);
    ownerChar->AddMovementInput(Forward, Move.Y);
}

void ULocomotionComponent::AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] AddMoveInput: Entered"), *GetNameSafe(this));
    
    if (!EnsureReferences() || HasOverrideExact(OverrideTags::Lock) || !controller) return;

	if (Target) controller->MoveToActorHNS(Target, AcceptanceRadius);
	else controller->MoveToLocationHNS(Loc, AcceptanceRadius);
}

void ULocomotionComponent::JumpStart()
{
    if (!EnsureReferences() || HasOverrideExact(OverrideTags::NoJump) || (ownerChar->JumpCurrentCount >= ownerChar->JumpMaxCount)) return;

    if (bDebug && GEngine)
    {
        // Success / action executed
        const FString ClassName = GetNameSafe(this);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("%s: Jumping"), *ClassName));
    }

    moveComp->bNotifyApex = true;
    ownerChar->Jump();
    
    if (animInst && ownerChar->JumpCurrentCount > 0 && doubleJumpMontage) animInst->PlayMontageHNS(doubleJumpMontage);
    else if (animInst && jumpMontage) animInst->PlayMontageHNS(jumpMontage);

    if (CanCoyoteJump())
    {
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Coyote Jumping"));
        --ownerChar->JumpCurrentCount;
    }

    UWorld* world = ownerChar->GetWorld();
    if (!world || !stateMachineComp) return;

    stateMachineComp->ChangeActionState(stateMachineComp->GetActionStateByTag(CombatTags::Jump), false);
}

void ULocomotionComponent::JumpStop()
{
    if (!EnsureReferences()) return;
    ownerChar->StopJumping();
}

void ULocomotionComponent::LaunchCharacterHNS(FVector Velocity, bool OverrideXY, bool OverrideZ, float TimeToStop, AActor* Actor)
{
    if (Velocity == FVector::ZeroVector || !EnsureReferences()) return;

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

void ULocomotionComponent::GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, bool bLockedOn)
{
	if (!EnsureReferences() || !Target) return;

	FVector ownerLoc = ownerChar->GetActorLocation();
	FVector targetLoc = Target->GetActorLocation();
	double distance = FVector::Dist(ownerLoc, targetLoc);

    // Decides whether to warp translation and/or rotation
    bWarpRotation = !bLockedOn;
    bWarpTranslation = (distance > WarpOffset);

    // Calculates potential warp location
    FVector dirVec = ownerLoc - targetLoc;
    FVector dirVecNorm = UKismetMathLibrary::Normal(dirVec);
    WarpLoc = (dirVecNorm * WarpOffset) + targetLoc;

    // Calculates potential warp rotation
    WarpRot = UKismetMathLibrary::FindLookAtRotation(ownerLoc, targetLoc);
    WarpRot.Pitch = 0.0f;
    WarpRot.Roll = 0.0f;
}

void ULocomotionComponent::GetWarpingLocRot(AActor* Target, FVector& WarpLoc, FRotator& WarpRot, float WarpOffset, const FVector2D& InputDir, bool bLockedOn)
{
	if (!EnsureReferences() || !Target) return;

	FVector ownerLoc = ownerChar->GetActorLocation();
	FVector targetLoc = Target->GetActorLocation();
	double distance = FVector::Dist(ownerLoc, targetLoc);

    // Decides whether to warp translation and/or rotation
    bWarpRotation = !bLockedOn;
    bWarpTranslation = (distance > WarpOffset) && !InputDir.IsNearlyZero() && !bLockedOn;

    // Calculates potential warp location
    FVector dirVec = ownerLoc - targetLoc;
    FVector dirVecNorm = UKismetMathLibrary::Normal(dirVec);
    WarpLoc = (dirVecNorm * WarpOffset) + targetLoc;

    // Calculates potential warp rotation
    WarpRot = UKismetMathLibrary::FindLookAtRotation(ownerLoc, targetLoc);
    WarpRot.Pitch = 0.0f;
    WarpRot.Roll = 0.0f;
}

void ULocomotionComponent::UpdateMotionWarpData(const FVector& DesiredLoc, const FRotator& DesiredRot)
{
    if (!EnsureReferences() || !motionWarpComp) return;
    if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("[LocomotionComp] Motion Warping"));
    
    static const FName rotationWarpName(TEXT("Target_Rotation"));
    static const FName translationWarpName(TEXT("Target_Translation"));

    if (bWarpRotation) motionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(rotationWarpName, DesiredLoc, DesiredRot);
    else motionWarpComp->RemoveWarpTarget(rotationWarpName);

    if (bWarpTranslation) motionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(translationWarpName, DesiredLoc, DesiredRot);
    else motionWarpComp->RemoveWarpTarget(translationWarpName);
}

void ULocomotionComponent::ClearMotionWarpData()
{
	if (motionWarpComp) motionWarpComp->RemoveAllWarpTargets();
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceConstant(float Duration, FVector Force, FVector VelocityOnFinish, float ClampVelocityOnFinish, ERootMotionFinishVelocityMode VelocityOnFinishMode, UCurveFloat* StrengthOverTime, bool bAdditive)
{
    if (Force.IsNearlyZero() || Duration <= 0.0f) return nullptr;

    ClearRootMotionSource();

    activeAsyncRootMotion = UAsyncRootMovement::AsyncRootMovement_ConstantForce(
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

    if (activeAsyncRootMotion) activeAsyncRootMotion->Activate();
    return activeAsyncRootMotion;
}

UAsyncRootMovement *ULocomotionComponent::ApplyRootMotionSourceJump(FVector Direction, float Distance, float Height, float Duration, ERootMotionFinishVelocityMode VelocityOnFinishMode, FVector SetVelocityOnFinish, float ClampVelocityOnFinish)
{
    if (Distance <= 0.0f || Duration <= 0.0f || Height <= 0.0f) return nullptr;

    ClearRootMotionSource();

    activeAsyncRootMotion = UAsyncRootMovement::AsyncRootMovement_JumpForce(
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

    if (activeAsyncRootMotion) activeAsyncRootMotion->Activate();
    return activeAsyncRootMotion;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceMoveTo(FVector StartLoc, FVector TargetLoc, float Duration, bool bRestrictSpeedToExpected)
{
    if (Duration <= 0.0f || StartLoc.Equals(TargetLoc)) return nullptr;

    ClearRootMotionSource();

    activeAsyncRootMotion = UAsyncRootMovement::AsyncRootMovement_MoveTo(
        ownerChar,
        moveComp,
        StartLoc,
        TargetLoc,
        Duration,
        bRestrictSpeedToExpected
    );

    if (activeAsyncRootMotion) activeAsyncRootMotion->Activate();
    return activeAsyncRootMotion;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceMoveToDynamic(FVector StartLoc, FVector InitTargetLoc, float Duration, bool bRestrictSpeedToExpected)
{
    if (Duration <= 0.0f) return nullptr;

    ClearRootMotionSource();

    activeAsyncRootMotion = UAsyncRootMovement::AsyncRootMovement_MoveToDynamic(
        ownerChar,
        moveComp,
        StartLoc,
        InitTargetLoc,
        Duration,
        bRestrictSpeedToExpected
    );

    if (activeAsyncRootMotion) activeAsyncRootMotion->Activate();
    return activeAsyncRootMotion;
}

UAsyncRootMovement* ULocomotionComponent::ApplyRootMotionSourceRadial(FVector Origin, float Radius, float Strength, float Duration, bool bIsPush, UCurveFloat* StrengthOverTime)
{
    if (Radius <= 0.0f || Strength <= 0.0f || Duration <= 0.0f) return nullptr;
    
    ClearRootMotionSource();

    activeAsyncRootMotion = UAsyncRootMovement::AsyncRootMovement_RadialForce(
        ownerChar,
        moveComp,
        Origin,
        Radius,
        Strength,
        Duration,
        bIsPush,
        StrengthOverTime
    );
    
    if (activeAsyncRootMotion) activeAsyncRootMotion->Activate();
    return activeAsyncRootMotion;
}

void ULocomotionComponent::ClearRootMotionSource()
{
    if (!EnsureReferences() || !activeAsyncRootMotion) return;
    
    activeAsyncRootMotion->Cancel();
    activeAsyncRootMotion = nullptr;
}