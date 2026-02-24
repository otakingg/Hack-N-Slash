#include "EnemyLocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

    // Safe default
    activeMoveProfile = TAG_Move_Profile_Ground_Jog;
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
    if (!EnsureOwnerCharacter()) return;

    // ---- Base values from stats + profile (with fallback if stats missing) ----
    float speed = 0.f;
    float accel = 0.f;
    float jumpZ = 0.f;

    if (statsComp)
    {
        speed = ResolveSpeedForProfile(activeMoveProfile);
        accel = statsComp->GetStat(EStat::AccelerationMax);
        jumpZ = statsComp->GetStat(EStat::JumpZVel);
    }
    else
    {
        speed = FallbackSpeedForProfile(activeMoveProfile);
        accel = FallbackAcceleration();
        jumpZ = FallbackJumpZ();

        // Warning / non-fatal fallback
        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("EnemyLocomotionComp: StatsComponent missing (using fallback tuning)"));
    }

    // ---- Overrides ----
    if (HasOverrideExact(TAG_Move_Override_Slow)) speed *= 0.5f;
    if (HasOverrideExact(TAG_Move_Override_Root)) speed = 0.f;

    // Apply consistently; harmless even if current movement mode isn't walking/flying
    moveComp->MaxWalkSpeed    = speed;
    moveComp->MaxFlySpeed     = speed;
    moveComp->MaxAcceleration = accel;
    moveComp->JumpZVelocity   = jumpZ;
}

float UEnemyLocomotionComponent::ResolveSpeedForProfile(const FGameplayTag& Profile) const
{
    if (!statsComp) return 0.f;

    if (Profile == TAG_Move_Profile_Ground_Walk)     return statsComp->GetStat(EStat::SpeedWalk);
    if (Profile == TAG_Move_Profile_Ground_Jog)      return statsComp->GetStat(EStat::SpeedJog);
    if (Profile == TAG_Move_Profile_Ground_Sprint)   return statsComp->GetStat(EStat::SpeedSprint);

    // If you haven't added these stats yet, GetStat will return 0.f; that's OK during development.
    if (Profile == TAG_Move_Profile_Grind)           return statsComp->GetStat(EStat::SpeedGrind);
    if (Profile == TAG_Move_Profile_Climb)           return statsComp->GetStat(EStat::SpeedClimb);
    if (Profile == TAG_Move_Profile_Fly)             return statsComp->GetStat(EStat::SpeedFly);

    // Airborne: optionally add a dedicated stat later; jog is a sane default
    if (Profile == TAG_Move_Profile_Airborne)        return statsComp->GetStat(EStat::SpeedJog);

    return statsComp->GetStat(EStat::SpeedJog);
}

float UEnemyLocomotionComponent::FallbackSpeedForProfile(const FGameplayTag& Profile) const
{
    // These are intentionally conservative "dev defaults"
    if (Profile == TAG_Move_Profile_Ground_Walk)     return 250.f;
    if (Profile == TAG_Move_Profile_Ground_Jog)      return 450.f;
    if (Profile == TAG_Move_Profile_Ground_Sprint)   return 650.f;

    if (Profile == TAG_Move_Profile_Climb)           return 220.f;
    if (Profile == TAG_Move_Profile_Grind)           return 800.f;
    if (Profile == TAG_Move_Profile_Fly)             return 750.f;

    if (Profile == TAG_Move_Profile_Airborne)        return 450.f;

    return 450.f;
}

/***************************************** Locomotion Command Interface *****************************************/

void UEnemyLocomotionComponent::SetMoveProfileTag(FGameplayTag NewProfile)
{
    if (!NewProfile.IsValid()) return;
    if (activeMoveProfile == NewProfile) return;

    activeMoveProfile = NewProfile;
    ApplyMovementFromTagsAndStats();
}

void UEnemyLocomotionComponent::AddMoveOverrideTag(FGameplayTag OverrideTag)
{
    if (!OverrideTag.IsValid()) return;
    if (moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.AddTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void UEnemyLocomotionComponent::RemoveMoveOverrideTag(FGameplayTag OverrideTag)
{
    if (!OverrideTag.IsValid()) return;
    if (!moveOverrides.HasTagExact(OverrideTag)) return;

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

void UEnemyLocomotionComponent::AddMoveInputScaled(AActor *Target, const FVector &Loc, float AcceptanceRadius, float Scale)
{
    if (Scale <= 0.f) return;

    if (!EnsureOwnerCharacter()) return;

    // Treat both as "no movement"
    if (HasOverrideExact(TAG_Move_Override_Lock) || HasOverrideExact(TAG_Move_Override_Root)) return;

	if (Target) controller->MoveToActorHNS(Target, AcceptanceRadius);
	else controller->MoveToLocationHNS(Loc, AcceptanceRadius);
}

void UEnemyLocomotionComponent::LaunchUp(float JumpZ)
{
    if (!EnsureOwnerCharacter()) return;
    ownerChar->LaunchCharacter(FVector(0.f, 0.f, JumpZ), /*bXYOverride*/ false, /*bZOverride*/ true);
}