// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"
#include "g_functions.h"
#include "anims.h"
#include "b_local.h"
#include "objectives.h"
#include "..\cgame\cg_text.h"

extern vmCvar_t cg_thirdPerson;

static	vec3_t	_forward, vright, up;
static	vec3_t	muzzle;

#define	BORG_ADAPT_NUM_HITS 10
extern int killPlayerTimer;

void FX_BorgShield( vec3_t position, vec3_t dir );
void FX_BorgHit( vec3_t origin, vec3_t dir );

extern void CGCam_Shake( float intensity, int duration );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );

void drop_charge(gentity_t *ent, vec3_t start, vec3_t dir);
void ViewHeightFix( gentity_t *ent );
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker );
extern void CG_ChangeWeapon( int num );
extern qboolean G_PointInBounds( vec3_t point, vec3_t mins, vec3_t maxs );
extern qboolean G_BoxInBounds( vec3_t point, vec3_t mins, vec3_t maxs, vec3_t boundsMins, vec3_t boundsMaxs );

void W_TraceSetStart( gentity_t *ent, vec3_t start, vec3_t mins, vec3_t maxs )
{//make sure our start point isn't on the other side of a wall
	trace_t	tr;
	vec3_t	entMins;
	vec3_t	entMaxs;

	VectorAdd( ent->currentOrigin, ent->mins, entMins );
	VectorAdd( ent->currentOrigin, ent->maxs, entMaxs );

	if ( G_BoxInBounds( start, mins, maxs, entMins, entMaxs ) )
	{
		return;
	}

	if ( !ent->client )
	{
		return;
	}

	gi.trace( &tr, ent->client->renderInfo.eyePoint, mins, maxs, start, ent->s.number, MASK_SOLID|CONTENTS_SHOTCLIP );

	if ( tr.startsolid || tr.allsolid )
	{
		return;
	}

	if ( tr.fraction < 1.0f )
	{
		VectorCopy( tr.endpos, start );
	}
	else
	{
		//trace was cleah
	}
}

//---------------------------------------------------------
qboolean W_CheckBorgAdapt( gentity_t *attacker, int weapon, gentity_t *traceEnt, vec3_t endpos )
//---------------------------------------------------------
{
	if ( !traceEnt )
		return qtrue;

	if ( traceEnt->client && traceEnt->client->race == RACE_BORG && traceEnt->NPC && traceEnt->NPC->aiFlags&NPCAI_SHIELDS )
	{
		// Adapt info is stored under the players persistent client info
		if ( g_entities[0].client )
		{
			int *borgAdaptHits = g_entities[0].client->ps.borgAdaptHits;

			//FIXME: use damage accumulated rather than number of hits?
			borgAdaptHits[weapon]++;

			if(borgAdaptHits[weapon] >= BORG_ADAPT_NUM_HITS)
			{
				//They've adapted
				if(borgAdaptHits[weapon] == BORG_ADAPT_NUM_HITS)
				{
					G_UseTargets2(attacker, attacker->owner, "adapted");
				}
				
				vec3_t	dir;

				VectorSubtract( attacker->currentOrigin, endpos, dir );
				VectorNormalize( dir );

				FX_BorgShield( endpos, dir );
				traceEnt->s.powerups |= ( 1 << PW_BORG_SHIELD );
				traceEnt->client->ps.powerups[PW_BORG_SHIELD] = level.time + 50;
				
				//FIXME: Add this to cgs.media and play this on the cgame side
				G_Sound(traceEnt, G_SoundIndex("sound/enemies/borg/borgshield.wav"));

				//make them still react to absorbed shots, but no anim
				GEntity_PainFunc( traceEnt, attacker, -1 );

				return qfalse;
			}
			else
			{
				//Hit them
				vec3_t	backward;

				VectorScale(_forward, -1, backward);
				FX_BorgHit( endpos , backward );

				return qtrue;
			}
		}
	}

	return qtrue;
}

/*
----------------------------------------------
	PLAYER WEAPONS
----------------------------------------------


----------------------------------------------
	PHASER
----------------------------------------------
*/

#define	PHASER_WEAK_DAMAGE	1
#define	PHASER_DAMAGE		4
#define PHASER_ALT_DAMAGE	10

//---------------------------------------------------------
void WP_FirePhaser( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		start, end;
	gentity_t	*traceEnt;
	qboolean	do_damage = qtrue, weak = qfalse;

		// Make sure that the damage trace corresponds with the trace done for the actual effect or weird things could happen
	if ( ent->s.number == 0 )
		VectorCopy( ent->client->renderInfo.eyePoint, start );
	else
		VectorCopy( muzzle, start );

	VectorMA( start, -8, _forward, start );
	VectorMA( start, weaponData[WP_PHASER].range, _forward, end);

	// Find out who we've hit
	gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);
	traceEnt = &g_entities[ tr.entityNum ];

	AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
	AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
	// Make sure to check if the Borg have adapted...
	do_damage = W_CheckBorgAdapt( ent, WP_PHASER, traceEnt, tr.endpos );

	if ( ent->client )
	{
		if ( ent->client->ps.ammo[AMMO_PHASER] < 1 )
			weak = qtrue;
	}

	if ( traceEnt->takedamage ) 
	{
		if ( LogAccuracyHit( traceEnt, ent ) ) 
			ent->client->ps.persistant[PERS_ACCURACY_HITS]++;

		if ( do_damage )
		{
			if ( weak )
			{
				// If we are in weak mode, only damage every now and then.
				if ( (rand() & 1) )
					G_Damage( traceEnt, ent, ent, _forward, tr.endpos, PHASER_WEAK_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PHASER );
			}
			else
			{
				if ( alt_fire )
					G_Damage( traceEnt, ent, ent, _forward, tr.endpos, PHASER_ALT_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PHASER_ALT );
				else
					G_Damage( traceEnt, ent, ent, _forward, tr.endpos, PHASER_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_PHASER );
			}
		}
	}
}

/*
----------------------------------------------
	COMPRESSION RIFLE
----------------------------------------------
*/

#define	CRIFLE_DAMAGE		25
#define COMPRESSION_SPREAD	100

//---------------------------------------------------------
void WP_FireCompressionRifle ( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	gentity_t	*tent;
	gentity_t	*tent2 = 0;
	gentity_t	*traceEnt;
	int			damage = alt_fire ? CRIFLE_DAMAGE * 6 : CRIFLE_DAMAGE; // do butt-loads of damage for alt
	trace_t		tr;
	vec3_t		start, end;
	qboolean	do_damage = qtrue;
	qboolean	render_impact = qtrue;

	if ( ent->s.number == 0 && !cg_thirdPerson.integer )
		// The trace start will originate at the eye so we can ensure that it hits the crosshair.
		// This can cause small clipping errors, but it's probably not that big of a deal at this point.
		VectorCopy( ent->client->renderInfo.eyePoint, start );
	else
		VectorCopy( muzzle, start );

	VectorMA( start, -8, _forward, start );
	VectorMA( start, 8192, _forward, end );

	gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

	// If the beam hits a skybox, etc. it would look foolish to add in an explosion
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		render_impact = qfalse;
	}
	
	traceEnt = &g_entities[ tr.entityNum ];

	// Render a shot in any case.
	if ( alt_fire )
		tent = G_TempEntity( tr.endpos, EV_COMPRESSION_RIFLE_ALT );
	else
		tent = G_TempEntity( tr.endpos, EV_COMPRESSION_RIFLE );

	// Only add in impact stuff when told to do so
	if ( render_impact )
	{
		// send bullet impact
		if ( traceEnt->takedamage && traceEnt->client ) 
		{
			// Create a simple impact type mark
			tent2 = G_TempEntity( tr.endpos, EV_COMPRESSION_RIFLE_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			do_damage = W_CheckBorgAdapt( ent, WP_COMPRESSION_RIFLE, traceEnt, tr.endpos );
			
			if( LogAccuracyHit( traceEnt, ent ) ) 
			{
				ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
			}
		} 
		else 
		{
			if ( alt_fire)
			{
				// Do a discreet miss
				tent2 = G_TempEntity( tr.endpos, EV_COMPRESSION_RIFLE_ALT_MISS );
				AddSightEvent( ent, tr.endpos, 256, AEL_SUSPICIOUS );
			}
			else
			{
				// Create an explosion
				tent2 = G_TempEntity( tr.endpos, EV_COMPRESSION_RIFLE_MISS );

				// FIXME:  Is this really an ok place to add this?
				if ( ent->s.number == 0 )
				{
					//Add the event
					AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
					AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
				}
			}
		}
	}

	// Stash origins, etc. so that the effect can have access to them.
	VectorCopy( muzzle, tent->s.origin2 );

	if ( render_impact )
	{
		VectorCopy( muzzle, tent2->s.origin2 );
		tent2->s.eventParm = tent->s.eventParm = DirToByte( tr.plane.normal );
		tent2->s.weapon = tent->s.weapon = ent->s.weapon;
	} 

	if ( traceEnt->takedamage && do_damage )
	{
		// Do less damage when it's an NPC that is shooting this weapon, this is done so that they can still shoot a lot
		//	but not necessarily kill everything for you.
		if ( ent->client->ps.clientNum != 0 )
		{
			damage *= 0.5;
		}

		if ( alt_fire )
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_SNIPER );
		}
		else
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, damage, 0, MOD_CRIFLE );
		}
	}
}


/*
----------------------------------------------
	IMOD
----------------------------------------------
*/

#define	IMOD_DAMAGE		24
#define	IMOD_ALT_DAMAGE	60

//---------------------------------------------------------
void WP_FireIMOD ( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		start, end, d_dir;
	gentity_t	*tent;
	gentity_t	*tent2 = 0;
	gentity_t	*traceEnt;
	qboolean	render_impact = qtrue;

	if ( ent->s.number == 0 && !cg_thirdPerson.integer )
		// The trace start will originate at the eye so we can ensure that it hits the crosshair.
		// This can cause small clipping errors, but it's probably not that big of a deal at this point.
		VectorCopy( ent->client->renderInfo.eyePoint, start );
	else
		VectorCopy( muzzle, start );

	VectorMA( start, -8, _forward, start );
	VectorMA ( start, weaponData[WP_IMOD].range, _forward, end);
	gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);

	// If the beam hits a skybox, etc. it would look foolish to add in an explosion
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		render_impact = qfalse;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if ( ent->s.number == 0 )
	{
		//Add the event
		AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
		AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
	}
	// Create the events that will add in the necessary effects
	if ( alt_fire )
	{
		tent = G_TempEntity( tr.endpos, EV_IMOD_ALTFIRE );

		if ( render_impact )
		{
			// send bullet impact
			if ( traceEnt->takedamage && traceEnt->client ) 
			{
				tent2 = G_TempEntity( tr.endpos, EV_IMOD_ALTFIRE_HIT );
				tent->s.otherEntityNum = traceEnt->s.number;

				if( LogAccuracyHit( traceEnt, ent ) ) 
				{
					ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
				}
			}
			else
			{
				tent2 = G_TempEntity( tr.endpos, EV_IMOD_ALTFIRE_MISS );
			}
		}
	}
	else
	{
		tent = G_TempEntity( tr.endpos, EV_IMOD );

		if ( render_impact )
		{
			// send bullet impact
			if ( traceEnt->takedamage && traceEnt->client ) 
			{
				tent2 = G_TempEntity( tr.endpos, EV_IMOD_HIT );
				tent->s.otherEntityNum = traceEnt->s.number;

				if( LogAccuracyHit( traceEnt, ent ) ) 
				{
					ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
				}
			}
			else
			{
				tent2 = G_TempEntity( tr.endpos, EV_IMOD_MISS );
			}
		}
	}

	if ( traceEnt->takedamage) 
	{
		//For knockback - send them up in air
		VectorCopy(_forward, d_dir);
		if(d_dir[2] < 0.30f)
			d_dir[2] = 0.30f;

		VectorNormalize(d_dir);
	
		if ( alt_fire )
			G_Damage( traceEnt, ent, ent, d_dir, tr.endpos, IMOD_ALT_DAMAGE*Q_flrand(0.8f, 1.2f), DAMAGE_NO_HIT_LOC|DAMAGE_DEATH_KNOCKBACK|DAMAGE_EXTRA_KNOCKBACK, MOD_IMOD );
		else
			G_Damage( traceEnt, ent, ent, d_dir, tr.endpos, IMOD_DAMAGE*Q_flrand(0.8f, 1.2f), DAMAGE_NO_HIT_LOC|DAMAGE_DEATH_KNOCKBACK|DAMAGE_EXTRA_KNOCKBACK, MOD_IMOD );
	}

	// Stash origins, etc. so the effect can have access to them
	VectorCopy( muzzle, tent->s.origin2 );
	if ( render_impact )
	{
		VectorCopy( muzzle, tent2->s.origin2 );
		tent2->s.eventParm = tent->s.eventParm = DirToByte( tr.plane.normal );
		tent2->s.weapon = tent->s.weapon = ent->s.weapon;
	}
}

/*
----------------------------------------------
	SCAVENGER
----------------------------------------------
*/

#define SCAV_SPREAD			0.5
#define SCAV_VELOCITY		1500
#define SCAV_DAMAGE			10

#define	SCAV_NPC_DAMAGE_EASY		3
#define	SCAV_NPC_DAMAGE_NORMAL		7
#define	SCAV_NPC_DAMAGE_HARD		9
	
#define SCAV_ALT_DAMAGE		60
#define SCAV_ALT_SPLASH_RAD	80
#define SCAV_ALT_SPLASH_DAM	60	
#define SCAV_ALT_VELOCITY	900	

//---------------------------------------------------------
void FireScavengerBullet( gentity_t *ent, vec3_t start, vec3_t dir, qboolean greyscale )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	int			damage = SCAV_DAMAGE;
	int			velocity = SCAV_VELOCITY;

	bolt = G_Spawn();
	
	bolt->classname = "scav_proj";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	if ( greyscale )
	{
		bolt->s.weapon = WP_CHAOTICA_GUARD_GUN;
	}
	else
	{
		bolt->s.weapon = WP_SCAVENGER_RIFLE;
	}
	bolt->owner = ent;

	// See if we need to do less damage, otherwise you'll be dead instantly if this weapon is used against you
	if ( ent->client )
	{
		if ( ent->client->ps.clientNum != 0 )
		{	// Flags effect as being lite version for NPC's
			bolt->count = 1;
			
			// Lowering velocity seemed to help the effect look a bit cooler from 3rd person, that way you see a volley
			//	of bullets racing at you...
			velocity *= 0.5;
		}
		else
		{	// Flags effect as being the full beefy version for the player
			bolt->count = 0;
		}
	}
	
	//Do the damages
	if ( ent->s.number != 0 )
	{
		if ( g_spskill->integer == 0 )
			damage = SCAV_NPC_DAMAGE_EASY;
		else if ( g_spskill->integer == 1 )
			damage = SCAV_NPC_DAMAGE_NORMAL;
		else
			damage = SCAV_NPC_DAMAGE_HARD;
	}
	else
	{
		damage = SCAV_DAMAGE;
	}

	bolt->damage = damage;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_SCAVENGER;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_LINEAR	;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, velocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->currentOrigin);
}

// Alt-fire...
//---------------------------------------------------------
void FireScavengerGrenade( gentity_t *ent, vec3_t start, vec3_t dir )
//---------------------------------------------------------
{
	gentity_t	*grenade;

	grenade = G_Spawn();
	
	grenade->classname = "scav_grenade";
	grenade->nextthink = level.time + 9000;
	grenade->e_ThinkFunc = thinkF_G_FreeEntity;
	grenade->s.eType = ET_MISSILE;
	grenade->s.weapon = WP_SCAVENGER_RIFLE;
	grenade->owner = ent;
	grenade->damage = SCAV_ALT_DAMAGE; 
	grenade->dflags = 0;
	grenade->splashDamage = SCAV_ALT_SPLASH_DAM;
	grenade->splashRadius = SCAV_ALT_SPLASH_RAD;
	grenade->methodOfDeath = MOD_GRENADE;
	grenade->splashMethodOfDeath = MOD_GRENADE_SPLASH;
	grenade->clipmask = MASK_SHOT;
	grenade->alt_fire = qtrue;

	// How 'bout we give this thing a size...
	VectorSet( grenade->maxs, 1.0f, 1.0f, 1.0f );
	VectorScale( grenade->maxs, -1, grenade->mins );

	grenade->s.pos.trType = TR_GRAVITY;
	grenade->s.pos.trTime = level.time;		// move a bit on the very first frame
	
	VectorCopy( start, grenade->s.pos.trBase );
	VectorScale( dir, random() * 100 + SCAV_ALT_VELOCITY, grenade->s.pos.trDelta );
	grenade->s.pos.trDelta[2] += 140;

	SnapVector( grenade->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, grenade->currentOrigin);
	VectorCopy (start, grenade->pos1);
}

//---------------------------------------------------------
void WP_FireScavenger( gentity_t *ent, qboolean alt_fire, qboolean greyscale )
//---------------------------------------------------------
{
	vec3_t		dir, angles, temp_ang, temp_org;
	vec3_t		start;
	float		offset;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );

	if ( alt_fire )
	{
		FireScavengerGrenade( ent, start, dir );
	}
	else
	{
		if ( ent->s.number == 0 )
		{
			// FIXME: Move in from the side muzzle.  But, it might be much better to have it just come from the main barrel...
			VectorMA( start, -20, _forward, start );
			VectorMA( start, 1, vright, start );
			VectorMA( start, -4, up, start );
		}

		vectoangles( dir, angles );

		//Scavengers use their aim values as well as the gun's inherent inaccuracy
		if ( ent->client && ent->NPC && ent->client->playerTeam == TEAM_SCAVENGERS )
		{
			float	xofs, yofs;

			xofs = angles[0] + ( crandom() * (SCAV_SPREAD+(ent->NPC->stats.aim*0.5)) );
			yofs = angles[1] + ( crandom() * (SCAV_SPREAD+(ent->NPC->stats.aim*0.5)) );

			VectorSet( temp_ang, xofs, yofs, angles[2] );
		}
		else
		{
			VectorSet( temp_ang, angles[0] + (crandom() * SCAV_SPREAD), angles[1] + (crandom() * SCAV_SPREAD), angles[2] );
		}

		AngleVectors( temp_ang, dir, NULL, NULL );

		// try to make the shot alternate between barrels
		offset = Q_irand(0, 1) * 2 + 1;

		VectorMA( start, offset, vright, temp_org );
		VectorMA( temp_org, offset, up, temp_org );
		//FIXME if temp_org does not have clear trace to inside the bbox, don't shoot!
		FireScavengerBullet( ent, temp_org, dir, greyscale );
	}
}

/*
----------------------------------------------
	STASIS
----------------------------------------------
*/

#define STASIS_VELOCITY		1000	//650
#define STASIS_SPREAD		5.0 //1.8	// Keep the spread relatively small so that you can get multiple projectile impacts when a badie is close

#define STASIS_ALT_DAMAGE	40
#define STASIS_ALT_DAMAGE2	20

#define STASIS_ALT_RIGHT_OFS	0.10
#define STASIS_ALT_UP_OFS		0.02

#define MAXRANGE_STASIS		8192

//---------------------------------------------------------
void FireStasisMissile( gentity_t *ent, vec3_t origin, vec3_t dir, int size )
//---------------------------------------------------------
{
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "stasis_projectile";
	
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_STASIS;
	bolt->owner = ent;
	bolt->damage = size * 12;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_STASIS;
	//bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->trigger_formation = qfalse;		// don't draw tail on first frame	

	// There are going to be a couple of different sized projectiles, so store 'em here
	bolt->count = size;

	// How 'bout we give this thing a size...
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );
	VectorScale( bolt->maxs, -size, bolt->mins );
	VectorScale( bolt->maxs, size, bolt->maxs );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( origin, bolt->s.pos.trBase );
	
	VectorScale( dir, STASIS_VELOCITY + ( 50 * size ), bolt->s.pos.trDelta );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (origin, bolt->currentOrigin);
	// Used by trails
	VectorCopy (origin, bolt->pos1 );
	VectorCopy (origin, bolt->pos2 );
}

//---------------------------------------------------------
void WP_FireStasisMain( gentity_t *ent )
//---------------------------------------------------------
{
	vec3_t	dir, start;
	vec3_t	angles, temp;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );

	vectoangles( dir, angles );
	FireStasisMissile( ent, start, dir, 4 );

	VectorCopy( angles, temp );
	temp[YAW] += STASIS_SPREAD;
	AngleVectors( temp, temp, NULL, NULL );
	FireStasisMissile( ent, start, temp, 2 );

	VectorCopy( angles, temp );
	temp[YAW] -= STASIS_SPREAD;
	AngleVectors( temp, temp, NULL, NULL );
	FireStasisMissile( ent, start, temp, 2 );
}

void DoSmallStasisBeam(gentity_t *ent, vec3_t start, vec3_t dir)
{
	qboolean	do_damage;
	vec3_t		end;
	trace_t		tr;
	gentity_t	*traceEnt;

	VectorMA(start, MAXRANGE_STASIS, dir, end);
	gi.trace(&tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);

	traceEnt = &g_entities[ tr.entityNum ];
	do_damage = W_CheckBorgAdapt( ent, WP_STASIS, traceEnt, tr.endpos );

	if ( traceEnt->takedamage && do_damage ) 
	{
		//For knockback - send them up in air
		if ( dir[2] < 0.20f )
		{
			dir[2] = 0.20f;
		}

		VectorNormalize( dir );
	
		G_Damage(traceEnt, ent, ent, dir, tr.endpos, STASIS_ALT_DAMAGE2, DAMAGE_NO_HIT_LOC, MOD_STASIS );
	}
}

//---------------------------------------------------------
void WP_FireStasisAlt( gentity_t *ent )
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		end, d_dir, d_right, d_up={0,0,1};
	gentity_t	*tent;
	gentity_t	*tent2;
	gentity_t	*traceEnt;
	qboolean	render_impact = qtrue;
	qboolean	do_damage = qtrue;

	// Find the main impact point
	VectorMA (muzzle, MAXRANGE_STASIS, _forward, end);
	gi.trace ( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	
	// Rendering things like explosions when hitting a sky box would look bad, but you still want to see the beam
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		render_impact = qfalse;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// Why am I doing this when I've got a right and up already?  Well, because this is how it is calc'ed on the client side.
	CrossProduct(_forward, d_up, d_right);
	CrossProduct(d_right, _forward, d_up);	// Change the "fake up" (0,0,1) to a "real up" (perpendicular to the forward vector).

	// Fire a shot up and to the right.
	VectorMA(_forward, STASIS_ALT_RIGHT_OFS, d_right, d_dir);
	VectorMA(d_dir, STASIS_ALT_UP_OFS, d_up, d_dir);
	DoSmallStasisBeam(ent, muzzle, d_dir);

	// Fire a shot up and to the left.
	VectorMA(_forward, -STASIS_ALT_RIGHT_OFS, d_right, d_dir);
	VectorMA(d_dir, STASIS_ALT_UP_OFS, d_up, d_dir);
	DoSmallStasisBeam(ent, muzzle, d_dir);

	// Fire a shot a bit down and to the right.
	VectorMA(_forward, 2.0*STASIS_ALT_RIGHT_OFS, d_right, d_dir);
	VectorMA(d_dir, -0.5*STASIS_ALT_UP_OFS, d_up, d_dir);
	DoSmallStasisBeam(ent, muzzle, d_dir);

	// Fire a shot up and to the left.
	VectorMA(_forward, -2.0*STASIS_ALT_RIGHT_OFS, d_right, d_dir);
	VectorMA(d_dir, -0.5*STASIS_ALT_UP_OFS, d_up, d_dir);
	DoSmallStasisBeam(ent, muzzle, d_dir);

	// Main beam
	tent = G_TempEntity( tr.endpos, EV_STASIS );

	// Only add in impact stuff when told to do so
	if ( render_impact )
	{
		// send bullet impact
		if ( traceEnt->takedamage && traceEnt->client ) 
		{
			tent2 = G_TempEntity( tr.endpos, EV_STASIS_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			do_damage = W_CheckBorgAdapt( ent, WP_STASIS, traceEnt, tr.endpos );
		} 
		else 
		{
			tent2 = G_TempEntity( tr.endpos, EV_STASIS_MISS );
			if ( ent->s.number == 0 )
			{
				//Add the event
				AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
				AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
			}
		}

		// Stash origins, etc. so that the effects can have access to them
		VectorCopy( muzzle, tent2->s.origin2 );
		tent2->s.eventParm = tent->s.eventParm = DirToByte( tr.plane.normal );
		tent2->s.weapon = tent->s.weapon = ent->s.weapon;
	}

	if ( traceEnt->takedamage && do_damage ) 
	{
		//For knockback - send them up in air
		VectorCopy( _forward, d_dir );
		if ( d_dir[2] < 0.30f )
		{
			d_dir[2] = 0.30f;
		}

		VectorNormalize( d_dir );
	
		G_Damage( traceEnt, ent, ent, d_dir, tr.endpos, STASIS_ALT_DAMAGE, DAMAGE_NO_HIT_LOC, MOD_STASIS );
	}

	// Stash origins, etc. so that the effects can have access to them
	VectorCopy( muzzle, tent->s.origin2 );
}

//---------------------------------------------------------
void WP_FireStasis( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	// This was moved out of the FireWeapon switch statement below to keep things more consistent
	if ( alt_fire )
	{
		WP_FireStasisAlt( ent );
	}
	else
	{
		WP_FireStasisMain( ent );
	}
}


/*
----------------------------------------------
	GRENADE LAUNCHER
----------------------------------------------
*/

#define GRENADE_DAMAGE			100
#define GRENADE_SPLASH_RAD		128
#define GRENADE_SPLASH_DAM		90
#define GRENADE_VELOCITY		900
#define GRENADE_TIME			6000

#define GRENADE_ALT_DAMAGE		100
#define GRENADE_ALT_SPLASH_RAD	128
#define GRENADE_ALT_SPLASH_DAM	90
#define GRENADE_ALT_VELOCITY	1600
#define GRENADE_ALT_TIME		3000

#define SHRAPNEL_DAMAGE			30
#define SHRAPNEL_DISTANCE		4096
#define SHRAPNEL_BITS			6
#define SHRAPNEL_RANDOM			3
#define SHRAPNEL_SPREAD			0.75

//---------------------------------------------------------
void grenadeExplode( gentity_t *ent )
//---------------------------------------------------------
{
	vec3_t		normal = { 0, 0, 1 };
	vec3_t		pos;
	gentity_t	*tent;

	VectorSet( pos, ent->currentOrigin[0], ent->currentOrigin[1], ent->currentOrigin[2] + 8 );

	G_RadiusDamage( ent->currentOrigin, ent->owner, GRENADE_SPLASH_DAM, GRENADE_SPLASH_RAD,
					 NULL, MOD_GRENADE);

	tent = G_TempEntity( pos, EV_GRENADE_EXPLODE );
	VectorCopy( normal, tent->s.origin2 );

	G_FreeEntity( ent );
}

//---------------------------------------------------------
void WP_FireShrapnel( gentity_t *ent, vec3_t dir )
//---------------------------------------------------------
{
	vec3_t		end, org;
	trace_t		tr;
	gentity_t	*tent;

	VectorCopy( ent->s.pos.trBase, org );

	VectorMA( org, SHRAPNEL_DISTANCE, dir, end );
	gi.trace ( &tr, org, NULL, NULL, end, 0, MASK_SHOT );

	tent = G_TempEntity( org, EV_GRENADE_SHRAPNEL );
	VectorCopy( tr.endpos, tent->s.origin2 );
	VectorCopy( tr.plane.normal, tent->pos1 );
}

//---------------------------------------------------------
void grenadeSpewShrapnel( gentity_t *ent )
//---------------------------------------------------------
{
	int			i, t, num_bits;
	vec3_t		norm;
	gentity_t	*tent;

	num_bits = (int)( SHRAPNEL_BITS + random() * SHRAPNEL_RANDOM );

	tent = G_TempEntity( ent->currentOrigin, EV_GRENADE_SHRAPNEL_EXPLODE );
	VectorCopy( ent->pos1, tent->pos1 );

	if ( ent->owner && ent->owner->s.number == 0 )
	{
		//Add the event
		AddSoundEvent( ent->owner, ent->currentOrigin, 256, AEL_DISCOVERED );
		AddSightEvent( ent->owner, ent->currentOrigin, 512, AEL_DISCOVERED );
	}

	G_RadiusDamage( ent->currentOrigin, ent->owner, GRENADE_ALT_SPLASH_DAM, GRENADE_ALT_SPLASH_RAD,
					 NULL, MOD_GRENADE);

	for ( i = 0; i < num_bits; i++ )
	{
		VectorCopy( ent->pos1, norm );
		for ( t = 0; t < 3; t++ )
		{
			norm[t] += crandom() * SHRAPNEL_SPREAD;
		}

		VectorNormalize( norm );
		WP_FireShrapnel( ent, norm );
	}

	ent->nextthink = level.time + 100;
	ent->e_ThinkFunc = thinkF_G_FreeEntity;
}

//---------------------------------------------------------
void WP_FireGrenade( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();
	
	if ( alt_fire )
	{
		bolt->classname = "grenade_alt_projectile";
		bolt->nextthink = level.time + GRENADE_ALT_TIME; // How long 'til she blows
		bolt->e_ThinkFunc = thinkF_grenadeSpewShrapnel;
		bolt->alt_fire = qtrue;
		bolt->s.eFlags |= EF_MISSILE_STICK;
		VectorScale( dir, GRENADE_ALT_VELOCITY, bolt->s.pos.trDelta );

		bolt->s.pos.trType = TR_GRAVITY;
		bolt->trigger_formation = qtrue;

		bolt->damage = GRENADE_ALT_DAMAGE;
		bolt->dflags = 0;
		bolt->splashDamage = GRENADE_ALT_SPLASH_DAM;
		bolt->splashRadius = GRENADE_ALT_SPLASH_RAD;
	}
	else
	{
		bolt->classname = "grenade_projectile";
		bolt->nextthink = level.time + GRENADE_TIME; // How long 'til she blows
		bolt->e_ThinkFunc = thinkF_grenadeExplode;
		bolt->s.eFlags |= EF_BOUNCE_HALF;
		VectorScale( dir, GRENADE_VELOCITY, bolt->s.pos.trDelta );
		bolt->s.pos.trDelta[2] += 120;
		bolt->s.pos.trType = TR_GRAVITY;

		bolt->damage = GRENADE_DAMAGE;
		bolt->dflags = 0;
		bolt->splashDamage = GRENADE_SPLASH_DAM;
		bolt->splashRadius = GRENADE_SPLASH_RAD;

		// How 'bout we give this thing a size...
		VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
		VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );
	}

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_GRENADE_LAUNCHER;
	bolt->owner = bolt->owner = ent;

	bolt->methodOfDeath = MOD_GRENADE;
	//bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	W_TraceSetStart( ent, start, bolt->mins, bolt->maxs );//make sure our start point isn't on the other side of a wall
	VectorCopy( start, bolt->s.pos.trBase );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);

	VectorCopy( start, bolt->pos2 );
}

/*
----------------------------------------------
	TETRION
----------------------------------------------
*/

#define TETRION_SPREAD			275
#define TETRION_DAMAGE			4
#define NUM_TETRION_SHOTS		3

#define TETRION_ALT_VELOCITY	1500
#define TETRION_ALT_DAMAGE		40

//---------------------------------------------------------
void FireTetrionBullet( gentity_t *ent, vec3_t start, vec3_t dir )
//---------------------------------------------------------
{
	gentity_t	*tent = 0;
	gentity_t	*tent2 = 0;
	gentity_t	*traceEnt, *hitEnt = NULL;
	trace_t		tr;
	vec3_t		end;
	float		r;
	float		u;
	qboolean	render_impact = qtrue;
	qboolean	do_damage = qtrue;

	VectorCopy( muzzle, start );
	W_TraceSetStart( ent, start, vec3_origin, vec3_origin );//make sure our start point isn't on the other side of a wall

	// To simulate fast firing, we'll just generate a couple of shots at once
	for ( int i = 0; i < NUM_TETRION_SHOTS; i ++ )
	{
		// Add some sloppiness to the shot so it's harder to hit when you are far away
		r = crandom()*TETRION_SPREAD;
		u = crandom()*TETRION_SPREAD;

		VectorMA (start, 8192, _forward, end);

		VectorMA (end, r, vright, end);
		VectorMA (end, u, up, end);
		
		gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT);
		
		// Rendering things like impacts when hitting a sky box would look bad, but you still want to see the tracer
		if ( tr.surfaceFlags & SURF_NOIMPACT ) 
		{
			render_impact = qfalse;
		}

		traceEnt = &g_entities[ tr.entityNum ];

		// Don't always draw a tracer line
		if ( random() > 0.1 )
		{
			// Create the trace effect
			tent = G_TempEntity( tr.endpos, EV_TETRION );

			// Stash origins, etc. so that effects can have access to startPoints and other useful info.
			VectorCopy( start, tent->s.origin2 );
			tent->s.eventParm = DirToByte( tr.plane.normal );
			tent->s.weapon = ent->s.weapon;
			VectorScale( _forward, -1, tent->pos1 );		// Used for trace-back
			tent->s.otherEntityNum = traceEnt->s.number;
		}

		// Make sure we hit something that should generate an impact
		if ( render_impact )
		{
			// send bullet impact
			if ( traceEnt->takedamage && traceEnt->client ) 
			{
				tent2 = G_TempEntity( tr.endpos, EV_TETRION_HIT );
				do_damage = W_CheckBorgAdapt( ent, WP_TETRION_DISRUPTOR, traceEnt, tr.endpos );

				if( LogAccuracyHit( traceEnt, ent ) ) 
				{
					hitEnt = traceEnt;
				}
			} 
			else 
			{
				tent2 = G_TempEntity( tr.endpos, EV_TETRION_MISS );

				if ( ent->s.number == 0 )
				{
					//Add the event
					AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
					AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
				}
			}

			// Stash origins, etc. so that effects can have access to startPoints and other useful info.
			VectorCopy( start, tent2->s.origin2 );
			tent2->s.eventParm = DirToByte( tr.plane.normal );
			tent2->s.weapon = ent->s.weapon;
		}

		// Let's do some damage to whomever or whatever we hit
		if ( traceEnt->takedamage && do_damage ) 
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, TETRION_DAMAGE, DAMAGE_NO_KNOCKBACK, MOD_TETRION );
		}
	}	// next shot trace


	if ( hitEnt ) 
	{
		ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
	}
}

//---------------------------------------------------------
void FireTetrionProjectile( gentity_t *ent, vec3_t start, vec3_t dir )
//---------------------------------------------------------
{// Projectile that bounces off surfaces but does not have gravity
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "tetrion_projectile";
	bolt->nextthink = level.time + 4000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.eFlags |= EF_BOUNCE;
	bolt->bounceCount = Q_irand( 3, 5 ); // Give 'em three to five bounces

	bolt->alt_fire = qtrue;

	bolt->s.weapon = WP_TETRION_DISRUPTOR;
	bolt->damage = TETRION_ALT_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->owner = ent;
	bolt->methodOfDeath = MOD_TETRION;
	//bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	W_TraceSetStart( ent, start, bolt->mins, bolt->maxs );//make sure our start point isn't on the other side of a wall
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, TETRION_ALT_VELOCITY, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
}

//---------------------------------------------------------
void WP_FireTetrionDisruptor( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	vec3_t	dir;
	vec3_t	start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );
	VectorNormalize (dir);

	if ( alt_fire )
	{
		FireTetrionProjectile( ent, start, dir );
	}
	else
	{
		FireTetrionBullet( ent, start, dir );
	}
}

/*
----------------------------------------------
	DREADNOUGHT
----------------------------------------------
*/

#define DREADNOUGHT_DAMAGE		13
#define DN_VELOCITY				1400
#define DREADNOUGHT_ALT_DAMAGE	60

void WP_DreadnoughtBurstThink( gentity_t *ent );

//---------------------------------------------------------
void WP_FireDreadnoughtBeam( gentity_t *ent )
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		start, end, mins, maxs;
	gentity_t	*traceEnt;
	qboolean	do_damage = qtrue;

	// Make sure that the damage trace corresponds with the trace done for the actual effect or weird things could happen
	if ( ent->s.number == 0 )
		VectorCopy( ent->client->renderInfo.eyePoint, start );
	else
		VectorCopy( muzzle, start );

	VectorMA( start, weaponData[WP_DREADNOUGHT].range, _forward, end );

	// Give the beam a bit of meat
	VectorSet( maxs, 2.0f, 2.0f, 2.0f );
	VectorScale( maxs, -1.0f, mins );

	// Find out who we've hit
	gi.trace( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	if ( !traceEnt->takedamage && ent->s.number == 0 ) 
	{
		//Add the event
		AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
		AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
	}

	do_damage = W_CheckBorgAdapt( ent, WP_DREADNOUGHT, traceEnt, tr.endpos );

	if ( traceEnt->takedamage )
	{
		if ( LogAccuracyHit( traceEnt, ent ) ) 
		{
			ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
		}
		
		if ( do_damage )
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, DREADNOUGHT_DAMAGE, 0, MOD_DREADNOUGHT);
		}
	}
}

#define DN_SEARCH_DIST		128
#define DN_SIDE_DIST		128
#define DN_RAND_DEV			24
#define DN_ALT_THINK_TIME	100
#define DN_ALT_SIZE			12

//---------------------------------------------------------
void WP_FireDreadnoughtBurst( gentity_t *ent )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();
	bolt->classname = "dn_projectile";
	
	bolt->nextthink = level.time + 200;
	bolt->e_ThinkFunc = thinkF_WP_DreadnoughtBurstThink;
	bolt->count = 5;	//how many times it can damage something

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_DREADNOUGHT;
	bolt->owner = ent;
	bolt->damage = DREADNOUGHT_ALT_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_DREADNOUGHT;
	bolt->clipmask = MASK_SHOT;
	bolt->alt_fire = qtrue;

	bolt->fx_time = level.time;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	{
		vec3_t	mins={-DN_ALT_SIZE,-DN_ALT_SIZE,-1}, maxs={DN_ALT_SIZE,DN_ALT_SIZE,1};
		W_TraceSetStart( ent, start, mins, maxs );//make sure our start point isn't on the other side of a wall
	}
	VectorCopy( start, bolt->s.pos.trBase );
	VectorCopy( start, bolt->s.origin);

	//VectorScale( dir, DN_VELOCITY, bolt->s.pos.trDelta );
	VectorCopy( _forward, bolt->movedir);
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->currentOrigin );
	VectorCopy( start, bolt->pos1 );
	VectorCopy( start, bolt->pos2 );

	WP_DreadnoughtBurstThink(bolt);
}

//---------------------------------------------------------
void WP_DreadnoughtBurstThink( gentity_t *ent )
//---------------------------------------------------------
{
	static qboolean bAlreadyHere = qfalse;
	vec3_t	startpos, endpos, perp;
	trace_t tr;
	gentity_t *traceEnt, *tent;
	int		source;
	vec3_t	dest, mins={-DN_ALT_SIZE,-DN_ALT_SIZE,-1}, maxs={DN_ALT_SIZE,DN_ALT_SIZE,1};
	float	dot;	

	ent->count--;	//dec the count of repeat hits
	VectorCopy(ent->s.origin, startpos);

	// Search in a 3-way arc in front of it.
	VectorMA( startpos, DN_SEARCH_DIST, ent->movedir, endpos );
	endpos[0] += crandom() * DN_RAND_DEV;
	endpos[1] += crandom() * DN_RAND_DEV;
	endpos[2] += crandom() * DN_RAND_DEV * 0.5f;

	// unlink this entity, so the next trace will go past it
	gi.unlinkentity( ent );

	if ( ent->enemy ) {
		source = ent->enemy->s.number;
	} else if ( ent->owner ) {
		source = ent->owner->s.number;
	} else {
		source = ent->s.number;
	}

	gi.trace( &tr, startpos, mins, maxs, endpos, source, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	if ( traceEnt->takedamage && traceEnt->health > 0 && ent->count > 0 ) 
	{
		if ( traceEnt->client )
		{
			if ( traceEnt->client->playerTeam == TEAM_STARFLEET || traceEnt->client->noclip )
			{
				// Scale down damage, hurt them, but don't stick on them
				G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage * 0.5f, 0, MOD_DREADNOUGHT );
				goto ignore1;
			}
			G_SetEnemy(ent, traceEnt);
			VectorCopy( ent->s.origin, ent->s.origin2 );
			VectorCopy( traceEnt->currentOrigin, ent->s.origin );

			gi.linkentity( ent );

			VectorNormalize( ent->movedir );
			ent->nextthink = level.time + DN_ALT_THINK_TIME;

			G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
			return;
		}

		G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
	}
	else
	{
		if ( tr.fraction < 0.02f || ent->count < 1 )
		{	
			// Hit a wall...
			dot = DotProduct( ent->movedir, tr.plane.normal );

			if ( !traceEnt->takedamage && ent->owner && ent->owner->s.number == 0 ) 
			{
				//Add the event
				AddSoundEvent( ent->owner, tr.endpos, 256, AEL_DISCOVERED );
				AddSightEvent( ent->owner, tr.endpos, 512, AEL_DISCOVERED );
			}

			if ( dot < -0.6f || dot == 0.0f || (tr.surfaceFlags & SURF_NOIMPACT) || ent->count < 1 )
			{	
				// Stop.
				G_FreeEntity( ent );

				if ( !(tr.surfaceFlags & SURF_NOIMPACT) )
				{
					tent = G_TempEntity( tr.endpos, EV_DREADNOUGHT_MISS );

					// Stash origins, etc. so that the effects can have access to them
					VectorCopy( startpos, tent->s.origin2 );
					tent->s.eventParm = DirToByte( tr.plane.normal );
				}

				return;
			}
			else
			{
				// Bounce off the surface just a little
				VectorMA( ent->movedir, -1.25f * dot, tr.plane.normal, ent->movedir );
				VectorNormalize( ent->movedir );
				
				// Make sure we store a next think time, else the effect could sit around forever...
				ent->nextthink = level.time + DN_ALT_THINK_TIME;

				// this stops a runaway crash bug when the insects break through the corridor in voy16
				if (!bAlreadyHere)
				{
					bAlreadyHere = qtrue;
					WP_DreadnoughtBurstThink( ent );// NOTE RECURSION HERE.
					bAlreadyHere = qfalse;
				}
				return;
			}
		}
		VectorCopy( tr.endpos, dest );
	}

	// Didn't hit anything forward.  Try some side vectors.
	perp[0] = ent->movedir[1];
	perp[1] = -ent->movedir[0];
	perp[2] = ent->movedir[2];

	// Search a random interval from the side arc
	VectorMA( endpos, DN_SIDE_DIST, perp, endpos );
	endpos[0] += crandom() * DN_RAND_DEV;
	endpos[1] += crandom() * DN_RAND_DEV;
	endpos[2] += crandom() * DN_RAND_DEV * 0.5f;

	gi.trace( &tr, startpos, mins, maxs, endpos, source, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	if ( traceEnt->takedamage && traceEnt->health > 0 ) 
	{
		if ( traceEnt->client )
		{
			if ( traceEnt->client->playerTeam == TEAM_STARFLEET || traceEnt->client->noclip )
			{
				goto ignore1;
			}

			G_SetEnemy( ent, traceEnt );

			VectorCopy( ent->s.origin, ent->s.origin2 );
			VectorCopy( traceEnt->currentOrigin, ent->s.origin );

			gi.linkentity( ent );

			VectorNormalize( ent->movedir );
			ent->nextthink = level.time + DN_ALT_THINK_TIME;

			G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
			return;
		}

		G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
	}

	// Search a random interval in the opposite direction
	VectorMA( endpos, -2.0f * DN_SIDE_DIST, perp, endpos );
	endpos[0] += crandom() * DN_RAND_DEV;
	endpos[1] += crandom() * DN_RAND_DEV;
	endpos[2] += crandom() * DN_RAND_DEV * 0.5f;

	gi.trace( &tr, startpos, mins, maxs, endpos, source, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	if ( traceEnt->takedamage && traceEnt->health > 0 ) 
	{
		if ( traceEnt->client )
		{
			if ( traceEnt->client->playerTeam == TEAM_STARFLEET || traceEnt->client->noclip )
			{
				goto ignore1;
			}
			G_SetEnemy(ent, traceEnt);
			VectorCopy( ent->s.origin, ent->s.origin2 );
			VectorCopy( traceEnt->currentOrigin, ent->s.origin );

			gi.linkentity( ent );
			VectorNormalize( ent->movedir );
			ent->nextthink = level.time + DN_ALT_THINK_TIME;

			G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
			return;
		}

		G_Damage( traceEnt, ent, ent->owner, _forward, tr.endpos, ent->damage, 0, MOD_DREADNOUGHT );
	}

ignore1:

	// We didn't find anything, so move the entity to the middle destination.
	ent->enemy = NULL;

	VectorCopy( ent->s.origin, ent->s.origin2 );
	VectorCopy( dest, ent->s.origin );
	gi.linkentity( ent );

	ent->nextthink = level.time + DN_ALT_THINK_TIME;
	return;
}


//---------------------------------------------------------
void WP_FireDreadnought( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	// This was moved out of the FireWeapon switch statement below to keep things more consistent
	if ( alt_fire )
	{
		WP_FireDreadnoughtBurst( ent );
	}
	else
	{
		WP_FireDreadnoughtBeam( ent );
	}
}


/*
----------------------------------------------
	QUANTUM BURST
----------------------------------------------
*/

#define QUANTUM_VELOCITY	900
#define QUANTUM_DAMAGE		100
#define QUANTUM_SPLASH_DAM	64
#define QUANTUM_SPLASH_RAD	128

#define QUANTUM_ALT_VELOCITY	500
#define QUANTUM_ALT_DAMAGE		125
#define QUANTUM_ALT_SPLASH_DAM	100
#define QUANTUM_ALT_SPLASH_RAD	128
#define QUANTUM_ALT_THINK_TIME	200
#define QUANTUM_ALT_SEARCH_DIST	1024

//---------------------------------------------------------
void FireQuantumBurst( gentity_t *ent, vec3_t start, vec3_t dir )
//---------------------------------------------------------
{
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "quantum_projectile";
	
	bolt->nextthink = level.time + 6000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_QUANTUM_BURST;
	bolt->owner = ent;

	bolt->damage = QUANTUM_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = QUANTUM_SPLASH_DAM;
	bolt->splashRadius = QUANTUM_SPLASH_RAD;

	bolt->methodOfDeath = MOD_QUANTUM;
	bolt->splashMethodOfDeath = MOD_QUANTUM_SPLASH;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->maxs, 3.0f, 3.0f, 3.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	
	VectorScale( dir, QUANTUM_VELOCITY, bolt->s.pos.trDelta );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
	VectorCopy (start, bolt->pos1);
}

//---------------------------------------------------------
void FireQuantumBurstAlt( gentity_t *ent, vec3_t start, vec3_t dir )
//---------------------------------------------------------
{
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "quantum_alt_projectile";
	
//	bolt->nextthink = level.time + 6000;
//	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->nextthink = level.time + 100;
	bolt->e_ThinkFunc = thinkF_WP_QuantumAltThink;
	bolt->health = 50;	// 10 seconds.

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_QUANTUM_BURST;
	bolt->owner = ent;
	bolt->alt_fire = qtrue;	// Things won't work if this doesn't get set!!

	bolt->damage = QUANTUM_ALT_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = QUANTUM_ALT_SPLASH_DAM;
	bolt->splashRadius = QUANTUM_ALT_SPLASH_RAD;

	bolt->methodOfDeath = MOD_QUANTUM;
	bolt->splashMethodOfDeath = MOD_QUANTUM_SPLASH;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->maxs, 3.0f, 3.0f, 3.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	
	VectorScale( dir, QUANTUM_ALT_VELOCITY, bolt->s.pos.trDelta );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
	VectorCopy(dir, bolt->movedir);
}

//---------------------------------------------------------
qboolean SearchTarget( gentity_t *ent, vec3_t start, vec3_t end )
{
	trace_t tr;
	gentity_t *traceEnt;
	vec3_t fwd;

	gi.trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	if( traceEnt->takedamage && traceEnt->client && traceEnt->health > 0 ) 
	{
		G_SetEnemy(ent, traceEnt);
		if ( ent->enemy )
		{
			VectorSubtract( ent->enemy->currentOrigin, ent->currentOrigin, fwd );
			VectorNormalize( fwd );
			VectorScale( fwd, QUANTUM_ALT_VELOCITY, ent->s.pos.trDelta );
			VectorCopy( fwd, ent->movedir );

			SnapVector( ent->s.pos.trDelta );			// save net bandwidth
			VectorCopy( ent->currentOrigin, ent->s.pos.trBase );

			ent->s.pos.trTime = level.time;
			ent->nextthink = level.time + QUANTUM_ALT_THINK_TIME;

			return qtrue;
		}
	}
	return qfalse;
}

//---------------------------------------------------------
void WP_QuantumAltThink( gentity_t *ent )
{
	vec3_t start, newdir, targetdir, up={0,0,1}, right, search; 
	float dot, dot2;

	ent->health--;
	if( ent->health <= 0 )
	{
		G_FreeEntity( ent );
		return;
	}

	if ( ent->enemy )
	{	
		// Already have a target, start homing.
		if ( !ent->enemy->inuse || ent->enemy->health <= 0 )
		{	
			// No longer target this.
			ent->enemy = NULL;
			ent->nextthink = level.time + 1000;
			ent->health -= 5;
			return;
		}

		VectorSubtract( ent->enemy->currentOrigin, ent->currentOrigin, targetdir );
		VectorNormalize( targetdir );

		// Now the rocket can't do a 180 in space, so we'll limit the turn to about 45 degrees.
		dot = DotProduct( targetdir, ent->movedir );

		// a dot of 1.0 means right-on-target.
		if ( dot < 0.0f )
		{	
			// Go in the direction opposite, start a 180.
			CrossProduct( ent->movedir, up, right );
			dot2 = DotProduct( targetdir, right );

			if ( dot2 > 0 )
			{	
				// Turn 45 degrees right.
				VectorAdd( ent->movedir, right, newdir );
			}
			else
			{	
				// Turn 45 degrees left.
				VectorSubtract(ent->movedir, right, newdir);
			}

			// Yeah we've adjusted horizontally, but let's split the difference vertically, so we kinda try to move towards it.
			newdir[2] = ( targetdir[2] + ent->movedir[2] ) * 0.5;
			VectorNormalize( newdir );
		}
		else if ( dot < 0.7 )
		{	
			// Need about one correcting turn.  Generate by meeting the target direction "halfway".
			// Note, this is less than a 45 degree turn, but it is sufficient.  We do this because the rocket may have to go UP.
			VectorAdd( ent->movedir, targetdir, newdir );
			VectorNormalize( newdir );
		}
		else
		{	
			// else adjust to right on target.
			VectorCopy( targetdir, newdir );
		}

		VectorScale( newdir, QUANTUM_ALT_VELOCITY, ent->s.pos.trDelta );
		VectorCopy( newdir, ent->movedir );
		SnapVector( ent->s.pos.trDelta );			// save net bandwidth
		VectorCopy( ent->currentOrigin, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
	}
	else
	{	
		// Search in front of the missile for targets.
		VectorCopy( ent->currentOrigin, start );
		CrossProduct( ent->movedir, up, right );

		// Search straight ahead.
		VectorMA( start, QUANTUM_ALT_SEARCH_DIST, ent->movedir, search );
		if ( SearchTarget( ent, start, search ))
			return;

		// Search to the right.
		VectorMA( search, QUANTUM_ALT_SEARCH_DIST * 0.1f, right, search );
		if ( SearchTarget(ent, start, search ))
			return;
		
		// Search to the left.
		VectorMA( search, -QUANTUM_ALT_SEARCH_DIST * 0.2f, right, search );
		if ( SearchTarget( ent, start, search ))
			return;
	}

	ent->nextthink = level.time + QUANTUM_ALT_THINK_TIME;	// Nothing at all spectacular happened, continue.
	return;
}

//---------------------------------------------------------
void WP_FireQuantumBurst( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	vec3_t	dir, start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );

	if ( alt_fire )
	{
		FireQuantumBurstAlt( ent, start, dir );
	}
	else
	{
		FireQuantumBurst( ent, start, dir );
	}
}

//---------------------------------------------------------
void WP_TricorderScan (gentity_t *ent, qboolean alt_fire)
//---------------------------------------------------------
{
	static	int	sound_debounce_time;

	if ( sound_debounce_time < level.time )
	{
		if ( alt_fire )
		{
			sound_debounce_time = level.time + Q_irand(500, 1500);
		}
		else
		{
			sound_debounce_time = level.time + 5000;
		}
	}
}

/*
----------------------------------------------
	PROTON GUN
----------------------------------------------
*/

#define	PROTON_DAMAGE		25
#define COMPRESSION_SPREAD	100

//---------------------------------------------------------
void WP_FireProtonGun ( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	gentity_t	*tent;
	gentity_t	*tent2 = 0;
	gentity_t	*traceEnt;
	int			damage = alt_fire ? PROTON_DAMAGE * 6 : PROTON_DAMAGE; // do butt-loads of damage for alt
	trace_t		tr;
	vec3_t		start, end;
	qboolean	do_damage = qtrue;
	qboolean	render_impact = qtrue;

	if ( ent->s.number == 0 && !cg_thirdPerson.integer )
		// The trace start will originate at the eye so we can ensure that it hits the crosshair.
		// This can cause small clipping errors, but it's probably not that big of a deal at this point.
		VectorCopy( ent->client->renderInfo.eyePoint, start );
	else
		VectorCopy( muzzle, start );

	VectorMA( start, -8, _forward, start );
	VectorMA( start, 8192, _forward, end );

	gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

	// If the beam hits a skybox, etc. it would look foolish to add in an explosion
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		render_impact = qfalse;
	}
	
	traceEnt = &g_entities[ tr.entityNum ];

	// Render a shot in any case.
	if ( alt_fire )
		tent = G_TempEntity( tr.endpos, EV_PROTON_GUN_ALT );
	else
		tent = G_TempEntity( tr.endpos, EV_PROTON_GUN );

	// Only add in impact stuff when told to do so
	if ( render_impact )
	{
		// send bullet impact
		if ( traceEnt->takedamage && traceEnt->client ) 
		{
			// Create a simple impact type mark
			tent2 = G_TempEntity( tr.endpos, EV_PROTON_GUN_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
			do_damage = W_CheckBorgAdapt( ent, WP_PROTON_GUN, traceEnt, tr.endpos );
			
			if( LogAccuracyHit( traceEnt, ent ) ) 
			{
				ent->client->ps.persistant[PERS_ACCURACY_HITS]++;
			}
		} 
		else 
		{
			if ( alt_fire)
			{
				// Do a discreet miss
				tent2 = G_TempEntity( tr.endpos, EV_PROTON_GUN_ALT_MISS );
				AddSightEvent( ent, tr.endpos, 256, AEL_SUSPICIOUS );
			}
			else
			{
				// Create an explosion
				tent2 = G_TempEntity( tr.endpos, EV_PROTON_GUN_MISS );

				// FIXME:  Is this really an ok place to add this?
				if ( ent->s.number == 0 )
				{
					//Add the event
					AddSoundEvent( ent, tr.endpos, 256, AEL_DISCOVERED );
					AddSightEvent( ent, tr.endpos, 512, AEL_DISCOVERED );
				}
			}
		}
	}

	// Stash origins, etc. so that the effect can have access to them.
	VectorCopy( muzzle, tent->s.origin2 );

	if ( render_impact )
	{
		VectorCopy( muzzle, tent2->s.origin2 );
		tent2->s.eventParm = tent->s.eventParm = DirToByte( tr.plane.normal );
		tent2->s.weapon = tent->s.weapon = ent->s.weapon;
	} 

	if ( traceEnt->takedamage && do_damage )
	{
		// Do less damage when it's an NPC that is shooting this weapon, this is done so that they can still shoot a lot
		//	but not necessarily kill everything for you.
		if ( ent->client->ps.clientNum != 0 )
		{
			damage *= 0.5;
		}

		if ( alt_fire )
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_SNIPER );
		}
		else
		{
			G_Damage( traceEnt, ent, ent, _forward, tr.endpos, damage, 0, MOD_CRIFLE );
		}
	}
}

/*
----------------------------------------------
	BORG WEAPONS
----------------------------------------------
*/

#define BORG_PROJ_DAMAGE	10
#define BORG_PROJ_SPEED		500

//---------------------------------------------------------
void WP_FireBorgWeapon( gentity_t *ent )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );
	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "borg_projectile";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BORG_WEAPON;
	bolt->owner = ent;
	bolt->damage = BORG_PROJ_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_ENERGY;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, BORG_PROJ_SPEED, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
}


#define	BORG_ATTACK_DELAY	100

void CG_DrawNode( vec3_t origin, int type );

//---------------------------------------------------------
void WP_BorgAssimilate(gentity_t *borg)
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;

	if ( borg->attackDebounceTime > level.time )
		return;

	muzzle[2] -= 8;

	VectorMA (muzzle, 64, _forward, end);

	gi.trace ( &tr, muzzle, NULL, NULL, end, borg->s.number, MASK_SHOT );
	
	if ( tr.surfaceFlags & SURF_NOIMPACT )
		return;

	traceEnt = &g_entities[ tr.entityNum ];

	//FIXME: generate effect and sound
	if ( traceEnt->takedamage && traceEnt->client && traceEnt->client->ps.stats[STAT_HEALTH] > 0) 
	{
		//You've been assimilated!

		float damage = ( g_spskill->integer == 2 ) ? 5.0f : 1.0f;

		G_Damage( traceEnt, borg, borg, _forward, tr.endpos, damage, DAMAGE_NO_KNOCKBACK, MOD_ASSIMILATE );
	}

	//Basically, longer on easy, less on hard
	int	attackDelay = ( ( 2 - ( ( g_spskill->integer ) % 3 ) ) + 1 ) * BORG_ATTACK_DELAY;

	borg->attackDebounceTime = level.time + attackDelay;
}

//---------------------------------------------------------
void WP_FireBorgTaser( gentity_t *ent )
//---------------------------------------------------------
{
	trace_t		tr;
	vec3_t		end, d_dir;
	gentity_t	*tent;
	gentity_t	*traceEnt;

	VectorMA (muzzle, weaponData[WP_BORG_TASER].range, _forward, end);

	gi.trace ( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
	{
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) 
	{
		tent = G_TempEntity( tr.endpos, EV_BTASER_HIT );
	} 
	else 
	{
		tent = G_TempEntity( tr.endpos, EV_BTASER_MISS );
	}

	VectorCopy( muzzle, tent->s.origin2 );
	tent->s.eventParm = DirToByte( tr.plane.normal );
	tent->s.weapon = ent->s.weapon;
	if ( ent->NPC_type && Q_stricmp( "satan", ent->NPC_type ) == 0 )
	{//satan's robot- more damage, greyscale effect
		tent->count = 1;
	}

	if ( traceEnt->takedamage) 
	{
		//For knockback - send them up in air
		VectorCopy(_forward, d_dir);

		if(d_dir[2] < 0.30f)
		{
			d_dir[2] = 0.30f;
		}

		VectorNormalize(d_dir);

		G_Damage( traceEnt, ent, ent, d_dir, tr.endpos, 5, 0, MOD_TASER );
	}
}

/*
----------------------------------------------
	SCOUT BOT
----------------------------------------------
*/

#define BOT_WELDER_RANGE	128

#define BOT_WELDER_DAMAGE_EASY		4
#define BOT_WELDER_DAMAGE_NORMAL	8
#define BOT_WELDER_DAMAGE_HARD		10

#define BOT_WELDER_SPEED_EASY	500
#define BOT_WELDER_SPEED_NORMAL	650
#define BOT_WELDER_SPEED_HARD	800

#define BOT_LASER_DAMAGE	5
#define BOT_LASER_RANGE		1024

//---------------------------------------------------------
void WP_BotWelder( gentity_t *ent )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	VectorCopy( _forward, dir );
	VectorCopy( muzzle, start );
	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "bot_projectile";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BOT_WELDER;
	bolt->owner = ent;
	
	if ( g_spskill->integer == 0 )
		bolt->damage = BOT_WELDER_DAMAGE_EASY;
	else if ( g_spskill->integer == 1 )
		bolt->damage = BOT_WELDER_DAMAGE_NORMAL;
	else
		bolt->damage = BOT_WELDER_DAMAGE_HARD;

	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_ENERGY;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
//	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
//	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );

	float speed;

	if ( g_spskill->integer == 0 )
		speed = BOT_WELDER_SPEED_EASY;
	else if ( g_spskill->integer == 1 )
		speed = BOT_WELDER_SPEED_NORMAL;
	else
		speed = BOT_WELDER_SPEED_HARD;

	VectorScale( dir, speed, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
	VectorCopy( start, bolt->pos1 );
}

void WP_BotLaser( gentity_t *ent )
//---------------------------------------------------------
{//Short burst laser, works like a long range lightning gun (held fire)
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;//, *tent;

	VectorMA( muzzle, BOT_LASER_RANGE, _forward, end );

	gi.trace( &tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT );

	if ( tr.entityNum == ENTITYNUM_NONE )
	{
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send bullet impact
	if ( traceEnt->takedamage && traceEnt->client ) 
	{
/*		tent = G_TempEntity( tr.endpos, EV_DREADNOUGHT_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
*/
	} 
	else if ( !( tr.surfaceFlags & SURF_NOIMPACT ) ) 
	{
/*	tent = G_TempEntity( tr.endpos, EV_DREADNOUGHT_MISS );
		tent->s.eventParm = DirToByte( tr.plane.normal );
*/
	}

	if ( traceEnt->takedamage)
	{
		G_Damage( traceEnt, ent, ent, _forward, tr.endpos, BOT_LASER_DAMAGE, 0, MOD_TARGET_LASER );
	}

	if ( !ent->s.loopSound )
	{
		G_Sound( ent, G_SoundIndex( "sound/weapons/scout_bot/laser_start.wav" ) );
		ent->s.loopSound = G_SoundIndex( "sound/weapons/scout_bot/laser_firing.wav" );
	}
}

/*
----------------------------------------------
	HUNTER SEEKER
----------------------------------------------
*/

#define BOT_ROCKET_DAMAGE		10
#define BOT_ROCKET_SPLASH_RAD	64
#define BOT_ROCKET_SPLASH_DAM	8
#define BOT_ROCKET_SPEED		250
#define BOT_ROCKET_CORRECT_AMT	0.05f // (0.0f - 1.0f) -- higher values are better
#define BOT_ROCKET_DRUNK_AMT	0.009f	

void bot_rocket_think( gentity_t *ent )
//---------------------------------------------------------
{
	vec3_t newdir, targetdir, dir, org;
	float	dot;

	if ( ent->owner && ent->owner->enemy )
	{
		// Already have a target, start homing.
		if ( !ent->owner->enemy->inuse || ent->owner->enemy->health <= 0 )
		{	
			// No longer target this.
			ent->owner = NULL;
			ent->nextthink = level.time + 1000;
			return;
		}

		AngleVectors( ent->owner->enemy->client->ps.viewangles, dir, NULL, NULL );

		dot = DotProduct( dir, ent->movedir );
		
		// If the thing gets behind us, don't track anymore
		if ( ( dot < -0.5f ) || ( ent->spawnflags & 2 ) )
		{
			VectorCopy( ent->owner->enemy->currentOrigin, org );
			org[2] += 22;

			VectorSubtract( org, ent->currentOrigin, targetdir );
			VectorNormalize( targetdir );

			// Do some direction correction
			if ( ent->spawnflags & 2 )
			{
				VectorMA( ent->movedir, BOT_ROCKET_CORRECT_AMT * 4, targetdir, newdir );
			}
			else
			{
				VectorMA( ent->movedir, BOT_ROCKET_CORRECT_AMT * (g_spskill->integer + 1), targetdir, newdir );
			}

			// Make the rocket act a bit "drunk"
			for ( int i = 0; i < 3; i++ )
			{
				newdir[i] += crandom() * BOT_ROCKET_DRUNK_AMT * (g_spskill->integer + 1) * (g_spskill->integer + 1);
			}
			VectorNormalize( newdir );

			if ( ent->spawnflags & 2 )
			{
				VectorScale( newdir, 400, ent->s.pos.trDelta );
			}
			else
			{
				VectorScale( newdir, ( g_spskill->integer ) * 75 + BOT_ROCKET_SPEED, ent->s.pos.trDelta );
			}

			VectorCopy( newdir, ent->movedir );
			SnapVector( ent->s.pos.trDelta );			// save net bandwidth
			VectorCopy( ent->currentOrigin, ent->s.pos.trBase );
			ent->s.pos.trTime = level.time;
		}
	}

	ent->nextthink = level.time + 100;
}

//---------------------------------------------------------
void bot_rocket_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	gentity_t	*tent;

	// Death
	tent = G_TempEntity(self->currentOrigin, EV_BOT_ROCKET_DIE);
	VectorAdd( self->currentOrigin, self->movedir, tent->s.origin2 );

	// remove the rocket after a bit
	self->e_ThinkFunc = thinkF_G_FreeEntity;
	self->nextthink = level.time + 100;
}

void WP_BotRocket( gentity_t *ent )
//---------------------------------------------------------
{//Rocket projectile
	gentity_t	*bolt;
	vec3_t		start;

	VectorMA( ent->currentOrigin, 6, _forward, start );
	VectorMA( start, 2, up, start );

	bolt = G_Spawn();

	VectorSet( bolt->mins, -4, -4, -4 );
	VectorSet( bolt->maxs, 4, 4, 4 );

	bolt->classname = "bot_rocket";
	bolt->nextthink = level.time + 200;
	bolt->e_ThinkFunc = thinkF_bot_rocket_think;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_BOT_ROCKET;
	bolt->owner = ent;
	bolt->damage = BOT_ROCKET_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = BOT_ROCKET_SPLASH_DAM;
	bolt->splashRadius = BOT_ROCKET_SPLASH_RAD;
	bolt->methodOfDeath = MOD_BOTROCKET;
	bolt->splashMethodOfDeath = MOD_BOTROCKET_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->contents = CONTENTS_SOLID;
	bolt->takedamage = qtrue;
	bolt->health = 10;
	bolt->e_DieFunc  = dieF_bot_rocket_die;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( _forward, BOT_ROCKET_SPEED, bolt->s.pos.trDelta );
	VectorCopy( _forward, bolt->movedir );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( start, bolt->currentOrigin );
	VectorCopy( start, bolt->pos1 );

	gi.linkentity( bolt );
}


void WP_BotTurret( gentity_t * ent )
//---------------------------------------------------------
{
	vec3_t		end;
	trace_t		tr;
	gentity_t	*tr_ent;

	VectorMA( ent->client->renderInfo.muzzlePoint, 1024, ent->pos2, end );
	gi.trace( &tr, ent->client->renderInfo.muzzlePoint, NULL, NULL, end, ent->s.number, MASK_SHOT );

	tr_ent = &g_entities[ tr.entityNum ];

	G_Damage( tr_ent, ent, ent, ent->pos2, tr.endpos, 3, 0, MOD_TARGET_LASER );
}

void WP_HunterSeeker( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	if ( alt_fire )
		WP_BotTurret( ent );
	else
		WP_BotRocket( ent );
}

/*
----------------------------------------------
	REAVER
----------------------------------------------
*/

#define FORGE_PROJECTILE_DAMAGE			10
#define FORGE_PROJECTILE_SPLASH_RAD		64
#define FORGE_PROJECTILE_SPLASH_DAM		8
#define FORGE_PROJECTILE_VELOCITY		750

void WP_ForgeProjectile( gentity_t *ent )
//---------------------------------------------------------
{//Energy projectile weapon
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "forge_projectile";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_FORGE_PROJ;
	bolt->owner = ent;
	bolt->damage = FORGE_PROJECTILE_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = FORGE_PROJECTILE_SPLASH_DAM;
	bolt->splashRadius = FORGE_PROJECTILE_SPLASH_RAD;
	bolt->methodOfDeath = MOD_ENERGY;
	bolt->splashMethodOfDeath = MOD_ENERGY_SPLASH;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( _forward, FORGE_PROJECTILE_VELOCITY, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( muzzle, bolt->currentOrigin );
}

/*
----------------------------------------------
	AVATAR
----------------------------------------------
*/

#define PSYCHIC_BLAST_DAMAGE		12
#define PSYCHIC_BLAST_SPLASH_DAM	5
#define PSYCHIC_BLAST_SPLASH_RAD	32
#define PSYCHIC_BLAST_VELOCITY		800

void WP_ForgePsychicBlast( gentity_t *ent )
//---------------------------------------------------------
{
	gentity_t *bolt;

	bolt				= G_Spawn();
	bolt->classname		= "psychic_projectile";
	bolt->nextthink		= level.time + 10000;
	bolt->e_ThinkFunc	= thinkF_G_FreeEntity;
	bolt->s.eType		= ET_MISSILE;
	bolt->svFlags		= SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon		= WP_FORGE_PSYCH;
	bolt->owner			= ent;
	bolt->damage		= PSYCHIC_BLAST_DAMAGE;
	bolt->dflags		= 0;
	bolt->splashDamage	= PSYCHIC_BLAST_SPLASH_DAM;
	bolt->splashRadius	= PSYCHIC_BLAST_SPLASH_RAD;
	bolt->methodOfDeath	= MOD_ENERGY;
	bolt->splashMethodOfDeath = MOD_ENERGY_SPLASH;
	bolt->clipmask		= MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -2.0f, -2.0f, -2.0f );
	VectorSet( bolt->maxs, 2.0f, 2.0f, 2.0f );

	bolt->s.pos.trType	= TR_LINEAR;
	bolt->s.pos.trTime	= level.time;		// move a bit on the very first frame

	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( _forward, PSYCHIC_BLAST_VELOCITY, bolt->s.pos.trDelta );

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( muzzle, bolt->currentOrigin );
}

/*
----------------------------------------------
	PARASITE
----------------------------------------------
*/

#define PARASITE_DAMAGE			6
#define PARASITE_SPLASH_DAM		4
#define PARASITE_SPLASH_RAD		72
#define PARASITE_VELOCITY		500
#define PARASITE_UPWARD_KICK	200

//---------------------------------------------------------
void WP_ParasiteAcid( gentity_t *ent )
//---------------------------------------------------------
{
	//shoots acid spurts at player
	gentity_t	*bolt;

	bolt = G_Spawn();
	bolt->classname = "acid_projectile";
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;
	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_PARASITE;
	bolt->owner = ent;
	bolt->damage = PARASITE_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = PARASITE_SPLASH_DAM;
	bolt->splashRadius = PARASITE_SPLASH_RAD;
	bolt->methodOfDeath = MOD_ACID;
	bolt->splashMethodOfDeath = MOD_ACID_SPLASH;
	bolt->clipmask = MASK_SHOT;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;								// move a bit on the very first frame
	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( _forward, PARASITE_VELOCITY, bolt->s.pos.trDelta );
	bolt->s.pos.trDelta[2] += PARASITE_UPWARD_KICK;					// give the spurt a bit of an extra upward thrust so it'll travel a bit further.
	SnapVector( bolt->s.pos.trDelta );								// save net bandwidth
	VectorCopy( muzzle, bolt->currentOrigin );
}

/*
----------------------------------------------
	STASIS ALIEN
----------------------------------------------
*/
#define STASIS_ALIEN_VELOCITY	450
#define STASIS_ALIEN_DAMAGE		5


//---------------------------------------------------------
void FireStasisGuyAttack( gentity_t *ent )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	VectorSet( dir, 0,0,1 ); // up

	// Move the shot up by his hands
	VectorMA( muzzle, 24, dir, start );
	VectorCopy( _forward, dir );
	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "stasis_alien_proj";
	
	bolt->nextthink = level.time + 10000;
	bolt->e_ThinkFunc = thinkF_G_FreeEntity;

	bolt->s.eType = ET_MISSILE;
	bolt->svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_STASIS_ATTACK;
	bolt->owner = ent;
	bolt->damage = STASIS_ALIEN_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->methodOfDeath = MOD_STASIS;
	bolt->clipmask = MASK_SHOT;

	// How 'bout we give this thing a size...
	VectorSet( bolt->mins, -4.0f, -4.0f, -4.0f );
	VectorSet( bolt->maxs, 4.0f, 4.0f, 4.0f );

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;
	VectorCopy( start, bolt->s.pos.trBase );
	
	VectorScale( dir, STASIS_ALIEN_VELOCITY, bolt->s.pos.trDelta );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->currentOrigin);
}

//---------------------------------------------------------
extern void Q3_SetVar( int taskID, int entID, const char *type_name, const char *data );

void G_HypoUsedHack( gentity_t *ent )
{
	//HACKHACKHACK
	//switch back to scav rifle after one use
	if ( ent->client->ps.stats[STAT_WEAPONS]&(1<<WP_SCAVENGER_RIFLE) )
	{
		CG_ChangeWeapon( WP_SCAVENGER_RIFLE );
	}
	else
	{
		CG_ChangeWeapon( WP_PHASER );
	}
	//remove the weapon from the inventory
	ent->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_BLUE_HYPO );
	ent->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_RED_HYPO );
	//clear this variable
	Q3_SetVar( -1, 0, "whichhypo", "NULL" );
	//HACKHACKHACK
}

void WP_SprayBlueHypo( gentity_t *ent )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;

	// Move out to the end of the nozzle
	//VectorMA( muzzle, 20, forward, muzzle );
	//VectorMA( muzzle, 4, vright, muzzle );

	VectorMA( muzzle, 24, _forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	gi.trace ( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );

	G_HypoUsedHack( ent );

	if ( tr.entityNum >= ENTITYNUM_WORLD )
	{
		gentity_t	*t_ent;

		// Create the effect -- thought something was needed here, but apparently not.
		VectorMA( muzzle, 20, _forward, muzzle );
		VectorMA( muzzle, 4, vright, muzzle );
		t_ent = G_TempEntity( muzzle, EV_HYPO_PUFF );
		t_ent->s.eventParm = qfalse;
		VectorCopy( _forward, t_ent->pos1 );
		return;
	}

	tr_ent = &g_entities[tr.entityNum];

	// This should only work on things that have a client
	if ( tr_ent->client )
	{
		G_Damage( tr_ent, ent, ent, _forward, tr.endpos, 0, DAMAGE_NO_DAMAGE, MOD_UNKNOWN );
		//GEntity_PainFunc( tr_ent, ent, 1 );
	}
}

//---------------------------------------------------------
void WP_SprayRedHypo( gentity_t *ent )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;

	// Move out to the end of the nozzle
	//VectorMA( muzzle, 20, forward, muzzle );
	//VectorMA( muzzle, 4, vright, muzzle );

	VectorMA( muzzle, 24, _forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	gi.trace ( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );

	G_HypoUsedHack( ent );

	if ( tr.entityNum >= ENTITYNUM_WORLD )
	{
		gentity_t *t_ent;

		// Create the effect -- thought something was needed here, but apparently not.
		VectorMA( muzzle, 20, _forward, muzzle );
		VectorMA( muzzle, 4, vright, muzzle );
		t_ent = G_TempEntity( muzzle, EV_HYPO_PUFF );
		t_ent->s.eventParm = qtrue;
		VectorCopy( _forward, t_ent->pos1 );
		return;
	}

	tr_ent = &g_entities[tr.entityNum];

	// This should only work on things that have a client
	if ( tr_ent && tr_ent->client )
	{
		if ( tr_ent->NPC_type && tr_ent->NPC_type[0] && Q_stricmp( tr_ent->NPC_type, "hirogenalpha" ) == 0 )
		{//If scavboss, just fire his target2
			G_UseTargets2( tr_ent, tr_ent, tr_ent->target2 );
			tr_ent->target2 = NULL;
			//so he doesn't repeat himself
			if ( tr_ent->NPC )
			{
				tr_ent->NPC->localState = 4;
			}
			//take some health away
			tr_ent->health -= 25;
			if ( tr_ent->health < 1 )
			{//but don't kill him
				tr_ent->health = 1;
			}
		}
		else
		{
			//knock them out, no deathscript, no deathtarget
			tr_ent->behaviorSet[BSET_DEATH] = NULL;
			tr_ent->behaviorSet[BSET_FFDEATH] = NULL;
			tr_ent->target = NULL;
			tr_ent->health = 0;
			
			if ( tr_ent->client->playerTeam == TEAM_STARFLEET )
			{//"killed" a teammate, you lose
				killPlayerTimer = level.time + 5000;
				statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
			}

			player_die( tr_ent, ent, ent, 0, MOD_KNOCKOUT );
			//G_Damage ( tr_ent, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
		}
	}
}

//---------------------------------------------------------
void WP_SprayVoyagerHypo( gentity_t *ent )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;

	VectorMA( muzzle, 32, _forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	gi.trace ( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );

	if ( tr.entityNum >= ENTITYNUM_WORLD )
	{
		gentity_t	*t_ent;

		// Create the effect -- thought something was needed here, but apparently not.
		VectorMA( muzzle, 20, _forward, muzzle );
		VectorMA( muzzle, 4, vright, muzzle );
		t_ent = G_TempEntity( muzzle, EV_HYPO_PUFF );
		t_ent->s.eventParm = qfalse;
		VectorCopy( _forward, t_ent->pos1 );
		return;
	}

	tr_ent = &g_entities[tr.entityNum];

	// Not exactly sure the best way to do this, but ah well...
	if ( tr_ent && tr_ent->health > 0 )
	{
		if ( tr_ent->health > tr_ent->max_health - 20 )
		{
			tr_ent->health = tr_ent->max_health;
		}
		else
		{
			tr_ent->health += 20;
		}
	}
}

//---------------------------------------------------------
void WP_KnifeSwing( gentity_t *ent )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;

	VectorMA( muzzle, 64, _forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	gi.trace ( &tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT );

	if ( tr.entityNum >= ENTITYNUM_WORLD )
	{
		return;
	}

	tr_ent = &g_entities[tr.entityNum];

	if ( tr_ent && tr_ent->takedamage )
	{
		G_Sound( tr_ent, G_SoundIndex( va("sound/enemies/harvester/stab%d", Q_irand(1, 3)) ) );
		G_Damage( tr_ent, ent, ent, _forward, tr.endpos, (g_spskill->integer*2)+1, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
	}
}

#define PALADIN_VELOCITY		700
#define PALADIN_DAMAGE			4

//---------------------------------------------------------
void WP_PaladinShot( gentity_t *ent )
{
	gentity_t	*bolt;

	bolt				= G_Spawn();
	bolt->classname		= "paladin_proj";
	bolt->nextthink		= level.time + 10000;
	bolt->e_ThinkFunc	= thinkF_G_FreeEntity;
	bolt->s.eType		= ET_MISSILE;
	bolt->s.weapon		= WP_PALADIN;
	bolt->owner			= ent;
	bolt->damage		= PALADIN_DAMAGE*(g_spskill->integer+1);
	bolt->dflags		= 0;
	bolt->splashDamage	= 0;
	bolt->splashRadius	= 0;
	bolt->methodOfDeath = MOD_SCAVENGER;
	bolt->clipmask		= MASK_SHOT;

	bolt->s.pos.trType	= TR_LINEAR	;
	bolt->s.pos.trTime	= level.time;		// move a bit on the very first frame

	VectorCopy( muzzle, bolt->s.pos.trBase );
	VectorScale( _forward, PALADIN_VELOCITY, bolt->s.pos.trDelta );

	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy( muzzle, bolt->currentOrigin);
}

#define DESPERADO_DAMAGE	3

//---------------------------------------------------------
void WP_DesperadoShot( gentity_t *ent )
{
	gentity_t	*tent;
	gentity_t	*tent2 = 0;
	gentity_t	*traceEnt;
	trace_t		tr;
	vec3_t		start, end;
	qboolean	do_damage = qtrue;
	qboolean	render_impact = qtrue;

	VectorCopy( muzzle, start );

	VectorMA( start, -8, _forward, start );
	VectorMA( start, 8192, _forward, end );

	gi.trace ( &tr, start, NULL, NULL, end, ent->s.number, MASK_SHOT );

	// If the beam hits a skybox, etc. it would look foolish to add in an explosion
	if ( tr.surfaceFlags & SURF_NOIMPACT ) 
		render_impact = qfalse;
	
	traceEnt = &g_entities[ tr.entityNum ];

	tent = G_TempEntity( tr.endpos, EV_RIFLE );

	// Only add in impact stuff when told to do so
	if ( render_impact )
	{
		// send bullet impact
		if ( traceEnt->takedamage && traceEnt->client ) 
		{
			// Create a simple impact type mark
			tent2 = G_TempEntity( tr.endpos, EV_RIFLE_HIT );
			tent->s.otherEntityNum = traceEnt->s.number;
		} 
		else 
		{
			// Do a discreet miss
			tent2 = G_TempEntity( tr.endpos, EV_RIFLE_MISS );
		}
	}

	// Stash origins, etc. so that the effect can have access to them.
	VectorCopy( muzzle, tent->s.origin2 );

	if ( render_impact )
	{
		VectorCopy( muzzle, tent2->s.origin2 );
		tent2->s.eventParm = tent->s.eventParm = DirToByte( tr.plane.normal );
		tent2->s.weapon = tent->s.weapon = ent->s.weapon;
	} 

	if ( traceEnt->takedamage && do_damage )
		G_Damage( traceEnt, ent, ent, _forward, tr.endpos, DESPERADO_DAMAGE*(g_spskill->integer+1), 0, MOD_CRIFLE );
}

//---------------------------------------------------------
void AddLeanOfs(gentity_t *ent, vec3_t point)
//---------------------------------------------------------
{
	if(ent->client)
	{
		if(ent->client->ps.leanofs)
		{
			vec3_t	right;
			//add leaning offset
			AngleVectors(ent->client->ps.viewangles, NULL, right, NULL);
			VectorMA(point, (float)ent->client->ps.leanofs, right, point);
		}
	}
}

//---------------------------------------------------------
void ViewHeightFix(gentity_t *ent)
//---------------------------------------------------------
{
	//FIXME: this is hacky and doesn't need to be here.  Was only put here to make up
	//for the times a crouch anim would be used but not actually crouching.
	//When we start calcing eyepos (SPOT_HEAD) from the tag_eyes, we won't need
	//this (or viewheight at all?)
	if ( !ent->client || !ent->NPC )
		return;

	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 )
		return;//dead

	if ( ( ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) == BOTH_CROUCH1IDLE || (ent->client->ps.legsAnim&~ANIM_TOGGLEBIT) == BOTH_CROUCH1 || (ent->client->ps.legsAnim&~ANIM_TOGGLEBIT ) == BOTH_CROUCH1WALK )
	{
		if ( ent->client->ps.viewheight!=ent->client->crouchheight + STANDARD_VIEWHEIGHT_OFFSET )
			ent->client->ps.viewheight = ent->client->crouchheight + STANDARD_VIEWHEIGHT_OFFSET;
	}
	else
	{
		if ( ent->client->ps.viewheight!=ent->client->standheight + STANDARD_VIEWHEIGHT_OFFSET )
			ent->client->ps.viewheight = ent->client->standheight + STANDARD_VIEWHEIGHT_OFFSET;
	}
}

/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}

//---------------------------------------------------------
void CalcMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint, float lead_in ) 
//---------------------------------------------------------
{
	if((ent->client->ps.weapon != WP_BORG_ASSIMILATOR) && (!lead_in)) //&& ent->s.number != 0 
	{//Not players or melee
		if( ent->client )
		{
			if ( ent->client->renderInfo.mPCalcTime >= level.time - FRAMETIME*2 )
			{//Our muzz point was calced no more than 2 frames ago
				VectorCopy(ent->client->renderInfo.muzzlePoint, muzzlePoint);
				return;
			}
		}
	}

	VectorCopy( ent->currentOrigin, muzzlePoint );
	
	switch( ent->s.weapon )
	{
	case WP_IMOD:
		muzzlePoint[2] += ent->client->ps.viewheight;//By eyes
		muzzlePoint[2] -= 13;
		VectorMA( muzzlePoint, 26, forward, muzzlePoint );
		VectorMA( muzzlePoint, 6, vright, muzzlePoint );
		break;

	case WP_COMPRESSION_RIFLE:
	case WP_RED_HYPO:
	case WP_BLUE_HYPO:
	case WP_VOYAGER_HYPO:
	case WP_TETRION_DISRUPTOR:
	case WP_PROTON_GUN:	
		ViewHeightFix(ent);
		muzzlePoint[2] += ent->client->ps.viewheight;//By eyes
		muzzlePoint[2] -= 16;
		VectorMA( muzzlePoint, 28, forward, muzzlePoint );
		VectorMA( muzzlePoint, 6, vright, muzzlePoint );
		break;

	case WP_STASIS:
	case WP_QUANTUM_BURST:
	case WP_GRENADE_LAUNCHER:
		ViewHeightFix(ent);
		muzzlePoint[2] += ent->client->ps.viewheight;//By eyes
		muzzlePoint[2] -= 2;
		break;

	case WP_SCAVENGER_RIFLE:
	case WP_CHAOTICA_GUARD_GUN:
		ViewHeightFix(ent);
		muzzlePoint[2] += ent->client->ps.viewheight;//By eyes
		muzzlePoint[2] -= 1;
		if ( ent->s.number == 0 )
			VectorMA( muzzlePoint, 12, forward, muzzlePoint ); // player, don't set this any lower otherwise the projectile will impact immediately when your back is to a wall
		else
			VectorMA( muzzlePoint, 2, forward, muzzlePoint ); // NPC, don't set too far forward otherwise the projectile can go through doors

		VectorMA( muzzlePoint, 1, vright, muzzlePoint );
		break;

	case WP_BORG_TASER:
	case WP_BORG_WEAPON:
	case WP_PHASER:
		if(ent->NPC!=NULL &&
			(ent->client->ps.torsoAnim&~ANIM_TOGGLEBIT == TORSO_WEAPONREADY2 ||
			ent->client->ps.torsoAnim&~ANIM_TOGGLEBIT == BOTH_ATTACK2))//Sniper pose
		{
			ViewHeightFix(ent);
			muzzle[2] += ent->client->ps.viewheight;//By eyes
		}
		else
		{
			muzzlePoint[2] += 16;
		}
		VectorMA( muzzlePoint, 8, forward, muzzlePoint );
		VectorMA( muzzlePoint, 16, vright, muzzlePoint );
		break;

	case WP_BORG_ASSIMILATOR:
		ViewHeightFix(ent);
		muzzlePoint[2] += ent->maxs[2];//ent->client->ps.viewheight;//By eyes
		VectorMA( muzzlePoint, 8, forward, muzzlePoint );
		VectorMA( muzzlePoint, -4, vright, muzzlePoint );
		break;

	case WP_PARASITE:
		muzzlePoint[2] -= 23;
		break;

	case WP_BOT_WELDER:
		muzzlePoint[2] -= 8;	//Near the naughty-bits
		break;

	case WP_BOT_ROCKET:
		break;
	}

	AddLeanOfs(ent, muzzlePoint);
}

//---------------------------------------------------------
void FireWeapon( gentity_t *ent, qboolean alt_fire ) 
//---------------------------------------------------------
{
	// track shots taken for accuracy tracking. 
	ent->client->ps.persistant[PERS_ACCURACY_SHOTS]++;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, _forward, vright, up);

	ent->alt_fire = alt_fire;
	CalcMuzzlePoint ( ent, _forward, vright, up, muzzle , 0);

	// fire the specific weapon
	switch( ent->s.weapon ) 
	{
	// Player weapons
	//-----------------
	case WP_PHASER:
		WP_FirePhaser( ent, alt_fire );
		break;

	case WP_COMPRESSION_RIFLE:
		WP_FireCompressionRifle( ent, alt_fire );
		break;

	case WP_IMOD:
		WP_FireIMOD( ent, alt_fire );
		break;

	case WP_SCAVENGER_RIFLE:
		WP_FireScavenger( ent, alt_fire, qfalse );
		break;

	case WP_CHAOTICA_GUARD_GUN:
		WP_FireScavenger( ent, qfalse, qtrue );
		break;

	case WP_STASIS:
		WP_FireStasis( ent, alt_fire );
		break;

	case WP_GRENADE_LAUNCHER:
		WP_FireGrenade( ent, alt_fire );
		break;

	case WP_TETRION_DISRUPTOR:
		WP_FireTetrionDisruptor( ent, alt_fire );
		break;

	case WP_DREADNOUGHT:
		WP_FireDreadnought( ent, alt_fire );
		break;

	case WP_QUANTUM_BURST:
		WP_FireQuantumBurst( ent, alt_fire );
		break;

	case WP_TRICORDER:
		WP_TricorderScan( ent, alt_fire );
		break;

	case WP_PROTON_GUN:
		WP_FireProtonGun( ent, qfalse );//no alt-fire
		break;

	// Borg Weapons
	//-----------------
	case WP_BORG_ASSIMILATOR:
	case WP_BORG_DRILL:
		WP_BorgAssimilate( ent );
		break;

	case WP_BORG_TASER:
		WP_FireBorgTaser( ent );
		break;

	case WP_BORG_WEAPON:
		WP_FireBorgWeapon( ent );
		break;

	// Scout Bot
	//-----------------
	case WP_BOT_WELDER:
		WP_BotWelder( ent );
		break;

/*
	case WP_BOT_LASER:
		WP_BotLaser( ent );
		break;
*/
	// Hunter Seeker
	//-----------------
	case WP_BOT_ROCKET:
		WP_HunterSeeker( ent, alt_fire );
		break;

	// Reaver
	//-----------------
	case WP_FORGE_PROJ:
		WP_ForgeProjectile( ent );
		break;
		
	// Avatar
	//-----------------
	case WP_FORGE_PSYCH:
		WP_ForgePsychicBlast( ent );
		break;

	// Parasite
	//-----------------
	case WP_PARASITE:
		WP_ParasiteAcid( ent );
		break;

	// Melee Placeholder, doesn't do much of anything as you can tell
	//-----------------
	case WP_MELEE:
		break;

	// Stasis Bad Boy
	//-----------------
	case WP_STASIS_ATTACK:
		FireStasisGuyAttack( ent );
		break;

	// Fun hypo "weapons"
	//-----------------
	case WP_BLUE_HYPO:
		WP_SprayBlueHypo( ent );
		break;

	case WP_RED_HYPO:
		WP_SprayRedHypo( ent );
		break;

	case WP_VOYAGER_HYPO:
		WP_SprayVoyagerHypo( ent );
		break;

	// Holo-character weapons
	//-----------------
	case WP_PALADIN:
		WP_PaladinShot( ent );
		break;

	case WP_DESPERADO:
		WP_DesperadoShot( ent );
		break;

	case WP_KLINGON_BLADE:
	case WP_IMPERIAL_BLADE:
		WP_KnifeSwing( ent );
		break;

	default:
		break;
	}
}

//DETPACK
//---------------------------------------------------------
void UseCharge (int entityNum)
//---------------------------------------------------------
{
	gentity_t	*ent;

	if(entityNum < 0 || entityNum >= ENTITYNUM_WORLD)
	{
		return;
	}

	ent = &g_entities[entityNum];

	if(!ent || !ent->client)
	{
		return;
	}

	if(ent->client->ps.eFlags & EF_PLANTED_CHARGE)
	{
		gentity_t *found = NULL;
		G_Sound(ent, G_SoundIndex("sound/weapons/detpacklatch.wav"));
		while((found = G_Find(found, FOFS(classname), "charge")) != NULL)
		{
			if(found->owner == ent)
			{
				VectorCopy( found->currentOrigin, found->s.origin );
				found->e_ThinkFunc = thinkF_ExplodeDeath;
				found->nextthink = level.time + 500;
				G_Sound(found, G_SoundIndex("sound/weapons/detpackfire.wav"));
				//Fixme: this should really wake up enemies in the area
			}
		}
		ent->client->ps.eFlags &= ~EF_PLANTED_CHARGE;
	}
	else
	{
		AngleVectors (ent->client->ps.viewangles, _forward, vright, up);

		CalcMuzzlePoint ( ent, _forward, vright, up, muzzle, 0 );

		VectorNormalize( _forward );
		VectorMA(muzzle, -4, _forward, muzzle);
		drop_charge (ent, muzzle, _forward);

		ent->client->ps.eFlags |= EF_PLANTED_CHARGE;
	}
}