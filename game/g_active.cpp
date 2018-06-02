
#include "g_local.h"
#include "g_functions.h"
#include "..\cgame\cg_local.h"
#include "Q3_Interface.h"

#define	SLOWDOWN_DIST	128.0f
#define	MIN_NPC_SPEED	16.0f

extern qboolean Q3_TaskIDPending( gentity_t *ent, taskID_t taskType );
extern void G_MaintainFormations(gentity_t *self);
extern void BG_CalculateOffsetAngles( gentity_t *ent, usercmd_t *ucmd );//in bg_pangles.cpp
extern void TryUse( gentity_t *ent );
extern void ChangeWeapon( gentity_t *ent, int newWeapon );
extern void ScoreBoardReset(void);

extern	bool		in_camera;
extern	qboolean	player_locked;
extern qboolean	stop_icarus;
extern	cvar_t		*g_spskill;


/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 255 ) {
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->painDebounceTime) && !(player->flags & FL_GODMODE) ) 
	{
		player->painDebounceTime = level.time + 700;
		client->ps.damageEvent++;
		if ( !Q3_TaskIDPending( player, TID_CHAN_VOICE ) )
		{
			G_AddEvent( player, EV_PAIN, player->health );
		}
	}


	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	int			waterlevel;

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;
	//
	// check for drowning
	//
	if ( waterlevel == 3 && !(ent->watertype&CONTENTS_LADDER) ) {

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {
			// drown!
			ent->client->airOutTime += 1000;
			if ( ent->health > 0 ) {
				// take more damage the longer underwater
				ent->damage += 2;
				if (ent->damage > 15)
					ent->damage = 15;

				// play a gurp sound instead of a normal pain sound
				if (ent->health <= ent->damage) {
					G_Sound(ent, G_SoundIndex("sound/player/hm_male/drown.wav"));
				} else if (rand()&1) {
					G_Sound(ent, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, G_SoundIndex("sound/player/gurp2.wav"));
				}

				// don't play a normal pain sound
				ent->painDebounceTime = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && 
		(ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
		if (ent->health > 0
			&& ent->painDebounceTime < level.time	) {

			if (ent->watertype & CONTENTS_LAVA) {
				G_Damage (ent, NULL, NULL, NULL, NULL, 
					15*waterlevel, 0, MOD_LAVA);
			}

			if (ent->watertype & CONTENTS_SLIME) {
				G_Damage (ent, NULL, NULL, NULL, NULL, 
					1, 0, MOD_SLIME);
			}
		}
	}
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
//	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.loopSound = G_SoundIndex("sound/weapons/stasis/electricloop.wav");

//	else
//		ent->s.loopSound = 0;
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->NPC != NULL ) && ( ent->e_TouchFunc != touchF_NULL ) ) {	// last check unneccessary
			GEntity_TouchFunc( ent, other, &trace );
		}

		if ( other->e_TouchFunc == touchF_NULL ) {	// not needed, but I'll leave it I guess (cache-hit issues)
			continue;
		}
		GEntity_TouchFunc( other, ent, &trace );
	}

}

/*
============
G_TouchTriggersLerped

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.

This version checks at 6 unit steps between last and current origins
============
*/
void	G_TouchTriggersLerped( gentity_t *ent ) {
	int			i, num;
	float		dist, curDist = 0;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	trace_t		trace;
	vec3_t		end, mins, maxs, diff;
	const vec3_t	range = { 40, 40, 52 };
	qboolean	touched[MAX_GENTITIES];
	qboolean	done = qfalse;

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if ( g_fixFreezeBorg1->integer ) {
		// let us not have an infinite loop
		if (ent->maxs[1] <= 0.0) {
			return;
		}
	}

	VectorSubtract( ent->currentOrigin, ent->lastOrigin, diff );
	dist = VectorNormalize( diff );

	memset (touched, qfalse, sizeof(touched) );

	for ( curDist = 0; !done; curDist += (float)ent->maxs[1]/2.0f )
	{
		if ( curDist >= dist )
		{
			VectorCopy( ent->currentOrigin, end );
			done = qtrue;
		}
		else
		{
			VectorMA( ent->lastOrigin, curDist, diff, end );
		}
		VectorSubtract( end, range, mins );
		VectorAdd( end, range, maxs );

		num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->absmin, because that has a one unit pad
		VectorAdd( end, ent->mins, mins );
		VectorAdd( end, ent->maxs, maxs );

		for ( i=0 ; i<num ; i++ ) {
			hit = touch[i];

			if ( (hit->e_TouchFunc == touchF_NULL) && (ent->e_TouchFunc == touchF_NULL) ) {
				continue;
			}
			if ( !( hit->contents & CONTENTS_TRIGGER ) ) {
				continue;
			}

			if ( touched[i] == qtrue ) {
				continue;//already touched this move
			}
			// use seperate code for determining if an item is picked up
			// so you don't have to actually contact its bounding box
			/*
			if ( hit->s.eType == ET_ITEM ) {
				if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
					continue;
				}
			} else */
			{
				if ( !gi.EntityContact( mins, maxs, hit ) ) {
					continue;
				}
			}

			touched[i] = qtrue;

			memset( &trace, 0, sizeof(trace) );

			if ( hit->e_TouchFunc != touchF_NULL ) {
				GEntity_TouchFunc(hit, ent, &trace);
			}

			if ( ( ent->NPC != NULL ) && ( ent->e_TouchFunc != touchF_NULL ) ) {
				GEntity_TouchFunc( ent, hit, &trace );
			}
		}
	}
}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	const vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->mins, mins );
	VectorAdd( ent->client->ps.origin, ent->maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = touch[i];

		if ( (hit->e_TouchFunc == touchF_NULL) && (ent->e_TouchFunc == touchF_NULL) ) {
			continue;
		}
		if ( !( hit->contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		/*
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else */
		{
			if ( !gi.EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->e_TouchFunc != touchF_NULL ) {
			GEntity_TouchFunc(hit, ent, &trace);
		}

		if ( ( ent->NPC != NULL ) && ( ent->e_TouchFunc != touchF_NULL ) ) {
			GEntity_TouchFunc( ent, hit, &trace );
		}
	}
}


/*
============
G_MoverTouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void G_MoverTouchTeleportTriggers( gentity_t *ent, vec3_t oldOrg ) 
{
	int			i, num;
	float		step, stepSize, dist;
	gentity_t	*touch[MAX_GENTITIES], *hit;
	trace_t		trace;
	vec3_t		mins, maxs, dir, size, checkSpot;
	const vec3_t	range = { 40, 40, 52 };

	// non-moving movers don't hit triggers!
	if ( !VectorLengthSquared( ent->s.pos.trDelta ) ) 
	{
		return;
	}

	VectorSubtract( ent->mins, ent->maxs, size );
	stepSize = VectorLength( size );
	if ( stepSize < 1 )
	{
		stepSize = 1;
	}

	VectorSubtract( ent->currentOrigin, oldOrg, dir );
	dist = VectorNormalize( dir );
	for ( step = 0; step <= dist; step += stepSize )
	{
		VectorMA( ent->currentOrigin, step, dir, checkSpot );
		VectorSubtract( checkSpot, range, mins );
		VectorAdd( checkSpot, range, maxs );

		num = gi.EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

		// can't use ent->absmin, because that has a one unit pad
		VectorAdd( checkSpot, ent->mins, mins );
		VectorAdd( checkSpot, ent->maxs, maxs );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = touch[i];

			if ( hit->s.eType != ET_TELEPORT_TRIGGER )
			{
				continue;
			}

			if ( hit->e_TouchFunc == touchF_NULL ) 
			{
				continue;
			}

			if ( !( hit->contents & CONTENTS_TRIGGER ) ) 
			{
				continue;
			}


			if ( !gi.EntityContact( mins, maxs, hit ) ) 
			{
				continue;
			}

			memset( &trace, 0, sizeof(trace) );

			if ( hit->e_TouchFunc != touchF_NULL ) 
			{
				GEntity_TouchFunc(hit, ent, &trace);
			}
		}
	}
}

void G_NPCMunroMatchPlayerWeapon( gentity_t *ent )
{
	//special uber hack for cinematic Munro's to match player's weapon
	if ( !in_camera )
	{
		if ( ent && ent->client && ent->NPC && (ent->NPC->aiFlags&NPCAI_MATCHPLAYERWEAPON) )
		{//we're a Munro NPC
			int newWeap;
			if ( g_entities[0].client->ps.weapon == WP_PHASER || g_entities[0].client->ps.weapon > WP_DREADNOUGHT )//WP_VOYAGER_HYPO
			{
				newWeap = WP_COMPRESSION_RIFLE;
			}
			else
			{
				newWeap = g_entities[0].client->ps.weapon;
			}
			if ( newWeap != WP_NONE && ent->client->ps.weapon != newWeap )
			{
				ent->client->ps.stats[STAT_WEAPONS] = ( 1 << newWeap );
				ent->client->ps.ammo[weaponData[newWeap].ammoIndex] = 999;
				ChangeWeapon( ent, newWeap );
				ent->client->ps.weapon = newWeap;
				ent->client->ps.weaponstate = WEAPON_READY;
			}
		}
	}
}
/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
	if ( ! g_inactivity->integer ) {
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;
	} else if ( client->usercmd.forwardmove || 
		client->usercmd.rightmove || 
		client->usercmd.upmove ||
		(client->usercmd.buttons & BUTTON_ATTACK) ||
		(client->usercmd.buttons & BUTTON_ALT_ATTACK) ) {
		client->inactivityTime = level.time + g_inactivity->integer * 1000;
		client->inactivityWarning = qfalse;
	} else if ( !client->pers.localClient ) {
		if ( level.time > client->inactivityTime ) {
			gi.DropClient( client - level.clients, "Dropped due to inactivity" );
			return qfalse;
		}
		if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
			client->inactivityWarning = qtrue;
			gi.SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t *client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;
	}
}

/*
====================
ClientIntermissionThink
====================
*/
static qboolean ClientCinematicThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// swap button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->usercmd.buttons;
	if ( client->buttons & ( /*BUTTON_ATTACK |*/ BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) {
		return( qtrue );
	}
	return( qfalse );
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int		i;
	int		event;
	gclient_t *client;
	int		damage;
	qboolean	fired;

	client = ent->client;

	fired = qfalse;

	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL_MEDIUM:
		case EV_FALL_FAR://these come from bg_pmove, PM_CrashLand
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			//FIXME: isn't there a more accurate way to calculate damage from falls?
			if ( event == EV_FALL_FAR ) 
			{
				damage = 50;
			} 
			else 
			{
				damage = 25;
			}
			ent->painDebounceTime = level.time + 200;	// no normal pain sound
			G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
			break;

		case EV_FIRE_WEAPON:
#ifndef FINAL_BUILD
			if ( fired ) {
				gi.Printf( "DOUBLE EV_FIRE_WEAPON AND-OR EV_ALT_FIRE!!\n" );
			}
#endif
			fired = qtrue;
			FireWeapon( ent, qfalse );
			break;

		case EV_ALT_FIRE:
#ifndef FINAL_BUILD
			if ( fired ) {
				gi.Printf( "DOUBLE EV_FIRE_WEAPON AND-OR EV_ALT_FIRE!!\n" );
			}
#endif
			fired = qtrue;
			FireWeapon( ent, qtrue );
			break;

		default:
			break;
		}
	}

}

void BG_AddPushVecToUcmd(gentity_t *self, usercmd_t *ucmd)
{
	vec3_t	forward, right, moveDir;
	float	pushSpeed, fMove, rMove;

	pushSpeed = VectorLengthSquared(self->s.pushVec);
	if(!pushSpeed)
	{//not being pushed
		return;
	}

	AngleVectors(self->client->ps.viewangles, forward, right, NULL);
	VectorScale(forward, ucmd->forwardmove/127.0f * self->client->ps.speed, moveDir);
	VectorMA(moveDir, ucmd->rightmove/127.0f * self->client->ps.speed, right, moveDir);
	//moveDir is now our intended move velocity

	VectorAdd(moveDir, self->s.pushVec, moveDir);
	self->client->ps.speed = VectorNormalize(moveDir);
	//moveDir is now our intended move velocity plus our push Vector

	fMove = 127.0 * DotProduct(forward, moveDir);
	rMove = 127.0 * DotProduct(right, moveDir);
	ucmd->forwardmove = floor(fMove);//If in the same dir , will be positive
	ucmd->rightmove = floor(rMove);//If in the same dir , will be positive

	VectorClear(self->s.pushVec);
}

void NPC_Accelerate( gentity_t *ent, qboolean fullWalkAcc, qboolean fullRunAcc )
{
	if ( !ent->client || !ent->NPC )
	{
		return;
	}

	if ( !ent->NPC->stats.acceleration )
	{//No acceleration means just start and stop
		ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
	}
	//FIXME:  in cinematics always accel/decel?
	else if ( ent->NPC->desiredSpeed <= ent->NPC->stats.walkSpeed )
	{//Only accelerate if at walkSpeeds
		if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed + ent->NPC->stats.acceleration )
		{
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed += ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed )
		{
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
		else if ( fullWalkAcc && ent->NPC->desiredSpeed < ent->NPC->currentSpeed - ent->NPC->stats.acceleration )
		{//decelerate even when walking
			ent->NPC->currentSpeed -= ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
		{//stop on a dime
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
	}
	else//  if ( ent->NPC->desiredSpeed > ent->NPC->stats.walkSpeed )
	{//Only decelerate if at runSpeeds
		if ( fullRunAcc && ent->NPC->desiredSpeed > ent->NPC->currentSpeed + ent->NPC->stats.acceleration )
		{//Accelerate to runspeed
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed += ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed > ent->NPC->currentSpeed )
		{//accelerate instantly
			//ent->client->ps.friction = 0;
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
		else if ( fullRunAcc && ent->NPC->desiredSpeed < ent->NPC->currentSpeed - ent->NPC->stats.acceleration )
		{
			ent->NPC->currentSpeed -= ent->NPC->stats.acceleration;
		}
		else if ( ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
		{
			ent->NPC->currentSpeed = ent->NPC->desiredSpeed;
		}
	}
}

/*
-------------------------
NPC_GetWalkSpeed
-------------------------
*/

static int NPC_GetWalkSpeed( gentity_t *ent )
{
	int	walkSpeed = 0;

	if ( ( ent->client == NULL ) || ( ent->NPC == NULL ) )
		return 0;

	switch ( ent->client->playerTeam )
	{
	case TEAM_BORG:	//To shutup compiler, will add entries later (this is stub code)
	default:
		walkSpeed = ent->NPC->stats.walkSpeed;
		break;
	}

	return walkSpeed;
}

/*
-------------------------
NPC_GetRunSpeed
-------------------------
*/
#define	BORG_RUN_INCR		25
#define SPECIES_RUN_INCR	25	
#define STASIS_RUN_INCR		20
#define	WARBOT_RUN_INCR		20

static int NPC_GetRunSpeed( gentity_t *ent )
{
	int	runSpeed = 0;

	if ( ( ent->client == NULL ) || ( ent->NPC == NULL ) )
		return 0;

	switch ( ent->client->playerTeam )
	{
	case TEAM_BORG:
		runSpeed = ent->NPC->stats.runSpeed;

		runSpeed += BORG_RUN_INCR * (g_spskill->integer%3);
		break;

	case TEAM_8472:
		runSpeed = ent->NPC->stats.runSpeed;
		runSpeed += SPECIES_RUN_INCR * (g_spskill->integer%3);
		break;

	case TEAM_STASIS:
		runSpeed = ent->NPC->stats.runSpeed;
		runSpeed += STASIS_RUN_INCR * (g_spskill->integer%3);
		break;

	case TEAM_BOTS:
		
		//Only for warbot
		if ( ( Q_stricmp( ent->NPC_type, "warriorbot" ) == 0 ) || ( Q_stricmp( ent->NPC_type, "warriorbot_boss" ) == 0 ) )
		{
			runSpeed = ent->NPC->stats.runSpeed;
			runSpeed += WARBOT_RUN_INCR * (g_spskill->integer%3);
			break;
		}
		
		//NOTENOTE: Falls through for other bots

	default:
		runSpeed = ent->NPC->stats.runSpeed;
		break;
	}

	return runSpeed;
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

==============
*/

void ClientThink_real( gentity_t *ent, usercmd_t *ucmd ) 
{
	gclient_t	*client;
	pmove_t		pm;
	vec3_t		oldOrigin;
	int			oldEventSequence;
	int			msec;

	//Don't let the player do anything if in a camera
	if ( ent->s.number == 0 ) {
extern cvar_t	*g_skippingcin;
		if ( in_camera )
		{
			// watch the code here, you MUST "return" within this IF(), *unless* you're stopping the cinematic skip.
			//
			if ( ClientCinematicThink(ent->client) )
			{
				if (g_skippingcin->integer)	// already doing cinematic skip?
				{
					// yes...   so stop skipping...
					gi.cvar_set("skippingCinematic", "0");
					gi.cvar_set("timescale", "1");
				}
				else
				{
					// no... so start skipping...
					gi.cvar_set("skippingCinematic", "1");
					gi.cvar_set("timescale", "100");
					return;
				}
			}
			else
			{
				return;
			}
		}
		else 
		{
			if ( g_skippingcin->integer )
			{//We're skipping the cinematic and it's over now
				gi.cvar_set("timescale", "1");
				gi.cvar_set("skippingCinematic", "0");
			}
			if ( ent->client->ps.pm_type == PM_DEAD && cg.missionStatusDeadTime < level.time )
			{//mission status screen is up because player is dead, stop all scripts
				if (Q_stricmpn(level.mapname,"_holo",5)) {
					stop_icarus = qtrue;
				}
			}
		}

		// Don't allow the player to adjust the pitch when they are in third person overhead cam.
extern vmCvar_t cg_thirdPerson;
		if ( cg_thirdPerson.integer == 2 )
		{
			ucmd->angles[PITCH] = 0;
		}

		if ( player_locked && ent->client->ps.pm_type < PM_DEAD ) {//lock out player control unless dead
			VectorClear(ucmd->angles) ;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->buttons = 0;
			ucmd->upmove = 0;
		}
	}
	else
	{
		G_NPCMunroMatchPlayerWeapon( ent );
	}
	client = ent->client;

	// mark the time, so the connection sprite can be removed
	client->lastCmdTime = level.time;
	client->pers.lastCommand = *ucmd;

	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) 
	{
		ucmd->serverTime = level.time + 200;
	}
	if ( ucmd->serverTime < level.time - 1000 ) 
	{
		ucmd->serverTime = level.time - 1000;
	} 

	msec = ucmd->serverTime - client->ps.commandTime;
	if ( msec < 1 ) 
	{
		msec = 1;
	}
	if ( msec > 200 ) 
	{
		msec = 200;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if ( !ClientInactivityTimer( client ) ) 
		return;

	if ( client->noclip ) 
	{
		client->ps.pm_type = PM_NOCLIP;
	} 
	else if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		client->ps.pm_type = PM_DEAD;
	} 
	else 
	{
		client->ps.pm_type = PM_NORMAL;
	}

	//FIXME: if global gravity changes this should update everyone's personal gravity...
	if ( !(ent->svFlags & SVF_CUSTOM_GRAVITY) )
	{
		client->ps.gravity = g_gravity->value;
	}

	// set speed
	if ( ent->NPC != NULL )
	{//we don't actually scale the ucmd, we use actual speeds
		if ( ent->NPC->combatMove == qfalse )
		{
			if ( !(ucmd->buttons & BUTTON_USE) )
			{//Not leaning
				qboolean Flying = (ucmd->upmove && ent->NPC->stats.moveType == MT_FLYSWIM);
				qboolean Climbing = (ucmd->upmove && ent->watertype&CONTENTS_LADDER );

				client->ps.friction = 6;

				if ( ucmd->forwardmove || ucmd->rightmove || Flying )
				{
					if ( ent->NPC->behaviorState != BS_FORMATION )
					{//In - Formation NPCs set thier desiredSpeed themselves
						if ( ucmd->buttons & BUTTON_WALKING )
						{
							ent->NPC->desiredSpeed = NPC_GetWalkSpeed( ent );//ent->NPC->stats.walkSpeed;
						}
						else//running
						{
							ent->NPC->desiredSpeed = NPC_GetRunSpeed( ent );//ent->NPC->stats.runSpeed;
						}

						if ( ent->NPC->currentSpeed >= 80 )
						{//At higher speeds, need to slow down close to stuff
							//Slow down as you approach your goal
							if ( ent->NPC->distToGoal < SLOWDOWN_DIST && client->race != RACE_BORG && !(ent->NPC->aiFlags&NPCAI_NO_SLOWDOWN) )//128
							{
								if ( ent->NPC->desiredSpeed > MIN_NPC_SPEED )
								{
									float slowdownSpeed = ((float)ent->NPC->desiredSpeed) * ent->NPC->distToGoal / SLOWDOWN_DIST;

									ent->NPC->desiredSpeed = ceil(slowdownSpeed);
									if ( ent->NPC->desiredSpeed < MIN_NPC_SPEED )
									{//don't slow down too much
										ent->NPC->desiredSpeed = MIN_NPC_SPEED;
									}
								}
							}
						}
					}
				}
				else if ( Climbing )
				{
					ent->NPC->desiredSpeed = ent->NPC->stats.walkSpeed;
				}
				else
				{//We want to stop
					ent->NPC->desiredSpeed = 0;
				}

				NPC_Accelerate( ent, (ent->NPC->behaviorState==BS_FORMATION), (ent->NPC->behaviorState==BS_FORMATION) );

				if ( ent->NPC->currentSpeed <= 24 && ent->NPC->desiredSpeed < ent->NPC->currentSpeed )
				{//No-one walks this slow
					client->ps.speed = ent->NPC->currentSpeed = 0;//Full stop
					ucmd->forwardmove = 0;
					ucmd->rightmove = 0;
				}
				else
				{
					if ( ent->NPC->currentSpeed <= ent->NPC->stats.walkSpeed )
					{//Play the walkanim
						ucmd->buttons |= BUTTON_WALKING;
					}
					else
					{
						ucmd->buttons &= ~BUTTON_WALKING;
					}

					if ( ent->NPC->currentSpeed > 0 )
					{//We should be moving
						if ( Climbing || Flying )
						{
							if ( !ucmd->upmove )
							{//We need to force them to take a couple more steps until stopped
								ucmd->upmove = ent->NPC->last_ucmd.upmove;//was last_upmove;
							}
						}
						else if ( !ucmd->forwardmove && !ucmd->rightmove )
						{//We need to force them to take a couple more steps until stopped
							ucmd->forwardmove = ent->NPC->last_ucmd.forwardmove;//was last_forwardmove;
							ucmd->rightmove = ent->NPC->last_ucmd.rightmove;//was last_rightmove;
						}
					}

					client->ps.speed = ent->NPC->currentSpeed;
					//Slow down on turns - don't orbit!!!
					float turndelta = (180 - fabs( AngleDelta( ent->currentAngles[YAW], ent->NPC->desiredYaw ) ))/180;
					
					if ( turndelta < 0.75f )
					{
						client->ps.speed = 0;
					}
					else if ( ent->NPC->distToGoal < 100 && turndelta < 1.0 )
					{//Turn is greater than 45 degrees or closer than 100 to goal
						client->ps.speed = floor(((float)(client->ps.speed))*turndelta);
					}
				}
			}
		}
		else
		{	
			ent->NPC->desiredSpeed = ( ucmd->buttons & BUTTON_WALKING ) ? NPC_GetWalkSpeed( ent ) : NPC_GetRunSpeed( ent );

			client->ps.speed = ent->NPC->desiredSpeed;
		}
	}
	else
	{//Client sets ucmds and such for speed alterations
		client->ps.speed = g_speed->value;//default is 320
	}

	//Apply forced movement
	if ( client->forced_forwardmove )
	{
		ucmd->forwardmove = client->forced_forwardmove;
		if ( !client->ps.speed )
		{
			if ( ent->NPC != NULL )
			{
				client->ps.speed = ent->NPC->stats.runSpeed;
			}
			else
			{
				client->ps.speed = g_speed->value;//default is 320
			}
		}
	}

	if ( client->forced_rightmove )
	{
		ucmd->rightmove = client->forced_rightmove;
		if ( !client->ps.speed )
		{
			if ( ent->NPC != NULL  )
			{
				client->ps.speed = ent->NPC->stats.runSpeed;
			}
			else
			{
				client->ps.speed = g_speed->value;//default is 320
			}
		}
	}

	//FIXME: need to do this before check to avoid walls and cliffs (or just cliffs?)
	BG_AddPushVecToUcmd( ent, ucmd );

	BG_CalculateOffsetAngles( ent, ucmd );

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	memset( &pm, 0, sizeof(pm) );

	pm.gent = ent;
	pm.ps = &client->ps;
	pm.cmd = *ucmd;
//	pm.tracemask = MASK_PLAYERSOLID;	// used differently for navgen
	pm.tracemask = ent->clipmask;
	pm.trace = gi.trace;
	pm.pointcontents = gi.pointcontents;
	pm.debugLevel = g_debugMove->integer;
	pm.noFootsteps = 0;//( g_dmflags->integer & DF_NO_FOOTSTEPS ) > 0;

	VectorCopy( client->ps.origin, oldOrigin );

	// perform a pmove
	Pmove( &pm );

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) 
	{
		ent->eventTime = level.time;
		{
			int		seq;

			seq = (ent->client->ps.eventSequence-1) & (MAX_PS_EVENTS-1);
			ent->s.event = ent->client->ps.events[ seq ] | ( ( ent->client->ps.eventSequence & 3 ) << 8 );
			ent->s.eventParm = ent->client->ps.eventParms[ seq ];
		}
	}
	PlayerStateToEntityState( &ent->client->ps, &ent->s );

	VectorCopy ( ent->currentOrigin, ent->lastOrigin );
#if 1
	// use the precise origin for linking
	VectorCopy( ent->client->ps.origin, ent->currentOrigin );
#else
	//We don't use prediction anymore, so screw this
	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->currentOrigin );
#endif

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	VectorCopy( ucmd->angles, client->pers.cmd_angles );

	// execute client events
	ClientEvents( ent, oldEventSequence );

	if ( pm.useEvent )
	{
		//TODO: Use
		TryUse( ent );
	}

	// link entity now, after any personal teleporters have been used
	gi.linkentity( ent );
	ent->client->hiddenDist = 0;
	if ( !ent->client->noclip ) 
	{
		G_TouchTriggersLerped( ent );
	}

	// touch other objects
	ClientImpacts( ent, &pm );

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		// wait for the attack button to be pressed
		if ( ent->NPC == NULL && level.time > client->respawnTime ) 
		{
			// don't allow respawn if they are still flying through the
			// air, unless 10 extra seconds have passed, meaning something
			// strange is going on, like the corpse is caught in a wind tunnel
			if ( level.time < client->respawnTime + 10000 ) 
			{
				if ( client->ps.groundEntityNum == ENTITYNUM_NONE ) 
				{
					return;
				}
			}

			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ) 
			{
				respawn( ent );
			}
		}
		return;
	}

	if ((cg.missionStatusShow) && ((cg.missionStatusDeadTime + 1) < level.time))
	{
		if ( ucmd->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) ) 
		{
			cg.missionStatusShow = 0;
			ScoreBoardReset();
//			Q3_TaskIDComplete( ent, TID_MISSIONSTATUS );
		}
	}
	// perform once-a-second actions
	//ClientTimerActions( ent, msec );

	//DEBUG INFO
/*
	if ( client->ps.clientNum < 1 )
	{//Only a player
		if ( ucmd->buttons & BUTTON_USE )
		{
			NAV_PrintLocalWpDebugInfo( ent );
		}
	}
*/
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum, usercmd_t *ucmd ) {
	gentity_t *ent;

	ent = g_entities + clientNum;
	ent->client->usercmd = *ucmd;
//	if ( !g_syncronousClients->integer ) 
	{
		ClientThink_real( ent, ucmd );
	}
}


/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;

	// turn off any expired powerups
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ent->client->ps.powerups[ i ] < level.time ) {
			ent->client->ps.powerups[ i ] = 0;
		}
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

//	G_SetClientSound (ent);
}


