//Head Bot

#include "b_local.h"
#include "g_nav.h"
#include "g_functions.h"
#include "Q3_Interface.h"

extern void Q3_RemoveEnt( gentity_t *victim );
extern void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );

/*
-------------------------
HeadBot_ApplyFriction
-------------------------
*/

#define VELOCITY_DECAY	0.25

static void HeadBot_ApplyFriction( void )
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( NPC->client->ps.velocity[i] )
		{
			NPC->client->ps.velocity[i] *= VELOCITY_DECAY;

			if ( fabs( NPC->client->ps.velocity[i] ) < 1 )
			{
				NPC->client->ps.velocity[i] = 0;
			}
		}
	}
}

/*
-------------------------
HeadBot_MaintainHeight
-------------------------
*/

static void HeadBot_MaintainHeight( void )
{
	trace_t	trace;

	vec3_t	endPos;

	VectorSet( endPos, NPC->currentOrigin[0], NPC->currentOrigin[1], NPC->currentOrigin[2] - 1024 );
	gi.trace( &trace, NPC->currentOrigin, NULL, NULL, endPos, NPC->s.number, MASK_SOLID );

	if ( trace.fraction != 1.0f )
	{
		float	length = ( trace.fraction * 1024 );

		if ( length < 64 )
		{
			ucmd.upmove = 32;
		}
		else if ( length > 120 )
		{
			ucmd.upmove = -32;
		}
	}

	HeadBot_ApplyFriction();
}

/*
-------------------------
Headbot_MoveToGoal
-------------------------
*/

#define	MIN_NAVGOAL_DIST	32
#define	MAX_ZMOVE			128
#define	MIN_ZMOVE			-MAX_ZMOVE
#define	MOVE_INCR			25

static void Headbot_MoveToGoal( void ) 
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
Headbot_CalculatePlugSpot
-------------------------
*/

#define	SCOUTBOT_PLUG_X		-2
#define	SCOUTBOT_PLUG_Y		-2
#define	SCOUTBOT_PLUG_Z		-14

#define WARBOT_PLUG_X		0
#define	WARBOT_PLUG_Y		12
#define WARBOT_PLUG_Z		32

static void Headbot_CalculatePlugSpot( gentity_t *target, vec3_t out )
{
	if ( _stricmp( target->NPC_type, "scoutbot" ) == 0 )
	{
		vec3_t	forward, right;

		AngleVectors( target->currentAngles, forward, right, NULL );

		VectorMA( target->currentOrigin, SCOUTBOT_PLUG_Y, forward, out );
		VectorMA( out, SCOUTBOT_PLUG_X, right, out );
		out[2] += SCOUTBOT_PLUG_Z;

		return;
	}

	if ( ( _stricmp( target->NPC_type, "warriorbot" ) == 0 ) || ( _stricmp( target->NPC_type, "warriorbot_boss" ) == 0 ) )
	{
		vec3_t	forward, right;

		AngleVectors( target->currentAngles, forward, right, NULL );

		VectorMA( target->currentOrigin, WARBOT_PLUG_Y, forward, out );
		VectorMA( out, WARBOT_PLUG_X, right, out );
		out[2] += WARBOT_PLUG_Z;

		return;
	}
}

/*
-------------------------
Headbot_Remove
-------------------------
*/

static void Headbot_Remove( void )
{	
	//Remove ourself
	NPC->s.eFlags |= EF_NODRAW;
	NPC->s.eFlags &= ~EF_NPC;
	NPC->svFlags &= ~SVF_NPC;
	NPC->s.eType = ET_INVISIBLE;
	NPC->contents = 0;
	NPC->health = 0;
	NPC->targetname = NULL;

	//Disappear in half a second
	NPC->e_ThinkFunc = thinkF_G_FreeEntity;
	NPC->nextthink = level.time + 50;
}

/*
-------------------------
Headbot_Plugin
-------------------------
*/

#define	MIN_PLUG_DIST		16
#define MIN_PLUG_DIST_SQR	( MIN_PLUG_DIST * MIN_PLUG_DIST )

static void Headbot_Plugin( const char *name )
{
	gentity_t	*target = G_Find( NULL, FOFS(targetname), (char*) name );

	//If we can't find a target, then just go away
	if ( ( target == NULL ) || ( target->health < 0 ) || ( Q_stricmp( name, "!INVALID!" ) == 0 ) )
	{
		Headbot_Remove();
		
		vec3_t	normal = { 0, 0, 1 };
		CG_SurfaceExplosion( NPC->currentOrigin, normal, 16, 0.2f, qfalse );

		return;
	}

	//Must be of a valid type
	if ( ( _stricmp( target->NPC_type, "scoutbot" ) ) && ( _stricmp( target->NPC_type, "warriorbot" ) ) && ( _stricmp( target->NPC_type, "warriorbot_boss" ) ))
		return;

	vec3_t	targetPos;

	//Get our destination point
	Headbot_CalculatePlugSpot( target, targetPos );

	if ( DistanceSquared( targetPos, NPC->currentOrigin ) < MIN_PLUG_DIST_SQR )
	{
		//TODO: Probably going to have to play a special effect here to cover the transition
		//Turn on the target's bolt on
		G_AddBoltOn( target, va("headbot_%s", target->NPC_type) );
		
		//Make it a target to enemies
		target->flags &= ~FL_NOTARGET;

		Headbot_Remove();

		//Callback a completion
		Q3_TaskIDComplete( NPC, TID_MOVE_NAV );

		return;
	}

	//Move there
	NPC_SetMoveGoal( NPC, targetPos, 1, qtrue );
	Headbot_MoveToGoal();

	VectorScale( NPC->client->ps.velocity, 2, NPC->client->ps.velocity );
	//HeadBot_ApplyFriction();

	//Turn to match
	NPCInfo->desiredYaw = target->currentAngles[YAW];

	NPC_UpdateAngles( qtrue, qtrue );
}

/*
-------------------------
NPC_BSHeadBot_Run
-------------------------
*/

void NPC_BSHeadBot_Run( void )
{
	//Plug into our target, if we have one
	if VALIDSTRING( NPC->target )
	{
		Headbot_Plugin( NPC->target );
		return;
	}

	HeadBot_MaintainHeight();

	NPCInfo->goalEntity = UpdateGoal();

	if ( NPCInfo->goalEntity )
	{
		Headbot_MoveToGoal();
	}

	NPC_UpdateAngles( qtrue, qtrue );
}
