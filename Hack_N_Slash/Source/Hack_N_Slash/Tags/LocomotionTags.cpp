#include "LocomotionTags.h"

namespace ProfileTags
{
    FGameplayTag None         = FGameplayTag::RequestGameplayTag("Move.Profile.None");
    FGameplayTag Grounded     = FGameplayTag::RequestGameplayTag("Move.Profile.Grounded");
    FGameplayTag Grind        = FGameplayTag::RequestGameplayTag("Move.Profile.Grind");
    FGameplayTag Climb        = FGameplayTag::RequestGameplayTag("Move.Profile.Climb");
    FGameplayTag WallRun      = FGameplayTag::RequestGameplayTag("Move.Profile.WallRun");
    FGameplayTag Falling      = FGameplayTag::RequestGameplayTag("Move.Profile.Falling");
    FGameplayTag Fly          = FGameplayTag::RequestGameplayTag("Move.Profile.Fly");
}

namespace OverrideTags
{
    FGameplayTag Lock       = FGameplayTag::RequestGameplayTag("Move.Override.Lock");
    FGameplayTag MoveStats  = FGameplayTag::RequestGameplayTag("Move.Override.MoveStats");
    FGameplayTag NoJump     = FGameplayTag::RequestGameplayTag("Move.Override.NoJump");
}