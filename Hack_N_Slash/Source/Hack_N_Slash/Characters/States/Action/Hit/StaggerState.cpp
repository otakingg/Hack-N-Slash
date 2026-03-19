#include "StaggerState.h"
#include "GameFramework/Character.h"
//#include "Kismet/KismetSystemLibrary.h"
#include "../../../../Combat/CombatResolutionComponent.h"
#include "../../Interfaces/LocomotionCmdInterface.h"

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
    if (combatResComp->IsAirborne()) combatResComp->PlayHitReaction(combatResComp->GetHitReactions().airStagger);
    else
    {
        // Convert hit direction to local space
        /**
         * X → Forward/Backward
         * Y → Right/Left
         * Z → Up/Down (usually ignored for hit reactions)
         */
        FVector localHitDir = ownerChar->GetActorTransform().InverseTransformVectorNoScale(HitData.hitDir);

        // Convert to angle (for BlendSpace or logic)
        /**
         * 0° = front hit
         * 180° or -180° = back hit
         * 90° = right hit
         * -90° = left hit
         */
        float angle = FMath::Atan2(localHitDir.Y, localHitDir.X);
        angle = FMath::RadiansToDegrees(angle);

        FName sectionName;

        if (angle >= -45.f && angle <= 45.f) sectionName = "Front";
        else if (angle > 45.f && angle < 135.f) sectionName = "Right";
        else if (angle < -45.f && angle > -135.f) sectionName = "Left";
        else sectionName = "Back";

        combatResComp->PlayHitReaction(combatResComp->GetHitReactions().stagger, sectionName);
    }

    // Apply hit velocity
    if (ILocomotionCmdInterface* locoCMD = GetLocoCmd()) locoCMD->LaunchCharacterHNS(HitData.motionVelocity, true, true, HitData.timeToStop, HitData.attacker);
}