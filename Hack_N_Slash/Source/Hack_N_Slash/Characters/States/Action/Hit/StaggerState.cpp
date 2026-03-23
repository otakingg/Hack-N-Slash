#include "StaggerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "Kismet/KismetSystemLibrary.h"
#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"
#include "../../../StateMachineComponent.h"

void UStaggerState::EnterState()
{
    /*UWorld* world = ownerChar->GetWorld();
    if (world && combatResComp && combatResComp->IsAirborne())
    {
        // Does timer exist (active or paused)
        if (UKismetSystemLibrary::K2_TimerExistsHandle(this, TH_Gravity)) UKismetSystemLibrary::K2_ClearTimerHandle(this, TH_Gravity);
        world->GetTimerManager().SetTimer(TH_Gravity, this, &UStatsComponent::ExitAirJuggleState, gravityRestoreDelay, false);
    }*/
}

void UStaggerState::ExitState()
{
    //if (UKismetSystemLibrary::K2_TimerExistsHandle(this, TH_Gravity)) UKismetSystemLibrary::K2_ClearTimerHandle(this, TH_Gravity);
}

void UStaggerState::ReceiveHit(const FAtkHitData& HitData)
{
    if (!ownerChar || !combatResComp) return;

    // Don't currently have directional air hits, so no need to calculate direction for it right now
    bool bAirborne = (ownerStateMachineComp && ownerStateMachineComp->IsAirborne()) || (ownerChar->GetMovementComponent() && ownerChar->GetMovementComponent()->IsFalling());
    if (bAirborne) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger);
    else
    {
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
    }

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}