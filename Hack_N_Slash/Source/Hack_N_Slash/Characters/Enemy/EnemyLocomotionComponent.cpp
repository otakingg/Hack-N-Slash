#include "EnemyLocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "../../Controllers/EnemyController.h"
#include "../../Tags/LocomotionTags.h"
#include "../StatsComponent.h"

// Sets default values for this component's properties
UEnemyLocomotionComponent::UEnemyLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyLocomotionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureOwnerCharacter()) return;

    moveComp->GravityScale = gravity;
    activeMoveProfile = TAG_Move_Profile_Grounded; // Safe default
    ApplyMovementFromTagsAndStats();
}

bool UEnemyLocomotionComponent::EnsureOwnerCharacter()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyLocomotionComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

	if (!controller) controller = ownerChar->GetController<AEnemyController>();
	if (!controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UEnemyLocomotionComponent] AEnemyController not an ACharacter: %s"), *GetNameSafe(GetOwner()));
		return false;
	}

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UEnemyLocomotionComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    // Stats are optional (lets you prototype without wiring everything)
    if (!statsComp) statsComp = ownerChar->FindComponentByClass<UStatsComponent>();

    return true;
}

bool UEnemyLocomotionComponent::HasOverrideExact(const FGameplayTag& Tag) const { return Tag.IsValid() && moveOverrides.HasTagExact(Tag); }

void UEnemyLocomotionComponent::ApplyMovementFromTagsAndStats()
{
    if (!EnsureOwnerCharacter() || HasOverrideExact(TAG_Move_Override_MoveStats) || !activeMoveProfile.IsValid()) return;

    moveComp->GravityScale = gravity;

    if (activeMoveProfile.MatchesTagExact(TAG_Move_Profile_Grounded))
    {
        moveComp->BrakingDecelerationWalking = groundBrakingDecelleration;
        moveComp->GroundFriction = groundFriction;
        moveComp->RotationRate = groundRotationRate;
    }
    else if (activeMoveProfile.MatchesTagExact(TAG_Move_Profile_Grind))
    {
        /* code */
    }
    else if (activeMoveProfile.MatchesTagExact(TAG_Move_Profile_Climb))
    {
        /* code */
    }
    else if (activeMoveProfile.MatchesTagExact(TAG_Move_Profile_Falling))
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
    else if (activeMoveProfile.MatchesTagExact(TAG_Move_Profile_Fly))
    {
        moveComp->BrakingDecelerationFlying = flyingBrakingDecelleration;
        moveComp->RotationRate = flyingRotationRate;
    }
}

void UEnemyLocomotionComponent::StopLaunch()
{
    if (!EnsureOwnerCharacter()) return;
    moveComp->StopMovementImmediately();
}

/***************************************** Locomotion Command Interface *****************************************/
void UEnemyLocomotionComponent::SetMoveProfileTag(const FGameplayTag& NewProfile)
{
    if (!NewProfile.IsValid() || activeMoveProfile == NewProfile) return;

    activeMoveProfile = NewProfile;
    ApplyMovementFromTagsAndStats();
}

void UEnemyLocomotionComponent::AddMoveOverrideTag(const FGameplayTag& OverrideTag)
{
    if (!OverrideTag.IsValid() || moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.AddTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void UEnemyLocomotionComponent::RemoveMoveOverrideTag(const FGameplayTag& OverrideTag)
{
    if (!OverrideTag.IsValid() || !moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.RemoveTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void UEnemyLocomotionComponent::RefreshMovement() { ApplyMovementFromTagsAndStats(); }

void UEnemyLocomotionComponent::SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode)
{
    if (!EnsureOwnerCharacter()) return;
    moveComp->SetMovementMode(NewMode, CustomMode);
}

bool UEnemyLocomotionComponent::CanUseBufferedJump(bool& bWantsJump, float& JumpPressedTime) const
{
    if (!ownerChar || !moveComp) return false;
    UWorld* World = ownerChar->GetWorld();
    if (!World) return false;

    const float Now = World->GetTimeSeconds();

    // Must have a recorded press
    if (!bWantsJump || JumpPressedTime < 0.f) return false;

    // "Buffer" window: how recent the press was
    const bool bBuffered = (Now - JumpPressedTime) <= jumpBufferSeconds;
    if (!bBuffered && bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Jump buffer expired"));

    // "Coyote" window: how recently we were grounded
    const bool bGroundOrCoyote = moveComp->IsMovingOnGround() || ((Now - lastGroundedTime) <= coyoteSeconds);
    if (!bGroundOrCoyote && bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Coyote time expired.\nNow = %f\nLast Ground = %f\nCoyote Seconds = %f"), Now, lastGroundedTime, coyoteSeconds));

    // Prevent consuming into 2nd jump automatically (keeps double jump separate)
    const bool bFirstJumpOnly = (ownerChar->JumpCurrentCount == 0);
    if (!bFirstJumpOnly && bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Not first jump"));

    return bBuffered && bGroundOrCoyote && bFirstJumpOnly;
}

void UEnemyLocomotionComponent::MarkGroundedNow()
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

void UEnemyLocomotionComponent::AddMoveInput(AActor* Target, const FVector& Loc, float AcceptanceRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] AddMoveInput: Entered"), *GetNameSafe(this));
    
    if (!EnsureOwnerCharacter() || HasOverrideExact(TAG_Move_Override_Lock)) return;

	if (Target) controller->MoveToActorHNS(Target, AcceptanceRadius);
	else controller->MoveToLocationHNS(Loc, AcceptanceRadius);
}

void UEnemyLocomotionComponent::LaunchCharacterHNS(FVector Velocity, bool OverrideXY, bool OverrideZ, float TimeToStop, AActor* Actor)
{
    if (Velocity == FVector::ZeroVector || !EnsureOwnerCharacter()) return;

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
		FVector fwdVel {ownerChar->GetActorForwardVector() * Velocity.X};
		FVector sideVel {ownerChar->GetActorRightVector() * Velocity.Y};
		FVector vertVel {ownerChar->GetActorUpVector() * Velocity.Z};
		Velocity = {fwdVel + sideVel + vertVel};
	}

    if (Velocity.Z > 5.0f) moveComp->bNotifyApex = true;
	ownerChar->LaunchCharacter(Velocity, OverrideXY, OverrideZ);

    UWorld* world {ownerChar->GetWorld()};
    if (!world) return;

	if (UKismetSystemLibrary::K2_IsTimerActiveHandle(world, TH_StopLaunch)) {UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(world, TH_StopLaunch);}
	if (TimeToStop > 0.0f) {world->GetTimerManager().SetTimer(TH_StopLaunch, this, &UEnemyLocomotionComponent::StopLaunch, TimeToStop, false);}
}