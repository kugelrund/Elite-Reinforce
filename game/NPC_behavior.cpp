//NPC_behavior.cpp
/*
FIXME - MCG:
These all need to make use of the snapshots.  Write something that can look for only specific
things in a snapshot or just go through the snapshot every frame and save the info in case
we need it...
*/

#include "b_local.h"
#include "anims.h"
#include "g_functions.h"
#include "g_nav.h"
#include "g_navigator.h"
#include "Q3_Interface.h"
extern CNavigator	navigator;
extern	qboolean	showBBoxes;
extern	cvar_t	*g_spskill;

extern qboolean NPC_EnemyTooFar(gentity_t *enemy, float dist, qboolean toShoot);
extern void NPC_AimWiggle( vec3_t enemy_org );
extern int PM_AnimLength( int index, animNumber_t anim );
static vec3_t BLUE = {0.0, 0.0, 1.0};
extern void CG_Cube( vec3_t mins, vec3_t maxs, vec3_t color, float alpha );

extern qboolean PM_HasAnimation( gentity_t *ent, int animation );
extern int PM_AnimLength( int index, animNumber_t anim );
extern qboolean PM_InOnGroundAnim (gentity_t *self);

#define	MAX_IDLE_ANIMS	8

/*
-------------------------
NPC_StandIdle
-------------------------
*/

void NPC_StandIdle( void )
{
/*
	//Must be done with any other animations
	if ( NPC->client->ps.legsAnimTimer != 0 )
		return;

	//Not ready to do another one
	if ( TIMER_Done( NPC, "idleAnim" ) == false )
		return;

	int anim = NPC->client->ps.legsAnim & ~ANIM_TOGGLEBIT;

	if ( anim != BOTH_STAND1 && anim != BOTH_STAND2 )
		return;

	//FIXME: Account for STAND1 or STAND2 here and set the base anim accordingly
	int	baseSeq = ( anim == BOTH_STAND1 ) ? BOTH_STAND1_RANDOM1 : BOTH_STAND2_RANDOM1;

	//Must have at least one random idle animation
	//NOTENOTE: This relies on proper ordering of animations, which SHOULD be okay
	if ( PM_HasAnimation( NPC, baseSeq ) == false )
		return;

	int	newIdle = Q_irand( 0, MAX_IDLE_ANIMS-1 );

	//FIXME: Technically this could never complete.. but that's not really too likely
	while( 1 )
	{
		if ( PM_HasAnimation( NPC, baseSeq + newIdle ) )
			break;

		newIdle = Q_irand( 0, MAX_IDLE_ANIMS );
	}
	
	//Start that animation going
	NPC_SetAnim( NPC, SETANIM_BOTH, baseSeq + newIdle, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
	
	int newTime = PM_AnimLength( NPC->client->clientInfo.animFileIndex, (animNumber_t) (baseSeq + newIdle) );

	//Don't do this again for a random amount of time
	TIMER_Set( NPC, "idleAnim", newTime + Q_irand( 2000, 10000 ) );
*/
}

/*
-------------------------
NPC_KeepHidingGoal
-------------------------
*/

void NPC_KeepHidingGoal(qboolean doMove)
{
	if( NPCInfo->hidingGoal )
	{
		vec3_t	diff;
		float	dist;

		VectorSubtract(NPCInfo->hidingGoal->currentOrigin, NPC->currentOrigin, diff);
		dist = VectorLengthSquared( diff );
		if( dist > MAX_GOAL_REACHED_DIST_SQUARED )
		{
			if( !NPCInfo->goalEntity )
			{//We're not going after anything, lets go for the hiding spot!
				NPCInfo->goalEntity = NPCInfo->hidingGoal;
				//NAV_ClearLastRoute(NPC);
				NPCInfo->goalTime = level.time + 100000;

				/*if(urgent)
				{//Get to our goal!  Don't come back till we're there
					NPCInfo->tempBehavior = BS_RUN;
					NPC_BSIdle();
					return;
				}
				else*/ if( doMove )
				{
					NPC_MoveToGoal();
				}
			}
		}
		else
		{//UHOH!  Nowhere to hide!
		}
	}
}

float NPC_MaxDistSquaredForWeapon (void);
//NOTE: BE SURE TO CHECK PVS BEFORE THIS!
qboolean NPC_CheckCanAttack (float attack_scale, qboolean stationary)
{
	vec3_t		delta, forward;
	vec3_t		angleToEnemy;
	vec3_t		hitspot, muzzle, diff, enemy_org;//, enemy_head;
	float		distanceToEnemy;
	qboolean	attack_ok = qfalse;
//	qboolean	duck_ok = qfalse;
	qboolean	dead_on = qfalse;
	float		aim_off;
	float		max_aim_off = 128 - (16 * (float)NPCInfo->stats.aim);
	trace_t		tr;
	gentity_t	*traceEnt = NULL;

	if(NPC->enemy->flags & FL_NOTARGET)
	{
		return qfalse;
	}

	//FIXME: only check to see if should duck if that provides cover from the
	//enemy!!!
	if(!attack_scale)
	{
		attack_scale = 1.0;
	}
	//Yaw to enemy
	CalcEntitySpot( NPC->enemy, SPOT_HEAD, enemy_org );
	NPC_AimWiggle( enemy_org );

	CalcEntitySpot( NPC, SPOT_WEAPON, muzzle );
	
	VectorSubtract (enemy_org, muzzle, delta);
	vectoangles ( delta, angleToEnemy );
	distanceToEnemy = VectorNormalize(delta);

	NPC->NPC->desiredYaw = angleToEnemy[YAW];
	NPC_UpdateFiringAngles(qfalse, qtrue);

	if( NPC_EnemyTooFar(NPC->enemy, distanceToEnemy*distanceToEnemy, qtrue) )
	{//Too far away?  Do not attack
		if(stationary)
		{//Need to start looking for another enemy eventually
			NPC->cantHitEnemyCounter++;
		}
		return qfalse;
	}

	if(client->fireDelay > 0)
	{//already waiting for a shot to fire
		NPC->NPC->desiredPitch = angleToEnemy[PITCH];
		NPC_UpdateFiringAngles(qtrue, qfalse);
		return qfalse;
	}

	if(NPCInfo->aiFlags & NPCAI_DONT_FIRE)
	{
		return qfalse;
	}

	NPCInfo->enemyLastVisibility = enemyVisibility;
	//See if they're in our FOV and we have a clear shot to them
	enemyVisibility = NPC_CheckVisibility ( NPC->enemy, CHECK_360|CHECK_FOV);////CHECK_PVS|

	if(enemyVisibility >= VIS_FOV)
	{//He's in our FOV
		
		attack_ok = qtrue;
		//CalcEntitySpot( NPC->enemy, SPOT_HEAD, enemy_head);

		//Check to duck
		if(NPC->enemy->client)
		{
			if(NPC->enemy->enemy == NPC)
			{
				if(NPC->enemy->client->buttons & BUTTON_ATTACK)
				{//FIXME: determine if enemy fire angles would hit me or get close
					if(NPC_CheckDefend(1.0))//FIXME: Check self-preservation?  Health?
					{//duck and don't shoot
						attack_ok = qfalse;
						ucmd.upmove = -127;
					}
				}
			}
		}

		if(attack_ok)
		{
			//are we gonna hit him
			gi.trace ( &tr, muzzle, NULL, NULL, enemy_org, NPC->s.number, MASK_SHOT );
			ShotThroughGlass(&tr, NPC->enemy, enemy_org, MASK_SHOT);

			traceEnt = &g_entities[tr.entityNum];

			/*
			if( traceEnt != NPC->enemy &&//FIXME: if someone on our team is in the way, suggest that they duck if possible
				(!traceEnt || !traceEnt->client || !NPC->client->enemyTeam || NPC->client->enemyTeam != traceEnt->client->playerTeam) )
			{//no, so shoot for somewhere between the head and torso
				//NOTE: yes, I know this looks weird, but it works
				enemy_org[0] += 0.3*Q_flrand(NPC->enemy->mins[0], NPC->enemy->maxs[0]);
				enemy_org[1] += 0.3*Q_flrand(NPC->enemy->mins[1], NPC->enemy->maxs[1]);
				enemy_org[2] -= NPC->enemy->maxs[2]*Q_flrand(0.0f, 1.0f);

				attack_scale *= 0.75;
				gi.trace ( &tr, muzzle, NULL, NULL, enemy_org, NPC->s.number, MASK_SHOT );
				ShotThroughGlass(&tr, NPC->enemy, enemy_org, MASK_SHOT);
				traceEnt = &g_entities[tr.entityNum];
			}
			*/

			VectorCopy( tr.endpos, hitspot );

			if( traceEnt == NPC->enemy || (traceEnt->client && NPC->client->enemyTeam && NPC->client->enemyTeam == traceEnt->client->playerTeam) )
			{
				dead_on = qtrue;
			}
			else
			{
				attack_scale *= 0.5;
				if(NPC->client->playerTeam)
				{
					if(traceEnt && traceEnt->client && traceEnt->client->playerTeam)
					{
						if(NPC->client->playerTeam == traceEnt->client->playerTeam)
						{//Don't shoot our own team
							attack_ok = qfalse;
						}
					}
				}
			}
		}

		if( attack_ok )
		{
			//ok, now adjust pitch aim
			VectorSubtract (hitspot, muzzle, delta);
			vectoangles ( delta, angleToEnemy );
			NPC->NPC->desiredPitch = angleToEnemy[PITCH];
			NPC_UpdateFiringAngles(qtrue, qfalse);

			if( !dead_on )
			{//We're not going to hit him directly, try a suppressing fire
				//see if where we're going to shoot is too far from his origin
				if(traceEnt && (traceEnt->health <= 30 || EntIsGlass(traceEnt)))
				{//easy to kill - go for it
					if(traceEnt->e_DieFunc == dieF_ExplodeDeath_Wait && traceEnt->splashDamage)
					{//going to explode, don't shoot if close to self
						VectorSubtract(NPC->currentOrigin, traceEnt->currentOrigin, diff);
						if(VectorLengthSquared(diff) < traceEnt->splashRadius*traceEnt->splashRadius)
						{//Too close to shoot!
							attack_ok = qfalse;
						}
						else 
						{//Hey, it might kill him, do it!
							attack_scale *= 2;//
						}
					}
				}
				else
				{
					AngleVectors (client->ps.viewangles, forward, NULL, NULL);
					VectorMA ( muzzle, distanceToEnemy, forward, hitspot);
					VectorSubtract(hitspot, enemy_org, diff);
					aim_off = VectorLength(diff);
					if(aim_off > random() * max_aim_off)//FIXME: use aim value to allow poor aim?
					{
						attack_scale *= 0.75;
						//see if where we're going to shoot is too far from his head
						VectorSubtract(hitspot, enemy_org, diff);
						aim_off = VectorLength(diff);
						if(aim_off > random() * max_aim_off)
						{
							attack_ok = qfalse;
						}
					}
					attack_scale *= (max_aim_off - aim_off + 1)/max_aim_off;
				}
			}
		}
	}
	else
	{//Update pitch anyway
		NPC->NPC->desiredPitch = angleToEnemy[PITCH];
		NPC_UpdateFiringAngles(qtrue, qfalse);
	}

	if( attack_ok )
	{
		if( NPC_CheckAttack( attack_scale ))
		{//check aggression to decide if we should shoot
			enemyVisibility = VIS_SHOOT;
			WeaponThink(qtrue);
		}
		else
			attack_ok = qfalse;
	}
	else if(stationary)
	{//Need to start looking for another enemy eventually
		NPC->cantHitEnemyCounter++;
	}

	return attack_ok;
}

qboolean NPC_StandTrackAndShoot (qboolean canDuck)
{
	qboolean	attack_ok = qfalse;
	qboolean	duck_ok = qfalse;
	qboolean	faced = qfalse;
	float		attack_scale = 1.0;

	//First see if we're hurt bad- if so, duck
	//FIXME: if even when ducked, we can shoot someone, we should.
	//Maybe is can be shot even when ducked, we should run away to the nearest cover?
	if ( canDuck )
	{
		if ( NPC->health < 20 )
		{
			if( NPC->svFlags&SVF_HEALING || random() )
			{
				duck_ok = qtrue;
			}
		}
		else if ( NPC->health < 40 )
		{
			if ( NPC->svFlags&SVF_HEALING )
			{//Medic is on the way, get down!
				duck_ok = qtrue;
			}
			else if ( NPC->client->playerTeam!= TEAM_BORG )
			{//Borg don't care if they're about to die
				//attack_scale will be a max of .66
				attack_scale = NPC->health/60;
			}
		}
	}

	//NPC_CheckEnemy( qtrue, qfalse );

	if ( !duck_ok )
	{//made this whole part a function call
		attack_ok = NPC_CheckCanAttack( attack_scale, qtrue );
		faced = qtrue;
	}

	if ( canDuck && (duck_ok || (!attack_ok && client->fireDelay == 0)) && ucmd.upmove != -127 )
	{//if we didn't attack check to duck if we're not already
		if( !duck_ok )
		{
			if ( NPC->enemy->client )
			{
				if ( NPC->enemy->enemy == NPC )
				{
					if ( NPC->enemy->client->buttons & BUTTON_ATTACK )
					{//FIXME: determine if enemy fire angles would hit me or get close
						if ( NPC_CheckDefend( 1.0 ) )//FIXME: Check self-preservation?  Health?
						{
							duck_ok = qtrue;
						}
					}
				}
			}
		}

		if ( duck_ok )
		{//duck and don't shoot
			attack_ok = qfalse;
			ucmd.upmove = -127;
			NPCInfo->duckDebounceTime = level.time + 1000;//duck for a full second
		}
	}

	return faced;
}


void NPC_BSIdle( void ) 
{
	//FIXME if there is no nav data, we need to do something else
	// if we're stuck, try to move around it
	if ( blocked ) 
	{
		//FIXME: make us step to one side and back up, make other NPC, if a NPC,
		//step to other side and back up
		//do this for 1 full second
		//FIXME: Only push each other if vec to other is within 0.5 dot product of movedir
		if(random() > 0.5)
			ucmd.rightmove = 127;
		else
			ucmd.rightmove = -127;

//		Debug_NPCPrintf ( NPC, debugNPCAI, DEBUG_LEVEL_INFO, "NPCAI: stuck\n" );
//		NPC_ClearGoal();
//		NPCInfo->blockTime = 0;
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}
		ucmd.forwardmove = 0;
	}
	else
	{
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}
	}

	if ( ( ucmd.forwardmove == 0 ) && ( ucmd.rightmove == 0 ) && ( ucmd.upmove == 0 ) )
	{
//		NPC_StandIdle();
	}

	NPC_UpdateAngles( qtrue, qtrue );
	ucmd.buttons |= BUTTON_WALKING;
}

void NPC_BSRoam (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

void NPC_BSWalk (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

void NPC_BSCrouch (void)
{
	NPC_KeepHidingGoal( qfalse );
	if ( NPC->enemy )
	{
		NPC_CheckEnemy( qfalse, qfalse );
		NPC_CheckCanAttack( 1.0, (!ucmd.forwardmove&&!ucmd.rightmove) );
	}
	else
	{//only look for enemy if don't have one
		NPC_BSIdle();
		NPC_CheckEnemy( qtrue, qfalse );
	}

	ucmd.upmove = -127;
	ucmd.buttons &= ~BUTTON_WALKING;
}

qboolean NPC_CheckSlide(float dist)
{
	vec3_t	mins, right, endpos, start;
	trace_t	trace;

	if(!dist)
	{
		return qfalse;
	}

	VectorCopy(NPC->mins, mins);
	mins[2] += 18;

	AngleVectors(NPC->client->ps.viewangles, NULL, right, NULL);
	VectorMA(NPC->currentOrigin, dist, right, endpos);

	gi.trace(&trace, NPC->currentOrigin, mins, NPC->maxs, endpos, NPC->s.number, MASK_SOLID);
	if(trace.fraction < 1.0)
	{//can't get that far
		return qfalse;
	}
	
	VectorCopy(endpos, start);
	endpos[2] -= 19;

	gi.trace(&trace, start, NPC->mins, NPC->maxs, endpos, NPC->s.number, MASK_SOLID);
	if(trace.fraction == 1.0)
	{//will fall off
		return qfalse;
	}

	return qtrue;
}

void NPC_BSRun (void)
{
	//FIXME if there is no nav data, we need to do something else
	// if we're stuck, try to move around it
	if ( blocked ) 
	{
		//FIXME:  Blocked doesn't tell the NPC he's been pushing another stationary
		//NPC all over the place!

		//FIXME: make us step to one side and back up, make other NPC, if a NPC,
		//step to other side and back up
		//do this for 1 full second

		//FIXME: Only push each other if vec to other is within 0.5 dot product of movedir

//		Debug_NPCPrintf ( NPC, debugNPCAI, DEBUG_LEVEL_INFO, "NPCAI: stuck\n" );

		if( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}

		if(Q_irand(0, 1))
		{//fixme - make these calls use speed to determine length of slidecheck
			if(NPC_CheckSlide( sqrt(NPC->maxs[0]+NPC->maxs[1]) ))
				ucmd.rightmove = 127;
			else
				ucmd.rightmove = -127;
		}
		else
		{
			if(NPC_CheckSlide( sqrt(NPC->maxs[0]+NPC->maxs[1]) * -1 ))
				ucmd.rightmove = -127;
			else
				ucmd.rightmove = 127;
		}

/*
		if(NPCInfo->blockTime < level.time)
		{
			NPCInfo->blockTime = level.time + 1000;//do this for 2 seconds
		}
*/
//		ucmd.forwardmove = 0;//???
	}
	else
	{
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
			/*
			if(!(NPC->s.torsoAnim&ANIM_TOGGLEBIT ))
			{//only set if anim wasn't just set this turn already
				NPC->s.torsoAnim = TORSO_RUN;
			}*/
		}
		else
		{//achieved our goal - clear any temp behavior
			if ( NPCInfo->tempBehavior == BS_RUN )
			{//FIXME: Check others that call this too?
				NPCInfo->tempBehavior = BS_DEFAULT;
			}
		}
	}

	if(NPCInfo->behaviorState == BS_STAND_AND_SHOOT ||
		NPCInfo->behaviorState == BS_TAKECOVER )
	{//We're supposed to be hiding, so DUCK!
		ucmd.upmove = -127;
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

void NPC_BSStandGuard (void)
{
	//FIXME: Use Snapshot info
	if ( NPC->enemy == NULL )
	{//Possible to pick one up by being shot
		if( random() < 0.5 )
		{
			if(NPC->client->enemyTeam)
			{
				gentity_t *newenemy = NPC_PickEnemy(NPC, NPC->client->enemyTeam, (NPC->cantHitEnemyCounter < 10), (NPC->client->enemyTeam == TEAM_STARFLEET), qtrue);
				//only checks for vis if couldn't hit last enemy
				if(newenemy)
				{
					G_SetEnemy( NPC, newenemy );
				}
			}
		}
	}

	if ( NPC->enemy != NULL )
	{
		if( NPCInfo->tempBehavior == BS_STAND_GUARD )
		{
			NPCInfo->tempBehavior = BS_DEFAULT;
		}
		
		if( NPCInfo->behaviorState == BS_STAND_GUARD )
		{
			NPCInfo->behaviorState = BS_STAND_AND_SHOOT;
		}
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSHuntAndKill
-------------------------
*/

void NPC_BSHuntAndKill( void )
{
	qboolean	turned = qfalse;
	vec3_t		vec;
	float		enemyDist;
	visibility_t	oEVis;
	int			curAnim;

	NPC_CheckEnemy( NPCInfo->tempBehavior != BS_HUNT_AND_KILL, NPC->client->ps.weapon == WP_BORG_ASSIMILATOR );//don't find new enemy if this is tempbehav

	if ( NPC->enemy )
	{
		oEVis = enemyVisibility = NPC_CheckVisibility ( NPC->enemy, CHECK_FOV|CHECK_SHOOT );//CHECK_360|//CHECK_PVS|
		if(enemyVisibility > VIS_PVS)
		{
			if ( !NPC_EnemyTooFar( NPC->enemy, 0, qtrue ) )
			{//Enemy is close enough to shoot - FIXME: this next func does this also, but need to know here for info on whether ot not to turn later
				NPC_CheckCanAttack( 1.0, qfalse );
				turned = qtrue;
			}
		}

		curAnim = (NPC->client->ps.legsAnim&~ANIM_TOGGLEBIT);
		if(curAnim != BOTH_ATTACK1 && curAnim != BOTH_ATTACK2 && curAnim != BOTH_ATTACK3 && curAnim != BOTH_MELEE1 && curAnim != BOTH_MELEE2 )
		{//Don't move toward enemy if we're in a full-body attack anim
			//FIXME, use IdealDistance to determin if we need to close distance
			VectorSubtract(NPC->enemy->currentOrigin, NPC->currentOrigin, vec);
			enemyDist = VectorLength(vec);
			if( enemyDist > 48 && ((enemyDist*1.5)*(enemyDist*1.5) >= NPC_MaxDistSquaredForWeapon() ||
				oEVis != VIS_SHOOT ||
				//!(ucmd.buttons & BUTTON_ATTACK) || 
				enemyDist > IdealDistance(NPC)*3 ) )
			{//We should close in?
				NPCInfo->goalEntity = NPC->enemy;

				NPC_MoveToGoal();
			}
			else if(enemyDist < IdealDistance(NPC))
			{//We should back off?
				//if(ucmd.buttons & BUTTON_ATTACK)
				{
					NPCInfo->goalEntity = NPC->enemy;
					NPCInfo->goalRadius = 12;
					NPC_MoveToGoal();

					ucmd.forwardmove *= -1;
					ucmd.rightmove *= -1;

					ucmd.buttons |= BUTTON_WALKING;
				}
			}//otherwise, stay where we are
		}
	}
	else 
	{//ok, stand guard until we find an enemy
		if( NPCInfo->tempBehavior == BS_HUNT_AND_KILL )
		{
			NPCInfo->tempBehavior = BS_DEFAULT;
		}
		else
		{
			NPCInfo->tempBehavior = BS_STAND_GUARD;
			NPC_BSStandGuard();
		}
		return;
	}

	if(!turned)
	{
		NPC_UpdateAngles(qtrue, qtrue);
	}
}

void NPC_BSStandAndShoot (void)
{
	//FIXME:
	//When our numbers outnumber enemies 3 to 1, or only one of them,
	//go into hunt and kill mode

	//FIXME:
	//When they're all dead, go to some script or wander off to sickbay?
	
	if(NPC->client->playerTeam && NPC->client->enemyTeam)
	{
		//FIXME: don't realize this right away- or else enemies show up and we're standing around
		/*
		if( teamNumbers[NPC->enemyTeam] == 0 )
		{//ok, stand guard until we find another enemy
			//reset our rush counter
			teamCounter[NPC->playerTeam] = 0;
			NPCInfo->tempBehavior = BS_STAND_GUARD;
			NPC_BSStandGuard();
			return;
		}*/
		/*
		//FIXME: whether to do this or not should be settable
		else if( NPC->playerTeam != TEAM_BORG )//Borg don't rush
		{
		//FIXME: In case reinforcements show up, we should wait a few seconds
		//and keep checking before rushing!
		//Also: what if not everyone on our team is going after playerTeam?
		//Also: our team count includes medics!
			if(NPC->health > 25)
			{//Can we rush the enemy?
				if(teamNumbers[NPC->enemyTeam] == 1 ||
					teamNumbers[NPC->playerTeam] >= teamNumbers[NPC->enemyTeam]*3)
				{//Only one of them or we outnumber 3 to 1
					if(teamStrength[NPC->playerTeam] >= 75 ||
						(teamStrength[NPC->playerTeam] >= 50 && teamStrength[NPC->playerTeam] > teamStrength[NPC->enemyTeam]))
					{//Our team is strong enough to rush
						teamCounter[NPC->playerTeam]++;
						if(teamNumbers[NPC->playerTeam] * 17 <= teamCounter[NPC->playerTeam])
						{//ok, we waited 1.7 think cycles on average and everyone is go, let's do it!
							//FIXME: Should we do this to everyone on our team?
							NPCInfo->behaviorState = BS_HUNT_AND_KILL;
							//FIXME: if the tide changes, we should retreat!
							//FIXME: when do we reset the counter?
							NPC_BSHuntAndKill ();
							return;
						}
					}
					else//Oops!  Something's wrong, reset the counter to rush
						teamCounter[NPC->playerTeam] = 0;
				}
				else//Oops!  Something's wrong, reset the counter to rush
					teamCounter[NPC->playerTeam] = 0;
			}
		}
		*/
	}

	NPC_CheckEnemy(qtrue, qfalse);
	
	if(NPCInfo->duckDebounceTime > level.time && 
		NPC->client->ps.weapon != WP_KLINGON_BLADE &&
		NPC->client->ps.weapon != WP_IMPERIAL_BLADE )
	{//we're hiding!
		NPC_KeepHidingGoal(qtrue);
		ucmd.upmove = -127;
		if(NPC->enemy)
		{
			NPC_CheckCanAttack(1.0, qtrue);
		}
		return;		
	}

	if(NPC->enemy)
	{
		//Let's check and see if we're still at our hiding spot
		NPC_KeepHidingGoal(qtrue);

		if(!NPC_StandTrackAndShoot( qtrue ))
		{//That func didn't update our angles
			NPCInfo->desiredYaw = NPC->client->ps.viewangles[YAW];
			NPCInfo->desiredPitch = NPC->client->ps.viewangles[PITCH];
			NPC_UpdateAngles(qtrue, qtrue);
		}
	}
	else
	{
		NPCInfo->desiredYaw = NPC->client->ps.viewangles[YAW];
		NPCInfo->desiredPitch = NPC->client->ps.viewangles[PITCH];
		NPC_UpdateAngles(qtrue, qtrue);
//		NPC_BSIdle();//only moves if we have a goal
	}
}

void NPC_BSAreaCombat (void)
{//FIXME: Move as needed between a few selected battle waypoints
	//while looking for enemies to fire
}

void NPC_BSEvade (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

void NPC_BSEvadeAndShoot (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

void NPC_BSRunAndShoot (void)
{
	/*if(NPC->playerTeam && NPC->enemyTeam)
	{
		//FIXME: don't realize this right away- or else enemies show up and we're standing around
		if( teamNumbers[NPC->enemyTeam] == 0 )
		{//ok, stand guard until we find another enemy
			//reset our rush counter
			teamCounter[NPC->playerTeam] = 0;
			NPCInfo->tempBehavior = BS_STAND_GUARD;
			NPC_BSStandGuard();
			return;
		}
	}*/

	//NOTE: are we sure we want ALL run and shoot people to move this way?
	//Shouldn't it check to see if we have an enemy and our enemy is our goal?!
	//Moved that check into NPC_MoveToGoal
	//NPCInfo->combatMove = qtrue;

	NPC_CheckEnemy( qtrue, qfalse );
	
	if ( NPCInfo->duckDebounceTime > level.time && NPCInfo->hidingGoal )
	{//we're hiding!
		NPC_KeepHidingGoal( qtrue );
		ucmd.upmove = -127;
		if ( NPC->enemy )
		{
			NPC_CheckCanAttack( 1.0, qfalse );
		}
		return;		
	}

	if ( NPC->enemy )
	{
		int monitor = NPC->cantHitEnemyCounter;
		NPC_StandTrackAndShoot( NPCInfo->hidingGoal != NULL );

		if ( !(ucmd.buttons & BUTTON_ATTACK) && ucmd.upmove >= 0 && NPC->cantHitEnemyCounter > monitor )
		{//not crouching and not firing
			vec3_t	vec;

			VectorSubtract( NPC->enemy->currentOrigin, NPC->currentOrigin, vec );
			vec[2] = 0;
			if ( VectorLength( vec ) > 128 || NPC->cantHitEnemyCounter >= 10 )
			{//run at enemy if too far away
				//The cantHitEnemyCounter getting high has other repercussions
				//100 (10 seconds) will make you try to pick a new enemy... 
				//But we're chasing, so we clamp it at 50 here
				if ( NPC->cantHitEnemyCounter > 60 )
				{
					NPC->cantHitEnemyCounter = 60;
				}
				
				if ( NPC->cantHitEnemyCounter >= (NPCInfo->stats.aggression+1) * 10 )
				{
					NPC_LostEnemyDecideChase();
				}

				//chase and face
				ucmd.angles[YAW] = 0;
				ucmd.angles[PITCH] = 0;
				NPCInfo->goalEntity = NPC->enemy;
				NPCInfo->goalRadius = 12;
				//NAV_ClearLastRoute(NPC);
				NPC_MoveToGoal();
				NPC_UpdateAngles(qtrue, qtrue);
			}
			else
			{
				//FIXME: this could happen if they're just on the other side
				//of a thin wall or something else blocking out shot.  That
				//would make us just stand there and not go around it...
				//but maybe it's okay- might look like we're waiting for
				//him to come out...?  
				//Current solution: runs around if cantHitEnemyCounter gets
				//to 10 (1 second).  
			}
		}
		else
		{//Clear the can't hit enemy counter here
			NPC->cantHitEnemyCounter = 0;
		}
	}
	else
	{
		NPC_BSRun();//only moves if we have a goal
	}
}

void NPC_BSDefend (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

void NPC_BSCombat (void)
{
	//FIXME: IMPLEMENT
	NPC_BSIdle();
}

/*
 void NPC_BSGetAmmo (void)

Go and get ammo until full or can;t find any more

See what ammo or weapon I need - none, return to what we were doing
Check Snapshot to see if there's anything around I can use
Go get it if so - if not: ??? (Keep looking?  Hide?)
*/
void NPC_BSGetAmmo (void)
{//FIXME: IMPLEMENT
	NPC_BSIdle();
}

/*
 void NPC_BSAdvanceFight (void)

Advance towards your captureGoal and shoot anyone you can along the way.
*/
void NPC_BSAdvanceFight (void)
{//FIXME: IMPLEMENT
//Head to Goal if I can

	//Make sure we're still headed where we want to capture
	if ( NPCInfo->captureGoal )
	{//FIXME: if no captureGoal, what do we do?
		//VectorCopy( NPCInfo->captureGoal->currentOrigin, NPCInfo->tempGoal->currentOrigin );
		//NPCInfo->goalEntity = NPCInfo->tempGoal;

		NPC_SetMoveGoal( NPC, NPCInfo->captureGoal->currentOrigin, 16, qtrue );

//		NAV_ClearLastRoute(NPC);
		NPCInfo->goalTime = level.time + 100000;
	}

	NPC_BSRun();

	NPC_CheckEnemy(qtrue, qfalse);

	//FIXME: Need melee code
	if( NPC->enemy )
	{//See if we can shoot him
		vec3_t		delta, forward;
		vec3_t		angleToEnemy;
		vec3_t		hitspot, muzzle, diff, enemy_org, enemy_head;
		float		distanceToEnemy;
		qboolean	attack_ok = qfalse;
		qboolean	dead_on = qfalse;
		float		attack_scale = 1.0;
		float		aim_off;
		float		max_aim_off = 64;

		//Yaw to enemy
		VectorMA(NPC->enemy->absmin, 0.5, NPC->enemy->maxs, enemy_org);
		CalcEntitySpot( NPC, SPOT_WEAPON, muzzle );
		
		VectorSubtract (enemy_org, muzzle, delta);
		vectoangles ( delta, angleToEnemy );
		distanceToEnemy = VectorNormalize(delta);

		if(!NPC_EnemyTooFar(NPC->enemy, distanceToEnemy*distanceToEnemy, qtrue))
		{
			attack_ok = qtrue;
		}

		if(attack_ok)
		{
			NPC_UpdateShootAngles(angleToEnemy, qfalse, qtrue);

			NPCInfo->enemyLastVisibility = enemyVisibility;
			enemyVisibility = NPC_CheckVisibility ( NPC->enemy, CHECK_FOV);//CHECK_360|//CHECK_PVS|

			if(enemyVisibility == VIS_FOV)
			{//He's in our FOV
				
				attack_ok = qtrue;
				CalcEntitySpot( NPC->enemy, SPOT_HEAD, enemy_head);

				if(attack_ok)
				{
					trace_t		tr;
					gentity_t	*traceEnt;
					//are we gonna hit him if we shoot at his center?
					gi.trace ( &tr, muzzle, NULL, NULL, enemy_org, NPC->s.number, MASK_SHOT );
					traceEnt = &g_entities[tr.entityNum];
					if( traceEnt != NPC->enemy &&
						(!traceEnt || !traceEnt->client || !NPC->client->enemyTeam || NPC->client->enemyTeam != traceEnt->client->playerTeam) )
					{//no, so shoot for the head
						attack_scale *= 0.75;
						gi.trace ( &tr, muzzle, NULL, NULL, enemy_head, NPC->s.number, MASK_SHOT );
						traceEnt = &g_entities[tr.entityNum];
					}

					VectorCopy( tr.endpos, hitspot );

					if( traceEnt == NPC->enemy || (traceEnt->client && NPC->client->enemyTeam && NPC->client->enemyTeam == traceEnt->client->playerTeam) )
					{
						dead_on = qtrue;
					}
					else
					{
						attack_scale *= 0.5;
						if(NPC->client->playerTeam)
						{
							if(traceEnt && traceEnt->client && traceEnt->client->playerTeam)
							{
								if(NPC->client->playerTeam == traceEnt->client->playerTeam)
								{//Don't shoot our own team
									attack_ok = qfalse;
								}
							}
						}
					}
				}

				if( attack_ok )
				{
					//ok, now adjust pitch aim
					VectorSubtract (hitspot, muzzle, delta);
					vectoangles ( delta, angleToEnemy );
					NPC->NPC->desiredPitch = angleToEnemy[PITCH];
					NPC_UpdateShootAngles(angleToEnemy, qtrue, qfalse);

					if( !dead_on )
					{//We're not going to hit him directly, try a suppressing fire
						//see if where we're going to shoot is too far from his origin
						AngleVectors (NPCInfo->shootAngles, forward, NULL, NULL);
						VectorMA ( muzzle, distanceToEnemy, forward, hitspot);
						VectorSubtract(hitspot, enemy_org, diff);
						aim_off = VectorLength(diff);
						if(aim_off > random() * max_aim_off)//FIXME: use aim value to allow poor aim?
						{
							attack_scale *= 0.75;
							//see if where we're going to shoot is too far from his head
							VectorSubtract(hitspot, enemy_head, diff);
							aim_off = VectorLength(diff);
							if(aim_off > random() * max_aim_off)
							{
								attack_ok = qfalse;
							}
						}
						attack_scale *= (max_aim_off - aim_off + 1)/max_aim_off;
					}
				}
			}
		}

		if( attack_ok )
		{
			if( NPC_CheckAttack( attack_scale ))
			{//check aggression to decide if we should shoot
				enemyVisibility = VIS_SHOOT;
				WeaponThink(qtrue);
			}
			else
				attack_ok = qfalse;
		}
//Don't do this- only for when stationary and trying to shoot an enemy
//		else
//			NPC->cantHitEnemyCounter++;
	}
	else
	{//FIXME: 
		NPC_UpdateShootAngles(NPC->client->ps.viewangles, qtrue, qtrue);
	}

	if(!ucmd.forwardmove && !ucmd.rightmove)
	{//We reached our captureGoal
		if(NPC->taskManager)
		{
			Q3_TaskIDComplete( NPC, TID_BSTATE );
		}
	}
}

void NPC_BSTakeCover (void)
{
//	gentity_t	*found = NULL;
//look for closest empty cover point that gives cover from enemy

	NPC_KeepHidingGoal(qtrue);

	if( NPCInfo->goalEntity )
	{
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}
	}
	NPC_UpdateAngles(qtrue, qtrue);
}

void Disappear(gentity_t *self)
{
//	ClientDisconnect(self);
	self->s.eFlags |= EF_NODRAW;
	self->e_ThinkFunc = thinkF_NULL;
	self->nextthink = -1;
}

void MakeOwnerInvis (gentity_t *self);
void BeamOut (gentity_t *self)
{
//	gentity_t *tent = G_Spawn();
	
	self->client->ps.powerups[PW_QUAD] = level.time + 2000;
/*
	tent->owner = self;
	tent->think = MakeOwnerInvis;
	tent->nextthink = level.time + 1800;
	//G_AddEvent( ent, EV_PLAYER_TELEPORT, 0 );
	tent = G_TempEntity( self->client->pcurrentOrigin, EV_PLAYER_TELEPORT );
*/
	//fixme: doesn't actually go away!
	self->nextthink = level.time + 1500;
	self->e_ThinkFunc = thinkF_Disappear;
	self->client->squadname = NULL;
	self->client->playerTeam = TEAM_FREE;
	self->svFlags |= SVF_BEAMING;
}

void NPC_MedicHeal( gentity_t *patient )
{
	patient->health += ((3 - g_spskill->integer) * 4);
	if ( patient->health > patient->max_health )
	{
		patient->health = patient->max_health;
	}
}

void NPC_ClearPatient (void)
{
	NPC->svFlags &= ~SVF_HEALING;
	//NPC->s.loopSound = 0;
	if ( NPCInfo->eventualGoal )
	{
		NPCInfo->eventualGoal->svFlags &= ~SVF_MUST_HEAL;
		NPCInfo->eventualGoal->svFlags &= ~SVF_HEALING;
//		if( NPCInfo->eventualGoal->health <= 0 )
//			G_Sound(NPC, G_SoundIndex("sound/mgtest/deadjim.wav"));
		if ( NPC->client->renderInfo.lookTarget == NPCInfo->eventualGoal->s.number )
		{
			NPC->client->renderInfo.lookTarget = ENTITYNUM_NONE;
		}
	}
	NPC_ClearGoal();
	NPCInfo->eventualGoal = NULL;
}

void NPC_TakePatient (gentity_t *patient)
{
	NPC_ClearPatient();

	patient->svFlags |= SVF_HEALING;
	patient->svFlags |= SVF_MUST_HEAL;
	SetGoal(patient, 0.0);
	NPCInfo->eventualGoal = patient;
	NPC->client->renderInfo.lookTarget = patient->s.number;
}

qboolean isMedic (gentity_t *self)
{
	if(!self->NPC)
		return qfalse;

	if(self->NPC->behaviorState == BS_MEDIC ||
		self->NPC->behaviorState == BS_MEDIC_COMBAT ||
		self->NPC->behaviorState == BS_MEDIC_HIDE)
		return qtrue;

	return qfalse;
}

void NPC_BSMedic_Go (void)
{
	vec3_t		diff;
	float		dist;

	if ( UpdateGoal() )
	{
		//did this so he doesn't follow you around
		if ( NPCInfo->behaviorState != BS_MEDIC_HIDE )
		{
			NPC_MoveToGoal();
		}

		NPC_UpdateAngles(qtrue, qtrue);
	}

	VectorSubtract(NPCInfo->eventualGoal->currentOrigin, NPC->currentOrigin, diff);
	dist = VectorLength(diff);

	if( dist < 96 )
	{//We're there! - fixme: line of sight?
		//DUCK! - FIXME: what if we can't reach our guy like this?
		if( !(NPC->svFlags&SVF_HEALING) )
		{
			NPC->svFlags |= SVF_HEALING;
			//FIXME: needs to loop aound until done
//			G_Sound(NPC, CHAN_ITEM, G_SoundIndex("sound/mgtest/medhndsc.wav"));
			//FIXME: loopsound doesn't wqork for clients
			//NPC->s.loopSound = G_SoundIndex("sound/ambience/voyager/medictricorder.wav");
		}

		if( (NPC->s.torsoAnim&~ANIM_TOGGLEBIT) != TORSO_MEDICORDER1 )
		{
			NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_MEDICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);
		}

		if ( NPCInfo->behaviorState != BS_MEDIC && NPCInfo->eventualGoal && NPCInfo->eventualGoal->client && 
			NPCInfo->eventualGoal->client->usercmd.upmove < 0 )
		{
			ucmd.upmove = -127;
		}

		if( NPC_CheckVisibility( NPCInfo->eventualGoal, CHECK_PVS|CHECK_360|CHECK_FOV ) == VIS_FOV)
		{//FIXME: can't check to shoot, on same team, will return false
			if((NPCInfo->eventualGoal->health <= 0 || PM_InOnGroundAnim(NPCInfo->eventualGoal)) && NPCInfo->behaviorState == BS_MEDIC && NPCInfo->eventualGoal->client)
			{//if in regular medic mode and someone is dead or can't move, beam them out
				BeamOut(NPCInfo->eventualGoal);
				NPC_ClearPatient();
				//FIXME: crouch and stick around here for a bit...
				return;
			}

			if(NPCInfo->eventualGoal->health < NPCInfo->eventualGoal->max_health && NPCInfo->eventualGoal->health > 0)
			{//can't raise the dead, sorry
//				gi.Printf( "%s healing %s\n", NPC->targetname, NPCInfo->eventualGoal->targetname );
				NPC_MedicHeal( NPCInfo->eventualGoal );
//				NPCInfo->eventualGoal->health += Q_irand(0,1);
			}
			else
			{
				//NPC->s.loopSound = 0;
			}

			NPC->NPC->desiredYaw = vectoyaw ( diff );

			NPC_UpdateAngles(qtrue, qtrue);
		}
		else if (!NPCInfo->goalEntity)
		{//face him - only if we didn't already face above
			vec3_t		goal_org, angleToGoal, muzzle, delta;

			VectorMA(NPCInfo->eventualGoal->absmin, 0.5, NPCInfo->eventualGoal->maxs, goal_org);
			CalcEntitySpot( NPC, SPOT_WEAPON, muzzle );
			
			VectorSubtract (goal_org, muzzle, delta);
			vectoangles ( delta, angleToGoal );
			NPC->NPC->desiredYaw = angleToGoal[YAW];
			NPC->NPC->desiredPitch = angleToGoal[PITCH];
			//FIXME:  He shakes- what's up with that?
			NPC_UpdateAngles(qtrue, qtrue);
		}
	}
	else if (!NPCInfo->goalEntity)
	{//He walked away!  go after him!
		NPC->svFlags &= ~SVF_HEALING;//clear the sound flag to play again
		SetGoal(NPCInfo->eventualGoal, 0.0);

		NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);//this should be overridden by walk/run anims

//		Nav_PathToEnt ( NPC, NPCInfo->goalEntity, &NPCInfo->currentWaypoint, &NPCInfo->finalWaypoint );
	}/*
	else if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=BOTH_RUN1)
	{
		NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
	}*/
}

void NPC_BSMedic (void)
{
	ucmd.weapon = client->ps.weapon;

	//Just check to see if we should clear our current enemy
	NPC_CheckEnemy( qfalse, qfalse );

	if( NPCInfo->eventualGoal )
	{
		if( NPCInfo->eventualGoal->health > (NPCInfo->eventualGoal->max_health-1) ||
			(NPC->client->squadname && (!NPCInfo->eventualGoal->client->squadname || Q_stricmp(NPC->client->squadname, NPCInfo->eventualGoal->client->squadname))) 
			)
		{//He's dead, Jim!  Or he's healed, or he's not my concern anymore
			NPC_ClearPatient();
			return;
		}
	}
	
	if(!NPCInfo->eventualGoal) 
	{//No patient, or ours is kinda ok and either a player or not a NPC waiting for healing - find the weakest and help him
		int			i;
		gentity_t	*patient = NULL;
		gentity_t	*found = NULL;
		int			lowest = g_entities[0].max_health;
//		int			current_low = 0;

		for( i = 0; i < 1; i++ )
		{
			found = &g_entities[i];
			if( found->client && found != NPC)
			{
				if( !NPC->client->playerTeam || NPC->client->playerTeam == found->client->playerTeam)
				{//don't heal enemy team members
					if(!NPC->client->squadname || (found->client->squadname && !Q_stricmp(NPC->client->squadname, found->client->squadname)))
					{//If I'm assigned to a squad, only look for members of my squad...
						if( !(found->svFlags&SVF_HEALING) && !(found->svFlags&SVF_BEAMING))
						{//check and see if he's being healed by a medic at the moment or being beamed out
							if(gi.inPVS( NPC->currentOrigin, found->currentOrigin))
							{
								if( found->health < lowest )
								{//FIXME: need to be able to get to him!
									patient = found;
									lowest = found->health;
								}
							}
						}
					}
				}
			}
		}

		if( !patient )
		{//Take cover!
			if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=TORSO_TRICORDER1)
			{
				NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);

			}

			if(NPC->health < NPC->max_health)
			{//Physician, heal thyself
				NPC_MedicHeal( NPC );
			}
			else
			{//	all done
				Q3_TaskIDComplete( NPC, TID_BSTATE );
			}
			return;
		}
		else
		{
			patient->svFlags |= SVF_HEALING;
			SetGoal(patient, 0.0);
			NPCInfo->eventualGoal = patient;
		}
	}

	NPC_BSMedic_Go();
}

void NPC_BSMedicHide (void)
{//FIXME: make sure we can get to our patient?
	//Or limit ourselves to the people with the same squadname and players?
	qboolean	look = qtrue;
	int			ok_level = 50;
	
	//Just check to see if we should clear our current enemy
	NPC_CheckEnemy( qfalse, qfalse );

	//FIXME: takes time to get to goal- maybe we need to check occaisionally
	//while en route and see if anyone else needs our help more?  Someone closer?

	//FIXME: only heal if below 90?
	ucmd.weapon = client->ps.weapon;
	if( NPCInfo->eventualGoal )
	{
		//FIXME: should we heal to full or always look for the weakest?
		//or something more complicated?
		//We COULD check to see if our health is lower than the person we're
		//healing and run off and heal if so, but this guy for now will be a martyr.
		if( (NPCInfo->eventualGoal->health <= 0&&NPCInfo->behaviorState!=BS_MEDIC) || NPCInfo->eventualGoal->health > (NPCInfo->eventualGoal->max_health-1) ||
				(NPC->client->squadname && NPCInfo->eventualGoal->client && !(NPCInfo->eventualGoal->svFlags&SVF_MUST_HEAL) && (!NPCInfo->eventualGoal->client->squadname || Q_stricmp(NPC->client->squadname, NPCInfo->eventualGoal->client->squadname))) 
			)
		{//He's dead, Jim!  Or he's healed, or he's not my concern anymore
			NPC_ClearPatient();
			return;
		}
	}
	
	if(NPCInfo->behaviorState == BS_MEDIC)
	{//Only look for a new one if don't have one
		if(NPCInfo->eventualGoal)
		{
			look = qfalse;
		}
		ok_level = g_entities[0].max_health;
	}
	
		
	if(NPC->client->squadname && NPCInfo->behaviorState == BS_MEDIC_HIDE)
	{//don't look for anyone
		if(!NPCInfo->eventualGoal)
		{//Take cover!
			if(NPC->health < NPC->max_health)
			{//Physician, heal thyself
//				gi.Printf( "%s healing himself\n", NPC->targetname);
				NPC_MedicHeal( NPC );
			}

			NPC_BSTakeCover();
			if(ucmd.forwardmove)
			{
				/*
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=BOTH_RUN1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
				}
				*/
			}
			else
			{
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=TORSO_TRICORDER1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);
				}
				//ucmd.upmove = -127;
			}
			return;
		}
	}
	else if(look &&
			(!NPCInfo->eventualGoal || 
				(NPCInfo->behaviorState == BS_MEDIC_COMBAT && NPCInfo->eventualGoal->health >= ok_level && 
					( (NPCInfo->eventualGoal->NPC && NPCInfo->eventualGoal->NPC->behaviorState != BS_WAITHEAL) || !NPCInfo->eventualGoal->NPC) 
				) 
			)
		)
	{//No patient, or ours is kinda ok and either a player or not a NPC waiting for healing - find the weakest and help him
		int			i;
		gentity_t	*patient = NPC;
		gentity_t	*found = NULL;
		int			lowest;
		int			current_low = 0;

		patient = NPC;
		lowest = NPC->health;
		
		if(NPCInfo->eventualGoal)
			current_low = NPCInfo->eventualGoal->health;

		if(lowest > 25)
		{//as long as our health is > 25
			for( i = 0; i < 1; i++ )
			{
				found = &g_entities[i];
				if( found->client)
				{
					if( found->health > 0 || NPCInfo->behaviorState == BS_MEDIC)
					{
						if( !NPC->client->playerTeam || NPC->client->playerTeam == found->client->playerTeam)
						{//don't heal enemy team members
							if(!NPC->client->squadname || (found->client->squadname && !Q_stricmp(NPC->client->squadname, found->client->squadname)))
							{//If I'm assigned to a squad, only look for members of my squad...
								if( !(found->svFlags&SVF_HEALING) && !(found->svFlags&SVF_BEAMING))
								{//check and see if he's being healed by a medic at the moment or being beamed out
									if( found->health < lowest || (found->NPC && found->NPC->behaviorState == BS_WAITHEAL))
									{//FIXME: need to be able to get to him!
										patient = found;
										lowest = found->health;
									}
								}
							}
						}
					}
				}
			}
		}
		else if( NPCInfo->eventualGoal )
		{//Heal ourselves!
			NPC_ClearPatient();
		}

		if( patient == NPC && !NPCInfo->eventualGoal )
		{//We're the patient, take cover!
			if(NPCInfo->behaviorState == BS_MEDIC_COMBAT)
			{
				NPC_BSTakeCover();
			}

			if(ucmd.forwardmove)
			{
				/*
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=BOTH_RUN1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
				}
				*/
			}
			else
			{
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=TORSO_TRICORDER1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);
				}
			}

			if(NPC->health < NPC->max_health)
			{//Physician, heal thyself
//				gi.Printf( "%s healing himself\n", NPC->targetname);
				NPC_MedicHeal( NPC );
				ucmd.upmove = -127;
			}
			else if(NPCInfo->behaviorState == BS_MEDIC)
			{//	all done
				Q3_TaskIDComplete( NPC, TID_BSTATE );
			}
			return;
		}
		else if( patient != NPC )
		{
			qboolean	new_ok = qfalse;

			if( !NPCInfo->eventualGoal )
			{
				new_ok = qtrue;
			}
			else if( (current_low - patient->health) > 20 )
			{//If found another whose 20 below this patient in health...
				//Clear current patient
				NPC_ClearPatient();
				//take on new
				new_ok = qtrue;
			}
			
			if(new_ok)
			{
				patient->svFlags |= SVF_HEALING;
				SetGoal(patient, 0.0);
				NPCInfo->eventualGoal = patient;
//				Nav_PathToEnt ( NPC, NPCInfo->goalEntity, &NPCInfo->currentWaypoint, &NPCInfo->finalWaypoint );
			}
		}
	}

	NPC_BSMedic_Go();
}

void NPC_BSMedicCombat (void)
{//FIXME: make sure we can get to our patient?
	//Or limit ourselves to the people with the same squadname and players?
	qboolean	look = qtrue;
	int			ok_level = 50;
	
	//Just check to see if we should clear our current enemy
	NPC_CheckEnemy( qfalse, qfalse );

	//FIXME: takes time to get to goal- maybe we need to check occaisionally
	//while en route and see if anyone else needs our help more?  Someone closer?

	//FIXME: only heal if below 90?
	ucmd.weapon = client->ps.weapon;
	if( NPCInfo->eventualGoal )
	{
		//FIXME: should we heal to full or always look for the weakest?
		//or something more complicated?
		//We COULD check to see if our health is lower than the person we're
		//healing and run off and heal if so, but this guy for now will be a martyr.
		if( (NPCInfo->eventualGoal->health <= 0&&NPCInfo->behaviorState!=BS_MEDIC) || NPCInfo->eventualGoal->health > (NPCInfo->eventualGoal->max_health-1) ||
				(NPC->client->squadname && NPCInfo->eventualGoal->client && !(NPCInfo->eventualGoal->svFlags&SVF_MUST_HEAL) && (!NPCInfo->eventualGoal->client->squadname || Q_stricmp(NPC->client->squadname, NPCInfo->eventualGoal->client->squadname))) 
			)
		{//He's dead, Jim!  Or he's healed, or he's not my concern anymore
			NPC_ClearPatient();
			return;
		}
	}
	
	if(NPCInfo->behaviorState == BS_MEDIC)
	{//Only look for a new one if don't have one
		if(NPCInfo->eventualGoal)
		{
			look = qfalse;
		}
		ok_level = g_entities[0].max_health;
	}
	
		
	if(NPC->client->squadname && NPCInfo->behaviorState == BS_MEDIC_HIDE)
	{//don't look for anyone
		if(!NPCInfo->eventualGoal)
		{//Take cover!
			if(NPC->health < NPC->max_health)
			{//Physician, heal thyself
//				gi.Printf( "%s healing himself\n", NPC->targetname);
				NPC_MedicHeal( NPC );
			}

			NPC_BSTakeCover();
			if(ucmd.forwardmove)
			{
				/*
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=BOTH_RUN1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
				}
				*/
			}
			else
			{
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=TORSO_TRICORDER1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);
				}
				ucmd.upmove = -127;
			}
			return;
		}
	}
	else if(look &&
			(!NPCInfo->eventualGoal || 
				(NPCInfo->behaviorState == BS_MEDIC_COMBAT && NPCInfo->eventualGoal->health >= ok_level && 
					( (NPCInfo->eventualGoal->NPC && NPCInfo->eventualGoal->NPC->behaviorState != BS_WAITHEAL) || !NPCInfo->eventualGoal->NPC) 
				) 
			)
		)
	{//No patient, or ours is kinda ok and either a player or not a NPC waiting for healing - find the weakest and help him
		int			i;
		gentity_t	*patient = NPC;
		gentity_t	*found = NULL;
		int			lowest;
		int			current_low = 0;

		patient = NPC;
		lowest = NPC->health;
		
		if(NPCInfo->eventualGoal)
			current_low = NPCInfo->eventualGoal->health;

		if(lowest > 25)
		{//as long as our health is > 25
			for( i = 0; i < 1; i++ )
			{
				found = &g_entities[i];
				if( found->client)
				{
					if( found->health > 0 || NPCInfo->behaviorState == BS_MEDIC)
					{
						if( !NPC->client->playerTeam || NPC->client->playerTeam == found->client->playerTeam)
						{//don't heal enemy team members
							if(!NPC->client->squadname || (found->client->squadname && !Q_stricmp(NPC->client->squadname, found->client->squadname)))
							{//If I'm assigned to a squad, only look for members of my squad...
								if( !(found->svFlags&SVF_HEALING) && !(found->svFlags&SVF_BEAMING))
								{//check and see if he's being healed by a medic at the moment or being beamed out
									if( found->health < lowest || (found->NPC && found->NPC->behaviorState == BS_WAITHEAL))
									{//FIXME: need to be able to get to him!
										patient = found;
										lowest = found->health;
									}
								}
							}
						}
					}
				}
			}
		}
		else if( NPCInfo->eventualGoal )
		{//Heal ourselves!
			NPC_ClearPatient();
		}

		if( patient == NPC && !NPCInfo->eventualGoal)
		{//Take cover!
			if(NPCInfo->behaviorState == BS_MEDIC_COMBAT)
			{
				NPC_BSTakeCover();
			}

			if(ucmd.forwardmove)
			{
				/*
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=BOTH_RUN1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,BOTH_RUN1,SETANIM_FLAG_NORMAL);
				}
				*/
			}
			else
			{
				if(( NPC->s.torsoAnim &~ANIM_TOGGLEBIT )!=TORSO_TRICORDER1)
				{
					NPC_SetAnim(NPC,SETANIM_TORSO,TORSO_TRICORDER1,SETANIM_FLAG_NORMAL|SETANIM_FLAG_HOLD);
				}
				ucmd.upmove = -127;
			}

			if(NPC->health < NPC->max_health)
			{//Physician, heal thyself
//				gi.Printf( "%s healing himself\n", NPC->targetname);
				NPC_MedicHeal( NPC );
			}
			else if(NPCInfo->behaviorState == BS_MEDIC)
			{//	all done
				Q3_TaskIDComplete( NPC, TID_BSTATE );
			}
			return;
		}
		else if( patient != NPC )
		{
			qboolean	new_ok = qfalse;

			if( !NPCInfo->eventualGoal )
			{
				new_ok = qtrue;
			}
			else if( (current_low - patient->health) > 20 )
			{//If found another whose 20 below this patient in health...
				//Clear current patient
				NPC_ClearPatient();
				//take on new
				new_ok = qtrue;
			}
			
			if(new_ok)
			{
				patient->svFlags |= SVF_HEALING;
				SetGoal(patient, 0.0);
				NPCInfo->eventualGoal = patient;
//				Nav_PathToEnt ( NPC, NPCInfo->goalEntity, &NPCInfo->currentWaypoint, &NPCInfo->finalWaypoint );
			}
		}
	}

	NPC_BSMedic_Go();
}

void NPC_BSWait( void ) 
{
	NPC_UpdateAngles( qtrue, qtrue );
}

//Simply turn until facing desired angles
void NPC_BSFace (void)
{
	//FIXME: once you stop sending turning info, they reset to whatever their delta_angles was last????
	//Once this is over, it snaps back to what it was facing before- WHY???
	if( NPC_UpdateAngles ( qtrue, qtrue ) )
	{
		Q3_TaskIDComplete( NPC, TID_BSTATE );
		
		if( NPCInfo->tempBehavior == BS_FACE )
			NPCInfo->tempBehavior = BS_DEFAULT;

		NPCInfo->desiredYaw = client->ps.viewangles[YAW];
		NPCInfo->desiredPitch = client->ps.viewangles[PITCH];

		NPCInfo->aimTime = 0;//ok to turn normally now
	}
}

void NPC_BSPointShoot (qboolean shoot)
{//FIXME: doesn't check for clear shot...
	vec3_t	muzzle, dir, angles, org;

	NPCInfo->lookMode = LT_AIM;

	if ( !NPC->enemy || !NPC->enemy->inuse || (NPC->enemy->NPC && NPC->enemy->health <= 0) )
	{//FIXME: should still keep shooting for a second or two after they actually die...
		Q3_TaskIDComplete( NPC, TID_BSTATE );
		goto finished;
		return;
	}

	CalcEntitySpot(NPC, SPOT_WEAPON, muzzle);
	CalcEntitySpot(NPC->enemy, SPOT_HEAD, org);//Was spot_org
	//Head is a little high, so let's aim for the chest:
	if ( NPC->enemy->client )
	{
		org[2] -= 12;//NOTE: is this enough?
	}

	VectorSubtract(org, muzzle, dir);
	vectoangles(dir, angles);

	switch( NPC->client->ps.weapon )
	{
	case WP_NONE:
	case WP_TRICORDER:
	case WP_BLUE_HYPO:
	case WP_RED_HYPO:
	case WP_MELEE:
	case WP_STASIS_ATTACK:
	case WP_KLINGON_BLADE:
	case WP_IMPERIAL_BLADE:
		//don't do any pitch change if not holding a firing weapon
		break;
	default:
		NPCInfo->desiredPitch = NPCInfo->lockedDesiredPitch = AngleMod(angles[PITCH]);
		break;
	}

	NPCInfo->desiredYaw = NPCInfo->lockedDesiredYaw = AngleMod(angles[YAW]);

	if ( NPC_UpdateAngles ( qtrue, qtrue ) )
	{//FIXME: if angles clamped, this may never work!
		//NPCInfo->shotTime = NPC->attackDebounceTime = 0;

		if ( shoot )
		{//FIXME: needs to hold this down if using a weapon that requires it, like phaser...
			ucmd.buttons |= BUTTON_ATTACK;
		}
		
		if ( !shoot || !(NPC->svFlags & SVF_LOCKEDENEMY) )
		{//If locked_enemy is on, dont complete until it is destroyed...
			Q3_TaskIDComplete( NPC, TID_BSTATE );
			goto finished;
		}
	}
	else if ( shoot && (NPC->svFlags & SVF_LOCKEDENEMY) )
	{//shooting them till their dead, not aiming right at them yet...
		/*
		qboolean movingTarget = qfalse;

		if ( NPC->enemy->client )
		{
			if ( VectorLengthSquared( NPC->enemy->client->ps.velocity ) )
			{
				movingTarget = qtrue;
			}
		}
		else if ( VectorLengthSquared( NPC->enemy->s.pos.trDelta ) )
		{
			movingTarget = qtrue;
		}

		if (movingTarget )
		*/
		{
			float	dist = VectorLength( dir );
			float	yawMiss, yawMissAllow = NPC->enemy->maxs[0];
			float	pitchMiss, pitchMissAllow = (NPC->enemy->maxs[2] - NPC->enemy->mins[2])/2;
			
			if ( yawMissAllow < 8.0f )
			{
				yawMissAllow = 8.0f;
			}

			if ( pitchMissAllow < 8.0f )
			{
				pitchMissAllow = 8.0f;
			}

			yawMiss = tan(DEG2RAD(AngleDelta ( NPC->client->ps.viewangles[YAW], NPCInfo->desiredYaw ))) * dist;
			pitchMiss = tan(DEG2RAD(AngleDelta ( NPC->client->ps.viewangles[PITCH], NPCInfo->desiredPitch))) * dist;

			if ( yawMissAllow >= yawMiss && pitchMissAllow > pitchMiss )
			{
				ucmd.buttons |= BUTTON_ATTACK;
			}
		}
	}
	
	return;
		
finished:
	if( NPCInfo->tempBehavior == BS_POINT_AND_SHOOT ||NPCInfo->tempBehavior == BS_FACE_ENEMY)
	{
		NPCInfo->tempBehavior = BS_DEFAULT;
	}

	NPCInfo->desiredYaw = client->ps.viewangles[YAW];
	NPCInfo->desiredPitch = client->ps.viewangles[PITCH];

	NPCInfo->aimTime = 0;//ok to turn normally now
}

/*
void NPC_BSMove(void)
Move in a direction, face another
*/
void NPC_BSMove(void)
{
	gentity_t	*goal = NULL;

	NPC_CheckEnemy(qtrue, qfalse);
	if(NPC->enemy)
	{
		NPC_CheckCanAttack(1.0, qfalse);
	}
	else
	{
		NPC_UpdateAngles(qtrue, qtrue);
	}

	goal = UpdateGoal();
	if(goal)
	{
//		NPCInfo->moveToGoalMod = 1.0;

		NPC_SlideMoveToGoal();
	}
}

/*
void NPC_BSWaitHeal(void)
Wait until health >= 75
*/

void NPC_BSWaitHeal(void)
{
	if ( NPC->enemy != NULL )
	{
		NPC_CheckEnemy( qfalse, qfalse );
		if ( NPC->enemy != NULL )
		{
			NPC_CheckCanAttack(1.0, qtrue);
		}
	}
	else
	{//only look for enemy if don't have one
		NPC_CheckEnemy(qtrue, qfalse);
	}

	if(NPC->health < (NPC->max_health*0.75))//FIXME: we could wait longer if we wanted to, I suppose.
		return;

	if(NPC->svFlags & SVF_HEALING)//This may be cleared if the doctor had to take a different patient before you were done, so you might go back out into combat with < 75 health
		return;

	NPC->svFlags &= ~SVF_MUST_HEAL;
	//OK, tell taskman we finished
	Q3_TaskIDComplete( NPC, TID_BSTATE );
}

/*
void NPC_BSShoot(void)
Move in a direction, face another
*/

void NPC_BSShoot(void)
{
	NPC_BSMove();

	enemyVisibility = VIS_SHOOT;

	if ( client->ps.weaponstate != WEAPON_READY && client->ps.weaponstate != WEAPON_FIRING ) 
	{
		client->ps.weaponstate = WEAPON_READY;
	}

	WeaponThink(qtrue);
}

void NPC_BSSniper (void)
{
	if(!(NPC->svFlags & SVF_LOCKEDENEMY))
	{//not finished until this guy is dead
		if(NPC->attackDebounceTime > level.time)
		{//we shot last turn
			Q3_TaskIDComplete( NPC, TID_BSTATE );
		}
	}

	NPC_CheckEnemy(qtrue, qfalse);
	if(NPC->enemy)
	{
		NPC_CheckCanAttack(3.0, qtrue);
	}
	else
	{
		NPC_UpdateAngles(qtrue, qtrue);
	}
}

void NPC_BSInvestigate (void)
{
/*
	//FIXME: maybe allow this to be set as a tempBState in a script?  Just specify the
	//investigateGoal, investigateDebounceTime and investigateCount? (Needs a macro)
	vec3_t		invDir, invAngles, spot;
	gentity_t	*saveGoal;
	//BS_INVESTIGATE would turn toward goal, maybe take a couple steps towards it,
	//look for enemies, then turn away after your investigate counter was down-
	//investigate counter goes up every time you set it...

	if(level.time > NPCInfo->enemyCheckDebounceTime)
	{
		NPCInfo->enemyCheckDebounceTime = level.time + (NPCInfo->stats.vigilance * 1000);
		NPC_CheckEnemy(qtrue, qfalse);
		if(NPC->enemy)
		{//FIXME: do anger script
			NPCInfo->goalEntity = NPC->enemy;
//			NAV_ClearLastRoute(NPC);
			NPCInfo->behaviorState = BS_RUN_AND_SHOOT;
			NPCInfo->tempBehavior = BS_DEFAULT;
			NPC_AngerSound();
			return;
		}
	}

	NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY2, SETANIM_FLAG_NORMAL );

	if(NPCInfo->stats.vigilance <= 1.0 && NPCInfo->eventOwner)
	{
		VectorCopy(NPCInfo->eventOwner->currentOrigin, NPCInfo->investigateGoal);
	}

	saveGoal = NPCInfo->goalEntity;
	if(	level.time > NPCInfo->walkDebounceTime )
	{
		vec3_t	vec;

		VectorSubtract(NPCInfo->investigateGoal, NPC->currentOrigin, vec);
		vec[2] = 0;
		if(VectorLength(vec) > 64)
		{
			if(Q_irand(0, 100) < NPCInfo->investigateCount)
			{//take a full step
				//NPCInfo->walkDebounceTime = level.time + 1400;
				//actually finds length of my BOTH_WALK anim
				NPCInfo->walkDebounceTime = PM_AnimLength( NPC->client->clientInfo.animFileIndex, BOTH_WALK1 );
			}
		}
	}

	if(	level.time < NPCInfo->walkDebounceTime )
	{//walk toward investigateGoal
		
		/*
		NPCInfo->goalEntity = NPCInfo->tempGoal;
//		NAV_ClearLastRoute(NPC);
		VectorCopy(NPCInfo->investigateGoal, NPCInfo->tempGoal->currentOrigin);
		*/

/*		NPC_SetMoveGoal( NPC, NPCInfo->investigateGoal, 16, qtrue );

		NPC_MoveToGoal();

		//FIXME: walk2?
		NPC_SetAnim(NPC,SETANIM_LEGS,BOTH_WALK1,SETANIM_FLAG_NORMAL);

		ucmd.buttons |= BUTTON_WALKING;
	}
	else
	{

		NPC_SetAnim(NPC,SETANIM_LEGS,BOTH_STAND1,SETANIM_FLAG_NORMAL);

		if(NPCInfo->hlookCount > 30)
		{
			if(Q_irand(0, 10) > 7) 
			{
				NPCInfo->hlookCount = 0;
			}
		}
		else if(NPCInfo->hlookCount < -30)
		{
			if(Q_irand(0, 10) > 7) 
			{
				NPCInfo->hlookCount = 0;
			}
		}
		else if(NPCInfo->hlookCount == 0)
		{
			NPCInfo->hlookCount = Q_irand(-1, 1);
		}
		else if(Q_irand(0, 10) > 7) 
		{
			if(NPCInfo->hlookCount > 0)
			{
				NPCInfo->hlookCount++;
			}
			else//lookCount < 0
			{
				NPCInfo->hlookCount--;
			}
		}

		if(NPCInfo->vlookCount >= 15)
		{
			if(Q_irand(0, 10) > 7) 
			{
				NPCInfo->vlookCount = 0;
			}
		}
		else if(NPCInfo->vlookCount <= -15)
		{
			if(Q_irand(0, 10) > 7) 
			{
				NPCInfo->vlookCount = 0;
			}
		}
		else if(NPCInfo->vlookCount == 0)
		{
			NPCInfo->vlookCount = Q_irand(-1, 1);
		}
		else if(Q_irand(0, 10) > 8) 
		{
			if(NPCInfo->vlookCount > 0)
			{
				NPCInfo->vlookCount++;
			}
			else//lookCount < 0
			{
				NPCInfo->vlookCount--;
			}
		}

		//turn toward investigateGoal
		CalcEntitySpot( NPC, SPOT_HEAD, spot );
		VectorSubtract(NPCInfo->investigateGoal, spot, invDir);
		VectorNormalize(invDir);
		vectoangles(invDir, invAngles);
		NPCInfo->desiredYaw = AngleMod(invAngles[YAW] + NPCInfo->hlookCount);
		NPCInfo->desiredPitch = AngleMod(invAngles[PITCH] + NPCInfo->hlookCount);
	}

	NPC_UpdateAngles(qtrue, qtrue);

	NPCInfo->goalEntity = saveGoal;
//	NAV_ClearLastRoute(NPC);

	if(level.time > NPCInfo->investigateDebounceTime)
	{
		NPCInfo->tempBehavior = BS_DEFAULT;
	}

	NPC_CheckSoundEvents();
	*/
}

qboolean NPC_CheckInvestigate( int alertEventNum )
{
	gentity_t	*owner = level.alertEvents[alertEventNum].owner;
	int		invAdd = level.alertEvents[alertEventNum].level;
	vec3_t	soundPos;
	float	soundRad = level.alertEvents[alertEventNum].radius;
	float	earshot = NPCInfo->stats.earshot;

	VectorCopy( level.alertEvents[alertEventNum].position, soundPos );

	//NOTE: Trying to preserve previous investigation behavior
	if ( !owner )
	{
		return qfalse;
	}

	if ( owner->s.eType != ET_PLAYER && owner == NPCInfo->goalEntity ) 
	{
		return qfalse;
	}

	if ( owner->s.eFlags & EF_NODRAW ) 
	{
		return qfalse;
	}

	if ( owner->flags & FL_NOTARGET ) 
	{
		return qfalse;
	}

	if ( soundRad < earshot )
	{
		return qfalse;
	}

	//if(!gi.inPVSIgnorePortals(ent->currentOrigin, NPC->currentOrigin))//should we be able to hear through areaportals?
	if ( !gi.inPVS( soundPos, NPC->currentOrigin ) )
	{//can hear through doors?
		return qfalse;
	}

	if ( owner->client && owner->client->playerTeam && NPC->client->playerTeam && owner->client->playerTeam != NPC->client->playerTeam )
	{
		if( (float)NPCInfo->investigateCount >= (NPCInfo->stats.vigilance*200) && owner )
		{//If investigateCount == 10, just take it as enemy and go
			if ( ValidEnemy( owner ) )
			{//FIXME: run angerscript
				G_SetEnemy( NPC, owner );
				NPCInfo->goalEntity = NPC->enemy;
				NPCInfo->goalRadius = 12;
				NPCInfo->behaviorState = BS_RUN_AND_SHOOT;
				return qtrue;
			}
		}
		else
		{
			NPCInfo->investigateCount += invAdd;
		}
		//run awakescript
		G_ActivateBehavior(NPC, BSET_AWAKE);

		if ( Q_irand(0, 10) > 7 )
		{
			NPC_AngerSound();
		}

		//NPCInfo->hlookCount = NPCInfo->vlookCount = 0;
		NPCInfo->eventOwner = owner;
		VectorCopy( soundPos, NPCInfo->investigateGoal );
		if ( NPCInfo->investigateCount > 20 )
		{
			NPCInfo->investigateDebounceTime = level.time + 10000;
		}
		else
		{
			NPCInfo->investigateDebounceTime = level.time + (NPCInfo->investigateCount*500);
		}
		NPCInfo->tempBehavior = BS_INVESTIGATE;
		return qtrue;
	}

	return qfalse;
}

/*
void NPC_BSPatrol( void ) 

  Same as idle, but you look for enemies every "vigilance"
  using your angles, HFOV, VFOV and visrange, and listen for sounds within earshot...
*/
void NPC_BSPatrol( void ) 
{
	int	alertEventNum;

	if(level.time > NPCInfo->enemyCheckDebounceTime)
	{
		NPCInfo->enemyCheckDebounceTime = level.time + (NPCInfo->stats.vigilance * 1000);
		NPC_CheckEnemy(qtrue, qfalse);
		if(NPC->enemy)
		{//FIXME: do anger script
			NPCInfo->behaviorState = BS_RUN_AND_SHOOT;
			NPC_AngerSound();
			return;
		}
	}

	//FIXME: Implement generic sound alerts
	alertEventNum = NPC_CheckSoundEvents();
	if( alertEventNum != -1 )
	{//If we heard something, see if we should check it out
		if ( NPC_CheckInvestigate( alertEventNum ) )
		{
			return;
		}
	}

	NPCInfo->investigateSoundDebounceTime = 0;
	//FIXME if there is no nav data, we need to do something else
	// if we're stuck, try to move around it
	if ( blocked ) 
	{
		//FIXME: make us step to one side and back up, make other NPC, if a NPC,
		//step to other side and back up
		//do this for 1 full second
		//FIXME: Only push each other if vec to other is within 0.5 dot product of movedir
		if(random() > 0.5)
			ucmd.rightmove = 127;
		else
			ucmd.rightmove = -127;

//		Debug_NPCPrintf ( NPC, debugNPCAI, DEBUG_LEVEL_INFO, "NPCAI: stuck\n" );
//		NPC_ClearGoal();
//		NPCInfo->blockTime = 0;
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}
		ucmd.forwardmove = 0;
	}
	else
	{
		if ( UpdateGoal() )
		{
			NPC_MoveToGoal();
		}
	}

	NPC_UpdateAngles( qtrue, qtrue );

//FIXME: allow running patrol!?  Or have a way to set walking button in script?  Might as well be able to set crouch too
//	if(NPCInfo->speed <= 1)
//	{
		ucmd.buttons |= BUTTON_WALKING;
//	}
}

/*
void NPC_BSSleep( void ) 
*/
void NPC_BSSleep( void ) 
{
	int alertEvent = NPC_CheckAlertEvents();

	//There is an event to look at
	if ( alertEvent >= 0 )
	{
		G_ActivateBehavior(NPC, BSET_AWAKE);
		return;
	}

	/*
	if ( level.time > NPCInfo->enemyCheckDebounceTime )
	{
		if ( NPC_CheckSoundEvents() != -1 )
		{//only 1 alert per second per 0.1 of vigilance
			NPCInfo->enemyCheckDebounceTime = level.time + (NPCInfo->stats.vigilance * 10000);
			G_ActivateBehavior(NPC, BSET_AWAKE);
		}
	}
	*/
}

/*
void NPC_BSSay (void)
Turn to sayTarg, use talk anim, say your sayString (look up string in your sounds table), exit tempBState when sound finished (anim of mouth should be timed to length of sound as well)
*/
void NPC_BSSay (void)
{
	//FIXME: Implement
	NPCInfo->tempBehavior = BS_DEFAULT;
};

/*
int NPC_FindClosestCombatPoint(void)

  FIXME:  This should probably make sure you can hit your enemy from here
  FIXME:  Should also make sure you don't have to run PAST enemy to get there!
  FIXME:  CPoint should be within a certain range too!
*/
int NPC_FindClosestCombatPoint(qboolean checkCanSeeEnemy, qboolean checkNotPastEnemy)
{
	int		i;
	float	distSq, bestDistSq = Q3_INFINITE;
	int		bestCp = -1;
	vec3_t	vec, vec2;

	if(checkCanSeeEnemy||checkNotPastEnemy)
	{
		if(!NPC->enemy)
		{
			return -1;
		}
	}

	for(i = 0; i < level.numCombatPoints; i++)
	{
		if(!level.combatPoints[i].occupied)
		{//Not already taken
			if(gi.inPVS(level.combatPoints[i].origin, NPC->currentOrigin))
			{//In PVS
				if(!checkCanSeeEnemy||gi.inPVS(level.combatPoints[i].origin, NPC->enemy->currentOrigin))
				{//In PVS of enemy
					if(checkNotPastEnemy)
					{//Make sure we don't run past enemy to get there!
						VectorSubtract(NPC->currentOrigin, NPC->enemy->currentOrigin, vec);
						VectorSubtract(NPC->currentOrigin, level.combatPoints[i].origin, vec2);
						if(DotProduct(vec, vec2) <= 0)
						{//Angle between 2 is greater than 90 meaning you have to run by him!
							continue;
						}
					}

					VectorSubtract(level.combatPoints[i].origin, NPC->currentOrigin, vec);
					distSq = VectorLengthSquared(vec);
					if(distSq < bestDistSq)
					{//Closest so far
						bestDistSq = distSq;
						bestCp = i;
					}
				}
			}
		}
	}

	return bestCp;
}

/*
void NPC_BSPointCombat (void)
Head toward closest empty point_combat and shoot from there

  FIXME: Check the valid firing angles on the point combat and see make sure my enemy is
	in one of those directions from the point... or just see if can shoot my enemy from that
	point?

  Has duck flag, add lean flags?  A Behavior state?
*/
qboolean NPC_BSPointCombat (void)
{
	vec3_t	vec;

	if(NPCInfo->combatPoint == -1)
	{
		if((NPCInfo->combatPoint = NPC_FindClosestCombatPoint(qtrue, qtrue)) == -1)
		{//FIXME: what do we do?
			//gi.Printf(S_COLOR_RED"ERROR: no combat point for %s\n", NPC->script_targetname);
			//Check the enemy for them anyway
			if(NPC->enemy)
			{
				NPC_CheckEnemy(qtrue, qfalse);
			}
			return qfalse;
		}
	}
	//FIXME: Reset combat point for new enemy?
	//level.combatPoints[NPCInfo->combatPoint].occupied = qtrue;

	/*
	NPC_CheckEnemy(qtrue, qfalse);
	if(NPC->enemy)
	{
		NPC_CheckCanAttack(1.0, qtrue);
	}//Else done with this behavior?
	else if(NPCInfo->behaviorState == BS_FORMATION)
	{
		return qfalse;
	}
	*/

	NPC_CheckEnemy(qtrue, qfalse);

	//G_SetOrigin(NPCInfo->tempGoal, level.combatPoints[NPCInfo->combatPoint].origin);
	
	VectorSubtract(level.combatPoints[NPCInfo->combatPoint].origin, NPC->currentOrigin, vec);

	if(VectorLengthSquared(vec) > MAX_WAYPOINT_REACHED_DIST_SQUARED)
	{
		//NPCInfo->goalEntity = NPCInfo->tempGoal;
		
		NPC_SetMoveGoal( NPC, level.combatPoints[NPCInfo->combatPoint].origin, 16, qtrue );
		NPC_SlideMoveToGoal();
	}
	else
	{
	}

	return qtrue;
};

void NPC_BSFollowLeader (void)
{
	vec3_t		vec;
	float		leaderDist;
	visibility_t	leaderVis;
	int			curAnim;

	if ( !NPC->client->leader )
	{//ok, stand guard until we find an enemy
		if( NPCInfo->tempBehavior == BS_HUNT_AND_KILL )
		{
			NPCInfo->tempBehavior = BS_DEFAULT;
		}
		else
		{
			NPCInfo->tempBehavior = BS_STAND_GUARD;
			NPC_BSStandGuard();
		}
		return;
	}

	if ( NPC->client->ps.weapon )
	{
		NPC_CheckEnemy( NPCInfo->tempBehavior != BS_FOLLOW_LEADER, NPC->client->ps.weapon == WP_BORG_ASSIMILATOR );//don't find new enemy if this is tempbehav
	}
	
	if ( NPC->enemy && NPC->client->ps.weapon )
	{//If have an enemy, face him and fire
		enemyVisibility = NPC_CheckVisibility ( NPC->enemy, CHECK_FOV|CHECK_SHOOT );//CHECK_360|CHECK_PVS|
		if(enemyVisibility > VIS_PVS)
		{
			NPC_CheckCanAttack(1.0, qfalse);
		}
	}
	else
	{//FIXME: combine with vector calc below
		vec3_t	head, leaderHead, delta, angleToLeader;

		CalcEntitySpot( NPC->client->leader, SPOT_HEAD, leaderHead );
		CalcEntitySpot( NPC, SPOT_HEAD, head );
		VectorSubtract (leaderHead, head, delta);
		vectoangles ( delta, angleToLeader );
		VectorNormalize(delta);
		NPC->NPC->desiredYaw = angleToLeader[YAW];
		NPC->NPC->desiredPitch = angleToLeader[PITCH];
		
		NPC_UpdateAngles(qtrue, qtrue);
	}

	//leader visible?
	leaderVis = NPC_CheckVisibility( NPC->client->leader, CHECK_PVS|CHECK_360|CHECK_FOV|CHECK_SHOOT );

	//Follow leader, stay within visibility and a certain distance, maintain a distance from.
	curAnim = (NPC->client->ps.legsAnim&~ANIM_TOGGLEBIT);
	if(curAnim != BOTH_ATTACK1 && curAnim != BOTH_ATTACK2 && curAnim != BOTH_ATTACK3 && curAnim != BOTH_MELEE1 && curAnim != BOTH_MELEE2 )
	{//Don't move toward leader if we're in a full-body attack anim
		//FIXME, use IdealDistance to determine if we need to close distance
		float	followDist = 96.0f, backupdist, walkdist, minrundist;

		if ( NPCInfo->followDist )
		{
			followDist = NPCInfo->followDist;
		}
		backupdist = followDist/2.0f;
		walkdist = followDist*0.83;
		minrundist = followDist*1.33;

		VectorSubtract(NPC->client->leader->currentOrigin, NPC->currentOrigin, vec);
		leaderDist = VectorLength(vec);
		if( leaderVis != VIS_SHOOT || leaderDist > walkdist )
		{//We should close in?
			NPCInfo->goalEntity = NPC->client->leader;

			NPC_SlideMoveToGoal();
			if ( leaderVis == VIS_SHOOT && leaderDist < minrundist )
			{
				ucmd.buttons |= BUTTON_WALKING;
			}
		}
		else if ( leaderDist < backupdist )
		{//We should back off?
			NPCInfo->goalEntity = NPC->client->leader;
			NPC_SlideMoveToGoal();

			ucmd.forwardmove *= -1;
			ucmd.rightmove *= -1;
		}//otherwise, stay where we are
	}
}
#define	APEX_HEIGHT		200.0f
#define	PARA_WIDTH		(sqrt(APEX_HEIGHT)+sqrt(APEX_HEIGHT))
#define	JUMP_SPEED		200.0f
void NPC_BSJump (void)
{
	vec3_t		dir, angles, p1, p2, apex;
	float		time, height, forward, z, xy, dist, yawError, apexHeight;

	if( !NPCInfo->goalEntity )
	{//Should have task completed the navgoal
		return;
	}

	if ( NPCInfo->jumpState != JS_JUMPING && NPCInfo->jumpState != JS_LANDING )
	{
		//Face navgoal
		VectorSubtract(NPCInfo->goalEntity->currentOrigin, NPC->currentOrigin, dir);
		vectoangles(dir, angles);
		NPCInfo->desiredPitch = NPCInfo->lockedDesiredPitch = AngleMod(angles[PITCH]);
		NPCInfo->desiredYaw = NPCInfo->lockedDesiredYaw = AngleMod(angles[YAW]);
	}

	NPC_UpdateAngles ( qtrue, qtrue );
	yawError = AngleDelta ( NPC->client->ps.viewangles[YAW], NPCInfo->desiredYaw );
	//We don't really care about pitch here

	switch ( NPCInfo->jumpState )
	{
	case JS_FACING:
		if ( yawError < MIN_ANGLE_ERROR )
		{//Facing it, Start crouching
			NPC_SetAnim(NPC, SETANIM_LEGS, BOTH_CROUCH1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			NPCInfo->jumpState = JS_CROUCHING;
		}
		break;
	case JS_CROUCHING:
		if ( NPC->client->ps.legsAnimTimer > 0 )
		{//Still playing crouching anim
			return;
		}

		//Create a parabola

		if ( NPC->currentOrigin[2] > NPCInfo->goalEntity->currentOrigin[2] )
		{
			VectorCopy( NPC->currentOrigin, p1 );
			VectorCopy( NPCInfo->goalEntity->currentOrigin, p2 );
		}
		else if ( NPC->currentOrigin[2] < NPCInfo->goalEntity->currentOrigin[2] )
		{
			VectorCopy( NPCInfo->goalEntity->currentOrigin, p1 );
			VectorCopy( NPC->currentOrigin, p2 );
		}
		else
		{
			VectorCopy( NPC->currentOrigin, p1 );
			VectorCopy( NPCInfo->goalEntity->currentOrigin, p2 );
		}

		//z = xy*xy
		VectorSubtract( p2, p1, dir );
		dir[2] = 0;

		//Get xy and z diffs
		xy = VectorNormalize( dir );
		z = p1[2] - p2[2];

		apexHeight = APEX_HEIGHT/2;
		/*
		//Determine most desirable apex height
		apexHeight = (APEX_HEIGHT * PARA_WIDTH/xy) + (APEX_HEIGHT * z/128);
		if ( apexHeight < APEX_HEIGHT * 0.5 )
		{
			apexHeight = APEX_HEIGHT*0.5;
		}
		else if ( apexHeight > APEX_HEIGHT * 2 )
		{
			apexHeight = APEX_HEIGHT*2;
		}
		*/

		//FIXME: length of xy will change curve of parabola, need to account for this
		//somewhere... PARA_WIDTH
		
		z = (sqrt(apexHeight + z) - sqrt(apexHeight));

		assert(z >= 0);

//		gi.Printf("apex is %4.2f percent from p1: ", (xy-z)*0.5/xy*100.0f);

		xy -= z;
		xy *= 0.5;
		
		assert(xy > 0);

		VectorMA( p1, xy, dir, apex );
		apex[2] += apexHeight;
	
		VectorCopy(apex, NPC->pos1);
		
		//Now we have the apex, aim for it
		height = apex[2] - NPC->currentOrigin[2];
		time = sqrt( height / ( .5 * NPC->client->ps.gravity ) );
		if ( !time ) 
		{
//			gi.Printf("ERROR no time in jump\n");
			return;
		}

		// set s.origin2 to the push velocity
		VectorSubtract ( apex, NPC->currentOrigin, NPC->client->ps.velocity );
		NPC->client->ps.velocity[2] = 0;
		dist = VectorNormalize( NPC->client->ps.velocity );

		forward = dist / time;
		VectorScale( NPC->client->ps.velocity, forward, NPC->client->ps.velocity );

		NPC->client->ps.velocity[2] = time * NPC->client->ps.gravity;

//		gi.Printf( "%s jumping %s, gravity at %4.0f percent\n", NPC->targetname, vtos(NPC->client->ps.velocity), NPC->client->ps.gravity/8.0f );

		NPC->flags |= FL_NO_KNOCKBACK;
		NPCInfo->jumpState = JS_JUMPING;
		//FIXME: jumpsound?
		break;
	case JS_JUMPING:

		if ( showBBoxes )
		{
			VectorAdd(NPC->mins, NPC->pos1, p1);
			VectorAdd(NPC->maxs, NPC->pos1, p2);
			CG_Cube( p1, p2, BLUE, 0.5 );
		}

		if ( NPC->s.groundEntityNum != ENTITYNUM_NONE)
		{//Landed, start landing anim
			//FIXME: if the 
			VectorClear(NPC->client->ps.velocity);
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_LAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			NPCInfo->jumpState = JS_LANDING;
			//FIXME: landsound?
		}
		else if ( NPC->client->ps.legsAnimTimer > 0 )
		{//Still playing jumping anim
			//FIXME: apply jump velocity here, a couple frames after start, not right away
			return;
		}
		else
		{//still in air, but done with jump anim, play inair anim
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_INAIR1, SETANIM_FLAG_OVERRIDE);
		}
		break;
	case JS_LANDING:
		if ( NPC->client->ps.legsAnimTimer > 0 )
		{//Still playing landing anim
			return;
		}
		else
		{
			NPCInfo->jumpState = JS_WAITING;

			
			//task complete no matter what...  
			NPC_ClearGoal();
			NPCInfo->goalTime = level.time;
			NPCInfo->aiFlags &= ~NPCAI_MOVING;
			ucmd.forwardmove = 0;
			NPC->flags &= ~FL_NO_KNOCKBACK;
			//Return that the goal was reached
			Q3_TaskIDComplete( NPC, TID_MOVE_NAV );
			
			//Or should we keep jumping until reached goal?
			
			/*
			NPCInfo->goalEntity = UpdateGoal();
			if ( !NPCInfo->goalEntity )
			{
				NPC->flags &= ~FL_NO_KNOCKBACK;
				Q3_TaskIDComplete( NPC, TID_MOVE_NAV );
			}
			*/
			
		}
		break;
	case JS_WAITING:
	default:
		NPCInfo->jumpState = JS_FACING;
		break;
	}
}

void NPC_BSRemove (void)
{
	NPC_UpdateAngles ( qtrue, qtrue );
	if( !gi.inPVS( NPC->currentOrigin, g_entities[0].currentOrigin ) )//FIXME: use cg.vieworg?
	{
		G_UseTargets2( NPC, NPC, NPC->target3 );
		NPC->s.eFlags |= EF_NODRAW;
		NPC->s.eFlags &= ~EF_NPC;
		NPC->svFlags &= ~SVF_NPC;
		NPC->s.eType = ET_INVISIBLE;
		NPC->contents = 0;
		NPC->health = 0;
		NPC->targetname = NULL;

		//Disappear in half a second
		NPC->e_ThinkFunc = thinkF_G_FreeEntity;
		NPC->nextthink = level.time + FRAMETIME;
	}//FIXME: else allow for out of FOV???
}

void NPC_BSSearch (void)
{
	NPC_CheckEnemy(qtrue, qfalse);
	//Look for enemies, if find one:
	if ( NPC->enemy )
	{
		if( NPCInfo->tempBehavior == BS_SEARCH )
		{//if tempbehavior, set tempbehavior to default
			NPCInfo->tempBehavior = BS_DEFAULT;
		}
		else
		{//if bState, change to run and shoot
			NPCInfo->behaviorState = BS_RUN_AND_SHOOT;
			NPC_BSRunAndShoot();
		}
		return;
	}

	//FIXME: what if our goalEntity is not NULL and NOT our tempGoal - they must
	//want us to do something else?  If tempBehavior, just default, else set
	//to run and shoot...?

	//FIXME: Reimplement

	if ( !NPCInfo->investigateDebounceTime )
	{//On our way to a tempGoal
		float	minGoalReachedDistSquared = 32*32;
		vec3_t	vec;

		//Keep moving toward our tempGoal
		NPCInfo->goalEntity = NPCInfo->tempGoal;

		VectorSubtract ( NPCInfo->tempGoal->currentOrigin, NPC->currentOrigin, vec);
		if ( vec[2] < 24 )
		{
			vec[2] = 0;
		}

		if ( NPCInfo->tempGoal->waypoint != WAYPOINT_NONE )
		{
			/*
			//FIXME: can't get the radius...
			float	wpRadSq = waypoints[NPCInfo->tempGoal->waypoint].radius * waypoints[NPCInfo->tempGoal->waypoint].radius;
			if ( minGoalReachedDistSquared > wpRadSq )
			{
				minGoalReachedDistSquared = wpRadSq;
			}
			*/

			minGoalReachedDistSquared = 32*32;//12*12;
		}

		if ( VectorLengthSquared( vec ) < minGoalReachedDistSquared )
		{
			//Close enough, just got there
			NPC->waypoint = NAV_FindClosestWaypointForEnt( NPC, WAYPOINT_NONE );

			if ( ( NPCInfo->homeWp == WAYPOINT_NONE ) || ( NPC->waypoint == WAYPOINT_NONE ) )
			{
				//Heading for or at an invalid waypoint, get out of this bState
				if( NPCInfo->tempBehavior == BS_SEARCH )
				{//if tempbehavior, set tempbehavior to default
					NPCInfo->tempBehavior = BS_DEFAULT;
				}
				else
				{//if bState, change to stand guard
					NPCInfo->behaviorState = BS_STAND_GUARD;
					NPC_BSRunAndShoot();
				}
				return;
			}

			if ( NPC->waypoint == NPCInfo->homeWp )
			{
				//Just Reached our homeWp, if this is the first time, run your lostenemyscript
				if ( NPCInfo->aiFlags & NPCAI_ENROUTE_TO_HOMEWP )
				{
					NPCInfo->aiFlags &= ~NPCAI_ENROUTE_TO_HOMEWP;
					G_ActivateBehavior( NPC, BSET_LOSTENEMY );
				}

			}

			//gi.Printf("Got there.\n");
			//gi.Printf("Looking...");
			if( !Q_irand(0, 1) )
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_LOOKAROUND1, SETANIM_FLAG_NORMAL);
			}
			else
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_IDLE1, SETANIM_FLAG_NORMAL);
			}
			NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
		}
		else
		{
			NPC_MoveToGoal();
		}
	}
	else
	{
		//We're there
		if ( NPCInfo->investigateDebounceTime > level.time )
		{
			//Still waiting around for a bit
			//Turn angles every now and then to look around
			if ( NPCInfo->tempGoal->waypoint != WAYPOINT_NONE )
			{
				if ( !Q_irand( 0, 30 ) )
				{
					int	numEdges = navigator.GetNodeNumEdges( NPCInfo->tempGoal->waypoint );

					if ( numEdges != WAYPOINT_NONE )
					{
						int branchNum = Q_irand( 0, numEdges - 1 );

						vec3_t	branchPos, lookDir;

						navigator.GetNodePosition( branchNum, branchPos );

						VectorSubtract( branchPos, NPCInfo->tempGoal->currentOrigin, lookDir );
						NPCInfo->desiredYaw = AngleMod( vectoyaw( lookDir ) + Q_flrand( -45, 45 ) );
					}

					//pick an angle +-45 degrees off of the dir of a random branch
					//from NPCInfo->tempGoal->waypoint
					//int branch = Q_irand( 0, (waypoints[NPCInfo->tempGoal->waypoint].numNeighbors - 1) );
					//int	nextWp = waypoints[NPCInfo->tempGoal->waypoint].nextWaypoint[branch][NPCInfo->stats.moveType];
					//vec3_t	lookDir;

					//VectorSubtract( waypoints[nextWp].origin, NPCInfo->tempGoal->currentOrigin, lookDir );
					//Look in that direction +- 45 degrees
					//NPCInfo->desiredYaw = AngleMod( vectoyaw( lookDir ) + Q_flrand( -45, 45 ) );
				}
			}
			//gi.Printf(".");
		}
		else
		{//Just finished waiting
			NPC->waypoint = NAV_FindClosestWaypointForEnt( NPC, WAYPOINT_NONE );
			
			if ( NPC->waypoint == NPCInfo->homeWp )
			{
				int	numEdges = navigator.GetNodeNumEdges( NPCInfo->tempGoal->waypoint );

				if ( numEdges != WAYPOINT_NONE )
				{
					int branchNum = Q_irand( 0, numEdges - 1 );

					int nextWp = navigator.GetBestNode( NPCInfo->homeWp, branchNum );
					navigator.GetNodePosition( nextWp, NPCInfo->tempGoal->currentOrigin );
					NPCInfo->tempGoal->waypoint = nextWp;
				}

				/*
				//Pick a random branch
				int branch = Q_irand( 0, (waypoints[NPCInfo->homeWp].numNeighbors - 1) );
				int	nextWp = waypoints[NPCInfo->homeWp].nextWaypoint[branch][NPCInfo->stats.moveType];

				VectorCopy( waypoints[nextWp].origin, NPCInfo->tempGoal->currentOrigin );
				NPCInfo->tempGoal->waypoint = nextWp;
				//gi.Printf("\nHeading for wp %d...\n", waypoints[NPCInfo->homeWp].nextWaypoint[branch][NPCInfo->stats.moveType]);
				*/
			}
			else
			{//At a branch, so return home
				navigator.GetNodePosition( NPCInfo->homeWp, NPCInfo->tempGoal->currentOrigin );
				NPCInfo->tempGoal->waypoint = NPCInfo->homeWp;
				/*
				VectorCopy( waypoints[NPCInfo->homeWp].origin, NPCInfo->tempGoal->currentOrigin );
				NPCInfo->tempGoal->waypoint = NPCInfo->homeWp;
				//gi.Printf("\nHeading for wp %d...\n", NPCInfo->homeWp);
				*/
			}

			NPCInfo->investigateDebounceTime = 0;
			//Start moving toward our tempGoal
			NPCInfo->goalEntity = NPCInfo->tempGoal;
			NPC_MoveToGoal();
		}
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSSearchStart
-------------------------
*/

void NPC_BSSearchStart( int homeWp, bState_t bState )
{
	//FIXME: Reimplement
	NPCInfo->homeWp = homeWp;
	NPCInfo->tempBehavior = bState;
	NPCInfo->aiFlags |= NPCAI_ENROUTE_TO_HOMEWP;
	NPCInfo->investigateDebounceTime = 0;
	navigator.GetNodePosition( homeWp, NPCInfo->tempGoal->currentOrigin );
	NPCInfo->tempGoal->waypoint = homeWp;
	//gi.Printf("\nHeading for wp %d...\n", NPCInfo->homeWp);
}

/*
void NPC_LostEnemyDecideChase(void)

  We lost our enemy and want to drop him but see if we should chase him if we are in the proper bState
*/

void NPC_LostEnemyDecideChase(void)
{
	switch( NPCInfo->behaviorState )
	{
	case BS_HUNT_AND_KILL:
	case BS_COMBAT:
	case BS_RUN_AND_SHOOT:
		//We were chasing him and lost him, so try to find him
		if ( NPC->enemy == NPCInfo->goalEntity && NPC->enemy->lastWaypoint != WAYPOINT_NONE )
		{//Remember his last valid Wp, then check it out
			//FIXME: Should we only do this if there's no other enemies or we've got LOCKED_ENEMY on?
			NPC_BSSearchStart( NPC->enemy->lastWaypoint, BS_SEARCH );
		}
		//If he's not our goalEntity, we're running somewhere else, so lose him
		break;
	default:
		break;
	}
	G_ClearEnemy( NPC );
}

/*
-------------------------
NPC_FlyToGoal
-------------------------
*/

#define	MOVE_INCR			16
#define	MIN_NAVGOAL_DIST	16

void NPC_FlyToGoal( void )
{
	//Must have a destination
	if ( NPCInfo->goalEntity == NULL )
		return;

	vec3_t	moveDir;

	//Get our move info
	VectorSubtract( NPCInfo->goalEntity->currentOrigin, NPC->currentOrigin, moveDir );

	float len = VectorNormalize( moveDir );

	//See if we're in range of the goal
	if ( len < MIN_NAVGOAL_DIST )
	{
		//FIXME: We want to increment our next goal if there is one
		NPCInfo->goalEntity = UpdateGoal();
	}

	if ( ( NPC->speed + MOVE_INCR ) < NPCInfo->stats.runSpeed )
		NPC->speed += MOVE_INCR;

	//Move
	VectorScale( moveDir, NPC->speed, NPC->client->ps.velocity );

	vec3_t	moveAngles;

	vectoangles( moveDir, moveAngles );

	NPCInfo->desiredYaw = AngleNormalize360( moveAngles[YAW] );
}

/*
-------------------------
NPC_BSFly
-------------------------
*/

void NPC_BSFly ( void )
{
	if ( UpdateGoal() )
	{
		NPC_MoveToGoal();
		//NPC_FlyToGoal();
	}
	else
	{
		//Cut velocity?
		VectorClear( NPC->client->ps.velocity );
	}
	
	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSNoClip

  Use in extreme circumstances only
-------------------------
*/

void NPC_BSNoClip ( void )
{
	if ( UpdateGoal() )
	{
		vec3_t	dir, forward, right, angles, up = {0, 0, 1};
		float	fDot, rDot, uDot;

		VectorSubtract( NPCInfo->goalEntity->currentOrigin, NPC->currentOrigin, dir );
		
		vectoangles( dir, angles );
		NPCInfo->desiredYaw = angles[YAW];

		AngleVectors( NPC->currentAngles, forward, right, NULL );

		VectorNormalize( dir );

		fDot = DotProduct(forward, dir) * 127;
		rDot = DotProduct(right, dir) * 127;
		uDot = DotProduct(up, dir) * 127;

		ucmd.forwardmove = floor(fDot);
		ucmd.rightmove = floor(rDot);
		ucmd.upmove = floor(uDot);
	}
	else
	{
		//Cut velocity?
		VectorClear( NPC->client->ps.velocity );
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

void NPC_BSWander (void)
{//FIXME: don't actually go all the way to the next waypoint, just move in fits and jerks...?
	if ( !NPCInfo->investigateDebounceTime )
	{//Starting out
		float	minGoalReachedDistSquared = 64;//32*32;
		vec3_t	vec;

		//Keep moving toward our tempGoal
		NPCInfo->goalEntity = NPCInfo->tempGoal;

		VectorSubtract ( NPCInfo->tempGoal->currentOrigin, NPC->currentOrigin, vec);

		if ( NPCInfo->tempGoal->waypoint != WAYPOINT_NONE )
		{
			minGoalReachedDistSquared = 64;
		}

		if ( VectorLengthSquared( vec ) < minGoalReachedDistSquared )
		{
			//Close enough, just got there
			NPC->waypoint = NAV_FindClosestWaypointForEnt( NPC, WAYPOINT_NONE );

			if( !Q_irand(0, 1) )
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_LOOKAROUND1, SETANIM_FLAG_NORMAL);
			}
			else
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_IDLE1, SETANIM_FLAG_NORMAL);
			}
			//Just got here, so Look around for a while
			NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
		}
		else
		{
			//Keep moving toward goal
			NPC_MoveToGoal();
		}
	}
	else
	{
		//We're there
		if ( NPCInfo->investigateDebounceTime > level.time )
		{
			//Still waiting around for a bit
			//Turn angles every now and then to look around
			if ( NPCInfo->tempGoal->waypoint != WAYPOINT_NONE )
			{
				if ( !Q_irand( 0, 30 ) )
				{
					int	numEdges = navigator.GetNodeNumEdges( NPCInfo->tempGoal->waypoint );

					if ( numEdges != WAYPOINT_NONE )
					{
						int branchNum = Q_irand( 0, numEdges - 1 );

						vec3_t	branchPos, lookDir;

						navigator.GetNodePosition( branchNum, branchPos );

						VectorSubtract( branchPos, NPCInfo->tempGoal->currentOrigin, lookDir );
						NPCInfo->desiredYaw = AngleMod( vectoyaw( lookDir ) + Q_flrand( -45, 45 ) );
					}
				}
			}
		}
		else
		{//Just finished waiting
			NPC->waypoint = NAV_FindClosestWaypointForEnt( NPC, WAYPOINT_NONE );
			
			if ( NPC->waypoint != WAYPOINT_NONE )
			{
				int	numEdges = navigator.GetNodeNumEdges( NPC->waypoint );

				if ( numEdges != WAYPOINT_NONE )
				{
					int branchNum = Q_irand( 0, numEdges - 1 );

					int nextWp = navigator.GetBestNode( NPC->waypoint, branchNum );
					navigator.GetNodePosition( nextWp, NPCInfo->tempGoal->currentOrigin );
					NPCInfo->tempGoal->waypoint = nextWp;
				}

				NPCInfo->investigateDebounceTime = 0;
				//Start moving toward our tempGoal
				NPCInfo->goalEntity = NPCInfo->tempGoal;
				NPC_MoveToGoal();
			}
		}
	}

	NPC_UpdateAngles( qtrue, qtrue );
}

void NPC_BSFaceLeader (void)
{
	vec3_t	head, leaderHead, delta, angleToLeader;

	if ( !NPC->client->leader )
	{//uh.... okay.
		return;
	}

	CalcEntitySpot( NPC->client->leader, SPOT_HEAD, leaderHead );
	CalcEntitySpot( NPC, SPOT_HEAD, head );
	VectorSubtract( leaderHead, head, delta );
	vectoangles( delta, angleToLeader );
	VectorNormalize( delta );
	NPC->NPC->desiredYaw = angleToLeader[YAW];
	NPC->NPC->desiredPitch = angleToLeader[PITCH];
	
	NPC_UpdateAngles(qtrue, qtrue);
}
