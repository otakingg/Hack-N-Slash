#include "BlockState.h"
#include "../../../Interfaces/CombatCmdInterface.h"
#include "../../../Interfaces/LocomotionCmdInterface.h"
#include "../../../Combat/Player/PlayerCombatComponent.h"
#include "../../../Tags/LocomotionTags.h"

void UBlockState::EnterState()
{
    Super::EnterState();
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->AddMoveOverrideTag(TAG_Move_Override_Lock);

    UWorld* world = GetWorld();
    if (world && playerCombatComp)
    {
        playerCombatComp->SetPerfectBlockWindow(true);
        world->GetTimerManager().SetTimer(TH_PerfectBlock, this, &UBlockState::ClearPerfectBlockWindow, playerCombatComp->GetPerfectBlockWindowTime(), false);
    }
}

void UBlockState::ExitState()
{
    if (UWorld* world = GetWorld())
    {
        FTimerManager& timerManager = world->GetTimerManager();
        if (timerManager.IsTimerActive(TH_PerfectBlock)) timerManager.ClearTimer(TH_PerfectBlock);
        if (playerCombatComp) playerCombatComp->SetPerfectBlockWindow(false);
    }
    
    if (ILocomotionCmdInterface* iLocoCmd = GetLocoCmd()) iLocoCmd->RemoveMoveOverrideTag(TAG_Move_Override_Lock);

    Super::ExitState();
}

bool UBlockState::OnBlockStopIntent()
{
    if (ICombatCmdInterface* iCombatCmd = GetCombatCmd())
    {
        iCombatCmd->BlockStopIntent();
        return true;
    }
    return false;
}

void UBlockState::ClearPerfectBlockWindow() { if (playerCombatComp) playerCombatComp->SetPerfectBlockWindow(false); }