#include "PlayerLocomotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../Tags/LocomotionTags.h"
#include "../StatsComponent.h"

void UPlayerLocomotionComponent::BeginPlay()
{
    Super::BeginPlay();
    if (!EnsureOwnerCharacter()) return;

    activeMoveProfile = TAG_Move_Profile_Ground_Jog; // Sane default
    ApplyMovementFromTagsAndStats();
}

bool UPlayerLocomotionComponent::EnsureOwnerCharacter()
{
    if (!ownerChar) ownerChar = Cast<ACharacter>(GetOwner());
    if (!ownerChar) return false;

    if (!moveComp) moveComp = ownerChar->GetCharacterMovement();
    if (!moveComp) return false;

    if (!statsComp) statsComp = ownerChar->FindComponentByClass<UStatsComponent>();
    if (!statsComp) return false;

    return true;
}

bool UPlayerLocomotionComponent::HasOverride(const FGameplayTag &Tag) const { return moveOverrides.HasTag(Tag); }

void UPlayerLocomotionComponent::ApplyMovementFromTagsAndStats()
{
    if (!EnsureOwnerCharacter()) return;

    // Base values from stats + profile
    float speed {ResolveSpeedForProfile(activeMoveProfile)};
    const float accel {statsComp->GetStat(EStat::AccelerationMax)};
    const float jumpZ {statsComp->GetStat(EStat::JumpZVel)};

    // Overrides
    if (HasOverride(TAG_Move_Override_Slow)) speed *= 0.5f;
    if (HasOverride(TAG_Move_Override_Root)) speed = 0.f;

    moveComp->MaxWalkSpeed    = speed; // Harmless even when not walking/jogging/springting; keeps things consistent
    moveComp->MaxFlySpeed     = speed; // Harmless even when not flying; keeps things consistent
    moveComp->MaxAcceleration = accel;
    moveComp->JumpZVelocity   = jumpZ;
}

float UPlayerLocomotionComponent::ResolveSpeedForProfile(const FGameplayTag& Profile) const
{
    if (!statsComp) return 0.f;

    if (Profile == TAG_Move_Profile_Ground_Walk)   return statsComp->GetStat(EStat::SpeedWalk);
    if (Profile == TAG_Move_Profile_Ground_Jog)    return statsComp->GetStat(EStat::SpeedJog);
    if (Profile == TAG_Move_Profile_Ground_Sprint) return statsComp->GetStat(EStat::SpeedSprint);

    // placeholders until you add EStat::SpeedGrind / SpeedClimb / SpeedFly, etc.
    if (Profile == TAG_Move_Profile_Grind)  return statsComp->GetStat(EStat::SpeedGrind);
    if (Profile == TAG_Move_Profile_Climb)  return statsComp->GetStat(EStat::SpeedClimb);
    if (Profile == TAG_Move_Profile_Fly)    return statsComp->GetStat(EStat::SpeedFly);
    if (Profile == TAG_Move_Profile_Airborne) return statsComp->GetStat(EStat::SpeedJog);

    return statsComp->GetStat(EStat::SpeedJog); //Fallback
}

/*****************************************Locomotion Command Interface***********************************************************************/
void UPlayerLocomotionComponent::SetMoveProfileTag(FGameplayTag NewProfile)
{
    if (!NewProfile.IsValid() || activeMoveProfile == NewProfile) return;
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

void UPlayerLocomotionComponent::RefreshMovement() { ApplyMovementFromTagsAndStats(); }

void UPlayerLocomotionComponent::SetMovementModeCmd(EMovementMode NewMode, uint8 CustomMode)
{
    if (!EnsureOwnerCharacter()) return;
    moveComp->SetMovementMode(NewMode, CustomMode);
}

void UPlayerLocomotionComponent::AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate)
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: AddLookInput Entered"));}
    if (!EnsureOwnerCharacter()) return;

    UWorld* World = ownerChar->GetWorld();
    if (!World) return;

    const float DT = World->GetDeltaSeconds();

    // Match your original pattern:
    // yaw uses X, pitch uses Y
    ownerChar->AddControllerYawInput(Look.X * YawRate * DT);
    ownerChar->AddControllerPitchInput(Look.Y * PitchRate * DT);
}

void UPlayerLocomotionComponent::AddMoveInput(const FVector2D& Move)
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: AddMoveInputScaled Entered"));}
    if (!EnsureOwnerCharacter() || HasOverride(TAG_Move_Override_Lock) || HasOverride(TAG_Move_Override_Root)) return;

    FRotator ControlRot = ownerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll  = 0.f;

    const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
    const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

    ownerChar->AddMovementInput(Right,   Move.X);
    ownerChar->AddMovementInput(Forward, Move.Y);
}

void UPlayerLocomotionComponent::JumpPressed()
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: JumpPressed Entered"));}
    if (!EnsureOwnerCharacter() || HasOverride(TAG_Move_Override_NoJump)) return;
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
    ownerChar->LaunchCharacter(FVector(0.f, 0.f, JumpZ), /*bXYOverride*/ false, /*bZOverride*/  true);
}