#include "g_local.h"
#include "g_functions.h"
#include "b_local.h"

extern team_t TranslateTeamName( const char *name );
extern	cvar_t	*g_spskill;

//client side shortcut hacks from cg_local.h
extern void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );
extern void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );
extern void CG_FireLaser( vec3_t start, vec3_t end, vec3_t normal, vec4_t laserRGB, qboolean hit_ent );
extern void CG_AimLaser( vec3_t start, vec3_t end, vec3_t normal );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );


#define	ARM_ANGLE_RANGE		60
#define	HEAD_ANGLE_RANGE	90
#define	TURR_FOFS	18.0f
#define	TURR_ROFS	0.0f
#define	TURR_UOFS	12.0f
#define	ARM_FOFS	0.0f
#define	ARM_ROFS	0.0f
#define	ARM_UOFS	0.0f
#define	FARM_FOFS	14.0f
#define	FARM_ROFS	0.0f
#define	FARM_UOFS	4.0f
#define	FTURR_FOFS	0.0f
#define	FTURR_ROFS	0.0f
#define	FTURR_UOFS	6.0f
#define	LARM_FOFS	2.0f
#define	LARM_ROFS	0.0f
#define	LARM_UOFS	-26.0f

void turret_die ( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	vec3_t	dir;

	//Turn off the thinking of the base & use it's targets
	self->activator->e_ThinkFunc = thinkF_NULL;
	self->activator->e_UseFunc = useF_NULL;
	if ( self->activator->target )
	{
		G_UseTargets( self->activator, attacker );
	}

	//Remove the arm
	G_FreeEntity( self->owner );

	//clear my data
	self->e_DieFunc  = dieF_NULL;
	self->e_ThinkFunc  = thinkF_NULL;
	self->takedamage = qfalse;
	self->health = 0;

	//Throw some chunks
	AngleVectors( self->activator->currentAngles, dir, NULL, NULL );
	VectorNormalize( dir );

	CG_Chunks( self->s.number, self->currentOrigin, dir, Q_flrand(150, 300), Q_irand(3, 7), self->material, -1, 1.0 );

	if ( self->splashDamage > 0 && self->splashRadius > 0 )
	{//FIXME: specify type of explosion?  (barrel, electrical, etc.)
		G_RadiusDamage( self->currentOrigin, self->activator, self->splashDamage, self->splashRadius, self->activator, MOD_UNKNOWN );

		CG_SurfaceExplosion(self->currentOrigin, dir, 20.0f, 12.0f, qtrue );
		G_Sound(self->activator, G_SoundIndex("sound/weapons/explosions/explode11.wav"));
	}

	if ( self->noDamageTeam == TEAM_FORGE )
	{
		self->activator->s.modelindex = self->activator->s.modelindex2;
	}
	G_FreeEntity( self );
}

#define FORGE_TURRET_DAMAGE			2
#define FORGE_TURRET_SPLASH_RAD		64
#define FORGE_TURRET_SPLASH_DAM		4
#define FORGE_TURRET_VELOCITY		500

void fturret_fire ( gentity_t *ent, vec3_t start, vec3_t dir )
{
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "forge_projectile";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_FORGE_PROJ;
	bolt->owner = ent;
	bolt->damage = FORGE_TURRET_DAMAGE+(3*g_spskill->value);
	bolt->dflags = 0;
	bolt->splashDamage = FORGE_TURRET_SPLASH_DAM;
	bolt->splashRadius = FORGE_TURRET_SPLASH_RAD;
	bolt->methodOfDeath = MOD_ENERGY;
	bolt->splashMethodOfDeath = MOD_ENERGY_SPLASH;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, FORGE_TURRET_VELOCITY, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->currentOrigin );
}

void turret_fire ( gentity_t *ent, vec3_t start, vec3_t dir )
{
	gentity_t	*bolt;

	bolt = G_Spawn();
	
	bolt->classname = "turret_proj";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->s.weapon = WP_DN_TURRET;
	bolt->owner = ent;
	bolt->damage = ent->damage;
	bolt->dflags = DAMAGE_NO_KNOCKBACK;		// Don't push them around, or else we are constantly re-aiming
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_SCAVENGER;	// ?
	bolt->clipmask = MASK_SHOT;
	bolt->trigger_formation = qfalse;		// don't draw tail on first frame	

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, 1100, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );		// save net bandwidth
	VectorCopy( start, bolt->currentOrigin);
}

void turret_head_think (gentity_t *self)
{
	qboolean	fire_now = qfalse;

	if ( !(self->activator->spawnflags & 2) )
	{//because forge turret heads have no anims... sigh...
		//animate
		if ( self->activator->enemy || self->painDebounceTime > level.time || self->s.frame )
		{
			self->s.frame++;
			if ( self->s.frame > 10 )
			{
				self->s.frame = 0;
			}

			if ( self->s.frame == 0 || self->s.frame == 4 )
			{
				fire_now = qtrue;
			}
		}
	}
	else
	{
		if ( self->pushDebounceTime < level.time )
		{
			self->pushDebounceTime = level.time + self->wait * 10;
			fire_now = qtrue;
		}
	}

	//Fire
	if ( fire_now && self->activator->enemy && self->attackDebounceTime < level.time )
	{//Only fire if ready to
		vec3_t	forward, right, up, muzzleSpot;
		float	rOfs = 0;

		AngleVectors(self->currentAngles, forward, right, up);
		VectorMA( self->currentOrigin, 16, forward, muzzleSpot );
		VectorMA( self->currentOrigin, 8, up, muzzleSpot );
		if ( !(self->activator->spawnflags & 2) )
		{//DN turrets have offsets
			if ( self->s.frame == 0 )
			{//Fire left barrel
				rOfs = -6;
			}
			else if ( self->s.frame == 4 )
			{//Fire right barrel
				rOfs = 6;
			}
		}

		VectorMA( self->currentOrigin, rOfs, right, muzzleSpot );

		
		if ( self->noDamageTeam == TEAM_FORGE )
		{//FIXME: do different attack than DN
			G_Sound(self, G_SoundIndex("sound/enemies/turret/ffire.wav"));
			fturret_fire( self, muzzleSpot, forward );
		}
		else
		{
			G_Sound(self, G_SoundIndex("sound/enemies/turret/fire.wav"));
			turret_fire( self, muzzleSpot, forward );
		}
	}

	//next think
	self->nextthink = level.time + self->wait;
}

void bolt_head_to_arm( gentity_t *arm, gentity_t *head, float fwdOfs, float rtOfs, float upOfs )
{
	vec3_t	headOrg, forward, right, up;

	AngleVectors( arm->currentAngles, forward, right, up );
	VectorMA( arm->currentOrigin, fwdOfs, forward, headOrg );
	VectorMA( headOrg, rtOfs, right, headOrg );
	VectorMA( headOrg, upOfs, up, headOrg );
	G_SetOrigin( head, headOrg );
	head->currentAngles[1] = head->s.apos.trBase[1] = head->s.angles[1] = arm->currentAngles[1];
	gi.linkentity( head );
}

void bolt_arm_to_base( gentity_t *base, gentity_t *arm, float fwdOfs, float rtOfs, float upOfs )
{
	vec3_t	headOrg, forward, right, up;

	AngleVectors( base->currentAngles, forward, right, up );
	VectorMA( base->currentOrigin, fwdOfs, forward, headOrg );
	VectorMA( headOrg, rtOfs, right, headOrg );
	VectorMA( headOrg, upOfs, up, headOrg );
	G_SetOrigin( arm, headOrg );
	gi.linkentity( arm );
	G_SetAngles( arm, base->currentAngles );
}

void rebolt_turret( gentity_t *base )
{
	vec3_t	headOrg, forward, right, up;

	if ( !base->lastEnemy )
	{//no arm
		return;
	}

	if ( !base->lastEnemy->lastEnemy )
	{//no head
		return;
	}

	if ( base->spawnflags&2 )
	{
		bolt_arm_to_base( base, base->lastEnemy, FARM_FOFS, FARM_ROFS, FARM_UOFS );
		bolt_head_to_arm( base->lastEnemy, base->lastEnemy->lastEnemy, FTURR_FOFS, FTURR_ROFS, FTURR_UOFS );
	}
	else
	{
		//FIXME: maybe move these seperately so they interpolate?
		G_SetOrigin( base->lastEnemy, base->s.pos.trBase );
		gi.linkentity(base->lastEnemy);
		//G_SetAngles( base->lastEnemy, base->currentAngles );
		AngleVectors( base->lastEnemy->currentAngles, forward, right, up );
		VectorMA( base->lastEnemy->currentOrigin, TURR_FOFS, forward, headOrg );
		VectorMA( headOrg, TURR_ROFS, right, headOrg );
		VectorMA( headOrg, TURR_UOFS, up, headOrg );
		G_SetOrigin( base->lastEnemy->lastEnemy, headOrg );
		//base->lastEnemy->lastEnemy->currentAngles[1] = base->lastEnemy->lastEnemy->s.apos.trBase[1] = base->lastEnemy->lastEnemy->s.angles[1] = base->lastEnemy->currentAngles[1];
		gi.linkentity( base->lastEnemy->lastEnemy );
	}
}
/*
void turret_aim( gentity_t *self )

  aims arm and head at enemy or neutral position
*/
void turret_aim( gentity_t *self )
{
	vec3_t	enemyDir;
	vec3_t	desiredAngles;
	float	diffAngle, armAngleDiff, headAngleDiff;
	//qboolean	turned = qfalse;
	int		upTurn = 0;
	int		yawTurn = 0;

	if ( self->enemy )
	{//Aim at enemy
		VectorSubtract( self->enemy->currentOrigin, self->currentOrigin, enemyDir );
		vectoangles( enemyDir, desiredAngles );
	}
	else
	{//Return to front
		VectorCopy( self->currentAngles, desiredAngles );
	}

	//yaw-aim arm at enemy at speed
	//FIXME: noise when turning?
	diffAngle = AngleSubtract(desiredAngles[1], self->lastEnemy->currentAngles[1]);
	if ( diffAngle )
	{
		if ( fabs(diffAngle) < self->speed )
		{//Just set the angle
			self->lastEnemy->currentAngles[1] = desiredAngles[1];
			//turned = qtrue;
		}
		else
		{//Add the increment
			self->lastEnemy->currentAngles[1] += (diffAngle < 0) ? -self->speed : self->speed;
			//turned = qtrue;
		}
		yawTurn = (diffAngle > 0) ? 1 : -1;
	}
	//Cap the range
	armAngleDiff = AngleSubtract(self->currentAngles[1], self->lastEnemy->currentAngles[1]);
	if ( armAngleDiff > ARM_ANGLE_RANGE )
	{
		self->lastEnemy->currentAngles[1] = AngleNormalize360(self->currentAngles[1] - ARM_ANGLE_RANGE);
		//turned = qfalse;
	}
	else if ( armAngleDiff < -ARM_ANGLE_RANGE )
	{
		self->lastEnemy->currentAngles[1] = AngleNormalize360(self->currentAngles[1] + ARM_ANGLE_RANGE);
		//turned = qfalse;
	}
	G_SetAngles( self->lastEnemy, self->lastEnemy->currentAngles );

	//Now put the turret at the tip of the arm
	if ( self->spawnflags&2 )
	{
		bolt_head_to_arm( self->lastEnemy, self->lastEnemy->lastEnemy, FTURR_FOFS, FTURR_ROFS, FTURR_UOFS );
	}
	else
	{
		bolt_head_to_arm( self->lastEnemy, self->lastEnemy->lastEnemy, TURR_FOFS, TURR_ROFS, TURR_UOFS );
	}

	//pitch-aim head at enemy at speed
	//FIXME: noise when turning?
	if ( self->enemy )
	{
		VectorSubtract( self->enemy->currentOrigin, self->lastEnemy->lastEnemy->currentOrigin, enemyDir );
		vectoangles( enemyDir, desiredAngles );
	}
	/*//Not necc
	else
	{
		VectorCopy(self->currentAngles, desiredAngles);
	}
	*/
	diffAngle = AngleSubtract( desiredAngles[0], self->lastEnemy->lastEnemy->currentAngles[0] );
	if ( diffAngle )
	{
		if ( fabs(diffAngle) < self->speed )
		{//Just set the angle
			self->lastEnemy->lastEnemy->currentAngles[0] = desiredAngles[0];
			//turned = qtrue;
		}
		else
		{//Add the increment
			self->lastEnemy->lastEnemy->currentAngles[0] += (diffAngle < 0) ? -self->speed : self->speed;
			//turned = qtrue;
		}
		upTurn = (diffAngle > 0) ? 1 : -1;
	}
	//Cap the range
	headAngleDiff = AngleSubtract(self->currentAngles[0], self->lastEnemy->lastEnemy->currentAngles[0]);
	if ( headAngleDiff > HEAD_ANGLE_RANGE )
	{
		self->lastEnemy->lastEnemy->currentAngles[0] = AngleNormalize360(self->currentAngles[0] - HEAD_ANGLE_RANGE);
		//turned = qfalse;
	}
	else if ( headAngleDiff < -HEAD_ANGLE_RANGE )
	{
		self->lastEnemy->lastEnemy->currentAngles[0] = AngleNormalize360(self->lastEnemy->currentAngles[0] + HEAD_ANGLE_RANGE);
		//turned = qfalse;
	}
	G_SetAngles( self->lastEnemy->lastEnemy, self->lastEnemy->lastEnemy->currentAngles );

	//Play sound if turret changes direction
	//Pitch:
	/*
	if ( upTurn && upTurn != self->count )
	{//changed dir
		G_Sound(self->lastEnemy->lastEnemy, G_SoundIndex("sound/enemies/turret/move.wav"));
	}
	else if ( !upTurn && self->count )
	{//Just stopped
		G_Sound(self->lastEnemy->lastEnemy, G_SoundIndex("sound/enemies/turret/stop.wav"));
	}
	self->count = upTurn;
	*/
	//Yaw:
	if ( yawTurn && yawTurn != self->bounceCount )
	{//changed dir
		G_Sound(self->lastEnemy, G_SoundIndex("sound/enemies/turret/move.wav"));
	}
	else if ( !yawTurn && self->bounceCount )
	{//Just stopped
		G_Sound(self->lastEnemy, G_SoundIndex("sound/enemies/turret/stop.wav"));
	}
	self->bounceCount = yawTurn;

	/*
	if ( turned )
	{
		G_Sound(self->lastEnemy, G_SoundIndex("sound/enemies/turret/move.wav"));
	}
	*/
}

void turret_turnoff (gentity_t *self)
{
	if ( self->enemy == NULL )
	{
		return;
	}
	//shut-down sound
	G_Sound(self, G_SoundIndex("sound/enemies/turret/shutdown.wav"));
	
	//make turret keep animating for 3 secs
	self->lastEnemy->lastEnemy->painDebounceTime = level.time + 3000;

	//Clear enemy
	self->enemy = NULL;
}

void turret_base_think (gentity_t *self)
{
	vec3_t		enemyDir;
	float		enemyDist;

	self->nextthink = level.time + FRAMETIME;

	if ( self->spawnflags & 1 )
	{//not turned on
		turret_turnoff( self );
		turret_aim( self );
		//No target
		if ( self->lastEnemy && self->lastEnemy->lastEnemy )
		{
			self->lastEnemy->lastEnemy->flags |= FL_NOTARGET;
		}
		return;
	}
	else
	{//I'm all hot and bothered
		if ( self->lastEnemy && self->lastEnemy->lastEnemy )
		{
			self->lastEnemy->lastEnemy->flags &= ~FL_NOTARGET;
		}
	}

	if ( !self->enemy )
	{//Find one
		gentity_t	*entity_list[MAX_GENTITIES], *target;
		int			count;
		float		bestDist = self->radius * self->radius;

		if ( self->attackDebounceTime > level.time )
		{//We're active and alert, had an enemy in the last 5 secs
			if ( self->painDebounceTime < level.time )
			{
				G_Sound(self, G_SoundIndex("sound/enemies/turret/ping.wav"));
				self->painDebounceTime = level.time + 1000;
			}
		}

		count = G_RadiusList( self->currentOrigin, self->radius, self->lastEnemy->lastEnemy, qtrue, entity_list );
		for ( int i = 0; i < count; i++ )
		{
			target = entity_list[i];
			if ( target == self )
			{
				continue;
			}

			if ( target->takedamage && target->health > 0 && !(target->flags & FL_NOTARGET) )
			{
				if ( !target->client && target->noDamageTeam == self->noDamageTeam )
				{//Something of ours we don't want to destroy
					continue;
				}
				if ( target->client && target->client->playerTeam == self->noDamageTeam )
				{//A bot we don't want to shoot
					continue;
				}

				if ( gi.inPVS( self->lastEnemy->lastEnemy->currentOrigin, target->currentOrigin ) )
				{
					trace_t	tr;

					gi.trace( &tr, self->lastEnemy->lastEnemy->currentOrigin, NULL, NULL, target->currentOrigin, self->lastEnemy->lastEnemy->s.number, MASK_SHOT );

					if ( !tr.allsolid && !tr.startsolid && (tr.fraction == 1.0 || tr.entityNum == target->s.number) )
					{//Only acquire if have a clear shot
						//Is it in range and closer than our best?
						VectorSubtract( target->currentOrigin, self->currentOrigin, enemyDir );
						enemyDist = VectorLengthSquared( enemyDir );
						if ( enemyDist < bestDist )//all things equal, keep current
						{
							if ( self->attackDebounceTime < level.time )
							{//We haven't fired or acquired an enemy in the last 5 seconds
								//start-up sound
								G_Sound(self, G_SoundIndex("sound/enemies/turret/startup.wav"));
								//Wind up turrets for a second
								self->lastEnemy->lastEnemy->attackDebounceTime = level.time + 1000;
							}
							G_SetEnemy( self, target );
							bestDist = enemyDist;
						}
					}
				}
			}
		}
	}

	if ( self->enemy )
	{//Check if still in radius
		if ( self->enemy->health <= 0 )
		{
			turret_turnoff( self );
			return;
		}

		VectorSubtract( self->enemy->currentOrigin, self->currentOrigin, enemyDir );
		enemyDist = VectorLengthSquared( enemyDir );
		if ( enemyDist > self->radius*self->radius )
		{
			turret_turnoff( self );
			return;
		}

		if ( !gi.inPVS( self->lastEnemy->lastEnemy->currentOrigin, self->enemy->currentOrigin ) )
		{
			turret_turnoff( self );
			return;
		}

		// Every now and again, check to see if we can even trace to the enemy
		if ( Q_irand( 0, 16 ) > 15 )
		{
			trace_t tr;

			gi.trace( &tr, self->lastEnemy->lastEnemy->currentOrigin, NULL, NULL, self->enemy->currentOrigin, self->lastEnemy->lastEnemy->s.number, MASK_SHOT );
			if ( tr.allsolid || tr.startsolid || tr.fraction != 1.0 )
			{
				// Couldn't see our enemy
				turret_turnoff( self );
			}
		}
	}

	if ( self->enemy )
	{//Aim
		//Won't need to wind up turrets for a while
		self->attackDebounceTime = level.time + 5000;
		turret_aim( self );
	}
	else if ( self->attackDebounceTime < level.time )
	{
		//Move arm and head back to neutral angles
		turret_aim( self );
	}
}

void turret_base_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{//Toggle on and off
	self->spawnflags = (self->spawnflags ^ 1);
}

/*QUAKED misc_turret (1 0 0) (-8 -8 -8) (8 8 8) START_OFF FORGE
Will aim and shoot at enemies

  START_OFF - Starts off
  FORGE - Uses Forge turret models and projectiles

  radius - How far away an enemy can be for it to pick it up (default 512)
  speed - How fast it turns (degrees per second, default 30)
  wait	- How fast it shoots (shots per second, default 4, can't be less)
  dmg	- How much damage each shot does (default 5)
  health - How much damage it can take before exploding (default 100)
  
  splashDamage - How much damage the explosion does
  splashRadius - The radius of the explosion
  NOTE: If either of the above two are 0, it will not make an explosion
  
  targetname - Toggles it on/off
  target - What to use when destroyed

  "team" - will not target members of this team (default "dreadnought"):
	"starfleet"
	"borg"
	"parasite"
	"scavengers"
	"klingon"
	"malon"
	"hirogen"
	"imperial"
	"stasis"
	"species8472"
	"dreadnought"
	"forge"
*/
void SP_misc_turret (gentity_t *base)
{
	//We're the base, spawn the arm and head
	gentity_t *arm = G_Spawn();
	gentity_t *head = G_Spawn();
	vec3_t		fwd;

	//Base
	//Base does the looking for enemies and pointing the arm and head
	G_SetAngles( base, base->s.angles );
	AngleVectors( base->currentAngles, fwd, NULL, NULL );
	VectorMA( base->s.origin, -8, fwd, base->s.origin );
	G_SetOrigin(base, base->s.origin);
	gi.linkentity(base);
	if ( base->spawnflags & 2 )
	{
		base->s.modelindex = G_ModelIndex("models/mapobjects/forge/turret.md3");
		base->s.modelindex2 = G_ModelIndex("models/mapobjects/forge/turret_d1.md3");
		base->noDamageTeam = TEAM_FORGE;
	}
	else
	{
		base->s.modelindex = G_ModelIndex("models/mapobjects/dn/gunturret_base.md3");
		base->noDamageTeam = TEAM_BOTS;
	}
	base->s.eType = ET_GENERAL;
	if ( base->team && base->team[0] )
	{
		base->noDamageTeam = TranslateTeamName( base->team );
	}
	//anglespeed - how fast it can track the player, entered in degrees per second, so we divide by FRAMETIME/1000
	if ( !base->speed )
	{
		base->speed = 3.0f;
	}
	else
	{
		base->speed /= FRAMETIME/1000.0f;
	}
	//range
	if ( !base->radius )
	{
		base->radius = 512;
	}
	base->e_UseFunc = useF_turret_base_use;
	base->e_ThinkFunc = thinkF_turret_base_think;
	base->nextthink = level.time + FRAMETIME;

	//Arm
	//Does nothing, not solid, gets removed when head explodes
	if ( base->spawnflags&2 )
	{
		bolt_arm_to_base( base, arm, FARM_FOFS, FARM_ROFS, FARM_UOFS );
		bolt_head_to_arm( arm, head, FTURR_FOFS, FTURR_ROFS, FTURR_UOFS );
	}
	else
	{
		bolt_arm_to_base( base, arm, ARM_FOFS, ARM_ROFS, ARM_UOFS );
		//G_SetOrigin( arm, base->s.origin );
		//gi.linkentity(arm);
		//G_SetAngles( arm, base->currentAngles );
		bolt_head_to_arm( arm, head, TURR_FOFS, TURR_ROFS, TURR_UOFS );
	}
	if ( base->spawnflags & 2 )
	{
		arm->s.modelindex = G_ModelIndex("models/mapobjects/forge/turret_neck.md3");
		arm->noDamageTeam = TEAM_FORGE;
	}
	else
	{
		arm->s.modelindex = G_ModelIndex("models/mapobjects/dn/gunturret_arm.md3");
		arm->noDamageTeam = TEAM_BOTS;
	}

	//Head
	//Fires when enemy detected, animates, can be blown up
	G_SetAngles( head, base->currentAngles );
	if ( base->spawnflags & 2 )
	{
		head->s.modelindex = G_ModelIndex("models/mapobjects/forge/turret_head.md3");
		head->noDamageTeam = TEAM_FORGE;
	}
	else
	{
		head->s.modelindex = G_ModelIndex("models/mapobjects/dn/gunturret_head.md3");
		head->noDamageTeam = TEAM_BOTS;
	}
	head->s.eType = ET_GENERAL;
	VectorSet( head->mins, -8, -8, -16 );
	VectorSet( head->maxs, 8, 8, 16 );
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		head->fullName = "Turm";
	}
	else
	{
		head->fullName = "Turret";
	}
	gi.linkentity(head);

	//How much health head takes to explode
	if ( !base->health )
	{
		head->health = 100;
	}
	else
	{
		head->health = base->health;
	}
	base->health = 0;
	//How quickly to fire
	if ( !base->wait )
	{
		head->wait = 50;
	}
	else
	{
		head->wait = 100/(base->wait/2);
	}
	base->wait = 0;
	//splashDamage
	if ( !base->splashDamage )
	{
		head->splashDamage = 10;
	}
	else
	{
		head->splashDamage = base->splashDamage;
	}
	base->splashDamage = 0;
	//splashRadius
	if ( !base->splashRadius )
	{
		head->splashRadius = 25;
	}
	else
	{
		head->splashRadius = base->splashRadius;
	}
	base->splashRadius = 0;
	//dmg
	if ( !base->damage )
	{
		head->damage = 5;
	}
	else
	{
		head->damage = base->damage;
	}
	base->damage = 0;

	//Precache firing and explode sounds
	G_SoundIndex("sound/weapons/explosions/explode11.wav");
	G_SoundIndex("sound/enemies/turret/startup.wav");
	G_SoundIndex("sound/enemies/turret/shutdown.wav");
	G_SoundIndex("sound/enemies/turret/move.wav");
	G_SoundIndex("sound/enemies/turret/stop.wav");
	G_SoundIndex("sound/enemies/turret/ping.wav");
	if ( base->spawnflags & 2 )
	{
		G_SoundIndex("sound/enemies/turret/ffire.wav");
	}
	else
	{
		G_SoundIndex("sound/enemies/turret/fire.wav");
	}

	head->contents = CONTENTS_BODY;
	head->max_health = head->health;
	head->takedamage = qtrue;
	head->e_DieFunc  = dieF_turret_die;

	head->e_ThinkFunc = thinkF_turret_head_think;
	head->nextthink = level.time + FRAMETIME;

	head->material = MAT_METAL;
	head->svFlags |= SVF_NO_TELEPORT|SVF_NONNPC_ENEMY|SVF_SELF_ANIMATING;

	//Link them up
	base->lastEnemy = arm;
	arm->lastEnemy = head;
	head->owner = arm;
	arm->activator = head->activator = base;

	if ( base->spawnflags & 2 )
	{
		//temp gfx and sounds
		RegisterItem( FindItemForWeapon( WP_FORGE_PROJ ) );	//precache the weapon
	}
	else
	{
		//temp gfx and sounds
		RegisterItem( FindItemForWeapon( WP_DN_TURRET ) );	//precache the weapon
	}
}

void laser_arm_fire (gentity_t *ent)
{
	vec3_t	start, end, fwd, rt, up;
	trace_t	trace;

	if ( ent->attackDebounceTime < level.time && ent->alt_fire )
	{
		// If I'm firing the laser and it's time to quit....then quit!
		ent->alt_fire = qfalse;
//		ent->e_ThinkFunc = thinkF_NULL;
//		return;
	}

	ent->nextthink = level.time + FRAMETIME;

	// If a fool gets in the laser path, fry 'em
	AngleVectors( ent->currentAngles, fwd, rt, up );

	VectorMA( ent->currentOrigin, 20, fwd, start );	
	//VectorMA( start, -6, rt, start );
	//VectorMA( start, -3, up, start );
	VectorMA( start, 4096, fwd, end );
	
	gi.trace( &trace, start, NULL, NULL, end, -1, MASK_SHOT );//ignore
	
	// Only deal damage when in alt-fire mode
	if ( trace.fraction < 1.0 && ent->alt_fire )
	{
		if ( trace.entityNum < ENTITYNUM_WORLD )
		{
			gentity_t *hapless_victim = &g_entities[trace.entityNum];
			if ( hapless_victim && hapless_victim->takedamage && ent->damage )
			{
				G_Damage( hapless_victim, ent, ent->nextTrain->activator, fwd, trace.endpos, ent->damage, DAMAGE_IGNORE_TEAM, MOD_SURGICAL_LASER );
			}
		}
	}
	
	if ( ent->alt_fire )
	{
		CG_FireLaser( start, trace.endpos, trace.plane.normal, ent->nextTrain->startRGBA, qfalse );
	}
	else
	{
		CG_AimLaser( start, trace.endpos, trace.plane.normal );
	}
}

void laser_arm_use (gentity_t *self, gentity_t *other, gentity_t *activator)
{
	vec3_t	newAngles;

	self->activator = activator;
	switch( self->count )
	{
	case 0:
	default:
		//Fire
		//gi.Printf("FIRE!\n");
//		self->lastEnemy->lastEnemy->e_ThinkFunc = thinkF_laser_arm_fire;
//		self->lastEnemy->lastEnemy->nextthink = level.time + FRAMETIME;
		//For 3 seconds
		self->lastEnemy->lastEnemy->alt_fire = qtrue; // Let 'er rip!
		self->lastEnemy->lastEnemy->attackDebounceTime = level.time + self->lastEnemy->lastEnemy->wait;
		G_Sound(self->lastEnemy->lastEnemy, G_SoundIndex("sound/enemies/l_arm/fire.wav"));
		break;
	case 1:
		//Yaw left
		//gi.Printf("LEFT...\n");
		VectorCopy( self->lastEnemy->currentAngles, newAngles );
		newAngles[1] += self->speed;
		G_SetAngles( self->lastEnemy, newAngles );
		bolt_head_to_arm( self->lastEnemy, self->lastEnemy->lastEnemy, LARM_FOFS, LARM_ROFS, LARM_UOFS );
		G_Sound( self->lastEnemy, G_SoundIndex( "sound/enemies/l_arm/move.wav" ) );
		break;
	case 2:
		//Yaw right
		//gi.Printf("RIGHT...\n");
		VectorCopy( self->lastEnemy->currentAngles, newAngles );
		newAngles[1] -= self->speed;
		G_SetAngles( self->lastEnemy, newAngles );
		bolt_head_to_arm( self->lastEnemy, self->lastEnemy->lastEnemy, LARM_FOFS, LARM_ROFS, LARM_UOFS );
		G_Sound( self->lastEnemy, G_SoundIndex( "sound/enemies/l_arm/move.wav" ) );
		break;
	case 3:
		//pitch up
		//gi.Printf("UP...\n");
		//FIXME: Clamp
		VectorCopy( self->lastEnemy->lastEnemy->currentAngles, newAngles );
		newAngles[0] -= self->speed;
		if ( newAngles[0] < -45 )
		{
			newAngles[0] = -45;
		}
		G_SetAngles( self->lastEnemy->lastEnemy, newAngles );
		G_Sound( self->lastEnemy->lastEnemy, G_SoundIndex( "sound/enemies/l_arm/move.wav" ) );
		break;
	case 4:
		//pitch down
		//gi.Printf("DOWN...\n");
		//FIXME: Clamp
		VectorCopy( self->lastEnemy->lastEnemy->currentAngles, newAngles );
		newAngles[0] += self->speed;
		if ( newAngles[0] > 90 )
		{
			newAngles[0] = 90;
		}
		G_SetAngles( self->lastEnemy->lastEnemy, newAngles );
		G_Sound( self->lastEnemy->lastEnemy, G_SoundIndex( "sound/enemies/l_arm/move.wav" ) );
		break;
	}
}
/*QUAKED misc_laser_arm (1 0 0) (-8 -8 -8) (8 8 8) 

What it does when used depends on it's "count" (can be set by a script)
	count:
		0 (default) - Fire in direction facing
		1 turn left
		2 turn right
		3 aim up
		4 aim down

  speed - How fast it turns (degrees per second, default 30)
  dmg	- How much damage the laser does 10 times a second (default 5 = 50 points per second)
  wait  - How long the beam lasts, in seconds (default is 3)
  
  targetname - to use it
  target - What thing for it to be pointing at to start with

  "startRGBA" - laser color, Red Green Blue Alpha, range 0 to 1 (default  1.0 0.85 0.15 0.75 = Yellow-Orange)
*/
void laser_arm_start (gentity_t *base)
{
	vec3_t	armAngles;
	vec3_t	headAngles;

	base->e_ThinkFunc = thinkF_NULL;
	//We're the base, spawn the arm and head
	gentity_t *arm = G_Spawn();
	gentity_t *head = G_Spawn();

	VectorCopy( base->s.angles, armAngles );
	VectorCopy( base->s.angles, headAngles );
	if ( base->target && base->target[0] )
	{//Start out pointing at something
		gentity_t *targ = G_Find( NULL, FOFS(targetname), base->target );
		if ( !targ )
		{//couldn't find it!
			Com_Printf(S_COLOR_RED "ERROR : laser_arm can't find target %s!\n", base->target);
		}
		else
		{//point at it
			vec3_t	dir, angles;

			VectorSubtract(targ->currentOrigin, base->s.origin, dir );
			vectoangles( dir, angles );
			armAngles[1] = angles[1];
			headAngles[0] = angles[0];
			headAngles[1] = angles[1];
		}
	}

	//Base
	//Base does the looking for enemies and pointing the arm and head
	G_SetAngles( base, base->s.angles );
	//base->s.origin[2] += 4;
	G_SetOrigin(base, base->s.origin);
	gi.linkentity(base);
	//FIXME: need an actual model
	base->s.modelindex = G_ModelIndex("models/mapobjects/dn/laser_base.md3");
	base->s.eType = ET_GENERAL;
	G_SpawnVector4( "startRGBA", "1.0 0.85 0.15 0.75", (float *)&base->startRGBA );
	//anglespeed - how fast it can track the player, entered in degrees per second, so we divide by FRAMETIME/1000
	if ( !base->speed )
	{
		base->speed = 3.0f;
	}
	else
	{
		base->speed *= FRAMETIME/1000.0f;
	}
	base->e_UseFunc = useF_laser_arm_use;
	base->nextthink = level.time + FRAMETIME;

	//Arm
	//Does nothing, not solid, gets removed when head explodes
	G_SetOrigin( arm, base->s.origin );
	gi.linkentity(arm);
	G_SetAngles( arm, armAngles );
	bolt_head_to_arm( arm, head, LARM_FOFS, LARM_ROFS, LARM_UOFS );
	arm->s.modelindex = G_ModelIndex("models/mapobjects/dn/laser_arm.md3");

	//Head
	//Fires when enemy detected, animates, can be blown up
	//Need to normalize the headAngles pitch for the clamping later
	if ( headAngles[0] < -180 )
	{
		headAngles[0] += 360;
	}
	else if ( headAngles[0] > 180 )
	{
		headAngles[0] -= 360;
	}
	G_SetAngles( head, headAngles );
	head->s.modelindex = G_ModelIndex("models/mapobjects/dn/laser_head.md3");
	head->s.eType = ET_GENERAL;
//	head->svFlags |= SVF_BROADCAST;// Broadcast to all clients
	VectorSet( head->mins, -8, -8, -8 );
	VectorSet( head->maxs, 8, 8, 8 );
	head->contents = CONTENTS_BODY;
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		head->fullName = "Medizinischer Laser";
	}
	else
	{
		head->fullName = "Surgical Laser";
	}
	gi.linkentity(head);

	//dmg
	if ( !base->damage )
	{
		head->damage = 5;
	}
	else
	{
		head->damage = base->damage;
	}
	base->damage = 0;
	//lifespan of beam
	if ( !base->wait )
	{
		head->wait = 3000;
	}
	else
	{
		head->wait = base->wait * 1000;
	}
	base->wait = 0;

	//Precache firing and explode sounds
	G_SoundIndex("sound/weapons/explosions/cargoexplode.wav");
	G_SoundIndex("sound/enemies/l_arm/fire.wav");
	G_SoundIndex("sound/enemies/l_arm/move.wav");

	//Link them up
	base->lastEnemy = arm;
	arm->lastEnemy = head;
	head->owner = arm;
	arm->nextTrain = head->nextTrain = base;

	// The head should always think, since it will be either firing a damage laser or just a target laser
	head->e_ThinkFunc = thinkF_laser_arm_fire;
	head->nextthink = level.time + FRAMETIME;
	head->alt_fire = qfalse; // Don't do damage until told to
}

void SP_laser_arm (gentity_t *base)
{
	base->e_ThinkFunc = thinkF_laser_arm_start;
	base->nextthink = level.time + FRAMETIME;
}

//Stasis Multi-Headed CyberHydra head's firing effects
void stasis_shooter_active( gentity_t *self )
{
	gentity_t	*radiusEnts[ 1024 ];
	vec3_t		mins, maxs, org, end, diff, shotEnd;
	int			nearestDist = 99999999;
	int			distance;
	int			numEnts;
	trace_t		tr;

	if ( self->owner )
	{
		if ( !self->owner->health )
		{
			// I'm dead, so don't think.
			return;
		}
	}
	self->enemy = NULL;
	self->nextthink = level.time + FRAMETIME * 5;

	if ( self->attackDebounceTime > level.time )
	{
		// don't fire if there is an attack debounce
		return;
	}

	//Setup the bbox to search in
	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = self->currentOrigin[i] - 512;
		maxs[i] = self->currentOrigin[i] + 512;
	}

	//Get a number of entities in a given space
	numEnts = gi.EntitiesInBox( mins, maxs, radiusEnts, 1024 );

	for ( int i = 0; i < numEnts; i++ )
	{
		//Don't consider self
		if ( radiusEnts[i] == self )
		{
			continue;
		}

		//Must be valid...
		//Must be a valid pointer
		if ( radiusEnts[i] == NULL )
		{
			continue;
		}

		//Must not be deleted
		if ( radiusEnts[i]->inuse == qfalse )
		{
			continue;
		}

		//Must be an NPC
		if ( radiusEnts[i]->client == NULL )
		{
			continue;
		}

		//Can't be on the same team
		if ( radiusEnts[i]->client->playerTeam == self->noDamageTeam )
		{
			continue;
		}

		//Can't be on the same team
		if ( radiusEnts[i]->noDamageTeam == self->noDamageTeam )
		{
			continue;
		}
		
		//Must be alive
		if ( radiusEnts[i]->health <= 0 )
		{
			continue;
		}

		//In case they're in notarget mode
		if ( radiusEnts[i]->flags & FL_NOTARGET )
		{
			continue;
		}

		if ( radiusEnts[i]->s.number == 0 )
		{//the player
			//Must be visible
			CalcEntitySpot( radiusEnts[i], SPOT_HEAD, end );
			CalcEntitySpot( radiusEnts[i], SPOT_ORIGIN, org );
			VectorSubtract(org, end, diff);
			VectorSet( end, end[0]+(diff[0]*Q_flrand(0, 1)), end[1]+(diff[1]*Q_flrand(0, 1)), end[2]+(diff[2]*Q_flrand(0, 1)) );
			if ( !gi.inPVS( self->currentOrigin, end ) )
			{
				continue;
			}
			gi.trace ( &tr, self->currentOrigin, NULL, NULL, end, self->s.number, CONTENTS_SOLID|CONTENTS_SHOTCLIP|CONTENTS_BODY );
			if ( tr.fraction != 1.0 && tr.entityNum != radiusEnts[i]->s.number ) 
			{
				continue;
			}

			if ( Q_irand(0, 100) < 75 )
			{
				//First try the player
				G_SetEnemy( self, radiusEnts[i] );
				VectorCopy( end, shotEnd );
				break;
			}
		}
		else
		{
			distance = DistanceSquared( self->currentOrigin, radiusEnts[i]->currentOrigin );
			//Found one closer to us
			if ( distance < nearestDist )
			{
				CalcEntitySpot( radiusEnts[i], SPOT_HEAD, end );
				CalcEntitySpot( radiusEnts[i], SPOT_ORIGIN, org );
				VectorSubtract(org, end, diff);
				VectorSet( end, end[0]+(diff[0]*Q_flrand(0, 1)), end[1]+(diff[1]*Q_flrand(0, 1)), end[2]+(diff[2]*Q_flrand(0, 1)) );
				if ( !gi.inPVS( self->currentOrigin, end ) )
				{
					continue;
				}
				gi.trace ( &tr, self->currentOrigin, NULL, NULL, end, self->s.number, CONTENTS_SOLID|CONTENTS_SHOTCLIP|CONTENTS_BODY );
				if ( tr.fraction != 1.0 && tr.entityNum != radiusEnts[i]->s.number ) 
				{
					continue;
				}
				G_SetEnemy(self, radiusEnts[i]);
				VectorCopy( end, shotEnd );
				nearestDist = distance;
			}
		}
	}

	if ( self->enemy )
	{
		self->attackDebounceTime = level.time + Q_irand(2000, 4000);
		//fire!
		//NOTE: cut and pased from FireStasisGuyAttack
		gentity_t	*bolt;
		vec3_t		dir;

		VectorSubtract( shotEnd, self->currentOrigin, dir );
		VectorNormalize (dir);

		bolt = G_Spawn();
		bolt->classname = "stasis_alien_proj";
		
		bolt->nextthink = level.time + 10000;
		bolt->e_ThinkFunc = thinkF_G_FreeEntity;

		bolt->s.eType = ET_MISSILE;
		bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
		bolt->s.weapon = WP_STASIS_ATTACK;
		bolt->owner = self->owner;
		bolt->damage = 3 + (g_spskill->integer * 2);//tweak?
		bolt->dflags = 0;
		bolt->splashDamage = 0;
		bolt->splashRadius = 0;
		bolt->methodOfDeath = MOD_STASIS;
		bolt->clipmask = MASK_SHOT;

		bolt->s.pos.trType = TR_LINEAR;
		bolt->s.pos.trTime = level.time;
		VectorCopy( self->currentOrigin, bolt->s.pos.trBase );
		
		VectorScale( dir, 550, bolt->s.pos.trDelta );//tweak?
		
		SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
		VectorCopy( self->currentOrigin, bolt->currentOrigin );

		G_Sound( self, G_SoundIndex( "sound/weapons/stasis_alien/fire.wav" ) );
	}
}

void stasis_shooter_toggle( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	self->count ^= 1;
	if ( self->count )
	{//active
		self->e_ThinkFunc = thinkF_stasis_shooter_active;
		self->nextthink = level.time + Q_irand( 0, 4000 );

		if ( !self->owner )
		{
			gentity_t *owner = NULL;
			while( NULL != (owner = G_Find( owner, FOFS(target), self->targetname )))
			{
				if ( Q_stricmp("misc_model_breakable", owner->classname) == 0 )
				{
					self->owner = owner;
					self->owner->svFlags |= SVF_NONNPC_ENEMY;
					self->owner->noDamageTeam = self->noDamageTeam;
					break;
				}
			}
		}
		if ( !self->owner )
		{
			Com_Error( ERR_DROP, "misc_stasis_shooter %s at %s must be directly targetted by a misc_model_breakable!\n", self->targetname, vtos(self->currentOrigin) );
		}
	}
	else
	{
		self->e_ThinkFunc = thinkF_NULL;
	}
}

/*QUAKED misc_stasis_shooter (1 0.5 0) (-4 -4 -4) (4 4 4) 
Starts inactive.

When active, will fire at enemies

Using it toggles it active/inactive

targetname - must be directly targetted by a misc_model_breakable
*/
void SP_misc_stasis_shooter (gentity_t *self)
{
	G_SetOrigin( self, self->s.origin );
	// Now register the weapon
	RegisterItem( FindItemForWeapon( WP_STASIS_ATTACK ) );
	G_SoundIndex( "sound/weapons/stasis_alien/fire.wav" );
	G_SoundIndex( "sound/weapons/stasis_alien/hit_wall.wav" );
	self->e_UseFunc = useF_stasis_shooter_toggle;
	self->noDamageTeam = TEAM_STASIS;
}
