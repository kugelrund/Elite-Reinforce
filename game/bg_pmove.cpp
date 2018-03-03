// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

// define GAME_INCLUDE so that g_public.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "q_shared.h"
#include "g_shared.h"
#include "bg_local.h"			   
#include "g_local.h"			   
#include "anims.h"
#include "../cgame/cg_local.h"	// yeah I know this is naughty, but we're shipping soon...
#include "../speedrun/overbounce_prediction/OverbouncePrediction.hpp"
#include "../speedrun/strafe_helper/StrafeHelper.hpp"

extern	qboolean	Q3_TaskIDPending( gentity_t *ent, taskID_t taskType );
extern void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
extern void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
extern	cvar_t			*g_timescale;
static qboolean MatrixMode;
qboolean	Flying;
pmove_t		*pm;
pml_t		pml;

// movement parameters
const float	pm_stopspeed = 100.0f;
const float	pm_duckScale = 0.50f;
const float	pm_swimScale = 0.50f;
float	pm_ladderScale = 0.7f;

const float	pm_accelerate = 12.0f;
const float	pm_airaccelerate = 4.0f;
const float	pm_wateraccelerate = 4.0f;
const float	pm_flyaccelerate = 8.0f;

const float	pm_friction = 6.0f;
const float	pm_waterfriction = 1.0f;
const float	pm_flightfriction = 3.0f;

const float	pm_frictionModifier	= 3.0f;	//Used for "careful" mode (when pressing use)
const float pm_airDecelRate = 1.35f;	//Used for air decelleration away from current movement velocity

int	c_pmove = 0;

extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
extern void PM_SetAnim(pmove_t	*pm,int setAnimParts,int anim,int setAnimFlags);
extern void PM_TorsoAnimation( void );
extern int PM_TorsoAnimForFrame( gentity_t *ent, int torsoFrame );
extern int PM_AnimLength( int index, animNumber_t anim );
extern qboolean PM_InDeathAnim ( void );
extern qboolean PM_InOnGroundAnim (gentity_t *self);
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern int PM_PickAnim( gentity_t *self, int minAnim, int maxAnim );

#define	PHASER_RECHARGE_TIME	100

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) 
{
	AddEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}




/*
==================
PM_ClipVelocity

Slide off of the impacting surface

  This will pull you down onto slopes if heading away from
  them and push you up them as you go up them.
  Also stops you when you hit walls.

==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) 
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop, friction = pm->ps->friction;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction, even if on ladder
	if ( (pm->watertype & CONTENTS_LADDER) || pm->waterlevel <= 1 ) {
		if ( (pm->watertype & CONTENTS_LADDER) || (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) ) {
			// if getting knocked back, no friction	
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				//If the use key is pressed. slow the player more quickly
				if ( pm->cmd.buttons & BUTTON_USE )
					friction *= pm_frictionModifier;
				
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*friction*pml.frametime;
			}
		}
	}
	// apply water friction even if just wading
	if ( pm->waterlevel && !(pm->watertype & CONTENTS_LADDER))
	{
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}
	
	// apply flying friction
	if ( pm->ps->pm_type == PM_SPECTATOR ) 	
	{
		drop += speed*pm_flightfriction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/

static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) 
{
	int			i;
	float		addspeed, accelspeed, currentspeed;

	if (pm->ps->clientNum == 0) {
		StrafeHelper::setAccelerationValues(pml.forward, pm->ps->velocity, wishdir,
		                                    wishspeed, accel, pml.frametime);
	}

	currentspeed = DotProduct (pm->ps->velocity, wishdir);

	addspeed = wishspeed - currentspeed;
	
	if (addspeed <= 0) {
		return;
	}
	accelspeed = ( accel * pml.frametime ) * wishspeed;

	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}
	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed * wishdir[i];	
	}
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) 
{
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}
	total = sqrt(	( cmd->forwardmove * cmd->forwardmove ) 
				  + ( cmd->rightmove * cmd->rightmove ) 
				  + ( cmd->upmove * cmd->upmove ) );
	
	scale = (float) pm->ps->speed * max / ( 127.0f * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		} 
	}
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
	//Don't allow jump until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;		
	}

	//Not jumping
	if ( pm->cmd.upmove < 10 ) {
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	{
		gentity_t *groundEnt = &g_entities[pm->ps->groundEntityNum];
		if ( groundEnt && groundEnt->NPC )
		{//Can't jump off of someone's head
			return qfalse;
		}
	}

	//Jumping
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = JUMP_VELOCITY;
	if ( pm->gent )
	{
		if ( !Q3_TaskIDPending( pm->gent, TID_CHAN_VOICE ) )
		{
			PM_AddEvent( EV_JUMP );
		}
	}
	else
	{
		PM_AddEvent( EV_JUMP );
	}

	//Set the animations
	if ( pm->cmd.forwardmove >= 0 ) {
		if(!PM_InDeathAnim())
		{
			PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMP1,SETANIM_FLAG_OVERRIDE);
		}
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		if(!PM_InDeathAnim())
		{
			PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMPBACK1,SETANIM_FLAG_OVERRIDE);
		}
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	if ( pm->watertype & CONTENTS_LADDER ) {
		if (pm->ps->velocity[2] <= 0)
			return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize( flatforward );

	VectorMA( pm->ps->origin, 30, flatforward, spot );
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( !(cont & CONTENTS_SOLID) ) {
		return qfalse;
	}
	
	spot[2] += 16;
	cont = pm->pointcontents( spot, pm->ps->clientNum );
	if ( cont ) {
		return qfalse;
	}

	// jump out of water
	VectorScale( pml.forward, 200, pm->ps->velocity );
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) 
{
	// waterjump has no control, but falls

	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if (pm->ps->velocity[2] < 0) 
	{
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	if ( PM_CheckWaterJump() ) 	{
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype == CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if (pm->watertype == CONTENTS_SLIME) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		if ( pm->watertype & CONTENTS_LADDER ) {
			wishvel[2] = 0;
		} else {
			wishvel[2] = -60;		// sink towards bottom
		}
	} else {
		for (i=0 ; i<3 ; i++) {
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}
		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( pm->watertype & CONTENTS_LADDER )	//ladder
	{
		if ( wishspeed > pm->ps->speed * pm_ladderScale ) {
			wishspeed = pm->ps->speed * pm_ladderScale;
		}
		PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );
	} else {
		if ( wishspeed > pm->ps->speed * pm_swimScale ) {
			wishspeed = pm->ps->speed * pm_swimScale;
		}
		PM_Accelerate( wishdir, wishspeed, pm_wateraccelerate );
	}

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove( qfalse );
}



/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) 
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) 
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} 
	else
	{
		for (i=0 ; i<3 ; i++) 
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );

	PM_Accelerate( wishdir, wishspeed, pm_flyaccelerate );

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	/*
	if ( pm->gent && pm->gent->client && pm->gent->client->playerTeam == TEAM_STASIS )
	{//FIXME: do a check for movetype_float
		//Can move fairly well in air while falling
		PM_Accelerate (wishdir, wishspeed, pm_accelerate/2.0f);
	}
	else
	{
	*/
		if ( ( DotProduct (pm->ps->velocity, wishdir) ) < 0.0f )
		{//Encourage deceleration away from the current velocity
			wishspeed *= pm_airDecelRate;
		}

		// not on ground, so little effect on velocity
		PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);
	//}

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) {
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );
	}

	PM_StepSlideMove ( qtrue );
}


/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}


	if ( PM_CheckJump () ) {
		// jumped away
		if ( pm->waterlevel > 1 ) {
			PM_WaterMove();
		} else {
			PM_AirMove();
		}
		return;
	}

	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		if ( wishspeed > pm->ps->speed * pm_duckScale ) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove( qfalse );

	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	if(pm->gent && pm->gent->client)
	{
		pm->ps->viewheight = pm->gent->client->standheight + STANDARD_VIEWHEIGHT_OFFSET;
		if ( !pm->gent->mins[0] || !pm->gent->mins[1] || !pm->gent->mins[2] || !pm->gent->maxs[0] || !pm->gent->maxs[1] || !pm->gent->maxs[2] )
		{
			assert(0);
		}

		VectorCopy( pm->gent->mins, pm->mins );
		VectorCopy( pm->gent->maxs, pm->maxs );
	}
	else
	{
		pm->ps->viewheight = DEFAULT_MAXS_2 + STANDARD_VIEWHEIGHT_OFFSET;//DEFAULT_VIEWHEIGHT;

		if ( !DEFAULT_MINS_0 || !DEFAULT_MINS_1 || !DEFAULT_MAXS_0 || !DEFAULT_MAXS_1 || !DEFAULT_MINS_2 || !DEFAULT_MAXS_2 )
		{
			assert(0);
		}

		pm->mins[0] = DEFAULT_MINS_0;
		pm->mins[1] = DEFAULT_MINS_1;
		pm->mins[2] = DEFAULT_MINS_2;

		pm->maxs[0] = DEFAULT_MAXS_0;
		pm->maxs[1] = DEFAULT_MAXS_1;
		pm->maxs[2] = DEFAULT_MAXS_2;
	}

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
		return 0;
	}
	if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS ) {
		return EV_FOOTSTEP_METAL;
	}
	return EV_FOOTSTEP;
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) 
	{
		return;
	}
	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta*delta * 0.0001;

	// ducking while falling doubles damage
/*	if ( pm->ps->pm_flags & PMF_DUCKED ) 
	{
		delta *= 2;
	}
*/
	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) 
	{
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) 
	{
		delta *= 0.25;
	}
	if ( pm->waterlevel == 1 ) 
	{
		delta *= 0.5;
	}

	if ( delta < 1 ) 
	{
		return;
	}

	if(!PM_InDeathAnim())
	{
		if((pm->ps->legsAnim&~ANIM_TOGGLEBIT) == BOTH_FALLDEATH1||(pm->ps->legsAnim&~ANIM_TOGGLEBIT) == BOTH_FALLDEATH1INAIR)
		{//FIXME: add a little bounce?
			//FIXME: cut voice channel?
			pm->ps->gravity = 1.0;
			PM_AddEvent( EV_FALL_FAR );
			int old_pm_type = pm->ps->pm_type;
			pm->ps->pm_type = PM_NORMAL;
			//Hack because for some reason PM_SetAnim just returns if you're dead...???
			PM_SetAnim(pm, SETANIM_BOTH, BOTH_FALLDEATH1LAND, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			pm->ps->pm_type = old_pm_type;
			return;
		}

		if ( delta > 20 || pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) //EV_FALL_SHORT or jumping back
		{// decide which landing animation to use
			if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) 
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_LANDBACK1,SETANIM_FLAG_OVERRIDE);
			} 
			else 
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_LAND1,SETANIM_FLAG_OVERRIDE);
			}
		}
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  
	{
		if ( delta > 50 ) 
		{
			PM_AddEvent( EV_FALL_FAR );//damage is dealt in g_active, ClientEvents
			if ( pm->gent && pm->gent->s.number == 0 ) 
			{
				vec3_t	bottom;

				VectorCopy( pm->ps->origin, bottom );
				bottom[2] += pm->mins[2];
				if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
				{
					AddSoundEvent( pm->gent, bottom, 256, AEL_SUSPICIOUS );
				}
			}
		} 
		else if ( delta > 25 ) 
		{
			// this is a pain grunt, so don't play it if dead
			if ( pm->ps->stats[STAT_HEALTH] > 0 ) 
			{
				PM_AddEvent( EV_FALL_MEDIUM );//damage is dealt in g_active, ClientEvents
				if ( pm->gent && pm->gent->s.number == 0 ) 
				{
					vec3_t	bottom;

					VectorCopy( pm->ps->origin, bottom );
					bottom[2] += pm->mins[2];
					if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
					{
						AddSoundEvent( pm->gent, bottom, 256, AEL_MINOR );
					}
				}
			}
		} 
		else if ( delta > 7 ) 
		{
			PM_AddEvent( EV_FALL_SHORT );
			if ( pm->gent && pm->gent->s.number == 0 ) 
			{
				vec3_t	bottom;

				VectorCopy( pm->ps->origin, bottom );
				bottom[2] += pm->mins[2];
				if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
				{
					AddSoundEvent( pm->gent, bottom, 128, AEL_MINOR );
				}
			}
			if ( pm->gent )
			{
				if ( pm->gent->client && pm->gent->client->race == RACE_REAVER )
				{
					PM_SetAnim( pm,SETANIM_BOTH,BOTH_LAND1,SETANIM_FLAG_OVERRIDE );
				}
			}
		} 
		else 
		{
			PM_AddEvent( PM_FootstepForSurface() );
		}
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}



/*
=============
PM_CorrectAllSolid
=============
*/
static void PM_CorrectAllSolid( void ) {
	if ( pm->debugLevel ) {
		Com_Printf("%i:allsolid\n", c_pmove);	//NOTENOTE: If this ever happens, I'd really like to see this print!
	}

	// FIXME: jitter around

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}

/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
	trace_t		trace;
	vec3_t		point;

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) 
	{
		// we just transitioned into freefall
		if ( pm->debugLevel ) 
		{
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0 ) 
		{//FIXME: if velocity[2] < 0 and didn't jump, use some falling anim
			if ( pm->cmd.forwardmove >= 0 ) 
			{
				if(!PM_InDeathAnim())
				{
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMP1,SETANIM_FLAG_OVERRIDE);
				}
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} 
			else 
			{
				if(!PM_InDeathAnim())
				{
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMPBACK1,SETANIM_FLAG_OVERRIDE);
				}
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t		point;
	trace_t		trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		PM_CorrectAllSolid();
		return;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
		if ( pm->cmd.forwardmove >= 0 ) 
		{
			if(!PM_InDeathAnim())
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMP1,SETANIM_FLAG_OVERRIDE);
			}
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} 
		else 
		{
			if(!PM_InDeathAnim())
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_JUMPBACK1,SETANIM_FLAG_OVERRIDE);
			}
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	
	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf("%i:Land\n", c_pmove);
		}
		
		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
		if (!pm->cmd.forwardmove && !pm->cmd.rightmove) {
			pm->ps->velocity[2] = 0;	//wouldn't normally want this because of slopes, but we aren't tyring to move...
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + DEFAULT_MINS_2 + 1;	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & (MASK_WATER|CONTENTS_LADDER) ) 
	{
		sample2 = pm->ps->viewheight - DEFAULT_MINS_2;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + DEFAULT_MINS_2 + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & (MASK_WATER|CONTENTS_LADDER) ) 
		{
			if ( Q_stricmp( "biohulk", pm->gent->NPC_type ) != 0 )
			{//special hack for biohulk- they're never waist-high in water
				pm->waterlevel = 2;
			}
			point[2] = pm->ps->origin[2] + DEFAULT_MINS_2 + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & (MASK_WATER|CONTENTS_LADDER) )
			{
				pm->waterlevel = 3;
			}
		}
	}

}



/*
==============
PM_SetBounds

Sets mins, maxs
==============
*/
static void PM_SetBounds (void)
{
	if ( pm->gent && pm->gent->client )
	{
		if ( !pm->gent->mins[0] || !pm->gent->mins[1] || !pm->gent->mins[2] || !pm->gent->maxs[0] || !pm->gent->maxs[1] || !pm->gent->maxs[2] )
		{
			//assert(0);
		}

		VectorCopy( pm->gent->mins, pm->mins );
		VectorCopy( pm->gent->maxs, pm->maxs );
	}
	else
	{
		if ( !DEFAULT_MINS_0 || !DEFAULT_MINS_1 || !DEFAULT_MAXS_0 || !DEFAULT_MAXS_1 || !DEFAULT_MINS_2 || !DEFAULT_MAXS_2 )
		{
			assert(0);
		}

		pm->mins[0] = DEFAULT_MINS_0;
		pm->mins[1] = DEFAULT_MINS_1;

		pm->maxs[0] = DEFAULT_MAXS_0;
		pm->maxs[1] = DEFAULT_MAXS_1;

		pm->mins[2] = DEFAULT_MINS_2;
		pm->maxs[2] = DEFAULT_MAXS_2;
	}
}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck (void)
{
	trace_t	trace;
	int		standheight;
	int		crouchheight;
	int		oldHeight;

	if ( pm->gent && pm->gent->client )
	{
		if ( !pm->gent->mins[0] || !pm->gent->mins[1] || !pm->gent->mins[2] || !pm->gent->maxs[0] || !pm->gent->maxs[1] || !pm->gent->maxs[2] )
		{
			//assert(0);
		}

		standheight = pm->gent->client->standheight;
		crouchheight = pm->gent->client->crouchheight;
	}
	else
	{
		if ( !DEFAULT_MINS_0 || !DEFAULT_MINS_1 || !DEFAULT_MAXS_0 || !DEFAULT_MAXS_1 || !DEFAULT_MINS_2 || !DEFAULT_MAXS_2 )
		{
			assert(0);
		}

		standheight = DEFAULT_MAXS_2;
		crouchheight = CROUCH_MAXS_2;
	}

	oldHeight = pm->maxs[2];

	if ( pm->cmd.upmove < 0 )
	{	// trying to duck
		pm->maxs[2] = crouchheight;
		pm->ps->viewheight = crouchheight + STANDARD_VIEWHEIGHT_OFFSET;//CROUCH_VIEWHEIGHT;
		if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
		{//Not ducked already and trying to duck in mid-air
			//will raise your feet, unducking whilst in air will drop feet
			if ( !(pm->ps->pm_flags&PMF_DUCKED) )
			{
				pm->ps->eFlags ^= EF_TELEPORT_BIT;
			}
			pm->ps->origin[2] += oldHeight - pm->maxs[2];//diff will be zero if were already ducking
			//Don't worry, we know we fit in a smaller size
		}
		pm->ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	// want to stop ducking, stand up if possible
		if ( pm->ps->pm_flags & PMF_DUCKED )
		{//Was ducking
			if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
			{//unducking whilst in air will try to drop feet
				pm->maxs[2] = standheight;
				pm->ps->origin[2] += oldHeight - pm->maxs[2];
				pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid )
				{
					pm->ps->eFlags ^= EF_TELEPORT_BIT;
					pm->ps->pm_flags &= ~PMF_DUCKED;
				}
				else
				{//Put us back
					pm->ps->origin[2] -= oldHeight - pm->maxs[2];
				}
				//NOTE: this isn't the best way to check this, you may have room to unduck
				//while in air, but your feet are close to landing.  Probably won't be a
				//noticable shortcoming
			}
			else
			{
				// try to stand up
				pm->maxs[2] = standheight;
				pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid )
				{
					pm->ps->pm_flags &= ~PMF_DUCKED;
				}
			}
		}

		if ( pm->ps->pm_flags & PMF_DUCKED )
		{//Still ducking
			pm->maxs[2] = crouchheight;
			pm->ps->viewheight = crouchheight + STANDARD_VIEWHEIGHT_OFFSET;//CROUCH_VIEWHEIGHT;
		}
		else
		{//standing now
			pm->maxs[2] = standheight;
			//FIXME: have a crouchviewheight and standviewheight on ent?
			pm->ps->viewheight = standheight + STANDARD_VIEWHEIGHT_OFFSET;//DEFAULT_VIEWHEIGHT;
		}
	}
}



//===================================================================

/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) 
{
	float		bobmove;
	int			old, oldAnim;
	qboolean	footstep = qfalse;
	qboolean	careful = qfalse;
	qboolean	validNPC = qfalse;

	if( pm->gent == NULL || pm->gent->client == NULL )
		return;

	if( pm->gent->NPC != NULL )
	{
		validNPC = qtrue;

		if( pm->cmd.buttons & BUTTON_CAREFUL )
		{
			careful = qtrue;
		}
	}
	pm->gent->client->renderInfo.legsFpsMod = 1.0f;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) 
	{
		// airborne leaves position in cycle intact, but doesn't advance
		if ( pm->waterlevel > 0 ) 
		{
			if ( pm->watertype & CONTENTS_LADDER ) 
			{//FIXME: check for watertype, save waterlevel for whether to play
				//the get off ladder transition anim!
				if ( pm->ps->velocity[2]  ) 
				{//going up or down it
					int	anim;
					if ( pm->ps->velocity[2] > 0 )
					{
						anim = BOTH_LADDER_UP1;
					}
					else
					{
						anim = BOTH_LADDER_DWN1;
					}
					PM_SetAnim( pm, SETANIM_LEGS, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
					if ( pm->waterlevel >= 2 )	//arms on ladder
					{
						PM_SetAnim( pm, SETANIM_TORSO, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
					}
					if (fabs(pm->ps->velocity[2]) >5) {
						bobmove = 0.005 * fabs(pm->ps->velocity[2]);	// climbing bobs slow
						if (bobmove > 0.3)
							bobmove = 0.3F;
						goto DoFootSteps;
					}
				}
				else
				{
					PM_SetAnim( pm, SETANIM_LEGS, BOTH_LADDER_IDLE, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_RESTART );
					pm->ps->legsAnimTimer += 300;
					if ( pm->waterlevel >= 2 )	//arms on ladder
					{
						PM_SetAnim( pm, SETANIM_TORSO, BOTH_LADDER_IDLE, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_RESTART );
						pm->ps->torsoAnimTimer += 300;
					}
				}
				return;
			}
			else if ( pm->waterlevel > 1 )	//off ground and in deep water
			{
//				PM_SetAnim(pm,SETANIM_LEGS,BOTH_SWIM1,SETANIM_FLAG_NORMAL);
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_FLOAT2,SETANIM_FLAG_NORMAL);
				return;
			}
		}
		else 
		{
			if ( pm->ps->pm_flags & PMF_DUCKED ) 
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_CROUCH1IDLE,SETANIM_FLAG_NORMAL);
			} 
			return;
		}
	}

	// if not trying to move
	if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) 
	{
		if (  pm->xyspeed < 5 ) 
		{
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm->ps->pm_flags & PMF_DUCKED ) 
			{
				if(!pm->gent || !PM_InOnGroundAnim(pm->gent))
				{
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_CROUCH1IDLE,SETANIM_FLAG_NORMAL);
				}
			} 
			else 
			{
				if ( pm->ps->weapon == WP_IMPERIAL_BLADE || pm->ps->weapon == WP_KLINGON_BLADE )
				{
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND3,SETANIM_FLAG_NORMAL);
				}
				else if( careful || (validNPC && pm->ps->weapon > WP_PHASER && pm->ps->weapon < WP_TRICORDER && pm->gent->client->race != RACE_BORG))//Being careful or carrying a 2-handed weapon
				{//Squadmates use BOTH_STAND2
					oldAnim = pm->ps->legsAnim&~ANIM_TOGGLEBIT;
					if(oldAnim != BOTH_GUARD_LOOKAROUND1 && oldAnim != BOTH_GUARD_IDLE1 && 
						oldAnim != BOTH_STAND2_RANDOM1 && oldAnim != BOTH_STAND2_RANDOM2 && oldAnim != BOTH_STAND2_RANDOM3 && oldAnim != BOTH_STAND2_RANDOM4 && 
						oldAnim != BOTH_STAND2TO4 && oldAnim != BOTH_STAND4TO2 && oldAnim != BOTH_STAND4 )
					{//Don't auto-override the guard idles
						PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND2,SETANIM_FLAG_NORMAL);
						//if(oldAnim != BOTH_STAND2 && (pm->ps->legsAnim&~ANIM_TOGGLEBIT) == BOTH_STAND2)
						//{
						//	pm->ps->legsAnimTimer = 500;
						//}
					}
				}
				/*
				else if(pm->gent && pm->gent->client->race == RACE_BORG && pm->gent->enemy)
				{//Angry borg
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND2,SETANIM_FLAG_NORMAL);
				}
				*/
				else
				{
					PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				}
			}
		}
		else
		{//not trying to move, but I am, must be sliding?  Default to standing?
			PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND1,SETANIM_FLAG_NORMAL);
		}
		return;
	}
	

	if ( pm->ps->pm_flags & PMF_DUCKED ) 
	{
		bobmove = 0.5;	// ducked characters bob much faster
		if(!pm->gent || !PM_InOnGroundAnim(pm->gent))
		{
			PM_SetAnim(pm,SETANIM_LEGS,BOTH_CROUCH1WALK,SETANIM_FLAG_NORMAL);
		}
		// ducked characters never play footsteps
	} 
	else if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) 
	{//Moving backwards
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) 
		{//running backwards
			bobmove = 0.4F;	// faster speeds bob faster
			if( careful )
			{//Squadmates use LEGS_RUNBACK2
				oldAnim = pm->ps->legsAnim&~ANIM_TOGGLEBIT;
				PM_SetAnim(pm,SETANIM_LEGS,LEGS_RUNBACK2,SETANIM_FLAG_NORMAL);
				//Now figure out an FPS bias
				if(pm->gent->client->ps.speed)
				{
					pm->gent->client->renderInfo.legsFpsMod = (float)pm->gent->NPC->stats.runSpeed/(float)pm->gent->client->ps.speed;
				}
				if(oldAnim != LEGS_RUNBACK2 && (pm->ps->legsAnim&~ANIM_TOGGLEBIT) == LEGS_RUNBACK2)
				{
					pm->ps->legsAnimTimer = 500;
				}
			}
			else
			{
				PM_SetAnim(pm,SETANIM_LEGS,LEGS_RUNBACK1,SETANIM_FLAG_NORMAL);
			}
			footstep = qtrue;
		} 
		else 
		{//walking backwards
			bobmove = 0.3F;	// faster speeds bob faster
			if( careful )
			{//Squadmates use BOTH_WALK2
				PM_SetAnim(pm,SETANIM_LEGS,LEGS_WALKBACK2,SETANIM_FLAG_NORMAL);
			}
			else
			{
				PM_SetAnim(pm,SETANIM_LEGS,LEGS_WALKBACK1,SETANIM_FLAG_NORMAL);
			}
//			footstep = qtrue;
		}
	}
	else 
	{
		if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) 
		{
			bobmove = 0.4F;	// faster speeds bob faster
			if( careful )
			{//Squadmates use BOTH_RUN2
				oldAnim = pm->ps->legsAnim&~ANIM_TOGGLEBIT;
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_RUN2,SETANIM_FLAG_NORMAL);	//FIXME: Until we get a better running animation
				//Now figure out an FPS bias
				if(pm->gent->client->ps.speed)
				{
					pm->gent->client->renderInfo.legsFpsMod = (float)pm->gent->NPC->stats.runSpeed/(float)pm->gent->client->ps.speed;
				}
				if(oldAnim != BOTH_RUN2 && (pm->ps->legsAnim&~ANIM_TOGGLEBIT) == BOTH_RUN2)
				{
					pm->ps->legsAnimTimer = 500;
				}
			}
			else
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_RUN1,SETANIM_FLAG_NORMAL);
			}
			footstep = qtrue;
		} 
		else 
		{
			bobmove = 0.3F;	// walking bobs slow
			if( careful || (validNPC && pm->ps->weapon > WP_PHASER && pm->ps->weapon < WP_TRICORDER && pm->gent->client->race != RACE_BORG))//Being careful or carrying a 2-handed weapon
			{//Squadmates use BOTH_WALK2
				oldAnim = pm->ps->legsAnim&~ANIM_TOGGLEBIT;
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_WALK2,SETANIM_FLAG_NORMAL);
				if(pm->gent->client->ps.speed)
				{
					pm->gent->client->renderInfo.legsFpsMod = (float)pm->gent->NPC->stats.walkSpeed/(float)pm->gent->client->ps.speed;
				}
				if(oldAnim != BOTH_WALK2 && (pm->ps->legsAnim&~ANIM_TOGGLEBIT) == BOTH_WALK2)
				{
					pm->ps->legsAnimTimer = 500;
				}
			}
			/*
			else if(pm->gent && pm->gent->client->race == RACE_BORG && pm->gent->enemy)
			{//Angry borg
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_WALK2,SETANIM_FLAG_NORMAL);
			}
			*/
			else
			{
				PM_SetAnim(pm,SETANIM_LEGS,BOTH_WALK1,SETANIM_FLAG_NORMAL);
			}
			
			//Enemy NPCs always make footsteps for the benefit of the player
			if ( pm->gent && pm->gent->NPC && pm->gent->client && pm->gent->client->playerTeam != TEAM_STARFLEET )
			{
				footstep = qtrue;
			}
		}
	}

	if(pm->gent != NULL)
	{
		if( pm->gent->client->renderInfo.legsFpsMod > 2 )
		{
			pm->gent->client->renderInfo.legsFpsMod = 2;
		}
		else if(pm->gent->client->renderInfo.legsFpsMod < 0.5)
		{
			pm->gent->client->renderInfo.legsFpsMod = 0.5;
		}
	}

	//Stasis have no feet, hence, no footsteps
	if ( pm->gent->client->playerTeam == TEAM_STASIS )
		return;

DoFootSteps:

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) 
	{
		if ( pm->watertype & CONTENTS_LADDER ) 
		{
			if ( !pm->noFootsteps ) 
			{
				if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {// on ladder
					 PM_AddEvent( EV_FOOTSTEP_METAL );
				} else {
					PM_AddEvent( PM_FootstepForSurface() );	//still on ground
				}
				if ( pm->gent && pm->gent->s.number == 0 ) 
				{
					if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
					{
						AddSoundEvent( pm->gent, pm->ps->origin, 128, AEL_MINOR );
					}
				}
			}
		}
		else if ( pm->waterlevel == 0 ) 
		{
			// on ground will only play sounds if running
			if ( footstep && !pm->noFootsteps ) 
			{
				PM_AddEvent( PM_FootstepForSurface() );
				if ( pm->gent && pm->gent->s.number == 0 ) 
				{
					vec3_t	bottom;

					VectorCopy( pm->ps->origin, bottom );
					bottom[2] += pm->mins[2];
					if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
					{
						AddSoundEvent( pm->gent, bottom, 256, AEL_MINOR );
					}
				}
			}
		} 
		else if ( pm->waterlevel == 1 ) 
		{
			// splashing
			PM_AddEvent( EV_FOOTSPLASH );
			if ( pm->gent && pm->gent->s.number == 0 ) 
			{
				vec3_t	bottom;

				VectorCopy( pm->ps->origin, bottom );
				bottom[2] += pm->mins[2];
				if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
				{
					AddSoundEvent( pm->gent, bottom, 256, AEL_MINOR );
				}
			}
		} 
		else if ( pm->waterlevel == 2 ) 
		{
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
			if ( pm->gent && pm->gent->s.number == 0 ) 
			{
				if ( pm->gent->client && pm->gent->client->playerTeam != TEAM_DISGUISE )
				{
					AddSoundEvent( pm->gent, pm->ps->origin, 128, AEL_MINOR );
				}
			}
		} // no sound when completely underwater
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
	
	if ( pm->watertype & CONTENTS_LADDER )	//fake water for ladder
	{
		return;
	}
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent( EV_WATER_LEAVE );
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}


qboolean G_WeaponChangeEffect ( gentity_t *ent )
{
	if ( !ent || !ent->client )
	{
		return qfalse;
	}

	int torsoAnim = PM_TorsoAnimForFrame( ent, ent->client->renderInfo.torsoFrame );

	if ( torsoAnim == TORSO_DROPWEAP1 || torsoAnim == TORSO_DROPWEAP2 || torsoAnim == TORSO_DROPWEAP3 ||
		torsoAnim == TORSO_RAISEWEAP1 || torsoAnim == TORSO_RAISEWEAP2 || torsoAnim == TORSO_RAISEWEAP3 )
	{//are getting the weapon from our transporter buffer
		if ( ent->s.number == 0 || strstr( ent->client->renderInfo.torsoModelName , "hazard" ) != NULL ||
			   Q_stricmp( "alexascav", ent->NPC_type ) == 0 ||
			   Q_stricmp( "munroscav", ent->NPC_type ) == 0 )
		{//UGH... hacky way of knowing if this is a hazard guy
			//Do weapon select transporter effect
			ent->fx_time = level.time;
			ent->s.powerups |= ( 1 << PW_DISINT_6 );
			ent->client->ps.powerups[PW_DISINT_6] = level.time + 600;
			return qtrue;
		}
	}

	return qfalse;
}
/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		return;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		return;
	}
	
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		return;
	}

	if ( cg.time > 0 )
	{//this way we don't get that annoying change weapon sound every time a map starts
		PM_AddEvent( EV_CHANGE_WEAPON );
	}
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += 200;
	PM_SetAnim(pm,SETANIM_TORSO,TORSO_DROPWEAP1,SETANIM_FLAG_NORMAL);

	if ( pm->gent && pm->gent->client )
	{//predicting weapon switch effect
		pm->gent->fx_time = level.time;
		pm->gent->s.powerups |= ( 1 << PW_DISINT_6 );
		pm->gent->client->ps.powerups[PW_DISINT_6] = level.time + PM_AnimLength( pm->gent->client->clientInfo.animFileIndex, TORSO_DROPWEAP1) + PM_AnimLength( pm->gent->client->clientInfo.animFileIndex, TORSO_RAISEWEAP1);
	}
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
	int		weapon;

	weapon = pm->cmd.weapon;
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
		weapon = WP_NONE;
	}

	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 250;
	PM_SetAnim(pm,SETANIM_TORSO,TORSO_RAISEWEAP1,SETANIM_FLAG_NORMAL);
}




/*
==============
PM_Use

Generates a use event
==============
*/
#define USE_DELAY 2000

void PM_Use( void ) 
{
	if ( pm->ps->useTime > 0 )
		pm->ps->useTime -= 100;//pm->cmd.msec;

	if ( pm->ps->useTime > 0 ) {
		return;
	}

	if ( ! (pm->cmd.buttons & BUTTON_USE ) )
	{
		pm->useEvent = 0;
		pm->ps->useTime = 0;
		return;
	}

	pm->useEvent = EV_USE;
	pm->ps->useTime = USE_DELAY;
}

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) 
{
	int			addTime,amount;
	qboolean	delayed_fire = qfalse;
	weaponInfo_t	*weapon;

	if(pm->gent && pm->gent->client && pm->gent->client->fireDelay > 0)
	{//FIXME: this is going to fire off one frame before you expect, actually
		pm->gent->client->fireDelay -= pml.msec;
		if(pm->gent->client->fireDelay <= 0)
		{//just finished delay timer
			pm->gent->client->fireDelay = 0;
			delayed_fire = qtrue;
		}
	}

   // don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}

	//Check for phaser ammo recharge
	if ( pm->ps->rechargeTime <= 0 ) {
		if ( pm->ps->ammo[weaponData[WP_PHASER].ammoIndex] < ammoData[AMMO_PHASER].max) {
			pm->ps->rechargeTime = PHASER_RECHARGE_TIME;
			pm->ps->ammo[weaponData[WP_PHASER].ammoIndex]++;
		}
	} else {
		pm->ps->rechargeTime -= pml.msec;
	}

	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 )
	{
		if ( pm->gent && pm->gent->client )
		{
			// Sigh..borg shouldn't drop their weapon attachments when they die.
			if ( pm->gent->client->playerTeam != TEAM_BORG )
			{
				pm->ps->weapon = WP_NONE;
			}
		}

		if ( pm->gent )
		{
			pm->gent->s.loopSound = 0;
		}
		return;
	}

	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
	}

	// check for weapon change
	// can't change if weapon is firing, but can change again if lowering or raising
	if ( pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING ) {
		if ( pm->ps->weapon != pm->cmd.weapon ) {
			PM_BeginWeaponChange( pm->cmd.weapon );
		}
	}

	if ( pm->ps->weaponTime > 0 ) 
	{
		return;
	}

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		PM_FinishWeaponChange();
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_RAISING ) 
	{//Just selected the weapon
		pm->ps->weaponstate = WEAPON_IDLE;
		if(pm->gent && pm->gent->s.number == 0)
		{
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
		}
		else
		{
			switch(pm->ps->weapon)
			{
			case WP_PHASER:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE1,SETANIM_FLAG_NORMAL);
				break;
			case WP_COMPRESSION_RIFLE:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE2,SETANIM_FLAG_NORMAL);
				break;
			case WP_BORG_WEAPON:
			case WP_BORG_TASER:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;
			case WP_TRICORDER:
			case WP_RED_HYPO:
			case WP_BLUE_HYPO:
			case WP_VOYAGER_HYPO:
				PM_SetAnim(pm,SETANIM_TORSO,BOTH_STAND1,SETANIM_FLAG_NORMAL);
				break;
			default:
				PM_SetAnim(pm,SETANIM_TORSO,TORSO_WEAPONIDLE3,SETANIM_FLAG_NORMAL);
				break;
			}
		}
		return;
	}

	weapon = &cg_weapons[pm->ps->weapon];

	if(!delayed_fire)
	{
		// check for fire
		if ( !(pm->cmd.buttons & (BUTTON_ATTACK|BUTTON_ALT_ATTACK)) )
		{
			pm->ps->weaponTime = 0;
			
			if ( pm->gent && pm->gent->client && pm->gent->client->fireDelay > 0 )
			{//Still firing
				pm->ps->weaponstate = WEAPON_FIRING;
			}
			else if ( pm->ps->weaponstate != WEAPON_READY )
			{
				pm->ps->weaponstate = WEAPON_IDLE;
			}
			
			return;
		}

		// start the animation even if out of ammo
		switch(pm->ps->weapon)
		{
		case WP_PHASER://1 - handed
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;
		
		case WP_COMPRESSION_RIFLE://2-handed
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;
		
		case WP_BORG_WEAPON:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;
		
		case WP_BORG_TASER:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK3,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;
		
		case WP_BORG_ASSIMILATOR:
		case WP_BORG_DRILL:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;
		
		case WP_FORGE_PSYCH:			
			PM_SetAnim(pm,SETANIM_BOTH,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;

		case WP_FORGE_PROJ:
			if ( rand() & 1 )
				PM_SetAnim(pm,SETANIM_BOTH,BOTH_ATTACK2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			else
				PM_SetAnim(pm,SETANIM_BOTH,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;

		case WP_MELEE:

			// The slash trail "formation" hasn't been started yet, so don't try and use the old points.
			pm->gent->trigger_formation = qfalse;

			if ( pm->gent->client->race == RACE_REAVER || pm->gent->client->race == RACE_HARVESTER || pm->gent->client->race == RACE_STASIS || pm->gent->client->race == RACE_BOT)
			{
				if ( rand() & 1 )
					PM_SetAnim(pm,SETANIM_BOTH,BOTH_MELEE1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				else
					PM_SetAnim(pm,SETANIM_BOTH,BOTH_MELEE2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else if ( pm->gent->client->race == RACE_AVATAR ) 
			{
				PM_SetAnim(pm,SETANIM_BOTH,BOTH_ATTACK2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else
			{
				PM_SetAnim(pm,SETANIM_BOTH,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			break;

		case WP_STASIS_ATTACK:
			PM_SetAnim( pm, SETANIM_BOTH,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			break;

		case WP_TETRION_DISRUPTOR:
			PM_SetAnim( pm, SETANIM_BOTH, BOTH_ATTACK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;

		case WP_SCAVENGER_RIFLE:
		case WP_CHAOTICA_GUARD_GUN:
			if ( (pm->ps->torsoAnim&~ANIM_TOGGLEBIT) != BOTH_HEROSTANCE1 )
			{//HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK
				PM_SetAnim( pm, SETANIM_TORSO, BOTH_ATTACK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			break;

		case WP_PARASITE:
			PM_SetAnim( pm, SETANIM_BOTH, BOTH_ATTACK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;

		case WP_KLINGON_BLADE:
		case WP_IMPERIAL_BLADE:
			{
				int anim = -1;
				int	parts = SETANIM_TORSO;
				anim = PM_PickAnim( pm->gent, BOTH_MELEE1, BOTH_MELEE3 );
				if ( anim == -1 )
				{
					anim = BOTH_ATTACK1;
				}
				if ( !pm->cmd.forwardmove && !pm->cmd.rightmove && pm->cmd.upmove >= 0 )
				{
					parts = SETANIM_BOTH;
				}
				PM_SetAnim(pm,parts,anim,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			}
			break;

		case WP_DESPERADO:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK2,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;

		case WP_PALADIN:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK3,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;

		case WP_BLUE_HYPO:
		case WP_RED_HYPO:
		case WP_VOYAGER_HYPO:
			PM_SetAnim(pm,SETANIM_TORSO,TORSO_HYPOSPRAY1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;

		case WP_TRICORDER:
			PM_SetAnim(pm,SETANIM_TORSO,TORSO_MEDICORDER1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			break;

		case WP_BOT_WELDER:
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;

		default://2-handed heavy weapon
			PM_SetAnim(pm,SETANIM_TORSO,BOTH_ATTACK3,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD);
			break;
		}
	}

	pm->ps->weaponstate = WEAPON_FIRING;

	if ( pm->cmd.buttons & BUTTON_ALT_ATTACK )
		amount = weaponData[pm->ps->weapon].altEnergyPerShot;
	else
		amount = weaponData[pm->ps->weapon].energyPerShot;

	// take an ammo away if not infinite
	if ( pm->ps->ammo[ weaponData[pm->ps->weapon].ammoIndex ] != -1 )
	{
		// enough energy to fire this weapon?
		if ((pm->ps->ammo[weaponData[pm->ps->weapon].ammoIndex] - amount) >= 0) 
		{
			pm->ps->ammo[weaponData[pm->ps->weapon].ammoIndex] -= amount;
		}
		else	// Not enough energy
		{
			if ( pm->ps->weapon == WP_PHASER  )
			{	// force our ammo to be zero
				pm->ps->ammo[weaponData[pm->ps->weapon].ammoIndex] = 0;
			}
			else
			{	// Switch weapons
				PM_AddEvent( EV_NOAMMO );
				pm->ps->weaponTime += 500;
				return;
			}
		}
	}
	if ( pm->gent && pm->gent->client && pm->gent->client->fireDelay > 0 )
	{//FIXME: this is going to fire off one frame before you expect, actually
		// Clear these out since we're not actually firing yet
		pm->ps->eFlags &= ~EF_FIRING;
		pm->ps->eFlags &= ~EF_ALT_FIRING;
		return;
	}

	if ( pm->cmd.buttons & BUTTON_ALT_ATTACK ) 	{
		PM_AddEvent( EV_ALT_FIRE );
		addTime = weaponData[pm->ps->weapon].altFireTime;
	}
	else {
		PM_AddEvent( EV_FIRE_WEAPON );
		addTime = weaponData[pm->ps->weapon].fireTime;
	}

	//If the phaser has been fired, delay the next recharge time
	if ( pm->ps->weapon == WP_PHASER ) {
		pm->ps->rechargeTime = PHASER_RECHARGE_TIME;
	}	
	
	if(pm->gent && pm->gent->NPC != NULL )
	{//NPCs have their own refire logic
		return;
	}

	if( MatrixMode )
	{//Special test for Matrix Mode (tm)
		//player always fires at normal speed
		addTime *= g_timescale->value;
	}

	pm->ps->weaponTime += addTime;
}



/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) 
{
	// drop misc timing counter
	if ( pm->ps->pm_time ) 
	{
		if ( pml.msec >= pm->ps->pm_time ) 
		{
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} 
		else 
		{
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop legs animation counter
	if ( pm->ps->legsAnimTimer > 0 )
	{
		int newTime = pm->ps->legsAnimTimer - pml.msec;

		if ( newTime < 0 )
		{
			newTime = 0;
		}

		PM_SetLegsAnimTimer( pm->gent, &pm->ps->legsAnimTimer, newTime );
	}

	// drop torso animation counter
	if ( pm->ps->torsoAnimTimer > 0 )
	{
		int newTime = pm->ps->torsoAnimTimer - pml.msec;

		if ( newTime < 0 )
		{
			newTime = 0;
		}

		PM_SetTorsoAnimTimer( pm->gent, &pm->ps->torsoAnimTimer, newTime );
	}
}

void PM_SetSpecialMoveValues (void )
{
	Flying = qfalse;
	if ( pm->gent )
	{
		if ( pm->gent->NPC )
		{
			if ( pm->gent->NPC->stats.moveType == MT_FLYSWIM )
			{
				Flying = qtrue;
			}
		}
	}

	MatrixMode = qfalse;
	if ( g_timescale != NULL )
	{
		if ( g_timescale->value < 1.0f )
		{
			if ( pm->ps->clientNum == 0 )
			{
				MatrixMode = qtrue;
			}
		}
	}

	if( MatrixMode )
	{
		float upScale = 1.0f/g_timescale->value;
		//player always moves at full speed of 20fps
		pml.msec *= upScale;
		if ( pm->cmd.forwardmove * upScale > 127 )
		{
			pm->cmd.forwardmove = 127;
		}
		else if ( pm->cmd.forwardmove * upScale < -127 )
		{
			pm->cmd.forwardmove = -127;
		}
		else
		{
			pm->cmd.forwardmove = (int)(pm->cmd.forwardmove * upScale);
		}

		if ( pm->cmd.rightmove * upScale > 127 )
		{
			pm->cmd.rightmove = 127;
		}
		else if ( pm->cmd.rightmove * upScale < -127 )
		{
			pm->cmd.rightmove = -127;
		}
		else
		{
			pm->cmd.rightmove = (int)(pm->cmd.rightmove * upScale);
		}
	}
}
/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove( pmove_t *pmove )
{
	pm = pmove;
	int amount;

	// this counter lets us debug movement problems with a journal by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	// Clear the blocked flag
	pm->ps->pm_flags &= ~PMF_BLOCKED;
	pm->ps->pm_flags &= ~PMF_BUMPED;

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) 
		pm->ps->eFlags |= EF_TALK;
	else 
		pm->ps->eFlags &= ~EF_TALK;

	if ( pm->cmd.buttons & BUTTON_ALT_ATTACK )
		amount = pm->ps->ammo[weaponData[ pm->ps->weapon ].ammoIndex] - weaponData[pm->ps->weapon].altEnergyPerShot;
	else
		amount = pm->ps->ammo[weaponData[ pm->ps->weapon ].ammoIndex] - weaponData[pm->ps->weapon].energyPerShot;

	// set the firing flag for continuous beam weapons, phaser will fire even if out of ammo
	if ( (( pm->cmd.buttons & BUTTON_ATTACK || pm->cmd.buttons & BUTTON_ALT_ATTACK ) && ( amount >= 0 || pm->ps->weapon == WP_PHASER )) ) 
	{
		if ( pm->cmd.buttons & BUTTON_ALT_ATTACK )
		{
			pm->ps->eFlags |= EF_ALT_FIRING;
		}
		else
		{
			pm->ps->eFlags &= ~EF_ALT_FIRING;
		}

		// This flag should always get set, even when alt-firing
		pm->ps->eFlags |= EF_FIRING;
	} 
	else 
	{
		int iFlags = pm->ps->eFlags;

		// Clear 'em out
		pm->ps->eFlags &= ~EF_FIRING;
		pm->ps->eFlags &= ~EF_ALT_FIRING;

		// if I don't check the flags before stopping FX then it switches them off too often, which tones down
		//	the stronger FFFX so you can hardly feel them. However, if you only do iton these flags then the 
		//	repeat-fire weapons like tetrion and dreadnought don't switch off quick enough. So...
		//
		if (pm->ps->weapon == WP_TETRION_DISRUPTOR	||
			pm->ps->weapon == WP_DREADNOUGHT		||
			(iFlags & (EF_FIRING|EF_ALT_FIRING))
			)
		{
			cgi_FF_StopAllFX();
		}
	}

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && 
		!( pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) ) ) 
	{
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		pmove->cmd.buttons = 0;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	
	PM_SetSpecialMoveValues();

	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;
	if ( cg_drawOverbounceInfo.integer && pm->ps->clientNum == 0 ) {
		OverbouncePrediction::reportLastFrametime( pml.frametime );
	}

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd, pm->gent);

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 ) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
		if ( pm->ps->viewheight > -12 )
		{//slowly sink view to ground
			pm->ps->viewheight -= 1;
		}
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_CheckDuck ();
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION ) {
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	if ( !(pm->watertype & CONTENTS_LADDER) )
	{//Don't want to remember this for ladders, is only for waterlevel change events (sounds)
		pml.previous_waterlevel = pmove->waterlevel;
	}

	// set mins, maxs, and viewheight
	PM_SetBounds();

	if ( !Flying && !(pm->watertype & CONTENTS_LADDER) && pm->ps->pm_type != PM_DEAD )
	{//NOTE: noclippers shouldn't jump or duck either, no?
		PM_CheckDuck();
	}

	// set groundentity
	PM_GroundTrace();

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	PM_DropTimers();

	if ( Flying ) 
	{
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
	} 
	else if ( pm->ps->pm_flags & PMF_TIME_WATERJUMP ) 
	{
		PM_WaterJumpMove();
	} 
	else if ( pm->waterlevel > 1 ) 
	{
		// swimming or in ladder
		PM_WaterMove();
	} 
	else if ( pml.walking ) 
	{// walking on ground
		vec3_t	oldOrg;

		VectorCopy( pm->ps->origin, oldOrg );

		PM_WalkMove();

		if ( VectorCompare( oldOrg, pm->ps->origin ) )
		{//didn't move, play no legs anim
			pm->cmd.forwardmove = pm->cmd.rightmove = 0;
		}
	} 
	else 
	{
		// airborne
		PM_AirMove();
	}

	//PM_Animate();

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();

	if(pm->gent && pm->gent->s.number == 0)
	{//player only
		// Use
		PM_Use();
	}

	// footstep events / legs animations
	PM_Footsteps();

	// torso animation
	PM_TorsoAnimation();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	SnapVector( pm->ps->velocity );
}
