#include "ActionTags.h"

namespace CharacterActionTags
{
    FGameplayTag None                 = FGameplayTag::RequestGameplayTag("CharacterAction.None");
    FGameplayTag Attack               = FGameplayTag::RequestGameplayTag("CharacterAction.Attack");
    FGameplayTag AttackHeavyStart     = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Start");
    FGameplayTag AttackHeavyOngoing   = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Ongoing");
    FGameplayTag AttackHeavyFinish    = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Finish");
    FGameplayTag AttackLightStart     = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Start");
    FGameplayTag AttackLightOngoing   = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Ongoing");
    FGameplayTag AttackLightFinish    = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Finish");
    FGameplayTag BlockStart           = FGameplayTag::RequestGameplayTag("CharacterAction.Block.Start");
    FGameplayTag BlockFinish          = FGameplayTag::RequestGameplayTag("CharacterAction.Block.Finish");
    FGameplayTag FlyStart             = FGameplayTag::RequestGameplayTag("CharacterAction.Fly.Start");
    FGameplayTag FlyStop              = FGameplayTag::RequestGameplayTag("CharacterAction.Fly.Finish");  
    FGameplayTag Grind                = FGameplayTag::RequestGameplayTag("CharacterAction.Grind");
    FGameplayTag GrindJump            = FGameplayTag::RequestGameplayTag("CharacterAction.GrindJump");
    FGameplayTag JumpStart            = FGameplayTag::RequestGameplayTag("CharacterAction.Jump.Start");
    FGameplayTag JumpFinish           = FGameplayTag::RequestGameplayTag("CharacterAction.Jump.Finish");
    FGameplayTag Look                 = FGameplayTag::RequestGameplayTag("CharacterAction.Look");
    FGameplayTag Move                 = FGameplayTag::RequestGameplayTag("CharacterAction.Move");
    FGameplayTag MoveFly              = FGameplayTag::RequestGameplayTag("CharacterAction.Move.Fly");
    FGameplayTag TetherMeTo           = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.MeTo");  
    FGameplayTag TetherToMe           = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.ToMe");  
    FGameplayTag TetherSwing          = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.Swing");  
    FGameplayTag WallJump             = FGameplayTag::RequestGameplayTag("CharacterAction.Wall.Jump");  
    FGameplayTag WallRun              = FGameplayTag::RequestGameplayTag("CharacterAction.Wall.Run");  
}