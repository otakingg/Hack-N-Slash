#include "ChakraRegenState.h"
#include "../../../Interfaces/CombatInstigator.h"

FGameplayTag UChakraRegenState::ResolvePlayerAction_Implementation(const FGameplayTag& PlayerAction)
{
    if (PlayerAction.MatchesTagExact(Tags::PlayerAction::BlockRelease)) return Tags::PlayerAction::None;
    else return PlayerAction;
}
