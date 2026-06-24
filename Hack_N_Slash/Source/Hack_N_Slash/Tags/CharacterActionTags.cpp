#include "CharacterActionTags.h"

namespace CharacterActionTags
{
    FGameplayTag None                 = FGameplayTag::RequestGameplayTag("CharacterAction.None");
    FGameplayTag Attack               = FGameplayTag::RequestGameplayTag("CharacterAction.Attack");
    FGameplayTag AttackHeavyStart     = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Start");
    FGameplayTag AttackHeavyHold      = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Hold");
    FGameplayTag AttackHeavyRelease   = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Heavy.Release");
    FGameplayTag AttackLightStart     = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Start");
    FGameplayTag AttackLightHold      = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Hold");
    FGameplayTag AttackLightRelease   = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Light.Release");
    FGameplayTag AttackTether         = FGameplayTag::RequestGameplayTag("CharacterAction.Attack.Tether");
    FGameplayTag BlockStart           = FGameplayTag::RequestGameplayTag("CharacterAction.Block.Start");
    FGameplayTag BlockRelease         = FGameplayTag::RequestGameplayTag("CharacterAction.Block.Release");
    FGameplayTag Dodge                = FGameplayTag::RequestGameplayTag("CharacterAction.Dodge");
    FGameplayTag Fly                  = FGameplayTag::RequestGameplayTag("CharacterAction.Fly");
    FGameplayTag Grind                = FGameplayTag::RequestGameplayTag("CharacterAction.Grind");
    FGameplayTag GrindJump            = FGameplayTag::RequestGameplayTag("CharacterAction.GrindJump");
    FGameplayTag JumpStart            = FGameplayTag::RequestGameplayTag("CharacterAction.Jump.Start");
    FGameplayTag JumpRelease          = FGameplayTag::RequestGameplayTag("CharacterAction.Jump.Release");
    FGameplayTag LockOnOffStart       = FGameplayTag::RequestGameplayTag("CharacterAction.LockOnOff.Start");
    FGameplayTag LookMouse            = FGameplayTag::RequestGameplayTag("CharacterAction.Look.Mouse");
    FGameplayTag LookStick            = FGameplayTag::RequestGameplayTag("CharacterAction.Look.Stick");
    FGameplayTag Move                 = FGameplayTag::RequestGameplayTag("CharacterAction.Move");
    FGameplayTag TetherMeTo           = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.MeTo");  
    FGameplayTag TetherToMe           = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.ToMe");  
    FGameplayTag TetherSwing          = FGameplayTag::RequestGameplayTag("CharacterAction.Tether.Swing");  
    FGameplayTag WallJump             = FGameplayTag::RequestGameplayTag("CharacterAction.Wall.Jump");  
    FGameplayTag WallRun              = FGameplayTag::RequestGameplayTag("CharacterAction.Wall.Run");  
}