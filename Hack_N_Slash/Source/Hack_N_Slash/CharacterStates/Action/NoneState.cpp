#include "NoneState.h"

FGameplayTag UNoneState::ResolvePlayerAction_Implementation(const FGameplayTag &PlayerAction)
{
    FGameplayTagContainer doNothingTags;
    doNothingTags.AddTag(Tags::PlayerAction::BlockRelease);
    doNothingTags.AddTag(Tags::PlayerAction::RegenChakraRelease);

    if (PlayerAction.MatchesAnyExact(doNothingTags)) return Tags::PlayerAction::None;
    else return PlayerAction;
}
