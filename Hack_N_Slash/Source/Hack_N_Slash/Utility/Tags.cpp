#include "Tags.h"

namespace Tags
{
	namespace Attack
	{
		namespace Motion
		{
			FGameplayTag Dash       = FGameplayTag::RequestGameplayTag("Attack.Motion.Dash");
			FGameplayTag Leap       = FGameplayTag::RequestGameplayTag("Attack.Motion.Leap");
			FGameplayTag Stationary = FGameplayTag::RequestGameplayTag("Attack.Motion.Stationary");
			FGameplayTag Teleport   = FGameplayTag::RequestGameplayTag("Attack.Motion.Teleport");
		}

		namespace Type
		{
			FGameplayTag AOE        = FGameplayTag::RequestGameplayTag("Attack.Type.AOE");
			FGameplayTag Melee      = FGameplayTag::RequestGameplayTag("Attack.Type.Melee");
			FGameplayTag Projectile = FGameplayTag::RequestGameplayTag("Attack.Type.Projectile");
		}

	}

	namespace Element
	{
		FGameplayTag None      = FGameplayTag::RequestGameplayTag("Element.None");
		FGameplayTag Air       = FGameplayTag::RequestGameplayTag("Element.Air");
		FGameplayTag Electric  = FGameplayTag::RequestGameplayTag("Element.Electric");
		FGameplayTag Fire      = FGameplayTag::RequestGameplayTag("Element.Fire");
		FGameplayTag Ice       = FGameplayTag::RequestGameplayTag("Element.Ice");
		FGameplayTag Metal     = FGameplayTag::RequestGameplayTag("Element.Metal");
	}

	namespace NotifyEvent
	{
		namespace StateMachine
		{
			FGameplayTag ClearActionState   = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.ClearActionState");
			FGameplayTag IfDeadPauseMontage = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.IfDead.PauseMontage");
			FGameplayTag Jump               = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.Jump");
			FGameplayTag TryBounceGround    = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.TryBounceGround");
			FGameplayTag TryLand            = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.TryLand");
			FGameplayTag TryWallSplat       = FGameplayTag::RequestGameplayTag("NotifyEvent.StateMachine.TryWallSplat");
		}

		namespace EnemyBrain
		{
			FGameplayTag AdvanceSequence  = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.AdvanceSequence");
			FGameplayTag ClearFocus       = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.ClearFocus");
			FGameplayTag Dash             = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.Dash");
			FGameplayTag NotInterruptible = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.NotInterruptible");
			FGameplayTag RequestEvaluate  = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.RequestEvaluate");
			FGameplayTag SetFocus         = FGameplayTag::RequestGameplayTag("NotifyEvent.EnemyBrain.SetFocus");
		}
	}

	namespace PlayerAction
	{
		FGameplayTag None                 = FGameplayTag::RequestGameplayTag("PlayerAction.None");
		FGameplayTag Attack               = FGameplayTag::RequestGameplayTag("PlayerAction.Attack");
		FGameplayTag AttackHeavyStart     = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Heavy.Start");
		FGameplayTag AttackHeavyHold      = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Heavy.Hold");
		FGameplayTag AttackHeavyRelease   = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Heavy.Release");
		FGameplayTag AttackLightStart     = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Light.Start");
		FGameplayTag AttackLightHold      = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Light.Hold");
		FGameplayTag AttackLightRelease   = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Light.Release");
		FGameplayTag AttackNinjutsuStart  = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Ninjutsu.Start");
		FGameplayTag AttackTether         = FGameplayTag::RequestGameplayTag("PlayerAction.Attack.Tether");
		FGameplayTag BlockStart           = FGameplayTag::RequestGameplayTag("PlayerAction.Block.Start");
		FGameplayTag BlockHold            = FGameplayTag::RequestGameplayTag("PlayerAction.Block.Hold");
		FGameplayTag BlockRelease         = FGameplayTag::RequestGameplayTag("PlayerAction.Block.Release");
		FGameplayTag Dodge                = FGameplayTag::RequestGameplayTag("PlayerAction.Dodge");
		FGameplayTag Fly                  = FGameplayTag::RequestGameplayTag("PlayerAction.Fly");
		FGameplayTag Grind                = FGameplayTag::RequestGameplayTag("PlayerAction.Grind");
		FGameplayTag GrindJump            = FGameplayTag::RequestGameplayTag("PlayerAction.GrindJump");
		FGameplayTag JumpStart            = FGameplayTag::RequestGameplayTag("PlayerAction.Jump.Start");
		FGameplayTag JumpRelease          = FGameplayTag::RequestGameplayTag("PlayerAction.Jump.Release");
		FGameplayTag LockOnOffStart       = FGameplayTag::RequestGameplayTag("PlayerAction.LockOnOff.Start");
		FGameplayTag LookMouse            = FGameplayTag::RequestGameplayTag("PlayerAction.Look.Mouse");
		FGameplayTag LookStick            = FGameplayTag::RequestGameplayTag("PlayerAction.Look.Stick");
		FGameplayTag Move                 = FGameplayTag::RequestGameplayTag("PlayerAction.Move");
		FGameplayTag TetherMeTo           = FGameplayTag::RequestGameplayTag("PlayerAction.Tether.MeTo");  
		FGameplayTag TetherToMe           = FGameplayTag::RequestGameplayTag("PlayerAction.Tether.ToMe");  
		FGameplayTag TetherSwing          = FGameplayTag::RequestGameplayTag("PlayerAction.Tether.Swing");  
		FGameplayTag WallJump             = FGameplayTag::RequestGameplayTag("PlayerAction.Wall.Jump");  
		FGameplayTag WallRun              = FGameplayTag::RequestGameplayTag("PlayerAction.Wall.Run");
	}

	namespace StateMachine
	{
		namespace Action
		{
			FGameplayTag None = FGameplayTag::RequestGameplayTag("State.Action.None");

			namespace Combat
			{
				FGameplayTag Attack = FGameplayTag::RequestGameplayTag("State.Action.Combat.Attack");
				FGameplayTag Block  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Block");
				FGameplayTag Dodge  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Dodge");
				FGameplayTag Jump   = FGameplayTag::RequestGameplayTag("State.Action.Combat.Jump");
				FGameplayTag Parry  = FGameplayTag::RequestGameplayTag("State.Action.Combat.Parry");
			}

			namespace Reaction
			{
				FGameplayTag NoReact      = FGameplayTag::RequestGameplayTag("State.Action.Reaction.None");
				FGameplayTag Hit          = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit");
				FGameplayTag Flinch       = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Flinch");
				FGameplayTag Stagger      = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Stagger");
				FGameplayTag Air          = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Air");
				FGameplayTag Launch       = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Launch");
				FGameplayTag Knockback    = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Knockback");
				FGameplayTag Knockdown    = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Knockdown");
				FGameplayTag BounceGround = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.BounceGround");
				FGameplayTag BounceWall   = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.BounceWall");
				FGameplayTag WallSplat    = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.WallSplat");
				FGameplayTag BlockBreak   = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.BlockBreak");
				FGameplayTag BlockHit     = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Block");
				FGameplayTag BlockPerfect = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.BlockPerfect");
				FGameplayTag Countered    = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Hit.Countered");
				FGameplayTag Dead         = FGameplayTag::RequestGameplayTag("State.Action.Reaction.Dead");
			}

		}

		namespace Movement
		{
			FGameplayTag Grounded     = FGameplayTag::RequestGameplayTag("State.Movement.Grounded");
			FGameplayTag Walk         = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Walking");
			FGameplayTag Grind        = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Grinding");
			FGameplayTag Climb        = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.Climbing");
			FGameplayTag WallRun      = FGameplayTag::RequestGameplayTag("State.Movement.Grounded.WallRunning");
			FGameplayTag Airborne     = FGameplayTag::RequestGameplayTag("State.Movement.Airborne");
			FGameplayTag Fall         = FGameplayTag::RequestGameplayTag("State.Movement.Airborne.Falling");
			FGameplayTag Fly          = FGameplayTag::RequestGameplayTag("State.Movement.Airborne.Flying");
		}
	}

	namespace Status
	{
		FGameplayTag MovementLocked    = FGameplayTag::RequestGameplayTag("Status.MovementLocked");
		FGameplayTag MoveStatsOverride = FGameplayTag::RequestGameplayTag("Status.MoveStatsOverride");

		namespace ActionBlocked
		{
			FGameplayTag Attack    = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Attack");
			FGameplayTag Block     = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Block");
			FGameplayTag Dodge     = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Dodge");
			FGameplayTag Jump      = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Jump");
			FGameplayTag LockOnOff = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.LockOnOff");
			FGameplayTag Look      = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Look");
			FGameplayTag Move      = FGameplayTag::RequestGameplayTag("Status.ActionBlocked.Move");
		}

		namespace ActionCancelableBy { FGameplayTag Move = FGameplayTag::RequestGameplayTag("Status.ActionCancelableBy.Move"); }
	}
}