#include "PlayerLocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Tags/LocomotionTags.h"
#include "../StatsComponent.h"

UPlayerLocomotionComponent::UPlayerLocomotionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerLocomotionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureOwnerCharacter()) return;

    // Safe default
    activeMoveProfile = TAG_Move_Profile_Ground_Jog;
    ApplyMovementFromTagsAndStats();
}

bool UPlayerLocomotionComponent::EnsureOwnerCharacter()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerLocomotionComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerLocomotionComponent] No CharacterMovementComponent on: %s"), *GetNameSafe(ownerChar));
        return false;
    }

    // Stats are optional (lets you prototype without wiring everything)
    if (!statsComp) statsComp = ownerChar->FindComponentByClass<UStatsComponent>();

    return true;
}

bool UPlayerLocomotionComponent::HasOverrideExact(const FGameplayTag& Tag) const { return Tag.IsValid() && moveOverrides.HasTagExact(Tag); }

void UPlayerLocomotionComponent::ApplyMovementFromTagsAndStats()
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

        if (bDebug && GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("LocomotionComp: StatsComponent missing (using fallback tuning)"));
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

float UPlayerLocomotionComponent::ResolveSpeedForProfile(const FGameplayTag& Profile) const
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

float UPlayerLocomotionComponent::FallbackSpeedForProfile(const FGameplayTag& Profile) const
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

void UPlayerLocomotionComponent::SetMoveProfileTag(FGameplayTag NewProfile)
{
    if (!NewProfile.IsValid()) return;
    if (activeMoveProfile == NewProfile) return;

    activeMoveProfile = NewProfile;
    ApplyMovementFromTagsAndStats();
}

void UPlayerLocomotionComponent::AddMoveOverrideTag(FGameplayTag OverrideTag)
{
    if (!OverrideTag.IsValid()) return;
    if (moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.AddTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void UPlayerLocomotionComponent::RemoveMoveOverrideTag(FGameplayTag OverrideTag)
{
    if (!OverrideTag.IsValid()) return;
    if (!moveOverrides.HasTagExact(OverrideTag)) return;

    moveOverrides.RemoveTag(OverrideTag);
    ApplyMovementFromTagsAndStats();
}

void UPlayerLocomotionComponent::RefreshMovement()
{
    ApplyMovementFromTagsAndStats();
}

void UPlayerLocomotionComponent::SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode)
{
    if (!EnsureOwnerCharacter()) return;
    moveComp->SetMovementMode(NewMode, CustomMode);
}

void UPlayerLocomotionComponent::AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate)
{
    if (!EnsureOwnerCharacter()) return;

    UWorld* World = ownerChar->GetWorld();
    if (!World) return;

    const float DT = World->GetDeltaSeconds();

    ownerChar->AddControllerYawInput(Look.X * YawRate * DT);
    ownerChar->AddControllerPitchInput(Look.Y * PitchRate * DT);
}

void UPlayerLocomotionComponent::AddMoveInputScaled(const FVector2D& Move, float Scale)
{
    if (Scale <= 0.f) return;

    if (!EnsureOwnerCharacter()) return;

    // Treat both as "no movement"
    if (HasOverrideExact(TAG_Move_Override_Lock) || HasOverrideExact(TAG_Move_Override_Root)) return;

    FRotator ControlRot = ownerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll  = 0.f;

    const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
    const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

    ownerChar->AddMovementInput(Right,   Move.X * Scale);
    ownerChar->AddMovementInput(Forward, Move.Y * Scale);
}

void UPlayerLocomotionComponent::JumpPressed()
{
    if (!EnsureOwnerCharacter()) return;
    if (HasOverrideExact(TAG_Move_Override_NoJump)) return;

    ownerChar->Jump();
}

void UPlayerLocomotionComponent::JumpReleased()
{
    if (!EnsureOwnerCharacter()) return;
    ownerChar->StopJumping();
}

void UPlayerLocomotionComponent::LaunchUp(float JumpZ)
{
    if (!EnsureOwnerCharacter()) return;
    ownerChar->LaunchCharacter(FVector(0.f, 0.f, JumpZ), /*bXYOverride*/ false, /*bZOverride*/ true);
}
