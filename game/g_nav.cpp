#include "b_local.h"
#include "g_nav.h"
#include "g_navigator.h"

//Global navigator
CNavigator		navigator;

qboolean NAV_CheckAhead( gentity_t *self, vec3_t end, trace_t &trace, int clipmask );
qboolean G_FindClosestPointOnLineSegment( const vec3_t start, const vec3_t end, const vec3_t from, vec3_t result );

//For debug graphics
extern void CG_Line( vec3_t start, vec3_t end, vec3_t color, float alpha );
extern void CG_Cube( vec3_t mins, vec3_t maxs, vec3_t color, float alpha );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern qboolean NPC_CheckDisguise( gentity_t *ent );

#define	MIN_STOP_DIST 64

/*
-------------------------
NPC_Blocked
-------------------------
*/

#define	MIN_BLOCKED_SPEECH_TIME	4000

void NPC_Blocked( gentity_t *self, gentity_t *blocker )
{
	if ( self->NPC == NULL )
		return;

	//Don't do this too often
	if ( self->NPC->blockedSpeechDebounceTime > level.time )
		return;

	//Attempt to run any blocked scripts
	if ( VALIDSTRING( self->behaviorSet[BSET_BLOCKED] ) )
	{
		//Run our blocked script
		G_ActivateBehavior( self, BSET_BLOCKED );
		return;
	}

	//If this is one of our enemies, then just attack him
	if ( blocker->client && ( blocker->client->playerTeam == self->client->enemyTeam ) )
	{
		G_SetEnemy( self, blocker );
		return;
	}

	Debug_Printf( debugNPCAI, DEBUG_LEVEL_WARNING, "%s: Excuse me, %s %s!\n", self->targetname, blocker->classname, blocker->targetname );
	
	//If we're being blocked by the player, say something to them
	if ( ( blocker->s.number == 0 ) && ( ( blocker->client->playerTeam == self->client->playerTeam ) || NPC_CheckDisguise( blocker ) ) )
	{
		//guys in formation are not trying to get to a critical point, 
		//don't make them yell at the player (unless they have an enemy and
		//are in combat because BP thinks it sounds cool during battle)
		if ( self->NPC->behaviorState != BS_FORMATION || self->enemy )
		{
			//NOTE: only imperials, misc crewmen and hazard team have these wav files now
			G_AddVoiceEvent( self, Q_irand(EV_BLOCKED1, EV_BLOCKED3), 0 );
		}
	}

	self->NPC->blockedSpeechDebounceTime = level.time + MIN_BLOCKED_SPEECH_TIME + ( random() * 4000 );
	self->NPC->blockingEntNum = blocker->s.number;
}

/*
-------------------------
NPC_SetMoveGoal
-------------------------
*/

void NPC_SetMoveGoal( gentity_t *ent, vec3_t point, int radius, bool isNavGoal )
{
	//Must be an NPC
	if ( ent->NPC == NULL )
		return;

	//Copy the origin
	//VectorCopy( point, ent->NPC->goalPoint );	//FIXME: Make it use this, and this alone!
	VectorCopy( point, ent->NPC->tempGoal->currentOrigin );
	
	//Copy the mins and maxs to the tempGoal
	VectorCopy( ent->mins, ent->NPC->tempGoal->mins );
	VectorCopy( ent->mins, ent->NPC->tempGoal->maxs );

	ent->NPC->tempGoal->clipmask = ent->clipmask;
	ent->NPC->tempGoal->svFlags &= ~SVF_NAVGOAL;
	ent->NPC->tempGoal->waypoint = WAYPOINT_NONE;

	if ( isNavGoal )
		ent->NPC->tempGoal->svFlags |= SVF_NAVGOAL;

	ent->NPC->goalEntity = ent->NPC->tempGoal;
	ent->NPC->goalRadius = radius;
}

/*
-------------------------
NAV_HitNavGoal
-------------------------
*/

qboolean NAV_HitNavGoal( vec3_t point, vec3_t mins, vec3_t maxs, vec3_t dest, int radius )
{
	vec3_t	dmins, dmaxs, pmins, pmaxs;

	if ( radius & NAVGOAL_USE_RADIUS )
	{
		radius &= ~NAVGOAL_USE_RADIUS;
		//NOTE:  This needs to do a DistanceSquared on navgoals that had
		//			a radius manually set! We can't do the smaller navgoals against
		//			walls to get around this because player-sized traces to them
		//			from angles will not work... - MCG
		//FIXME:  Allow for a little z difference?
		return ( DistanceSquared(dest, point) <= (radius*radius) );
		//There is probably a better way to do this, either by preserving the original
		//		mins and maxs of the navgoal and doing this check ONLY if the radius 
		//		is non-zero (like the original implementation) or some boolean to
		//		tell us to do this check rather than the fake bbox overlap check...
	}
	else
	{
		//Construct a dummy bounding box from our radius value
		VectorSet( dmins, -radius, -radius, -radius );
		VectorSet( dmaxs,  radius,  radius,  radius );

		//Translate it
		VectorAdd( dmins, dest, dmins );
		VectorAdd( dmaxs, dest, dmaxs );

		//Translate the starting box
		VectorAdd( point, mins, pmins );
		VectorAdd( point, maxs, pmaxs );

		//See if they overlap
		return G_BoundsOverlap( pmins, pmaxs, dmins, dmaxs );
	}
}

/*
-------------------------
NAV_ClearPathToPoint
-------------------------
*/

qboolean NAV_ClearPathToPoint(gentity_t *self, vec3_t pmins, vec3_t pmaxs, vec3_t point, int clipmask)
{
//	trace_t	trace;
//	return NAV_CheckAhead( self, point, trace, clipmask );

	vec3_t	mins, maxs;
	trace_t	trace;

	//Test if they're even conceivably close to one another
	if ( !gi.inPVS( self->currentOrigin, point ) )
		return qfalse;

	if ( self->svFlags & SVF_NAVGOAL )
	{
		if ( !self->owner )
		{
			//SHOULD NEVER HAPPEN!!!
			assert(0);
			return qfalse;
		}
		VectorCopy( self->owner->mins, mins );
		VectorCopy( self->owner->maxs, maxs );
	}
	else
	{
		VectorCopy( pmins, mins );
		VectorCopy( pmaxs, maxs );
	}
	
	if ( self->client || ( self->svFlags & SVF_NAVGOAL ) )
	{
		//Clients can step up things, or if this is a navgoal check, a client will be using this info
		mins[2] += STEPSIZE;

		//don't let box get inverted
		if ( mins[2] > maxs[2] )
		{	
			mins[2] = maxs[2];
		}
	}

	if ( self->svFlags & SVF_NAVGOAL )
	{
		//Trace from point to navgoal
		gi.trace( &trace, point, mins, maxs, self->currentOrigin, self->owner->s.number, (clipmask|CONTENTS_MONSTERCLIP)&~CONTENTS_BODY );//clipmask );
		
		if ( trace.startsolid || trace.allsolid )
		{
			return qfalse;
		}
		
		//Made it
		if ( trace.fraction == 1.0 )
			return qtrue;
		
		//Okay, didn't get all the way there, let's see if we got close enough:
		return NAV_HitNavGoal( self->currentOrigin, self->owner->mins, self->owner->maxs, trace.endpos, NPCInfo->goalRadius );
	}
	else
	{
		gi.trace( &trace, self->currentOrigin, mins, maxs, point, self->s.number, clipmask|CONTENTS_MONSTERCLIP);

		if( ( ( trace.startsolid == qfalse ) && ( trace.allsolid == qfalse ) ) && ( trace.fraction == 1.0f ) )
		{//FIXME: check for drops
			return qtrue;
		}

		return qfalse;
	}
}

/*
-------------------------
NAV_FindClosestWaypointForEnt
-------------------------
*/

int NAV_FindClosestWaypointForEnt( gentity_t *ent, int targWp )
{
	//FIXME: Take the target into account
	return navigator.GetNearestNode( ent, ent->waypoint, NF_CLEAR_PATH );
}

/*
-------------------------
NAV_ClearBlockedInfo
-------------------------
*/

void NAV_ClearBlockedInfo( gentity_t *self )
{
	self->NPC->aiFlags &= ~NPCAI_BLOCKED;
	self->NPC->blockingEntNum = ENTITYNUM_WORLD;
}

/*
-------------------------
NAV_SetBlockedInfo
-------------------------
*/

void NAV_SetBlockedInfo( gentity_t *self, int entId )
{
	self->NPC->aiFlags |= NPCAI_BLOCKED;
	self->NPC->blockingEntNum = entId;
}

/*
-------------------------
NAV_Steer
-------------------------
*/

int NAV_Steer( gentity_t *self, vec3_t dir, float distance )
{
	vec3_t	right_test, left_test;
	vec3_t	deviation;

	float	right_ang	= dir[YAW] + 45;
	float	left_ang	= dir[YAW] - 45;

	//Get the steering angles
	VectorCopy( dir, deviation );
	deviation[YAW] = right_ang;

	AngleVectors( deviation, right_test, NULL, NULL );

	deviation[YAW] = left_ang;

	AngleVectors( deviation, left_test, NULL, NULL );

	//Find the end positions
	VectorMA( self->currentOrigin, distance, right_test, right_test );
	VectorMA( self->currentOrigin, distance, left_test,  left_test );

	//Draw for debug purposes
	if ( NAVDEBUG_showCollision )
	{
		CG_DrawEdge( self->currentOrigin, right_test, EDGE_PATH );
		CG_DrawEdge( self->currentOrigin, left_test,  EDGE_PATH );
	}

	//Find the right influence
	trace_t	tr;
	NAV_CheckAhead( self, right_test, tr, self->clipmask );

	float	right_push = -45 * ( 1.0f - tr.fraction );

	//Find the left influence
	NAV_CheckAhead( self, left_test, tr, self->clipmask );

	float	left_push = 45 * ( 1.0f - tr.fraction );

	//Influence the mover to respond to the steering
	VectorCopy( dir, deviation );
	deviation[YAW] += ( left_push + right_push );

	return deviation[YAW];
}

/*
-------------------------
NAV_CheckAhead
-------------------------
*/

#define	MIN_DOOR_BLOCK_DIST			16
#define	MIN_DOOR_BLOCK_DIST_SQR		( MIN_DOOR_BLOCK_DIST * MIN_DOOR_BLOCK_DIST )

qboolean NAV_CheckAhead( gentity_t *self, vec3_t end, trace_t &trace, int clipmask )
{
	vec3_t	mins;

	//Offset the step height
	VectorSet( mins, self->mins[0], self->mins[1], self->mins[2] + STEPSIZE );
	
	gi.trace( &trace, self->currentOrigin, mins, self->maxs, end, self->s.number, clipmask );

	//Do a simple check
	if ( ( trace.allsolid == qfalse ) && ( trace.startsolid == qfalse ) && ( trace.fraction == 1.0f ) )
		return qtrue;

	//See if we're too far above
	if ( fabs( self->currentOrigin[2] - end[2] ) > 48 )
		return qfalse;

	//This is a work around
	float	radius = ( self->maxs[0] > self->maxs[1] ) ? self->maxs[0] : self->maxs[1];
	float	dist = Distance( self->currentOrigin, end );
	float	tFrac = 1.0f - ( radius / dist );

	if ( trace.fraction >= tFrac )
		return qtrue;

	//Do a special check for doors
	if ( trace.entityNum < ENTITYNUM_WORLD )
	{
		gentity_t	*blocker = &g_entities[trace.entityNum];
		
		if VALIDSTRING( blocker->classname )
		{
			if ( Q_stricmp( blocker->classname, "func_door" ) == 0 )
			{
				//We're too close, try and avoid the door (most likely stuck on a lip)
				if ( DistanceSquared( self->currentOrigin, trace.endpos ) < MIN_DOOR_BLOCK_DIST_SQR )
					return qfalse;

				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
-------------------------
NAV_TestBypass
-------------------------
*/

static qboolean NAV_TestBypass( gentity_t *self, float yaw, float blocked_dist, vec3_t movedir )
{
	trace_t	tr;
	vec3_t	avoidAngles;
	vec3_t	block_test, block_pos;

	VectorClear( avoidAngles );
	avoidAngles[YAW] = yaw;

	AngleVectors( avoidAngles, block_test, NULL, NULL );
	VectorMA( self->currentOrigin, blocked_dist, block_test, block_pos );
		
	if ( NAVDEBUG_showCollision )
	{
		CG_DrawEdge( self->currentOrigin, block_pos, EDGE_BROKEN );
	}

	//See if we're clear to move in that direction
	if ( NAV_CheckAhead( self, block_pos, tr, ( self->clipmask & ~CONTENTS_BODY ) ) )
	{
		VectorCopy( block_test, movedir );
		
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
NAV_Bypass
-------------------------
*/

qboolean NAV_Bypass( gentity_t *self, gentity_t *blocker, vec3_t blocked_dir, float blocked_dist, vec3_t movedir ) 
{
	vec3_t	right;

	//Draw debug info if requested
	if ( NAVDEBUG_showCollision )
	{
		CG_DrawEdge( self->currentOrigin, blocker->currentOrigin, EDGE_NORMAL );
	}

	AngleVectors( self->currentAngles, NULL, right, NULL );

	//Get the blocked direction
	float yaw = vectoyaw( blocked_dir );

	//Get the avoid radius
	float avoidRadius = sqrt( ( blocker->maxs[0] * blocker->maxs[0] ) + ( blocker->maxs[1] * blocker->maxs[1] ) ) + 
						sqrt( ( self->maxs[0] * self->maxs[0] ) + ( self->maxs[1] * self->maxs[1] ) );

	//See if we're inside our avoidance radius
	float arcAngle = ( blocked_dist <= avoidRadius ) ? 135 : ( ( avoidRadius / blocked_dist ) * 90 );

	//FIXME: Although the below code will cause the NPC to take the "better" route, it can cause NPCs to become stuck on
	//		 one another in certain situations where both decide to take the same direction.

	float dot = DotProduct( blocked_dir, right );

	//Go right on the first try if that works better
	if ( dot < 0.0f )
		arcAngle *= -1;

	//Test full, best position first
	if ( NAV_TestBypass( self, AngleMod( yaw + arcAngle ), blocked_dist, movedir ) )
		return qtrue;

	//Try a smaller arc
	if ( NAV_TestBypass( self, AngleMod( yaw + ( arcAngle * 0.5f ) ), blocked_dist, movedir ) )
		return qtrue;

	//Try the other direction
	if ( NAV_TestBypass( self, AngleMod( yaw + ( arcAngle * -1 ) ), blocked_dist, movedir ) )
		return qtrue;

	//Try the other direction more precisely
	if ( NAV_TestBypass( self, AngleMod( yaw + ( ( arcAngle * -1 ) * 0.5f ) ), blocked_dist, movedir ) )
		return qtrue;

	//Unable to go around
	return qfalse;
}

/*
-------------------------
NAV_MoveBlocker
-------------------------
*/

#define	SHOVE_SPEED	200
#define SHOVE_LIFT	10

qboolean NAV_MoveBlocker( gentity_t *self, vec3_t shove_dir )
{
	//FIXME: This is a temporary method for making blockers move
	
	//FIXME: This will, of course, push blockers off of cliffs, into walls and all over the place

	vec3_t	temp_dir, forward;

	vectoangles( shove_dir, temp_dir );

	temp_dir[YAW] += 45;
	AngleVectors( temp_dir, forward, NULL, NULL );

	VectorScale( forward, SHOVE_SPEED, self->client->ps.velocity );
	self->client->ps.velocity[2] += SHOVE_LIFT;

	//self->NPC->shoveDebounce = level.time + 100;

	return qtrue;
}

/*
-------------------------
NAV_ResolveBlock
-------------------------
*/

qboolean NAV_ResolveBlock( gentity_t *self, gentity_t *blocker, vec3_t blocked_dir )
{
	//Stop double waiting
	if ( ( blocker->NPC ) && ( blocker->NPC->blockingEntNum == self->s.number ) )
		return qtrue;

	//For now, just complain about it
	NPC_Blocked( self, blocker );
	NPC_FaceEntity( blocker );

	return qfalse;
}

/*
-------------------------
NAV_TrueCollision
-------------------------
*/

qboolean NAV_TrueCollision( gentity_t *self, gentity_t *blocker, vec3_t movedir, vec3_t blocked_dir )
{
	//TODO: Handle all ents
	if ( blocker->client == NULL )
		return qfalse;

	vec3_t	velocityDir;

	//Get the player's move direction and speed
	float	speed = VectorNormalize2( self->client->ps.velocity, velocityDir );

	//See if it's even feasible
	float dot = DotProduct( movedir, velocityDir );

	if ( dot < 0.85 )
		return qfalse;

	vec3_t	testPos;
	vec3_t	ptmins, ptmaxs, tmins, tmaxs;

	VectorMA( self->currentOrigin, speed*FRAMETIME, velocityDir, testPos );

	VectorAdd( blocker->currentOrigin, blocker->mins, tmins );
	VectorAdd( blocker->currentOrigin, blocker->maxs, tmaxs );

	VectorAdd( testPos, self->mins, ptmins );
	VectorAdd( testPos, self->maxs, ptmaxs );

	if ( G_BoundsOverlap( ptmins, ptmaxs, tmins, tmaxs ) )
	{
		VectorCopy( velocityDir, blocked_dir );
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
NAV_StackedCanyon
-------------------------
*/

qboolean NAV_StackedCanyon( gentity_t *self, gentity_t *blocker, vec3_t pathDir )
{
	vec3_t	perp, cross, test;
	float	avoidRadius;

	PerpendicularVector( perp, pathDir );
	CrossProduct( pathDir, perp, cross );

	avoidRadius =	sqrt( ( blocker->maxs[0] * blocker->maxs[0] ) + ( blocker->maxs[1] * blocker->maxs[1] ) ) + 
					sqrt( ( self->maxs[0] * self->maxs[0] ) + ( self->maxs[1] * self->maxs[1] ) );

	VectorMA( blocker->currentOrigin, avoidRadius, cross, test );

	trace_t	tr;
	gi.trace( &tr, test, self->mins, self->maxs, test, self->s.number, self->clipmask );

	if ( NAVDEBUG_showCollision )
	{
		vec3_t	mins, maxs;
		vec3_t	RED = { 1.0f, 0.0f, 0.0f };

		VectorAdd( test, self->mins, mins );
		VectorAdd( test, self->maxs, maxs );
		CG_Cube( mins, maxs, RED, 0.25 );
	}

	if ( tr.startsolid == qfalse && tr.allsolid == qfalse )
		return qfalse;

	VectorMA( blocker->currentOrigin, -avoidRadius, cross, test );

	gi.trace( &tr, test, self->mins, self->maxs, test, self->s.number, self->clipmask );

	if ( tr.startsolid == qfalse && tr.allsolid == qfalse )
		return qfalse;

	if ( NAVDEBUG_showCollision )
	{
		vec3_t	mins, maxs;
		vec3_t	RED = { 1.0f, 0.0f, 0.0f };

		VectorAdd( test, self->mins, mins );
		VectorAdd( test, self->maxs, maxs );
		CG_Cube( mins, maxs, RED, 0.25 );
	}

	return qtrue;
}

/*
-------------------------
NAV_ResolveEntityCollision
-------------------------
*/

qboolean NAV_ResolveEntityCollision( gentity_t *self, gentity_t *blocker, vec3_t movedir, vec3_t pathDir )
{
	vec3_t	blocked_dir;

	//Doors are ignored
	if ( Q_stricmp( blocker->classname, "func_door" ) == 0 )
	{
		if ( DistanceSquared( self->currentOrigin, blocker->currentOrigin ) > MIN_DOOR_BLOCK_DIST_SQR )
			return qtrue;
	}

	VectorSubtract( blocker->currentOrigin, self->currentOrigin, blocked_dir );
	float blocked_dist = VectorNormalize( blocked_dir );

	//Make sure an actual collision is going to happen
//	if ( NAV_PredictCollision( self, blocker, movedir, blocked_dir ) == qfalse )
//		return qtrue;
	
	//See if we can get around the blocker at all (only for player!)
	if ( blocker->s.number == 0 )
	{
		if ( NAV_StackedCanyon( self, blocker, pathDir ) )
		{
			NPC_Blocked( self, blocker );
			NPC_FaceEntity( blocker, qtrue );

			return qfalse;
		}
	}

	//First, attempt to walk around the blocker
	if ( NAV_Bypass( self, blocker, blocked_dir, blocked_dist, movedir ) )
		return qtrue;

	//Second, attempt to calculate a good move position for the blocker
	if ( NAV_ResolveBlock( self, blocker, blocked_dir ) )
		return qtrue;

	return qfalse;
}

/*
-------------------------
NAV_TestForBlocked
-------------------------
*/

qboolean NAV_TestForBlocked( gentity_t *self, gentity_t *goal, gentity_t *blocker, float distance, int &flags )
{
	if ( goal == NULL )
		return qfalse;

	if ( blocker->s.eType == ET_ITEM )
		return qfalse;

	if ( NAV_HitNavGoal( blocker->currentOrigin, blocker->mins, blocker->maxs, goal->currentOrigin, 12 ) )
	{
		flags |= NIF_BLOCKED;

		if ( distance <= MIN_STOP_DIST )
		{
			NPC_Blocked( self, blocker );
			NPC_FaceEntity( blocker );
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NAV_AvoidCollsion
-------------------------
*/

qboolean NAV_AvoidCollision( gentity_t *self, gentity_t *goal, navInfo_t &info )
{
	vec3_t	movedir;
	vec3_t	movepos;

	//Clear our block info for this frame
	NAV_ClearBlockedInfo( NPC );

	//Cap our distance
	if ( info.distance > MAX_COLL_AVOID_DIST )
	{
		info.distance = MAX_COLL_AVOID_DIST;
	}

	//Get an end position
	VectorMA( self->currentOrigin, info.distance, info.direction, movepos );
	VectorCopy( info.direction, movedir );

	//Now test against entities
	if ( NAV_CheckAhead( self, movepos, info.trace, CONTENTS_BODY ) == qfalse )
	{
		//Get the blocker
		info.blocker = &g_entities[ info.trace.entityNum ];
		info.flags |= NIF_COLLISION;

		//Ok to hit our goal entity
		if ( goal == info.blocker )
			return qtrue;

		//See if we're moving along with them
		//if ( NAV_TrueCollision( self, info.blocker, movedir, info.direction ) == qfalse )
		//	return qtrue;

		//Test for blocking by standing on goal
		if ( NAV_TestForBlocked( self, goal, info.blocker, info.distance, info.flags ) == qtrue )
			return qfalse;

		//If the above function said we're blocked, don't do the extra checks
		if ( info.flags & NIF_BLOCKED )
			return qtrue;

		//See if we can get that entity to move out of our way
		if ( NAV_ResolveEntityCollision( self, info.blocker, movedir, info.pathDirection ) == qfalse )
			return qfalse;

		VectorCopy( movedir, info.direction );
		
		return qtrue;
	}

	//Our path is clear, just move there
	if ( NAVDEBUG_showCollision )
	{
		CG_DrawEdge( self->currentOrigin, movepos, EDGE_PATH );
	}

	return qtrue;
}

/*
-------------------------
NAV_TestBestNode
-------------------------
*/

int NAV_TestBestNode( vec3_t position, int startID, int endID )
{
	int bestNode = startID;
	int	testNode = navigator.GetBestNode( bestNode, endID );

	vec3_t	p1, p2, dir1, dir2;

	navigator.GetNodePosition( bestNode, p1 );
	navigator.GetNodePosition( testNode, p2 );

	/*
	if ( DistanceSquared( p1, p2 ) < DistanceSquared( position, p1 ) )
		return testNode;
	*/
	
	VectorSubtract( p2, p1, dir1 );
	//VectorNormalize( dir1 );

	VectorSubtract( position, p1, dir2 );
	//VectorNormalize( dir2 );

	if ( DotProduct( dir1, dir2 ) > 0 )
	{
		trace_t	trace;

		if ( NAV_CheckAhead( NPC, p2, trace, (NPC->clipmask&~CONTENTS_BODY) ) )
			bestNode = testNode;
	}
	
	return bestNode;
}

/*
-------------------------
NAV_GetNearestNode
-------------------------
*/

int NAV_GetNearestNode( gentity_t *self, int lastNode )
{
	return navigator.GetNearestNode( self, lastNode, NF_CLEAR_PATH );;
}

/*
-------------------------
NAV_MicroError
-------------------------
*/

qboolean NAV_MicroError( vec3_t start, vec3_t end )
{
	if ( VectorCompare( start, end ) )
	{
		if ( DistanceSquared( NPC->currentOrigin, start ) < (8*8) )
		{
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NAV_MoveToGoal
-------------------------
*/

int	NAV_MoveToGoal( gentity_t *self, navInfo_t &info )
{
	//Must have a goal entity to move there
	if( self->NPC->goalEntity == NULL )
		return WAYPOINT_NONE;

	//Check special player optimizations
	if ( self->NPC->goalEntity->s.number == 0 )
	{
		//If we couldn't find the point, then we won't be able to this turn
		if ( self->NPC->goalEntity->waypoint == WAYPOINT_NONE )
			return WAYPOINT_NONE;

		//NOTENOTE: Otherwise trust this waypoint for the whole frame (reduce all unnecessary calculations)
	}
	else
	{
		//Find the target's waypoint
		if ( ( self->NPC->goalEntity->waypoint = NAV_GetNearestNode( self->NPC->goalEntity, self->NPC->goalEntity->waypoint ) ) == WAYPOINT_NONE )
			return WAYPOINT_NONE;
	}

	//Find our waypoint
	if ( ( self->waypoint = NAV_GetNearestNode( self, self->lastWaypoint ) ) == WAYPOINT_NONE )
		return WAYPOINT_NONE;

	int bestNode = navigator.GetBestNode( self->waypoint, self->NPC->goalEntity->waypoint );

	if ( bestNode == WAYPOINT_NONE )
	{
		if ( NAVDEBUG_showEnemyPath )
		{
			vec3_t	origin, torigin;

			navigator.GetNodePosition( self->NPC->goalEntity->waypoint, torigin );
			navigator.GetNodePosition( self->waypoint, origin );

			CG_DrawNode( torigin, NODE_GOAL );
			CG_DrawNode( origin, NODE_GOAL );
			CG_DrawNode( self->NPC->goalEntity->currentOrigin, NODE_START );
		}
		
		return WAYPOINT_NONE;
	}

	//Check this node
	bestNode = NAV_TestBestNode( self->currentOrigin, bestNode, self->NPC->goalEntity->waypoint );

	vec3_t	origin, end;
	//trace_t	trace;

	//Get this position
	navigator.GetNodePosition( bestNode, origin );
	navigator.GetNodePosition( self->waypoint, end );

	//Basically, see if the path we have isn't helping
	//if ( NAV_MicroError( origin, end ) )
	//	return WAYPOINT_NONE;

	//Test the path connection from our current position to the best node
	if ( NAV_CheckAhead( self, origin, info.trace, (self->clipmask&~CONTENTS_BODY) ) == qfalse )
	{
		//First attempt to move to the closest point on the line between the waypoints
		G_FindClosestPointOnLineSegment( origin, end, self->currentOrigin, origin );

		//See if we can go there
		if ( NAV_CheckAhead( self, origin, info.trace, (self->clipmask&~CONTENTS_BODY) ) == qfalse )
		{
			//Just move towards our current waypoint
			bestNode = self->waypoint;
			navigator.GetNodePosition( bestNode, origin );
		}
	}

	//Setup our new move information
	VectorSubtract( origin, self->currentOrigin, info.direction );
	info.distance = VectorNormalize( info.direction );

	VectorSubtract( end, origin, info.pathDirection );
	VectorNormalize( info.pathDirection );

	//Draw any debug info, if requested
	if ( NAVDEBUG_showEnemyPath )
	{
		vec3_t	dest, start;

		//Get the positions
		navigator.GetNodePosition( self->NPC->goalEntity->waypoint, dest );
		navigator.GetNodePosition( bestNode, start );

		//Draw the route
		CG_DrawNode( start, NODE_START );
		CG_DrawNode( dest, NODE_GOAL );
		navigator.ShowPath( self->waypoint, self->NPC->goalEntity->waypoint );
	}

	if ( NPC_ShouldShowPath( self ) )
	{
		navigator.ShowPath( bestNode, self->NPC->goalEntity->waypoint, true );

		vec3_t	dest;
		navigator.GetNodePosition( self->NPC->goalEntity->waypoint, dest );
		if ( DistanceSquared( dest, self->NPC->goalEntity->currentOrigin ) > 0.1 )
		{
			CG_DrawNode( dest, NODE_NAVGOAL );
			CG_DrawEdge( dest, self->NPC->goalEntity->currentOrigin, EDGE_PATH );
		}
	}

	return bestNode;
}

/*
-------------------------
waypoint_testDirection
-------------------------
*/

#define	MAX_RADIUS_CHECK	1024

unsigned int waypoint_testDirection( vec3_t origin, float yaw )
{
	vec3_t	trace_dir, test_pos;
	vec3_t	maxs, mins;
	trace_t	tr;

	//Setup the mins and max
	VectorSet( maxs, DEFAULT_MAXS_0, DEFAULT_MAXS_1, DEFAULT_MAXS_2 );
	VectorSet( mins, DEFAULT_MINS_0, DEFAULT_MINS_1, DEFAULT_MINS_2 - STEPSIZE );

	//Get our test direction
	vec3_t	angles = { 0, yaw, 0 };
	AngleVectors( angles, trace_dir, NULL, NULL );

	//Move ahead
	VectorMA( origin, MAX_RADIUS_CHECK, trace_dir, test_pos );

	gi.trace( &tr, origin, mins, maxs, test_pos, ENTITYNUM_WORLD, ( CONTENTS_SOLID | CONTENTS_MONSTERCLIP ) );

	return (unsigned int) ( (float) MAX_RADIUS_CHECK * tr.fraction );
}

/*
-------------------------
waypoint_getRadius
-------------------------
*/

#define	YAW_ITERATIONS	16

unsigned int waypoint_getRadius( gentity_t *ent )
{
	unsigned int	minDist = (unsigned int) -1;
	unsigned int	dist;

	for ( int i = 0; i < YAW_ITERATIONS; i++ )
	{
		dist = waypoint_testDirection( ent->currentOrigin, ((360.0f/YAW_ITERATIONS) * i) );

		if ( dist < minDist )
			minDist = dist;
	}

	return minDist;
}

/*QUAKED waypoint  (0.7 0.7 0) (-12 -12 -24) (12 12 32) ONEWAY
a place to go.

ONEWAY - this waypoint has a route TO it's target(s), but not FROM it/them

radius is automatically calculated in-world.
*/
void SP_waypoint ( gentity_t *ent )
{
	if ( navCalculatePaths )
	{
		VectorSet(ent->mins, DEFAULT_MINS_0, DEFAULT_MINS_1, DEFAULT_MINS_2);
		VectorSet(ent->maxs, DEFAULT_MAXS_0, DEFAULT_MAXS_1, DEFAULT_MAXS_2);
		
		ent->contents = CONTENTS_TRIGGER;
		ent->clipmask = MASK_DEADSOLID;

		gi.linkentity( ent );

		ent->count = -1;
		ent->classname = "waypoint";

		if(G_CheckInSolid (ent, qtrue))
		{
			ent->maxs[2] = CROUCH_MAXS_2;
			if(G_CheckInSolid (ent, qtrue))
			{
				gi.Printf(S_COLOR_RED"ERROR: Waypoint %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
				assert(0 && "Waypoint in solid!");
#ifndef FINAL_BUILD
				G_Error("Waypoint %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
				G_FreeEntity(ent);
				return;
			}
		}

		unsigned int radius = waypoint_getRadius( ent );

		ent->health = navigator.AddRawPoint( ent->currentOrigin, ent->spawnflags, radius );
		return;
	}

	G_FreeEntity(ent);
}

/*QUAKED waypoint_small  (0.7 0.7 0) (-2 -2 -24) (2 2 32) ONEWAY
ONEWAY - this waypoint has a route TO it's target(s), but not FROM it/them
*/
void SP_waypoint_small (gentity_t *ent)
{
	if ( navCalculatePaths )
	{
		VectorSet(ent->mins, -2, -2, DEFAULT_MINS_2);
		VectorSet(ent->maxs, 2, 2, DEFAULT_MAXS_2);

		ent->contents = CONTENTS_TRIGGER;
		ent->clipmask = MASK_DEADSOLID;

		gi.linkentity( ent );

		ent->count = -1;
		ent->classname = "waypoint";

		if(G_CheckInSolid (ent, qtrue))
		{
			ent->maxs[2] = CROUCH_MAXS_2;
			if(G_CheckInSolid (ent, qtrue))
			{
				gi.Printf(S_COLOR_RED"ERROR: Waypoint_small %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
				assert(0);
#ifndef FINAL_BUILD
				G_Error("Waypoint_small %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
				G_FreeEntity(ent);
				return;
			}
		}

		ent->health = navigator.AddRawPoint( ent->currentOrigin, ent->spawnflags, 2 );
		return;
	}

	G_FreeEntity(ent);
}


/*QUAKED waypoint_navgoal (0.3 1 0.3) (-12 -12 -24) (12 12 32)
A waypoint for script navgoals
Not included in navigation data

targetname - name you would use in script when setting a navgoal (like so:)

  For example: if you give this waypoint a targetname of "console", make an NPC go to it in a script like so:

  set ("navgoal", "console");

radius - how far from the navgoal an ent can be before it thinks it reached it - default is "0" which means no radius check, just have to touch it
*/

void SP_waypoint_navgoal( gentity_t *ent )
{
	int radius = ( ent->radius ) ? ((int)(ent->radius)|NAVGOAL_USE_RADIUS) : 12;

	VectorSet( ent->mins, -12, -12, -24 );
	VectorSet( ent->maxs, 12, 12, 32 );
	if ( G_CheckInSolid( ent, qfalse ) )
	{
		gi.Printf(S_COLOR_RED"ERROR: Waypoint_navgoal %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
		assert(0);
#ifndef FINAL_BUILD
		G_Error("Waypoint_navgoal %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
	}
	TAG_Add( ent->targetname, NULL, ent->s.origin, ent->s.angles, radius, RTF_NAVGOAL );

	ent->classname = "navgoal";
	//G_FreeEntity( ent );//can't do this, they need to be found later by some functions, though those could be fixed, maybe?
}

/*QUAKED waypoint_navgoal_8 (0.3 1 0.3) (-8 -8 -24) (8 8 32)
A waypoint for script navgoals, 8 x 8 size
Not included in navigation data

targetname - name you would use in script when setting a navgoal (like so:)

  For example: if you give this waypoint a targetname of "console", make an NPC go to it in a script like so:

  set ("navgoal", "console");

You CANNOT set a radius on these navgoals, they are touch-reach ONLY
*/
void SP_waypoint_navgoal_8( gentity_t *ent )
{
	VectorSet( ent->mins, -8, -8, -24 );
	VectorSet( ent->maxs, 8, 8, 32 );
	if ( G_CheckInSolid( ent, qfalse ) )
	{
		gi.Printf(S_COLOR_RED"ERROR: Waypoint_navgoal_8 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#ifndef FINAL_BUILD
		G_Error("Waypoint_navgoal_8 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
		assert(0);
	}

	TAG_Add( ent->targetname, NULL, ent->s.origin, ent->s.angles, 8, RTF_NAVGOAL );

	ent->classname = "navgoal";
	//G_FreeEntity( ent );//can't do this, they need to be found later by some functions, though those could be fixed, maybe?
}

/*QUAKED waypoint_navgoal_4 (0.3 1 0.3) (-4 -4 -24) (4 4 32)
A waypoint for script navgoals, 4 x 4 size
Not included in navigation data

targetname - name you would use in script when setting a navgoal (like so:)

  For example: if you give this waypoint a targetname of "console", make an NPC go to it in a script like so:

  set ("navgoal", "console");

You CANNOT set a radius on these navgoals, they are touch-reach ONLY
*/
void SP_waypoint_navgoal_4( gentity_t *ent )
{
	VectorSet( ent->mins, -4, -4, -24 );
	VectorSet( ent->maxs, 4, 4, 32 );

	if ( G_CheckInSolid( ent, qfalse ) )
	{
		gi.Printf(S_COLOR_RED"ERROR: Waypoint_navgoal_4 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#ifndef FINAL_BUILD
		G_Error("Waypoint_navgoal_4 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
		assert(0);
	}

	TAG_Add( ent->targetname, NULL, ent->s.origin, ent->s.angles, 4, RTF_NAVGOAL );

	ent->classname = "navgoal";
	//G_FreeEntity( ent );//can't do this, they need to be found later by some functions, though those could be fixed, maybe?
}

/*QUAKED waypoint_navgoal_2 (0.3 1 0.3) (-2 -2 -24) (2 2 32)
A waypoint for script navgoals, 2 x 2 size
Not included in navigation data

targetname - name you would use in script when setting a navgoal (like so:)

  For example: if you give this waypoint a targetname of "console", make an NPC go to it in a script like so:

  set ("navgoal", "console");

You CANNOT set a radius on these navgoals, they are touch-reach ONLY
*/
void SP_waypoint_navgoal_2( gentity_t *ent )
{	
	VectorSet( ent->mins, -2, -2, -24 );
	VectorSet( ent->maxs, 2, 2, 32 );

	if ( G_CheckInSolid( ent, qfalse ) )
	{
		gi.Printf(S_COLOR_RED"ERROR: Waypoint_navgoal_2 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#ifndef FINAL_BUILD
		G_Error("Waypoint_navgoal_2 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
		assert(0);
	}

	TAG_Add( ent->targetname, NULL, ent->s.origin, ent->s.angles, 2, RTF_NAVGOAL );

	ent->classname = "navgoal";
	//G_FreeEntity( ent );//can't do this, they need to be found later by some functions, though those could be fixed, maybe?
}

/*QUAKED waypoint_navgoal_1 (0.3 1 0.3) (-1 -1 -24) (1 1 32)
A waypoint for script navgoals, 1 x 1 size
Not included in navigation data

targetname - name you would use in script when setting a navgoal (like so:)

  For example: if you give this waypoint a targetname of "console", make an NPC go to it in a script like so:

  set ("navgoal", "console");

You CANNOT set a radius on these navgoals, they are touch-reach ONLY
*/
void SP_waypoint_navgoal_1( gentity_t *ent )
{
	VectorSet( ent->mins, -1, -1, -24 );
	VectorSet( ent->maxs, 1, 1, 32 );

	if ( G_CheckInSolid( ent, qfalse ) )
	{
		gi.Printf(S_COLOR_RED"ERROR: Waypoint_navgoal_1 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#ifndef FINAL_BUILD
		G_Error("Waypoint_navgoal_1 %s at %s in solid!\n", ent->targetname, vtos(ent->currentOrigin));
#endif
		assert(0);
	}

	TAG_Add( ent->targetname, NULL, ent->s.origin, ent->s.angles, 1, RTF_NAVGOAL );

	ent->classname = "navgoal";
	//G_FreeEntity( ent );//can't do this, they need to be found later by some functions, though those could be fixed, maybe?
}

/*
-------------------------
Svcmd_Nav_f
-------------------------
*/

void Svcmd_Nav_f( void )
{
	char	*cmd = gi.argv( 1 );

	if ( Q_stricmp( cmd, "show" ) == 0 )
	{
		cmd = gi.argv( 2 );

		if ( Q_stricmp( cmd, "all" ) == 0 )
		{
			NAVDEBUG_showNodes = !NAVDEBUG_showNodes;
			
			//NOTENOTE: This causes the two states to sync up if they aren't already
			NAVDEBUG_showCollision = NAVDEBUG_showNavGoals = 
			NAVDEBUG_showCombatPoints = NAVDEBUG_showEnemyPath = 
			NAVDEBUG_showEdges = NAVDEBUG_showNodes;		
		}
		else if ( Q_stricmp( cmd, "nodes" ) == 0 )
		{
			NAVDEBUG_showNodes = !NAVDEBUG_showNodes;
		}
		else if ( Q_stricmp( cmd, "edges" ) == 0 )
		{
			NAVDEBUG_showEdges = !NAVDEBUG_showEdges;
		}
		else if ( Q_stricmp( cmd, "testpath" ) == 0 )
		{
			NAVDEBUG_showTestPath = !NAVDEBUG_showTestPath;
		}
		else if ( Q_stricmp( cmd, "enemypath" ) == 0 )
		{
			NAVDEBUG_showEnemyPath = !NAVDEBUG_showEnemyPath;
		}
		else if ( Q_stricmp( cmd, "combatpoints" ) == 0 )
		{
			NAVDEBUG_showCombatPoints = !NAVDEBUG_showCombatPoints;
		}
		else if ( Q_stricmp( cmd, "navgoals" ) == 0 )
		{
			NAVDEBUG_showNavGoals = !NAVDEBUG_showNavGoals;
		}
		else if ( Q_stricmp( cmd, "collision" ) == 0 )
		{
			NAVDEBUG_showCollision = !NAVDEBUG_showCollision;
		}
	}
	else if ( Q_stricmp( cmd, "set" ) == 0 )
	{
		cmd = gi.argv( 2 );

		if ( Q_stricmp( cmd, "testgoal" ) == 0 )
		{
			NAVDEBUG_curGoal = navigator.GetNearestNode( &g_entities[0], g_entities[0].waypoint, NF_CLEAR_PATH );
		}
	}
	else if ( Q_stricmp( cmd, "totals" ) == 0 )
	{
		Com_Printf("Navigation Totals:\n");
		Com_Printf("------------------\n");
		Com_Printf("Total Nodes:         %d\n", navigator.GetNumNodes() );
		Com_Printf("Total Combat Points: %d\n", level.numCombatPoints );
	}
	else
	{
		//Print the available commands
		Com_Printf("nav - valid commands\n---\n" );
		Com_Printf("show\n - nodes\n - edges\n - testpath\n - enemypath\n - combatpoints\n - navgoals\n---\n");
		Com_Printf("set\n - testgoal\n---\n" );
	}
}

//
//JWEIER ADDITIONS START

bool	navCalculatePaths	= false;

bool	NAVDEBUG_showNodes			= false;
bool	NAVDEBUG_showEdges			= false;
bool	NAVDEBUG_showTestPath		= false;
bool	NAVDEBUG_showEnemyPath		= false;
bool	NAVDEBUG_showCombatPoints	= false;
bool	NAVDEBUG_showNavGoals		= false;
bool	NAVDEBUG_showCollision		= false;
int		NAVDEBUG_curGoal			= 0;

/*
-------------------------
NAV_CalculatePaths
-------------------------
*/

void NAV_CalculatePaths( const char *filename, int checksum )
{
	gentity_t	*ent = NULL;

#if _HARD_CONNECT

	gentity_t	*target = NULL;
	
	//Find all connections and hard connect them
	ent = NULL;
	while( ( ent = G_Find( ent, FOFS( classname ), "waypoint" ) ) != NULL )
	{
		//Find the first connection
		target = G_Find( NULL, FOFS( targetname ), ent->target );

		if ( target != NULL )
		{
			navigator.HardConnect( ent->health, target->health, (ent->spawnflags&1) );
		}

		//Find a possible second connection
		target = G_Find( NULL, FOFS( targetname ), ent->target2 );

		if ( target != NULL )
		{
			navigator.HardConnect( ent->health, target->health, (ent->spawnflags&1) );
		}

		//Find a possible third connection
		target = G_Find( NULL, FOFS( targetname ), ent->target3 );

		if ( target != NULL )
		{
			navigator.HardConnect( ent->health, target->health, (ent->spawnflags&1) );
		}

		//Find a possible fourth connection
		target = G_Find( NULL, FOFS( targetname ), ent->target4 );

		if ( target != NULL )
		{
			navigator.HardConnect( ent->health, target->health, (ent->spawnflags&1) );
		}
	}

#endif

	//Remove all waypoints now that they're done
	ent = NULL;
	while( ( ent = G_Find( ent, FOFS( classname ), "waypoint" ) ) != NULL )
	{
		//We don't need these to stay around
		G_FreeEntity( ent );
	}

	//Calculate the paths based on the supplied waypoints
	//navigator.CalculatePaths();

	//Save the resulting information
	/*
	if ( navigator.Save( filename, checksum ) == qfalse )
	{
		Com_Printf("Unable to save navigations data for map \"%s\" (checksum:%d)\n", filename, checksum );
	}
	*/
}

/*
-------------------------
NAV_Shutdown
-------------------------
*/

void NAV_Shutdown( void )
{
	navigator.Free();
}

/*
-------------------------
NAV_ShowDebugInfo
-------------------------
*/

void NAV_ShowDebugInfo( void )
{
	if ( NAVDEBUG_showNodes )
	{
		navigator.ShowNodes();
	}

	if ( NAVDEBUG_showEdges )
	{
		navigator.ShowEdges();
	}

	if ( NAVDEBUG_showTestPath )
	{
		//Get the nearest node to the player
		int	nearestNode = navigator.GetNearestNode( &g_entities[0], g_entities[0].waypoint, NF_ANY );
		int	testNode = navigator.GetBestNode( nearestNode, NAVDEBUG_curGoal );
		
		nearestNode = NAV_TestBestNode( g_entities[0].currentOrigin, nearestNode, testNode );

		//Show the connection
		vec3_t	dest, start;

		//Get the positions
		navigator.GetNodePosition( NAVDEBUG_curGoal, dest );
		navigator.GetNodePosition( nearestNode, start );

		CG_DrawNode( start, NODE_START );
		CG_DrawNode( dest, NODE_GOAL );
		navigator.ShowPath( nearestNode, NAVDEBUG_curGoal );
	}

	if ( NAVDEBUG_showCombatPoints )
	{
		for ( int i = 0; i < level.numCombatPoints; i++ )
		{
			CG_DrawCombatPoint( level.combatPoints[i].origin, 0 );
		}
	}

	if ( NAVDEBUG_showNavGoals )
	{
		TAG_ShowTags( RTF_NAVGOAL );
	}
}

/*
-------------------------
NAV_FindPlayerWaypoint
-------------------------
*/

void NAV_FindPlayerWaypoint( void )
{
	g_entities[0].waypoint = navigator.GetNearestNode( &g_entities[0], g_entities[0].lastWaypoint, NF_CLEAR_PATH );
}

//
//JWEIER ADDITIONS END