#include "HitState.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../Tags/LocomotionTags.h"

void UHitState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize(InSM, InOwner);
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
}

void UHitState::EnterState()
{
    Super::EnterState();
    
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->AddMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->AddMoveOverrideTag(TAG_Move_Override_NoJump);
    }
}

void UHitState::ExitState()
{
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd())
    {
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_Lock);
        locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_NoJump);
    }

    UWorld* world = ownerChar->GetWorld();
    if (world && world->GetTimerManager().TimerExists(TH_Juggle)) world->GetTimerManager().ClearTimer(TH_Juggle);

    Super::ExitState();
}

void UHitState::EnterJuggle()
{
    if (!moveComp) return;
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->AddMoveOverrideTag(TAG_Move_Override_MoveStats);
    moveComp->GravityScale = juggleGravity;
}

void UHitState::ExitJuggle()
{
    if (!moveComp) return;
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->RemoveMoveOverrideTag(TAG_Move_Override_MoveStats);
}

void UHitState::ReceiveHit(const FAtkHitData& HitData)
{
    if (moveComp) moveComp->StopMovementImmediately();
    if (AAIController* aiController = Cast<AAIController>(ownerChar->GetController())) aiController->StopMovement();
}

float UHitState::CalculateHitAngle(const FAtkHitData& HitData) const
{
    // Calculate hit direction
    FVector hitDir = FVector::ZeroVector;
    if (HitData.attacker) hitDir = (HitData.attacker->GetActorLocation() - ownerChar->GetActorLocation()).GetSafeNormal();
    else hitDir = (HitData.hitLoc - ownerChar->GetActorLocation()).GetSafeNormal();
    
    // Flatten
    hitDir.Z = 0.f;
    hitDir.Normalize();

    // ✅ Use ONLY yaw rotation (ignores mesh weirdness)
    FRotator YawRot = ownerChar->GetActorRotation();
    YawRot.Pitch = 0.f;
    YawRot.Roll = 0.f;

    FVector Forward = YawRot.Vector(); // Clean forward
    FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    float ForwardDot = FVector::DotProduct(hitDir, Forward);
    float RightDot   = FVector::DotProduct(hitDir, Right);

    float angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    return angle;
}