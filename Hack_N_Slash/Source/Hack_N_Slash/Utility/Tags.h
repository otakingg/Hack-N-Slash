#pragma once
#include "GameplayTagContainer.h"

// These are all the tags used in the game
// They're defined here so they can be referenced in code without having to type the string name of the tag
// EX: Tags::Attack::Motion::Dash
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

	namespace NotifyEvent // Used animation notifies and/or notify states
	{
		namespace StateMachine // Evaluated by the State Machine
		{
			extern FGameplayTag ClearActionState;
			extern FGameplayTag IfDeadPauseMontage;
			extern FGameplayTag Jump;
			extern FGameplayTag TryBounceGround;
			extern FGameplayTag TryLand;
			extern FGameplayTag TryWallSplat;
		}

		namespace EnemyBrain // Evaluated by the Enemy Brain
		{
			extern FGameplayTag AdvanceSequence;
			extern FGameplayTag ClearFocus;
			extern FGameplayTag Dash;
			extern FGameplayTag NotInterruptible;
			extern FGameplayTag RequestEvaluate;
			extern FGameplayTag SetFocus;
		}
	}

	// Represents all the actions the player can take in the game
	// An "EPlayerInput" gets converted into a Player Action Tag by the State Machine
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

	// These represent all the different states characters can be in
	// They're used by the State Machine to determine what player inputs mean and what character actions are allowed
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
				extern FGameplayTag NoReact;
				extern FGameplayTag Hit;
				extern FGameplayTag Flinch;
				extern FGameplayTag Stagger;
				extern FGameplayTag Air;
				extern FGameplayTag Launch;
				extern FGameplayTag Knockback;
				extern FGameplayTag Knockdown;
				extern FGameplayTag BounceGround;
				extern FGameplayTag BounceWall;
				extern FGameplayTag WallSplat;
				extern FGameplayTag BlockBreak;
				extern FGameplayTag BlockHit;
				extern FGameplayTag BlockPerfect;
				extern FGameplayTag Countered;
				extern FGameplayTag Dead;
			}

		}

		namespace Movement
		{
			extern FGameplayTag Grounded;
			extern FGameplayTag Walk;
			extern FGameplayTag Grind;
			extern FGameplayTag Climb;
			extern FGameplayTag WallRun;
			extern FGameplayTag Airborne;
			extern FGameplayTag Fall;
			extern FGameplayTag Fly;
		}
	}

	// Represents different statuses that can be applied to characters
	// Heavily used by the State Machine to determine what character actions are allowed and what they're allowed to do
	namespace Status
	{
		extern FGameplayTag MovementLocked; // Physically can't move
		extern FGameplayTag MoveStatsOverride; // Character movement stats are overriden

		namespace ActionBlocked // A character action is blocked
		{
			extern FGameplayTag Attack;
			extern FGameplayTag Block;
			extern FGameplayTag Dodge;
			extern FGameplayTag Jump;
			extern FGameplayTag LockOnOff;
			extern FGameplayTag Look;
			extern FGameplayTag Move;
		}

		// Represents wether an action can cancel the effects of previous actions when it's happening
		// EX: Moving can cancel the current montage being played
		namespace ActionCancelableBy { extern FGameplayTag Move; }
	}
}