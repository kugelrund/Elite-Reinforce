#include "g_local.h"
#include "g_functions.h"

#define ENTDIST_PLAYER	1
#define ENTDIST_NPC		2

extern qboolean G_PointInBounds( vec3_t point, vec3_t mins, vec3_t maxs );
extern void CalcEntitySpot ( gentity_t *ent, spot_t spot, vec3_t point );
extern qboolean G_ClearTrace( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int ignore, int clipmask );
extern qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest );

void InitTrigger( gentity_t *self ) {
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	gi.SetBrushModel( self, self->model );
	self->contents = CONTENTS_TRIGGER;		// replaces the -1 from gi.SetBrushModel
	self->svFlags = SVF_NOCLIENT;

	if(self->spawnflags & 128)
	{
		self->svFlags |= SVF_INACTIVE;
	}
}


// the wait time has passed, so set back up for another activation
void multi_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger_run( gentity_t *ent ) 
{
	G_ActivateBehavior( ent, BSET_USE );

	if ( ent->soundSet && ent->soundSet[0] )
	{
		gi.SetConfigstring( CS_AMBIENT_SET, ent->soundSet );
	}

	G_UseTargets (ent, ent->activator);

	if ( ent->target2 && ent->target2[0] )
	{
		ent->e_ThinkFunc = thinkF_trigger_cleared_fire;
		ent->nextthink = level.time + ent->speed;
	}
	else if ( ent->wait > 0 ) 
	{
		ent->e_ThinkFunc = thinkF_multi_wait;
		ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
	} 
	else 
	{
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->e_TouchFunc = touchF_NULL;
		ent->e_UseFunc = useF_NULL;
		//Don't remove, Icarus may barf?
		//ent->nextthink = level.time + FRAMETIME;
		//ent->think = G_FreeEntity;
	}
}


void multi_trigger( gentity_t *ent, gentity_t *activator ) 
{
	if ( ent->nextthink ) 
	{
		return;		// can't retrigger until the wait is over
	}

	if ( ent->svFlags & SVF_INACTIVE )
	{//Not active at this time
		return;
	}

	ent->activator = activator;

	if(ent->delay)
	{//delay before firing trigger
		ent->e_ThinkFunc = thinkF_multi_trigger_run;
		ent->nextthink = level.time + ent->delay;
	}
	else
	{
		multi_trigger_run (ent);
	}
}

void Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator ) 
{
	multi_trigger( ent, activator );
}

void Touch_Multi( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
	if( !other->client ) 
	{
		return;
	}

	if ( self->svFlags & SVF_INACTIVE )
	{//set by target_deactivate
		return;
	}

	if( self->noDamageTeam )
	{
		if ( other->client->playerTeam != self->noDamageTeam )
		{
			return;
		}
	}

	if ( self->e_ThinkFunc == thinkF_trigger_cleared_fire )
	{//We're waiting to fire our target2 first
		self->nextthink = level.time + self->speed;
		return;
	}

	if ( self->trigger_formation )
	{//we only work on NPCs in formation
		if( !other->NPC || other->NPC->behaviorState != BS_FORMATION )
		{
			return;
		}
	}

	if ( self->spawnflags & 1 )
	{
		if ( other->s.number != 0 )
		{
			return;
		}
	}
	else
	{
		if ( self->spawnflags & 16 )
		{//NPCONLY
			if ( other->NPC == NULL )
			{
				return;
			}
		}

		if ( self->NPC_targetname && self->NPC_targetname[0] )
		{
			if ( other->script_targetname && other->script_targetname[0] )
			{
				if ( Q_stricmp( self->NPC_targetname, other->script_targetname ) != 0 )
				{//not the right guy to fire me off
					return;
				}
			}
			else
			{
				return;
			}
		}
	}

	if ( self->spawnflags & 2 )
	{//FACING
		vec3_t	forward;

		if ( other->client )
		{
			AngleVectors( other->client->ps.viewangles, forward, NULL, NULL );
		}
		else
		{
			AngleVectors( other->currentAngles, forward, NULL, NULL );
		}

		if ( DotProduct( self->movedir, forward ) < 0.5 )
		{//Not Within 45 degrees
			return;
		}
	}

	if ( self->spawnflags & 4 )
	{//USE_BUTTON
		if ( !other->client )
		{
			return;
		}

		if( !( other->client->usercmd.buttons & BUTTON_USE ) )
		{//not pressing use button
			return;
		}
	}

	if ( self->spawnflags & 8 )
	{//FIRE_BUTTON
		if ( !other->client )
		{
			return;
		}

		if( !( other->client->ps.eFlags & EF_FIRING /*usercmd.buttons & BUTTON_ATTACK*/ ) &&
			!( other->client->ps.eFlags & EF_ALT_FIRING/*usercmd.buttons & BUTTON_ALT_ATTACK*/ ) )
		{//not pressing fire button or altfire button
			return;
		}

		//FIXME: do we care about the sniper rifle or not?

		if( other->s.number == 0 && ( other->client->ps.weapon > MAX_PLAYER_WEAPONS || other->client->ps.weapon <= WP_NONE ) )
		{//don't care about non-player weapons if this is the player
			return;
		}
	}

	if ( other->client && self->radius )
	{
		vec3_t	eyeSpot;

		//Only works if your head is in it, but we allow leaning out
		//NOTE: We don't use CalcEntitySpot SPOT_HEAD because we don't want this
		//to be reliant on the physical model the player uses.
		VectorCopy(other->currentOrigin, eyeSpot);
		eyeSpot[2] += other->client->ps.viewheight;

		if ( G_PointInBounds( eyeSpot, self->absmin, self->absmax ) )
		{
			if( !( other->client->ps.eFlags & EF_FIRING ) &&
				!( other->client->ps.eFlags & EF_ALT_FIRING ) )
			{//not attacking, so hiding bonus
				//FIXME:  should really have sound events clear the hiddenDist
				other->client->hiddenDist = self->radius;
				//NOTE: movedir HAS to be normalized!
				if ( VectorLength( self->movedir ) )
				{//They can only be hidden from enemies looking in this direction
					VectorCopy( self->movedir, other->client->hiddenDir );
				}
				else
				{
					VectorClear( other->client->hiddenDir );
				}
			}
		}
	}

	multi_trigger( self, other );
}

void trigger_cleared_fire (gentity_t *self)
{
	G_UseTargets2( self, self->activator, self->target2 );
	self->e_ThinkFunc = thinkF_NULL;
}

/*QUAKED trigger_multiple (.5 .5 .5) ? PLAYERONLY FACING USE_BUTTON FIRE_BUTTON NPCONLY x x INACTIVE
PLAYERONLY - only a player can trigger this by touch
FACING - Won't fire unless triggering ent's view angles are within 45 degrees of trigger's angles (in addition to any other conditions)
USE_BUTTON - Won't fire unless player is in it and pressing use button (in addition to any other conditions)
FIRE_BUTTON - Won't fire unless player/NPC is in it and pressing fire button (in addition to any other conditions)
NPCONLY - only non-player NPCs can trigger this by touch
INACTIVE - Start off, has to be activated to be touchable/usable

"wait" : Seconds between triggerings, 0.5 default, number<=0 means one time only.
"random"	wait variance, default is 0
"delay"		how many seconds to wait to fire targets after tripped
"hiderange" As long as NPC's head is in this trigger, NPCs out of this hiderange cannot see him.  If you set an angle on the trigger, they're only hidden from enemies looking in that direction.  the player's crouch viewheight is 36, his standing viewheight is 54.  So a trigger thast should hide you when crouched but not standing should be 48 tall.
"target2" The trigger will fire this only when the trigger field has been crossed and subsequently cleared.  This will not fire the "target" more than once until the "target2" is fired (trigger field is cleared)
"speed" How many seconds to wait to fire the target2, default is 1

Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"NPC_targetname" - If set, only an NPC with a matching NPC_targetname will trip this trigger
"team" - If set, only this team can trip this trigger
	starfleet
	borg
	parasite
	scavengers (use this for all scavengers for now...)
	klingon
	malon
	hirogen
	imperial
	stasis
	species8472
	dreadnought
	forge

"soundSet"	Ambient sound set to play when this trigger is activated
*/
team_t TranslateTeamName( const char *name );
void SP_trigger_multiple( gentity_t *ent ) 
{
	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( (ent->wait > 0) && (ent->random >= ent->wait) ) {
		ent->random = ent->wait - FRAMETIME;
		gi.Printf(S_COLOR_YELLOW"trigger_multiple has random >= wait\n");
	}

	ent->delay *= 1000;//1 = 1 msec, 1000 = 1 sec
	if ( !ent->speed && ent->target2 && ent->target2[0] )
	{
		ent->speed = 1000;
	}
	else
	{
		ent->speed *= 1000;
	}

	ent->e_TouchFunc = touchF_Touch_Multi;
	ent->e_UseFunc   = useF_Use_Multi;

	if ( ent->team && ent->team[0] )
	{
		ent->noDamageTeam = TranslateTeamName( ent->team );
		ent->team = NULL;
	}

	InitTrigger( ent );
	gi.linkentity (ent);
}

/*QUAKED trigger_once (.5 1 .5) ? PLAYERONLY FACING USE_BUTTON FIRE_BUTTON NPCONLY x x INACTIVE
PLAYERONLY - only a player can trigger this by touch
FACING - Won't fire unless triggering ent's view angles are within 45 degrees of trigger's angles (in addition to any other conditions)
USE_BUTTON - Won't fire unless player is in it and pressing use button (in addition to any other conditions)
FIRE_BUTTON - Won't fire unless player/NPC is in it and pressing fire button (in addition to any other conditions)
NPCONLY - only non-player NPCs can trigger this by touch
INACTIVE - Start off, has to be activated to be touchable/usable

"random"	wait variance, default is 0
"delay"		how many seconds to wait to fire targets after tripped
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"NPC_targetname" - If set, only an NPC with a matching NPC_targetname will trip this trigger
"team" - If set, only this team can trip this trigger
	starfleet
	borg
	parasite
	scavengers (use this for all scavengers for now...)
	stasis
	species8472
	dreadnought
	forge

"soundSet"	Ambient sound set to play when this trigger is activated
*/
void SP_trigger_once( gentity_t *ent ) 
{
	ent->wait = -1;

	ent->e_TouchFunc = touchF_Touch_Multi;
	ent->e_UseFunc   = useF_Use_Multi;

	if ( ent->team && ent->team[0] )
	{
		ent->noDamageTeam = TranslateTeamName( ent->team );
		ent->team = NULL;
	}

	ent->delay *= 1000;//1 = 1 msec, 1000 = 1 sec

	InitTrigger( ent );
	gi.linkentity (ent);
}


/*QUAKED trigger_formation (.5 .5 .5) ? x FACING USE_BUTTON FIRE_BUTTON x x x INACTIVE
FACING - Won't fire unless triggering ent's view angles are within 45 degrees of trigger's angles (in addition to any other conditions)
USE_BUTTON - Won't fire unless player is in it and pressing use button (in addition to any other conditions)
FIRE_BUTTON - Won't fire unless player/NPC is in it and pressing fire button (in addition to any other conditions)
INACTIVE - Start off, has to be activated to be touchable/usable

Only NPC's (non-players) in formation (bState = BS_FORMATION) can trip this trigger
TRIGGERS ONLY ONCE BY DEFAULT!

INACTIVE - Start off, has to be activated to be touchable/usable

"NPC_targetname" - If set, only an NPC with a matching NPC_targetname will trip this trigger

"wait" - how long to wait between triggerings, default is -1 

  TODO:
	count
	delay
*/
void SP_trigger_formation( gentity_t *ent ) 
{
	//Remove "playeronly" if on by accident
	ent->spawnflags &= ~1;

	if(!ent->wait)
	{
		ent->wait = -1;
	}

	ent->e_TouchFunc = touchF_Touch_Multi;
	ent->e_UseFunc   = useF_Use_Multi;
	
	//This is how it knows to look only for NPCs in BS_FORMATION
	ent->trigger_formation = qtrue;

	InitTrigger( ent );
	gi.linkentity (ent);
}

/*QUAKED trigger_bidirectional (.5 .5 .5) ? PLAYER_ONLY x x x x x x INACTIVE
NOT IMPLEMENTED
INACTIVE - Start off, has to be activated to be touchable/usable

set "angle" for forward direction
Fires "target" when someone moves through it in direction of angle
Fires "backwardstarget" when someone moves through it in the opposite direction of angle

"NPC_targetname" - If set, only an NPC with a matching NPC_targetname will trip this trigger

"wait" - how long to wait between triggerings

  TODO:
	count
*/
void SP_trigger_bidirectional( gentity_t *ent ) 
{
	G_FreeEntity(ent);
	//FIXME: Implement
/*	if(!ent->wait)
	{
		ent->wait = -1;
	}

	ent->touch = Touch_Multi;
	ent->use = Use_Multi;
	
	InitTrigger( ent );
	gi.linkentity (ent);
*/
}

/*QUAKED trigger_location (.5 .5 .5) ? 
When an ent is asked for it's location, it will return this ent's "message" field if it is in it.
  "message" - location name

  NOTE: always rectangular
*/
char *G_GetLocationForEnt( gentity_t *ent )
{
	vec3_t		mins, maxs;
	gentity_t	*found = NULL;

	VectorAdd( ent->currentOrigin, ent->mins, mins );
	VectorAdd( ent->currentOrigin, ent->maxs, maxs );

	while( (found = G_Find(found, FOFS(classname), "trigger_location")) != NULL )
	{
		if ( gi.EntityContact( mins, maxs, found ) ) 
		{
			return found->message;
		}
	}

	return NULL;
}

void SP_trigger_location( gentity_t *ent ) 
{
	if ( !ent->message || !ent->message[0] )
	{
		gi.Printf("WARNING: trigger_location with no message!\n");
		G_FreeEntity(ent);
		return;
	}

	gi.SetBrushModel( ent, ent->model );
	ent->contents = 0;
	ent->svFlags = SVF_NOCLIENT;

	gi.linkentity (ent);
}
/*
==============================================================================

trigger_always

==============================================================================
*/

void trigger_always_think( gentity_t *ent ) {
	G_UseTargets(ent, ent);
	G_FreeEntity( ent );
}

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (gentity_t *ent) {
	// we must have some delay to make sure our use targets are present
	ent->nextthink = level.time + 300;
	ent->e_ThinkFunc = thinkF_trigger_always_think;
}


/*
==============================================================================

trigger_push

==============================================================================
*/

void trigger_push_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	if ( self->svFlags & SVF_INACTIVE )
	{//set by target_deactivate
		return;
	}

	if ( self->painDebounceTime > level.time )
	{
		return;
	}

	if ( !other->client ) {
		return;
	}

	if ( other->client->ps.pm_type != PM_NORMAL ) {
		return;
	}
	
	if ( self->spawnflags & 1 )
	{//PLAYERONLY
		if ( other->s.number != 0 )
		{
			return;
		}
	}
	else
	{
		if ( self->spawnflags & 16 )
		{//NPCONLY
			if ( other->NPC == NULL )
			{
				return;
			}
		}
	}

	VectorCopy (self->s.origin2, other->client->ps.velocity);

	if ( self->wait == -1 )
	{
		self->e_TouchFunc = touchF_NULL;
	}
	else if ( self->wait > 0 )
	{
		self->painDebounceTime = level.time + self->wait;
	}
}


/*
=================
AimAtTarget

Calculate origin2 so the target apogee will be hit
=================
*/
void AimAtTarget( gentity_t *self ) {
	gentity_t	*ent;
	vec3_t		origin;
	float		height, gravity, time, forward;
	float		dist;

	VectorAdd( self->absmin, self->absmax, origin );
	VectorScale ( origin, 0.5, origin );

	ent = G_PickTarget( self->target );
	if ( !ent ) {
		G_FreeEntity( self );
		return;
	}

	height = ent->s.origin[2] - origin[2];
	if ( height < 0 )
	{//sqrt of negative is bad!
		height = 0;
	}
	gravity = g_gravity->value;
	time = sqrt( height / ( .5 * gravity ) );
	if ( !time ) {
		G_FreeEntity( self );
		return;
	}

	// set s.origin2 to the push velocity
	VectorSubtract ( ent->s.origin, origin, self->s.origin2 );
	self->s.origin2[2] = 0;
	dist = VectorNormalize( self->s.origin2);

	forward = dist / time;
	VectorScale( self->s.origin2, forward, self->s.origin2 );

	self->s.origin2[2] = time * gravity;
}


/*QUAKED trigger_push (.5 .5 .5) ? PLAYERONLY x x x NPCONLY x x INACTIVE
Must point at a target_position, which will be the apex of the leap.
This will be client side predicted, unlike target_push
PLAYERONLY - only the player is affected
NPCONLY - only NPCs are affected
INACTIVE - not active until targeted by a target_activate

wait - how long to wait between pushes: -1 = push only once
*/
void SP_trigger_push( gentity_t *self ) {
	InitTrigger (self);

	if ( self->wait > 0 )
	{
		self->wait *= 1000;
	}

	// unlike other triggers, we need to send this one to the client
	self->svFlags &= ~SVF_NOCLIENT;

	self->s.eType = ET_PUSH_TRIGGER;
	self->e_TouchFunc = touchF_trigger_push_touch;
	self->e_ThinkFunc = thinkF_AimAtTarget;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}


void Use_target_push( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( !activator->client ) {
		return;
	}

	if ( activator->client->ps.pm_type != PM_NORMAL ) {
		return;
	}

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	VectorCopy (self->s.origin2, activator->client->ps.velocity);

	// play fly sound every 1.5 seconds
	if ( self->noise_index && activator->fly_sound_debounce_time < level.time ) {
		activator->fly_sound_debounce_time = level.time + 1500;
		G_Sound( activator, self->noise_index );
	}
}

/*QUAKED target_push (.5 .5 .5) (-8 -8 -8) (8 8 8) ENERGYNOISE
When triggered, pushes the activator in the direction of angles
"speed"		defaults to 1000
ENERGYNOISE plays energy noise
*/
void SP_target_push( gentity_t *self ) {
	if (!self->speed) {
		self->speed = 1000;
	}
	G_SetMovedir (self->s.angles, self->s.origin2);
	VectorScale (self->s.origin2, self->speed, self->s.origin2);

	if ( self->spawnflags & 1 ) {
		self->noise_index = G_SoundIndex("sound/ambience/forge/antigrav.wav");
	}
	if ( self->target ) {
		VectorCopy( self->s.origin, self->absmin );
		VectorCopy( self->s.origin, self->absmax );
		self->e_ThinkFunc = thinkF_AimAtTarget;
		self->nextthink = level.time + FRAMETIME;
	}
	self->e_UseFunc = useF_Use_target_push;
}

/*
==============================================================================

trigger_teleport

==============================================================================
*/
#define SNAP_ANGLES 1
#define NO_MISSILES 2
#define NO_NPCS		4
void TeleportMover( gentity_t *mover, vec3_t origin, vec3_t diffAngles, qboolean snapAngle );
void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace ) 
{
	gentity_t	*dest;

	if ( self->svFlags & SVF_INACTIVE )
	{//set by target_deactivate
		return;
	}
	
	dest = 	G_PickTarget( self->target );
	if (!dest) 
	{
		gi.Printf ("Couldn't find teleporter destination\n");
		return;
	}

	if ( other->client ) 
	{
		if ( other->client->ps.pm_type == PM_DEAD ) 
		{
			return;
		}
		if ( other->NPC )
		{
			if ( self->spawnflags & NO_NPCS )
			{
				return;
			}

			if ( other->NPC->behaviorState == BS_FORMATION && other->client->team_leader != other && !VectorLengthSquared( other->NPC->leaderTeleportSpot ) )
			{//When following someone in formation, don't go through teleporters on your own
				return;
			}
		}

		if ( other->client->playerTeam != TEAM_FREE && SpotWouldTelefrag2( other, dest->currentOrigin ) )//SpotWouldTelefrag( dest, other->client->playerTeam ) )
		{//Don't go through if something blocking on the other side
			return;
		}
		
		TeleportPlayer( other, dest->s.origin, dest->s.angles, self->count );
	}
	//FIXME: check for SVF_NO_TELEPORT
	else if ( !(self->svFlags & SVF_NO_TELEPORT) && !(self->spawnflags & NO_MISSILES) && VectorLengthSquared( other->s.pos.trDelta ) )
	{//It's a mover of some sort and is currently moving
		vec3_t	diffAngles = {0, 0, 0};
		qboolean	snap = qfalse;

		if ( self->lastEnemy )
		{
			VectorSubtract( dest->s.angles, self->lastEnemy->s.angles, diffAngles );
		}
		else
		{//snaps to angle
			VectorSubtract( dest->s.angles, other->currentAngles, diffAngles );
			snap = qtrue;
		}

		TeleportMover( other, dest->s.origin, diffAngles, snap );
	}
}

void trigger_teleporter_find_closest_portal( gentity_t *self )
{
	gentity_t *found = NULL;
	vec3_t		org, vec;
	float		dist, bestDist = 64*64;

	VectorAdd( self->mins, self->maxs, org );
	VectorScale( org, 0.5, org );
	while ( (found = G_Find( found, FOFS(classname), "misc_portal_surface" )) != NULL )
	{
		VectorSubtract( found->currentOrigin, org, vec );
		dist = VectorLengthSquared( vec );
		if ( dist < bestDist )
		{
			self->lastEnemy = found;
			bestDist = dist;
		}
	}

	if ( self->lastEnemy )
	{
		gi.Printf("trigger_teleporter found misc_portal_surface\n");
	}
	self->e_ThinkFunc = thinkF_NULL;
}

/*QUAKED trigger_teleport (.5 .5 .5) ? SNAP_ANGLES NO_MISSILES NO_NPCS STASIS x x x INACTIVE
Allows client side prediction of teleportation events.
Must point at a target_position, which will be the teleport destination.

SNAP_ANGLES - Make the entity that passes through snap to the target_position's angles
NO_MISSILES - Missiles and thrown objects cannot pass through
NO_NPCS - NPCs cannot pass through
STASIS - will play stasis teleport sound and fx instead of starfleet
*/
void SP_trigger_teleport( gentity_t *self ) 
{
	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	self->svFlags &= ~SVF_NOCLIENT;

	self->s.eType = ET_TELEPORT_TRIGGER;
	self->e_TouchFunc = touchF_trigger_teleporter_touch;

	self->e_ThinkFunc = thinkF_trigger_teleporter_find_closest_portal;
	self->nextthink = level.time + FRAMETIME;

	if ( self->spawnflags & 8 ) // STASIS
	{
		self->count = STASIS_TELEPORT_FX;
		// make sure the client precaches this sound
		G_SoundIndex( "sound/movers/stasistransporter.wav" );
	}
	else
	{
		self->count = STARFLEET_TELEPORT_FX;
		// make sure the client precaches this sound
	}

	gi.linkentity (self);
}



/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF - SILENT NO_PROTECTION SLOW FALLING - INACTIVE
Any entity that touches this will be hurt.
It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage
FALLING			Forces a falling scream and anim

"dmg"			default 5 (whole numbers only)
"delay"			How many seconds it takes to get from 0 to "dmg" (default is 0)
"wait"			Use in instead of "SLOW" - determines how often the player gets hurt, 0.1 is every frame, 1.0 is once per second.  -1 will stop after one use
*/
void hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	//FIXME: Targeting the trigger will toggle its on / off state???
	if ( self->linked ) {
		gi.unlinkentity( self );
	} else {
		gi.linkentity( self );
	}
}

void trigger_hurt_reset (gentity_t *self)
{
	self->attackDebounceTime = 0;
	self->e_ThinkFunc = thinkF_NULL;
}

void hurt_touch( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
	int		dflags;
	int		actualDmg = self->damage;

	if ( self->svFlags & SVF_INACTIVE )
	{//set by target_deactivate
		return;
	}
	
	if ( !other->takedamage ) 
	{
		return;
	}

	if ( self->painDebounceTime > level.time ) 
	{
		return;
	}

	//FIXME: only allows it to hurt one person at a time here!
	if ( self->spawnflags & 16 ) 
	{
		self->painDebounceTime = level.time + 1000;
	} 
	else if ( self->wait > 0 )
	{
		self->painDebounceTime = level.time + self->wait;
	}
	else 
	{
		self->painDebounceTime = level.time + FRAMETIME;
	}

	// play sound
	if ( !(self->spawnflags & 4) ) 
	{
		G_Sound( other, self->noise_index );
	}

	if ( self->spawnflags & 8 )
	{
		dflags = DAMAGE_NO_PROTECTION;
	}
	else
	{
		dflags = 0;
	}
	
	if ( self->delay )
	{//Increase dmg over time
		if ( self->attackDebounceTime < self->delay )
		{//FIXME: this is for the entire trigger, not per person, so if someone else jumped in after you were in it for 5 seconds, they'd get damaged faster
			actualDmg = floor( self->damage * self->attackDebounceTime / self->delay );
		}
		self->attackDebounceTime += FRAMETIME;

		self->e_ThinkFunc = thinkF_trigger_hurt_reset;
		self->nextthink = level.time + FRAMETIME*2;
	}

	if ( actualDmg )
	{
		if(self->spawnflags & 32)
		{//falling death
			G_Damage (other, self, self, NULL, NULL, actualDmg, dflags, MOD_FALLING);
		}
		else
		{
			G_Damage (other, self, self, NULL, NULL, actualDmg, dflags, MOD_TRIGGER_HURT);
		}
	}

	if ( self->wait < 0 )
	{
		self->e_TouchFunc = touchF_NULL;
	}
}

void SP_trigger_hurt( gentity_t *self ) {
	InitTrigger (self);

	self->noise_index = G_SoundIndex( "sound/world/electro.wav" );
	self->e_TouchFunc = touchF_hurt_touch;

	if ( !self->damage ) {
		self->damage = 5;
	}
	
	self->delay *= 1000;
	self->wait *= 1000;

	self->contents = CONTENTS_TRIGGER;

	//WHAT?  Shouldn't this be if self->targetname????
	if ( self->spawnflags & 2 ) {
		self->e_UseFunc = useF_hurt_use;
	}

	// link in to the world if starting active
	if ( ! (self->spawnflags & 1) ) {
		gi.linkentity (self);
	}
}


/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
This should be renamed trigger_timer...
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

*/
void func_timer_think( gentity_t *self ) {
	G_UseTargets (self, self->activator);
	// set time before next firing
	self->nextthink = level.time + 1000 * ( self->wait + crandom() * self->random );
}

void func_timer_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	self->activator = activator;

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}


	// if on, turn it off
	if ( self->nextthink ) {
		self->nextthink = 0;
		return;
	}

	// turn it on
	func_timer_think (self);
}

void SP_func_timer( gentity_t *self ) {
	G_SpawnFloat( "random", "1", &self->random);
	G_SpawnFloat( "wait", "1", &self->wait );

	self->e_UseFunc   = useF_func_timer_use;
	self->e_ThinkFunc = thinkF_func_timer_think;

	if ( self->random >= self->wait ) {
		self->random = self->wait - FRAMETIME;
		gi.Printf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ) );
	}

	if ( self->spawnflags & 1 ) {
		self->nextthink = level.time + FRAMETIME;
		self->activator = self;
	}

	self->svFlags = SVF_NOCLIENT;
}

/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED trigger_entdist (.5 .5 .5) (-8 -8 -8) (8 8 8) PLAYER NPC
fires if the given entity is within the given distance.  Sets itself inactive after one use.
----- KEYS -----
distance - radius entity can be away to fire trigger
target - fired if entity is within distance
target2 - fired if entity not within distance

NPC_target - NPC_types to look for
ownername - If any, who to calc the distance from- default is the trigger_entdist himself
example: target "biessman telsia" will look for the biessman and telsia NPC
if it finds either of these within distance it will fire.

  todo - 
  add delay, count
  add monster classnames?????
  add LOS to it???
*/

void trigger_entdist_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	vec3_t		diff;
	gentity_t	*found = NULL;
	gentity_t	*owner = NULL;
	qboolean	useflag;
	char		*token, *holdString;

	if ( self->svFlags & SVF_INACTIVE )	// Don't use INACTIVE
		return;

	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	if(self->ownername && self->ownername[0])
	{
		owner = G_Find(NULL, FOFS(targetname), self->ownername);
	}

	if(owner == NULL)
	{
		owner = self;
	}

	self->activator = activator;

	useflag = qfalse;

	self->svFlags |= SVF_INACTIVE;	// Make it inactive after one use

	if (self->spawnflags & ENTDIST_PLAYER)	// Look for player???
	{
		found = &g_entities[0];

		if (found)
		{	
			VectorSubtract(owner->currentOrigin, found->currentOrigin, diff);
			if(VectorLength(diff) < self->count)
			{
				useflag = qtrue;
			}
		}
	}

	if ((self->spawnflags & ENTDIST_NPC) && (!useflag))
	{
		holdString = self->NPC_target;

		while (holdString)
		{
			token = COM_Parse( &holdString);
			if ( !token ) // Nothing left to look at
			{
				break;
			}

			found = G_Find(found, FOFS(targetname), token);	// Look for the specified NPC
			if (found)	//Found???
			{	
				VectorSubtract(owner->currentOrigin, found->currentOrigin, diff);
				if(VectorLength(diff) < self->count)	// Within distance
				{
					useflag = qtrue;
					break;
				}
			}
		}
	}

	if (useflag)
	{
		G_UseTargets2 (self, self->activator, self->target);
	}
	else if (self->target2)
	{
		// This is the negative target
		G_UseTargets2 (self, self->activator, self->target2);
	}	


}

void SP_trigger_entdist( gentity_t *self ) 
{
	G_SpawnInt( "distance", "0", &self->count);

	self->e_UseFunc = useF_trigger_entdist_use;

}



void trigger_visible_check_player_visibility( gentity_t *self )
{
	//Check every FRAMETIME*2
	self->nextthink = level.time + FRAMETIME*2;

	if ( self->svFlags & SVF_INACTIVE )
	{
		return;
	}

	vec3_t	dir;
	float	dist;
	gentity_t	*player = &g_entities[0];

	if (!player || !player->client )
	{
		return;
	}

	//1: see if player is within 512*512 range
	VectorSubtract( self->currentOrigin, player->client->renderInfo.eyePoint, dir );
	dist = VectorNormalize( dir );
	if ( dist < self->radius )
	{//Within range
		vec3_t	forward;
		float	dot;
		//2: see if dot to us and player viewangles is > 0.7
		AngleVectors( player->client->renderInfo.eyeAngles, forward, NULL, NULL );
		dot = DotProduct( forward, dir );
		if ( dot > self->random )
		{//Within the desired FOV
			//3: see if player is in PVS
			if ( gi.inPVS( self->currentOrigin, player->client->renderInfo.eyePoint ) )
			{
				vec3_t	mins = {-1, -1, -1};
				vec3_t	maxs = {1, 1, 1};
				//4: If needbe, trace to see if there is clear LOS from player viewpos
				if ( (self->spawnflags&1) || G_ClearTrace( player->client->renderInfo.eyePoint, mins, maxs, self->currentOrigin, 0, MASK_OPAQUE ) )
				{
					//5: Fire!
					G_UseTargets( self, player );
					//6: Remove yourself
					G_FreeEntity( self );
				}
			}
		}
	}

}

/*QUAKED trigger_visible (.5 .5 .5) (-8 -8 -8) (8 8 8) NOTRACE x x x x x x INACTIVE

  Only fires when player is looking at it, fires only once then removes itself.

  NOTRACE - Doesn't check to make sure the line of sight is completely clear (penetrates walls, forcefields, etc)
  INACTIVE - won't check for player visibility until activated

  radius - how far this ent can be from player's eyes, max, and still be considered "seen"
  FOV - how far off to the side of the player's field of view this can be, max, and still be considered "seen".  Player FOV is 80, so the default for this value is 30.

  "target" - What to use when it fires.
*/
void SP_trigger_visible( gentity_t *self )
{
	if ( self->radius <= 0 )
	{
		self->radius = 512;
	}

	if ( self->random <= 0 )
	{//about 30 degrees
		self->random = 0.7f;
	}
	else
	{//convert from FOV degrees to number meaningful for dot products
		self->random = 1.0f - (self->random/90.0f);
	}

	if ( self->spawnflags & 128 )
	{// Make it inactive
		self->svFlags |= SVF_INACTIVE;	
	}

	G_SetOrigin( self, self->s.origin );
	gi.linkentity( self );

	self->e_ThinkFunc = thinkF_trigger_visible_check_player_visibility;
	self->nextthink = level.time + FRAMETIME*2;
}