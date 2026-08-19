#pragma once
#include "GameplayTagContainer.h"

namespace Tags
{
	namespace Attack
	{
		namespace Motion
		{
			extern FGameplayTag Dash;
			extern FGameplayTag Leap;
			extern FGameplayTag Stationary;
			extern FGameplayTag Teleport;
		}

		namespace Type
		{
			extern FGameplayTag AOE;
			extern FGameplayTag Melee;
			extern FGameplayTag Projectile;
		}

	}

	namespace Element
	{
		extern FGameplayTag None;
		extern FGameplayTag Air;
		extern FGameplayTag Electric;
		extern FGameplayTag Fire;
		extern FGameplayTag Ice;
		extern FGameplayTag Metal;
	}

	namespace NotifyEvent
	{
		namespace StateMachine
		{
			extern FGameplayTag ClearActionState;
			extern FGameplayTag DeathFreeze;
			extern FGameplayTag Grounded;
			extern FGameplayTag Jump;
		}

		namespace EnemyBrain
		{
			extern FGameplayTag AdvanceSequence;
			extern FGameplayTag ClearFocus;
			extern FGameplayTag Dash;
			extern FGameplayTag NotInterruptible;
			extern FGameplayTag RequestEvaluate;
			extern FGameplayTag SetFocus;
		}
	}

	namespace PlayerAction
	{
		extern FGameplayTag None;
		extern FGameplayTag Attack;
		extern FGameplayTag AttackHeavyStart;
		extern FGameplayTag AttackHeavyHold;
		extern FGameplayTag AttackHeavyRelease;
		extern FGameplayTag AttackLightStart;
		extern FGameplayTag AttackLightHold;
		extern FGameplayTag AttackLightRelease;
		extern FGameplayTag AttackNinjutsuStart;
		extern FGameplayTag AttackTether;
		extern FGameplayTag BlockStart;
		extern FGameplayTag BlockHold;
		extern FGameplayTag BlockRelease;
		extern FGameplayTag Dodge;
		extern FGameplayTag Fly;
		extern FGameplayTag Grind;
		extern FGameplayTag GrindJump;
		extern FGameplayTag JumpStart;
		extern FGameplayTag JumpRelease;
		extern FGameplayTag LockOnOffStart;
		extern FGameplayTag LookMouse;
		extern FGameplayTag LookStick;
		extern FGameplayTag Move;
		extern FGameplayTag TetherMeTo;  // Pull you toward something
		extern FGameplayTag TetherToMe;  // Pull something to you. Get Over Here!
		extern FGameplayTag TetherSwing; // Spider-Man Swing
		extern FGameplayTag WallJump;
		extern FGameplayTag WallRun;
	}

	namespace StateMachine
	{
		namespace Action
		{
			extern FGameplayTag None;

			namespace Combat
			{
				extern FGameplayTag Attack;
				extern FGameplayTag Block;
				extern FGameplayTag Dodge;
				extern FGameplayTag Jump;
				extern FGameplayTag Parry;
			}

			namespace Reaction
			{
				extern FGameplayTag Hit;
				extern FGameplayTag Flinch;
				extern FGameplayTag Stagger;
				extern FGameplayTag Air;
				extern FGameplayTag Launch;
				extern FGameplayTag Knockback;
				extern FGameplayTag Knockdown;
				extern FGameplayTag BlockBreak;
				extern FGameplayTag BlockHit;
				extern FGameplayTag BlockPerfect;
				extern FGameplayTag Countered;
				extern FGameplayTag Dead;
			}

		}

		namespace Movement
		{
			extern FGameplayTag Walk;
			extern FGameplayTag Grind;
			extern FGameplayTag Climb;
			extern FGameplayTag WallRun;
			extern FGameplayTag Fall;
			extern FGameplayTag Fly;
		}
	}

	namespace Status
	{
		extern FGameplayTag MovementLocked; // Physically can't move
		extern FGameplayTag MoveStatsOverride;

		namespace ActionBlocked
		{
			extern FGameplayTag Attack;
			extern FGameplayTag Block;
			extern FGameplayTag Dodge;
			extern FGameplayTag Jump;
			extern FGameplayTag LockOnOff;
			extern FGameplayTag Look;
			extern FGameplayTag Move;
		}

		namespace ActionCancelableBy { extern FGameplayTag Move; }
	}
}