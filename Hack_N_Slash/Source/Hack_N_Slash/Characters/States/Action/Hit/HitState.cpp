#include "HitState.h"
#include "GameFramework/Character.h"
#include "../../../../Combat/CombatResolutionComponent.h"

void UHitState::Initialize(UStateMachineComponent *InSM, ACharacter *InOwner)
{
    Super::Initialize(InSM, InOwner);
    combatResComp = ownerChar ? ownerChar->FindComponentByClass<UCombatResolutionComponent>() : nullptr;
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

    FVector Forward = YawRot.Vector(); // clean forward
    FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    float ForwardDot = FVector::DotProduct(hitDir, Forward);
    float RightDot   = FVector::DotProduct(hitDir, Right);

    float angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
    return angle;
}