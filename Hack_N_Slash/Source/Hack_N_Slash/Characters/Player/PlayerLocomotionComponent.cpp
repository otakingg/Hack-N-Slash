#include "PlayerLocomotionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UPlayerLocomotionComponent::UPlayerLocomotionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerLocomotionComponent::BeginPlay()
{
    Super::BeginPlay();
    EnsureOwnerCharacter();
}

bool UPlayerLocomotionComponent::EnsureOwnerCharacter()
{
    if (OwnerChar) return true;

    OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UPlayerLocomotionComponent] Owner is not an ACharacter: %s"), *GetNameSafe(GetOwner()));
        return false;
    }
    return true;
}

void UPlayerLocomotionComponent::AddLookInputScaled(const FVector2D& Look, float YawRate, float PitchRate)
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: AddLookInputScaled Entered"));}
    if (!EnsureOwnerCharacter()) return;

    UWorld* World = OwnerChar->GetWorld();
    if (!World) return;

    const float DT = World->GetDeltaSeconds();

    // Match your original pattern:
    // yaw uses X, pitch uses Y
    OwnerChar->AddControllerYawInput(Look.X * YawRate * DT);
    OwnerChar->AddControllerPitchInput(Look.Y * PitchRate * DT);
}

void UPlayerLocomotionComponent::AddMoveInputScaled(const FVector2D& Move, float Scale)
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: AddMoveInputScaled Entered"));}
    if (!EnsureOwnerCharacter()) return;
    if (Scale <= 0.f) return;

    // Match your original GroundLocomotion logic:
    // - use control rotation
    // - zero pitch/roll
    // - X = right, Y = forward
    FRotator ControlRot = OwnerChar->GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll  = 0.f;

    const FVector Right   = UKismetMathLibrary::GetRightVector(ControlRot);
    const FVector Forward = UKismetMathLibrary::GetForwardVector(ControlRot);

    // preserve your axis mapping
    OwnerChar->AddMovementInput(Right,   Move.X * Scale);
    OwnerChar->AddMovementInput(Forward, Move.Y * Scale);
}

void UPlayerLocomotionComponent::JumpPressed()
{
    if (bDebug && GEngine) {GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("Locomotion Comp: JumpPressed Entered"));}
    if (!EnsureOwnerCharacter()) return;
    OwnerChar->Jump();
}

void UPlayerLocomotionComponent::JumpReleased()
{
    if (!EnsureOwnerCharacter()) return;
    OwnerChar->StopJumping();
}

void UPlayerLocomotionComponent::LaunchUp(float JumpZ)
{
    if (!EnsureOwnerCharacter()) return;
    OwnerChar->LaunchCharacter(FVector(0.f, 0.f, JumpZ), /*bXYOverride*/ false, /*bZOverride*/  true);
}