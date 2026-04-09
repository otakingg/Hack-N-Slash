#include "StaggerState.h"
#include "../../../Combat/Shared/CombatResolutionComponent.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"


void UStaggerState::ReceiveHit(const FAtkHitData& HitData)
{
    Super::ReceiveHit(HitData);
    
    if (!ownerChar || !combatResComp) return;

    float angle = CalculateHitAngle(HitData);

    FName sectionName;

    if (angle >= -45.f && angle <= 45.f) sectionName = "Front";
    else if (angle > 45.f && angle < 135.f) sectionName = "Right";
    else if (angle < -45.f && angle > -135.f) sectionName = "Left";
    else sectionName = "Back";

    if (bDebug)
    {
        FString SectionString = sectionName.ToString();
        UE_LOG(LogTemp, Warning, TEXT("Section: %s"), *SectionString);
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Section: %s"), *SectionString));
    }

    combatResComp->PlayHitReaction(combatResComp->GetHitReactions().stagger, sectionName);

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}