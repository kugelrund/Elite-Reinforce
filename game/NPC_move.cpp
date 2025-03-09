//
// NPC_move.cpp
//

#include "b_local.h"
#include "g_nav.h"
#include "anims.h"

void CG_Cylinder( vec3_t start, vec3_t end, float radius, vec3_t color );

qboolean G_BoundsOverlap(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
int NAV_Steer( gentity_t *self, vec3_t dir, float distance );

navInfo_t	frameNavInfo;

/*
-------------------------
NPC_ClearPathToGoal
-------------------------
*/

qboolean NPC_ClearPathToGoal( vec3_t dir, gentity_t *goal )
{
	trace_t	trace;

	//FIXME: What does do about area portals?
	if ( gi.inPVS( NPC->currentOrigin, goal->currentOrigin ) == qfalse )
		return qfalse;

	//Look ahead and see if we're clear to move to our goal position
	if ( NAV_CheckAhead( NPC, goal->currentOrigin, trace, ( NPC->clipmask & ~CONTENTS_BODY ) ) )
	{
		//VectorSubtract( goal->currentOrigin, NPC->currentOrigin, dir );
		return qtrue;
	}
		
	//See if we're too far above
	if ( fabs( NPC->currentOrigin[2] - goal->currentOrigin[2] ) > 48 )
		return qfalse;

	//This is a work around
	float	radius = ( NPC->maxs[0] > NPC->maxs[1] ) ? NPC->maxs[0] : NPC->maxs[1];
	float	dist = Distance( NPC->currentOrigin, goal->currentOrigin );
	float	tFrac = 1.0f - ( radius / dist );

	if ( trace.fraction >= tFrac )
		return qtrue;

	//See if we're looking for a navgoal
	if ( goal->svFlags & SVF_NAVGOAL )
	{
		//Okay, didn't get all the way there, let's see if we got close enough:
		if ( NAV_HitNavGoal( trace.endpos, NPC->mins, NPC->maxs, goal->currentOrigin, NPCInfo->goalRadius ) )
		{
			//VectorSubtract(goal->currentOrigin, NPC->currentOrigin, dir);
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_CheckCombatMove
-------------------------
*/

inline qboolean NPC_CheckCombatMove( void )
{
	return ( ( NPCInfo->goalEntity && NPC->enemy && NPCInfo->goalEntity == NPC->enemy ) || ( NPCInfo->combatMove ) );
}

/*
-------------------------
NPC_LadderMove
-------------------------
*/

static void NPC_LadderMove( vec3_t dir )
{
	//FIXME: this doesn't guarantee we're facing ladder
	//ALSO: Need to be able to get off at top
	//ALSO: Need to play an anim
	//ALSO: Need transitionary anims?
	
	if ( ( dir[2] > 0 ) || ( dir[2] < 0 && NPC->client->ps.groundEntityNum == ENTITYNUM_NONE ) )
	{
		//Set our movement direction
		ucmd.upmove = (dir[2] > 0) ? 127 : -127;

		//Don't move around on XY
		ucmd.forwardmove = ucmd.rightmove = 0;
	}
}

/*
-------------------------
NPC_GetMoveInformation
-------------------------
*/

inline qboolean NPC_GetMoveInformation( vec3_t dir, float *distance )
{
	//NOTENOTE: Use path stacks!

	//Make sure we have somewhere to go
	if ( NPCInfo->goalEntity == NULL )
		return qfalse;

	//Get our move info
	VectorSubtract( NPCInfo->goalEntity->currentOrigin, NPC->currentOrigin, dir );
	*distance = VectorNormalize( dir );
	
	return qtrue;
}

/*
-------------------------
NAV_GetLastMove
-------------------------
*/

void NAV_GetLastMove( navInfo_t &info )
{
	info = frameNavInfo;
}

/*
-------------------------
NPC_GetMoveDirection
-------------------------
*/

qboolean NPC_GetMoveDirection( vec3_t out, float *distance )
{
	vec3_t		angles;

	//Clear the struct
	memset( &frameNavInfo, 0, sizeof( frameNavInfo ) );

	//Get our movement, if any
	if ( NPC_GetMoveInformation( frameNavInfo.direction, &frameNavInfo.distance ) == qfalse )
		return qfalse;

	//Setup the return value
	*distance = frameNavInfo.distance;

	//For starters
	VectorCopy( frameNavInfo.direction, frameNavInfo.pathDirection );

	//If on a ladder, move appropriately
	if ( NPC->watertype & CONTENTS_LADDER )
	{
		NPC_LadderMove( frameNavInfo.direction );
		return qtrue;
	}

	//Attempt a straight move to goal
	if ( NPC_ClearPathToGoal( frameNavInfo.direction, NPCInfo->goalEntity ) == qfalse )
	{
		//See if we're just stuck
		if ( NAV_MoveToGoal( NPC, frameNavInfo ) == WAYPOINT_NONE )
		{
			//Can't reach goal, just face
			vectoangles( frameNavInfo.direction, angles );
			NPCInfo->desiredYaw	= AngleNormalize360( angles[YAW] );		
			VectorCopy( frameNavInfo.direction, out );
			*distance = frameNavInfo.distance;
			return qfalse;
		}

		frameNavInfo.flags |= NIF_MACRO_NAV;
	}

	if ( NPC_ShouldShowPath( NPC ) )
	{
		vec3_t short_term_destination;
		VectorMA( NPC->currentOrigin, frameNavInfo.distance, frameNavInfo.direction, short_term_destination );
		CG_DrawEdge( NPC->currentOrigin, short_term_destination, EDGE_PATH );
		CG_DrawNode( NPCInfo->goalEntity->currentOrigin, NODE_GOAL );
	}

	//Avoid any collisions on the way
	if ( NAV_AvoidCollision( NPC, NPCInfo->goalEntity, frameNavInfo ) == qfalse )
	{
		//FIXME: Emit a warning, this is a worst case scenario
		return qfalse;
	}

	//Setup the return values
	VectorCopy( frameNavInfo.direction, out );
	*distance = frameNavInfo.distance;

	return qtrue;
}

/*
-------------------------
NPC_MoveToGoal

  Now assumes goal is goalEntity, was no reason for it to be otherwise
-------------------------
*/

qboolean NPC_MoveToGoal( void ) 
{
	//If taking full body pain, don't move
	if( ( (NPC->s.legsAnim&~ANIM_TOGGLEBIT) >= BOTH_PAIN1 ) && ( (NPC->s.legsAnim&~ANIM_TOGGLEBIT) <= BOTH_PAIN3 ) )
		return qtrue;

	float	distance;
	vec3_t	dir, angles;

	//Get our movement direction
	if ( NPC_GetMoveDirection( dir, &distance ) == qfalse )
		return qfalse;

	//Convert the move to angles
	vectoangles( dir, angles );
	
	//FIXME: strafe instead of turn if change in dir is small and temporary
	NPCInfo->desiredPitch	= 0.0f;
	NPCInfo->distToGoal		= distance;
	NPCInfo->desiredYaw		= AngleNormalize360( angles[YAW] );
	
	//Pitch towards the goal and also update if flying or swimming
	if ( NPCInfo->stats.moveType == MT_FLYSWIM )
	{
		NPCInfo->desiredPitch = AngleNormalize360( angles[PITCH] );
		
		if ( dir[2] )
		{
			ucmd.upmove = (dir[2] > 0) ? 64 : -64;
		}
	}

	//If in combat move, then move directly towards our goal
	if ( NPC_CheckCombatMove() )
	{
		vec3_t	forward, right;

		AngleVectors( NPC->currentAngles, forward, right, NULL );

		float fDot = DotProduct( forward, dir ) * 127;
		float rDot = DotProduct( right, dir ) * 127;
	
		ucmd.forwardmove = floor(fDot);
		ucmd.rightmove = floor(rDot);

		return qtrue;
	}

	//Set any final info
	ucmd.forwardmove = 127;
	
	return qtrue;
}

/*
-------------------------
void NPC_SlideMoveToGoal( void )

  Now assumes goal is goalEntity, if want to use tempGoal, you set that before calling the func
-------------------------
*/
qboolean NPC_SlideMoveToGoal( void )
{
	float	saveYaw = NPC->client->ps.viewangles[YAW];

	NPCInfo->combatMove = qtrue;
	
	qboolean ret = NPC_MoveToGoal();

	NPCInfo->desiredYaw	= saveYaw;

	return ret;
}


/*
-------------------------
NPC_ApplyRoff
-------------------------
*/

void NPC_ApplyRoff(void)
{
	PlayerStateToEntityState( &NPC->client->ps, &NPC->s );
	VectorCopy ( NPC->currentOrigin, NPC->lastOrigin );

	// use the precise origin for linking
	gi.linkentity(NPC);
}

