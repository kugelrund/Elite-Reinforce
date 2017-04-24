// g_combat.c

#include "g_local.h"
#include "b_local.h"
#include "g_functions.h"
#include "anims.h"
#include "objectives.h"
#include "..\cgame\cg_text.h"
#include "..\cgame\cg_local.h"

extern	cvar_t	*g_debugDamage;
extern qboolean	stop_icarus;

gentity_t *g_lastClientDamaged;

int	ffireLevel = 0;	//Current level of "anger" for friendly fire
int ffireForgivenessTimer = 0;
int killPlayerTimer = 0;
int loadBrigTimer = 0;
extern int	teamCount[];
extern int	teamLastEnemyTime[];
extern const int FFIRE_LEVEL_RETALIATION = 10;

extern void NPC_TempLookTarget ( gentity_t *self, int lookEntNum, int minLookTime, int maxLookTime );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern qboolean PM_HasAnimation( gentity_t *ent, int animation );
extern qboolean G_TeamEnemy( gentity_t *self );
extern void ChangeWeapon( gentity_t *ent, int newWeapon );

extern void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
extern void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );
extern int PM_PickAnim( gentity_t *self, int minAnim, int maxAnim );
extern qboolean PM_InOnGroundAnim (gentity_t *self);

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	ent->client->ps.persistant[PERS_SCORE] += score;
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;
//	if ( weapon > WP_PHASER && self->client->ps.ammo[ weapon ] ) 
	if ( weapon > WP_PHASER && self->client->ps.ammo[ weaponData[weapon].ammoIndex ] )	// checkme
	{

		// find the item type for this weapon
		item = FindItemForWeapon( (weapon_t) weapon );

		// spawn the item
		Drop_Item( self, item, 0, qtrue );
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->currentAngles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}

void ObjectDie (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	if(self->target)
		G_UseTargets(self, attacker);

	//remove my script_targetname
	G_FreeEntity( self );
}
/*
==================
ExplodeDeath
==================
*/

//FIXME: all hacked up...

void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );
void ExplodeDeath( gentity_t *self ) 
{
//	gentity_t	*tent;
	vec3_t		forward;

	self->takedamage = qfalse;//stop chain reaction runaway loops

	self->s.loopSound = 0;

	VectorCopy( self->currentOrigin, self->s.pos.trBase );

//	tent = G_TempEntity( self->s.origin, EV_FX_EXPLOSION );
	AngleVectors(self->s.angles, forward, NULL, NULL);
	CG_SurfaceExplosion( self->currentOrigin, forward, 20.0f, 12.0f, ((self->spawnflags&4)==qfalse) );	//FIXME: This needs to be consistent to all exploders!
	G_Sound(self, self->sounds );
	
	if(self->splashDamage > 0 && self->splashRadius > 0)
	{
		gentity_t *attacker = self;
		if ( self->owner )
		{
			attacker = self->owner;
		}
		G_RadiusDamage( self->currentOrigin, attacker, self->splashDamage, self->splashRadius, 
				attacker, MOD_UNKNOWN );
	}

	ObjectDie( self, self, self, 20, 0 );
}

void ExplodeDeath_Wait( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) 
{
	self->e_DieFunc = dieF_NULL;
	self->nextthink = level.time + Q_irand(100, 500);
	self->e_ThinkFunc = thinkF_ExplodeDeath;
}

void GoExplodeDeath( gentity_t *self, gentity_t *other, gentity_t *activator) 
{
	if (self->behaviorSet[BSET_USE])
	{
		G_ActivateBehavior(self,BSET_USE);
	}

	self->targetname = "";	//Make sure this entity cannot be told to explode again (recursive death fix)

	ExplodeDeath( self );
}

void G_ActivateBehavior (gentity_t *self, int bset );
void G_CheckVictoryScript(gentity_t *self)
{
	if ( self->behaviorSet[BSET_VICTORY] )
	{
		G_ActivateBehavior( self, BSET_VICTORY );
	}
	else
	{
		G_AddVoiceEvent( self, Q_irand(EV_VICTORY1, EV_VICTORY3), 2000 );
	}
}

qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 )
{
	if ( !ent1->client || !ent2->client )
	{
		if ( ent1->noDamageTeam )
		{
			if ( ent2->client && ent2->client->playerTeam == ent1->noDamageTeam )
			{
				return qtrue;
			}
			else if ( ent2->noDamageTeam == ent1->noDamageTeam )
			{
				if ( ent1->splashDamage && ent2->splashDamage && Q_stricmp("ambient_etherian_fliers", ent1->classname) != 0 )
				{//Barrels, exploding breakables and mines will blow each other up
					return qfalse;
				}
				else
				{
					return qtrue;
				}
			}
		}
		return qfalse;
	}

	return ( ent1->client->playerTeam == ent2->client->playerTeam );
}

/*
-------------------------
G_AlertTeam
-------------------------
*/

void G_AlertTeam( gentity_t *victim, gentity_t *attacker, float radius, float soundDist )
{
	gentity_t	*radiusEnts[ 128 ];
	vec3_t		mins, maxs;
	int			numEnts;

	if ( attacker == NULL || attacker->client == NULL )
		return;

	//Setup the bbox to search in
	for ( int i = 0; i < 3; i++ )
	{
		mins[i] = victim->currentOrigin[i] - radius;
		maxs[i] = victim->currentOrigin[i] + radius;
	}

	//Get the number of entities in a given space
	numEnts = gi.EntitiesInBox( mins, maxs, radiusEnts, 128 );

	//Cull this list
	for ( int i = 0; i < numEnts; i++ )
	{
		//Validate clients
		if ( radiusEnts[i]->client == NULL )
			continue;

		//only want NPCs
		if ( radiusEnts[i]->NPC == NULL )
			continue;

		//Don't bother if they're ignoring enemies
		if ( radiusEnts[i]->svFlags & SVF_IGNORE_ENEMIES )
			continue;

		//This NPC specifically flagged to ignore alerts
		if ( radiusEnts[i]->NPC->aiFlags & NPCAI_IGNORE_ALERTS )
			continue;

		//Skip the requested avoid radiusEnts[i] if present
		if ( radiusEnts[i] == victim )
			continue;

		//Must be on the same team
		if ( radiusEnts[i]->client->playerTeam != victim->client->playerTeam )
			continue;

		//Must be alive
		if ( radiusEnts[i]->health <= 0 )
			continue;

		if ( radiusEnts[i]->enemy == NULL )
		{
			if ( DistanceSquared( radiusEnts[i]->currentOrigin, victim->currentOrigin ) > (soundDist*soundDist) )
			{
				if ( InFOV( victim, radiusEnts[i], radiusEnts[i]->NPC->stats.hfov, radiusEnts[i]->NPC->stats.vfov ) &&  NPC_ClearLOS( radiusEnts[i], victim->currentOrigin ) )
				{
					G_SetEnemy( radiusEnts[i], attacker );
				}

				continue;
			}

			//FIXME: This can have a nasty cascading effect if setup wrong...
			G_SetEnemy( radiusEnts[i], attacker );
		}
	}
}

/*
-------------------------
G_DeathAlert
-------------------------
*/

#define	DEATH_ALERT_RADIUS			512
#define	DEATH_ALERT_SOUND_RADIUS	256

void G_DeathAlert( gentity_t *victim, gentity_t *attacker )
{
	G_AlertTeam( victim, attacker, DEATH_ALERT_RADIUS, DEATH_ALERT_SOUND_RADIUS );
}

/*
----------------------------------------
DeathFX

Applies appropriate special effects that occur while the entity is dying
Not to be confused with NPC_RemoveBodyEffects (NPC.cpp), which only applies effect when removing the body
----------------------------------------
*/

static void DeathFX( gentity_t *ent )
{
	if ( !ent || !ent->client )
		return;

	switch( ent->client->playerTeam )
	{
	case TEAM_STASIS:
		// Scale them down while they die, plus play the teleport out effect for them
		ent->s.eFlags |= EF_SCALE_DOWN;
		ent->client->ps.eFlags |= EF_SCALE_DOWN;
		ent->fx_time = level.time;
		G_AddEvent( ent, EV_STASIS_TELEPORT_OUT, 0 );
		break;

	case TEAM_BORG:
		ent->s.eFlags |= EF_BORG_SPARKIES;
		ent->client->ps.eFlags |= EF_BORG_SPARKIES;
		break;

	case TEAM_FORGE:
		G_AddEvent( ent, EV_FORGE_FADE, 0 );
		break;

	default:
		break;
	}
}

void G_SetMissionStatusText( gentity_t *attacker, int mod )
{
	if ( statusTextIndex >= 0 )
	{
		return;
	}

	if ( mod == MOD_FALLING )
	{//fell to your death
		statusTextIndex = IGT_WATCHYOURSTEP;
	}
	else if ( mod == MOD_SURGICAL_LASER )
	{//fell to your death
		statusTextIndex = IGT_JUDGEMENTMUCHDESIRED;
	}
	else if ( mod == MOD_CRUSH )
	{//crushed
		statusTextIndex = IGT_JUDGEMENTMUCHDESIRED;
	}
	else if ( attacker && attacker->client && attacker->client->playerTeam == TEAM_BORG )
	{//assimilated
		statusTextIndex = Q_irand( IGT_RESISTANCEISFUTILE, IGT_NAMEIS8OF12 );
	}
	else if ( attacker && Q_stricmp( "trigger_hurt", attacker->classname ) == 0 )
	{//Killed by something that should have been clearly dangerous
//		statusTextIndex = Q_irand( IGT_JUDGEMENTDESIRED, IGT_JUDGEMENTMUCHDESIRED );
		statusTextIndex = IGT_JUDGEMENTMUCHDESIRED;
	}
	else if ( attacker && attacker->s.number != 0 && attacker->client && attacker->client->playerTeam == TEAM_STARFLEET )
	{//killed by a teammate
		statusTextIndex = IGT_INSUBORDINATION;
	}
	/*
	else if ()
	{//killed a teammate- note: handled above
		if ( Q_irand( 0, 1 ) )
		{
			statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
		}
		else
		{
			statusTextIndex = IGT_KILLEDANINNOCENTCREWMAN;
		}
	}
	else
	{
		//This next block is not contiguous
		IGT_INADEQUATE,
		IGT_RESPONSETIME,
		IGT_SHOOTINRANGE,
		IGT_TRYAGAIN,
		IGT_TRAINONHOLODECK,
		IGT_WHATCOLORSHIRT,
		IGT_NOTIMPRESS7OF9,
		IGT_NEELIXFAREDBETTER,
		IGT_THATMUSTHURT,
		IGT_TUVOKDISAPPOINTED,
		IGT_STARFLEETNOTIFYFAMILY,
		IGT_TEAMMATESWILLMISSYOU,
		IGT_LESSTHANEXEMPLARY,
		IGT_SACRIFICEDFORTHEWHOLE,
		IGT_NOTLIVELONGANDPROSPER,
		IGT_BETTERUSEOFSIMULATIONS,
	}
	*/

	/*
	//These can be set by designers
	IGT_INSUBORDINATION,
	IGT_YOUCAUSEDDEATHOFTEAMMATE,
	IGT_DIDNTPROTECTTECH,
	IGT_DIDNTPROTECT7OF9,
	IGT_NOTSTEALTHYENOUGH,
	IGT_STEALTHTACTICSNECESSARY,
	*/
}

void G_MakeTeamVulnerable( void )
{
	int i, newhealth;
	gentity_t *ent = &g_entities[0];
	gentity_t *self = &g_entities[0];
	if ( !self->client )
	{
		return;
	}

	for ( i = 0; i < globals.num_entities ; i++, ent++) 
	{
		if ( !ent )
		{
			continue;
		}
		if ( !ent->inuse  )
		{
			continue;
		}
		if ( !ent->client  )
		{
			continue;
		}
		if ( ent->client->playerTeam != TEAM_STARFLEET )
		{
			continue;
		}
		if ( !(ent->flags&FL_UNDYING) )
		{
			continue;
		}
		if ( !ent->client->squadname )
		{
			continue;
		}
		if ( !ent->client->squadname[0] )
		{
			continue;
		}
		if ( Q_stricmp(self->client->squadname, ent->client->squadname) )
		{
			continue;
		}
		ent->flags &= ~FL_UNDYING;
		newhealth = Q_irand( 5, 40 );
		if ( ent->health > newhealth )
		{
			ent->health = newhealth;
		}
	}
}

/*
==================
player_die
==================
*/
void NPC_SetAnim(gentity_t	*ent,int type,int anim,int priority);
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) 
{
	int			anim;
	int			contents;
	qboolean	deathScript = qfalse;

	if ( self->client->ps.pm_type == PM_DEAD )
		return;

	//Use any target we had
	if ( meansOfDeath != MOD_KNOCKOUT )
	{
		G_UseTargets( self, self );
	}
	//HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK
	if ( self->s.number == 0 && attacker && attacker->client && attacker->client->playerTeam == TEAM_BORG )
	{//when Munro is killed by a Borg, try to use the scriptrunner that plays the assimilated mini-cinematic
		G_UseTargets2( self, self, "assimilatemunro" );
	}
	//HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK

	if ( attacker )
	{
		G_CheckVictoryScript(attacker);
	}

	self->enemy = attacker;
	self->client->renderInfo.lookTarget = ENTITYNUM_WORLD;

	self->client->ps.persistant[PERS_KILLED]++;
	if ( self->client->playerTeam == TEAM_STARFLEET )
	{//FIXME: just HazTeam members in formation on away missions?
		//or more controlled- via deathscripts?
		// Don't count player
		if (( &g_entities[0] != NULL && g_entities[0].client ) && (self->s.number != 0))
		{//add to the number of teammates lost
			g_entities[0].client->ps.persistant[PERS_TEAMMATES_KILLED]++;
		}
		else	// Player died, fire off scoreboard soon
		{
			cg.missionStatusDeadTime = level.time + 1000;	// Too long?? Too short??
		}
	}

	if ( self->s.number == 0 && attacker )
	{
		G_SetMissionStatusText( attacker, meansOfDeath );
		//TEST: If player killed, unmark all teammates from being undying so they can buy it too
		//NOTE: we want this to happen ONLY on our squad ONLY on missions... in the tutorial or on voyager levels this could be really weird.
		G_MakeTeamVulnerable();
	}

	if ( attacker && attacker->client) 
	{
		if ( attacker == self || OnSameTeam (self, attacker ) ) 
		{
			AddScore( attacker, -1 );
		} 
		else 
		{
			AddScore( attacker, 1 );
		}
	} 
	else 
	{
		AddScore( self, -1 );
	}

	// if client is in a nodrop area, don't drop anything
	contents = gi.pointcontents( self->currentOrigin, -1 );
	if ( self->s.number != 0 && !( contents & CONTENTS_NODROP ) && (self->s.weapon == WP_COMPRESSION_RIFLE || self->s.weapon == WP_SCAVENGER_RIFLE)) 
	{
		TossClientItems( self );
	}

	self->takedamage = qfalse;	// no gibbing

	// Sigh...borg shouldn't drop their weapon attachments when they die..
	if ( self->client->playerTeam != TEAM_BORG )
	{
		self->s.weapon = WP_NONE;
	}

	self->s.powerups = 0;
	//FIXME: do this on a callback?  So people can't walk through long death anims?
	//Maybe set on last frame?  Would be cool for big blocking corpses if the never got set?
	if ( self->client->playerTeam == TEAM_PARASITE )
	{
		self->contents = CONTENTS_NONE; // FIXME: temp fix
	}
	else
	{
		self->contents = CONTENTS_CORPSE;
		self->maxs[2] = -8;
	}
	self->clipmask&=~CONTENTS_MONSTERCLIP;//so dead NPC can fly off ledges
	self->currentAngles[0] = 0;
	self->currentAngles[2] = 0;
	if ( attacker )
	{
		LookAtKiller (self, inflictor, attacker);
	}

	VectorCopy( self->currentAngles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	if(meansOfDeath == MOD_FALLING && self->client->playerTeam == TEAM_STARFLEET)
	{
		if ( self->client->ps.groundEntityNum == ENTITYNUM_NONE )
		{
			NPC_SetAnim(self, SETANIM_BOTH, BOTH_FALLDEATH1INAIR, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			//FIXME: make this an event
//			G_Sound(self, G_SoundIndex("*falling1.wav"));	//we never got this sound...
			G_Sound(self, G_SoundIndex("*fall1.wav"));
			self->client->ps.gravity *= 0.5;//Fall a bit slower
		}
		else
		{
			NPC_SetAnim(self, SETANIM_BOTH, BOTH_FALLDEATH1LAND, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			//FIXME: make this an event
			//PM_AddEvent( EV_FALL_FAR );
			G_Sound(self, G_SoundIndex("*fall1.wav"));//FIXME: CRUNCH sounding hard land?
		}
	}
	else
	{// normal death
		anim = PM_PickAnim( self, BOTH_DEATH1, BOTH_DEATH7 );	//initialize to good data

		if ( (self->s.legsAnim&~ANIM_TOGGLEBIT) == BOTH_RESTRAINED1 || (self->s.legsAnim&~ANIM_TOGGLEBIT) == BOTH_RESTRAINED1POINT )
		{//super special case, floating in air lying down
			anim = BOTH_RESTRAINED1;
		}
		else if ( PM_InOnGroundAnim( self ) && PM_HasAnimation( self, BOTH_LYINGDEATH1 ) )
		{//on ground, need different death anim
			anim = BOTH_LYINGDEATH1;
		}
		else if(self->client->ps.pm_time > 0 && self->client->ps.pm_flags & PMF_TIME_KNOCKBACK && self->client->ps.velocity[2] > 0)
		{
			float	thrown, dot;
			vec3_t	throwdir, forward;
			
			AngleVectors(self->currentAngles, forward, NULL, NULL);
			thrown = VectorNormalize2(self->client->ps.velocity, throwdir);
			dot = DotProduct(forward, throwdir);
			if(thrown > 100) {
				if(dot > 0.3 && PM_HasAnimation( self, BOTH_DEATHFORWARD1 )) {
					self->client->ps.gravity *= 0.8;
					self->client->ps.friction = 0;
					if ( PM_HasAnimation( self, BOTH_DEATHFORWARD2) ) {
						anim = Q_irand(BOTH_DEATHFORWARD1, BOTH_DEATHFORWARD2);//okay, i assume he has 2 since he has 1
					} else {
						anim = BOTH_DEATHFORWARD1;
					}
				}
				else if(dot < -0.3 && PM_HasAnimation( self, BOTH_DEATHBACKWARD1 )) {
					self->client->ps.gravity *= 0.8;
					self->client->ps.friction = 0;
					if ( PM_HasAnimation( self, BOTH_DEATHBACKWARD2) ) {
						anim = Q_irand(BOTH_DEATHBACKWARD1, BOTH_DEATHBACKWARD2);
					} else {
						anim = BOTH_DEATHBACKWARD1;
					}
				}
			}
		}

		if ( meansOfDeath == MOD_KNOCKOUT )
		{
			//FIXME: knock-out sound, and don't remove me
			G_AddEvent( self, EV_JUMP, 0 );
			G_UseTargets2( self, self, self->target2 );
			G_AlertTeam( self, attacker, 512, 32 );
			if ( self->NPC )
			{//stick around for a while
				self->NPC->timeOfDeath = level.time + 10000;
			}
		}
		else if ( meansOfDeath == MOD_SNIPER )
		{
			gentity_t	*tent;
			vec3_t		spot;

			if ( self->client->playerTeam != TEAM_BOTS && self->client->playerTeam != TEAM_STASIS )
			{
				VectorCopy( self->currentOrigin, spot );

				if ( self->client->playerTeam != TEAM_PARASITE )
					spot[2] += 24;

				tent = G_TempEntity( spot, EV_DISINTEGRATION );
				tent->s.eventParm = PW_REGEN;
				tent->owner = self;
		
				G_AlertTeam( self, attacker, 512, 88 );

				if ( self->NPC )
				{//need to pad deathtime some to stick around long enough for death effect to play

					self->NPC->timeOfDeath = level.time + 2000;
				}
			}
		}
		else if ( meansOfDeath == MOD_PHASER_ALT )
		{
			gentity_t	*tent;

			if ( self->client->playerTeam != TEAM_BOTS && self->client->playerTeam != TEAM_STASIS )
			{
				tent = G_TempEntity( self->currentOrigin, EV_DISINTEGRATION );
				tent->s.eventParm = PW_DISINT_3;
				tent->owner = self;
		
				G_AlertTeam( self, attacker, 512, 88 );

				if ( self->NPC )
				{//need to pad deathtime some to stick around long enough for death effect to play

					self->NPC->timeOfDeath = level.time + 2000;
				}
			}
		}
		else 
		{
			if ( meansOfDeath == MOD_DREADNOUGHT )
			{
				gentity_t	*tent;

				tent = G_TempEntity( self->currentOrigin, EV_DISINTEGRATION );
				tent->s.eventParm = PW_DISINT_1;
				tent->owner = self;
			}
			else if ( meansOfDeath == MOD_QUANTUM )
			{
				gentity_t	*tent;

				tent = G_TempEntity( self->currentOrigin, EV_DISINTEGRATION );
				tent->s.eventParm = PW_DISINT_2;
				tent->owner = self;
			}

			G_AddEvent( self, Q_irand(EV_DEATH1, EV_DEATH3), self->health );
			G_DeathAlert( self, attacker );
		}

		NPC_SetAnim(self, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}

	// don't allow player to respawn for a few seconds
	self->client->respawnTime = level.time + 3000;//self->client->ps.legsAnimTimer;

	//lock it to this anim forever
	if ( self->client )
	{
		PM_SetLegsAnimTimer( self, &self->client->ps.legsAnimTimer, -1 );
		PM_SetTorsoAnimTimer( self, &self->client->ps.torsoAnimTimer, -1 );
	}

	//Flying creatures should drop when killed
	//FIXME: This may screw up certain things that expect to float even while dead <?>
	self->svFlags &= ~SVF_CUSTOM_GRAVITY;

	self->client->ps.pm_type = PM_DEAD;
	//need to update STAT_HEALTH here because ClientThink_real for self may happen before STAT_HEALTH is updated from self->health and pmove will stomp death anim with a move anim
	self->client->ps.stats[STAT_HEALTH] = self->health;

	if ( VALIDSTRING( self->behaviorSet[BSET_DEATH] ) )
	{
		G_ActivateBehavior( self, BSET_DEATH );
		deathScript = qtrue;
	}
	
	if ( self->NPC && (self->NPC->scriptFlags&SCF_FFDEATH) && VALIDSTRING( self->behaviorSet[BSET_FFDEATH] ) )
	{//FIXME: should running this preclude running the normal deathscript?
		G_ActivateBehavior( self, BSET_FFDEATH );
		deathScript = qtrue;
	}
	
	//WARNING!!! DO NOT DO THIS WHILE RUNNING A SCRIPT, ICARUS WILL CRASH!!!
	if ( !deathScript )
	{
		//Should no longer run scripts
		ICARUS_FreeEnt(self);
	}

	// Set pending objectives to failed
	OBJ_SetPendingObjectives(self);
	
	gi.linkentity (self);

	// Start any necessary death fx for this entity
	DeathFX( self );
}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( (float) damage * ARMOR_PROTECTION );

    //Always round up
	if (damage == 1)
	{
		if ( client->ps.stats[STAT_ARMOR] > 0 ) 
			client->ps.stats[STAT_ARMOR] -= save;

		return 0;
	}

	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

void G_ApplyKnockback( gentity_t *targ, vec3_t newDir, float knockback )
{
	vec3_t	kvel;
	float	mass;

	if (targ->physicsBounce > 0)	//overide the mass
		mass = targ->physicsBounce;
	else
		mass = 200;

	VectorScale (newDir, g_knockback->value * (float)knockback / mass * 0.8, kvel);
	kvel[2] = newDir[2] * g_knockback->value * (float)knockback / mass * 1.5;
	VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

	// set the timer so that the other client can't cancel
	// out the movement immediately
	if ( !targ->client->ps.pm_time ) {
		int		t;

		t = knockback * 2;
		if ( t < 50 ) {
			t = 50;
		}
		if ( t > 200 ) {
			t = 200;
		}
		targ->client->ps.pm_time = t;
		targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	}
}

void ReCalcDamageForLocation (gentity_t *target, vec3_t pdir, vec3_t ppoint, int *damage)
{
/*
	vec3_t			dir, point, point_dir, tempvec;
	vec3_t			forward, right, up;
	vec3_t			tangles, tcenter;
	float			tradius, hdist;
	float			udot, fdot, rdot;
	int				Vertical, Forward, Lateral;
	int				HitLoc;

//get target forward, right and up
	if(target->client)
	{//ignore player's pitch and roll
		VectorSet(tangles, 0, target->currentAngles[YAW], 0);
	}

	AngleVectors(tangles, forward, right, up);

//get center of target
	VectorAdd(target->absmin, target->absmax, tcenter);
	VectorScale(tcenter, 0.5, tcenter);

//get radius width of target
	tradius = (fabs(target->maxs[0]) + fabs(target->maxs[1]) + fabs(target->mins[0]) + fabs(target->mins[1]))/4;

//get impact point
	if(ppoint && !VectorCompare(ppoint, vec3_origin))
	{
		VectorCopy(ppoint, point);
	}
	else
	{
		return;
	}

//get impact dir
	if(pdir && !VectorCompare(pdir, vec3_origin))
	{
		VectorCopy(pdir, dir);
	}
	else
	{
		return;
	}

//put point at controlled distance from center
	VectorSubtract(point, tcenter, tempvec);
	tempvec[2] = 0;
	hdist = VectorLength(tempvec);

	VectorMA(point, hdist - tradius, dir, point);
	//now a point on the surface of a cylinder with a radius of tradius
	
	VectorSubtract(point, tcenter, point_dir);
	VectorNormalize(point_dir);

	//Get bottom to top (Vertical) position index
	udot = DotProduct(up, point_dir);
	if(udot>.666)
		Vertical = 4;
	else if(udot>.333)
		Vertical = 3;
	else if(udot>-.333)
		Vertical = 2;
	else if(udot>-.666)
		Vertical = 1;
	else
		Vertical = 0;

	//Get back to front (Forward) position index
	fdot = DotProduct(forward, point_dir);
	if(fdot>.666)
		Forward = 4;
	else if(fdot>.333)
		Forward = 3;
	else if(fdot>-.333)
		Forward = 2;
	else if(fdot>-.666)
		Forward = 1;
	else
		Forward = 0;

	//Get left to right (Lateral) position index
	rdot = DotProduct(right, point_dir);
	if(rdot>.666)
		Lateral = 4;
	else if(rdot>.333)
		Lateral = 3;
	else if(rdot>-.333)
		Lateral = 2;
	else if(rdot>-.666)
		Lateral = 1;
	else
		Lateral = 0;

	HitLoc = Vertical * 25 + Forward * 5 + Lateral;

	if(HitLoc <= 10)
	{//feet
		*damage = floor(*damage * 0.10);
	}
	else if(HitLoc <= 50)
	{//legs
		*damage = floor(*damage * 0.30);
	}
	else if(HitLoc == 56||HitLoc == 60||HitLoc == 61||HitLoc == 65||HitLoc == 66||HitLoc == 70||
		HitLoc == 83||HitLoc == 87||HitLoc == 88||HitLoc == 92||HitLoc == 93||HitLoc == 97)
	{//arms
		*damage = floor(*damage * 0.20);
	}
	else if((HitLoc >= 107 && HitLoc <= 109)||
		(HitLoc >= 112 && HitLoc <= 114)||
		(HitLoc >= 117 && HitLoc <= 119))
	{//head
		*damage *= 2;
	}
*/
}

/*
void G_TeamRetaliation ( gentity_t *targ, gentity_t *attacker, qboolean stopIcarus )

	1: Get entire team mad at player
	2: Stop team from running scripts
	3: If on a Voyager map (?!), load the brig after a certain delay-  Else, just bring up "mission failed"
	FIXME: need to do this because you could potentially kill everyone.
		can't rely on everyone being undying
*/
void G_TeamRetaliation ( gentity_t *targ, gentity_t *attacker, qboolean stopIcarus )
{
	gentity_t	*teammate;
	int			teamcount = 0;

	if ( !attacker->client )
	{
		return;
	}

	if ( Q_strncmp( "_holo", level.mapname, 5 ) == 0 )
	{//only do this if not on a holodeck
		return;
	}

	//attacker->client->playerTeam = TEAM_FREE;
	attacker->flags &= ~FL_GODMODE;
	attacker->flags &= ~FL_UNDYING;
	if ( !stop_icarus )
	{//don't restart ICARUS if it's already been stopped...?
		stop_icarus = stopIcarus;
	}
	//find each member of the team and get them angry at you
	for ( int i = 1; i < globals.num_entities; i++ )
	{
		teammate = &g_entities[i];
		if ( !teammate )
		{
			continue;
		}
		if ( !teammate->inuse )
		{
			continue;
		}
		if ( !teammate->NPC )
		{
			continue;
		}
		if ( !teammate->client )
		{
			continue;
		}
		if ( teammate->client->ps.stats[STAT_HEALTH] <= 0 )
		{
			continue;
		}
		if ( teammate->s.eFlags & EF_NODRAW )
		{
			continue;
		}
		if ( teammate->client->playerTeam != TEAM_STARFLEET )
		{
			continue;
		}
		if ( teammate == targ || !Q_irand(0, 1) )
		{
			G_AddVoiceEvent( teammate, Q_irand( EV_FF_3A, EV_FF_3C ), 2000 );
		}
		teamcount++;//count them up
		//mad at me forever
		G_ClearEnemy( teammate );
		G_SetEnemy( teammate, attacker );
		teammate->svFlags |= SVF_LOCKEDENEMY;
		teammate->NPC->behaviorState = BS_RUN_AND_SHOOT;
		teammate->client->renderInfo.lookTarget = 0;
		if ( teammate->client->race != RACE_HOLOGRAM )
		{//doctor unkillable
			if ( !teammate->contents )
			{
				teammate->contents = CONTENTS_BODY;
			}
			teammate->NPC->ignorePain = qfalse;
			teammate->flags &= ~FL_UNDYING;
			teammate->flags &= ~FL_GODMODE;
		}

		teammate->svFlags &= ~SVF_IGNORE_ENEMIES;
		teammate->flags &= ~FL_DONT_SHOOT;
		teammate->behaviorSet[BSET_DEATH] = NULL;
		teammate->behaviorSet[BSET_FFDEATH] = NULL;
		teammate->behaviorSet[BSET_FFIRE] = NULL;
		teammate->behaviorSet[BSET_PAIN] = NULL;

		//give 'em a weapon
		if ( teammate->client->ps.weapon == WP_NONE || teammate->client->ps.weapon == WP_TRICORDER )
		{
			int	wp;

			if ( !Q_irand(0, 1) )
			{
				wp = WP_COMPRESSION_RIFLE;
			}
			else
			{
				wp = WP_PHASER;
			}
			teammate->client->ps.stats[STAT_WEAPONS] = ( 1 << wp );
			teammate->client->ps.ammo[weaponData[wp].ammoIndex] = 999;
			RegisterItem( FindItemForWeapon( (weapon_t) wp) );	//make sure the weapon is cached in case this runs at startup
			ChangeWeapon( teammate, wp );
			teammate->client->ps.weapon = wp;
			teammate->client->ps.weaponstate = WEAPON_READY;
		}
	}
	
	if ( !teamcount )
	{//killed all teammates
		//end map now
		ffireLevel = FFIRE_LEVEL_RETALIATION;
		killPlayerTimer = level.time + 500;
		if ( Q_irand( 0, 1 ) )
		{
			statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
		}
		else
		{
			statusTextIndex = IGT_KILLEDANINNOCENTCREWMAN;
		}
	}
}

void G_FriendlyFireReaction( gentity_t *targ, gentity_t *attacker, qboolean inCombat )
{//NOTE: you only get in here if a starfleet teammember shot another starfleet teammember
	int	ffireEvent = 0;

	if ( attacker != &g_entities[0] || targ == &g_entities[0] )
	{//attacker isn't player, or target is the player ignore it
		return;
	}
	
	if ( !targ->NPC )
	{//Only bother with friendly fire checks if the player shot an NPC
		return;
	}

	if ( targ->health <= 0 )
	{//player killed an NPC teammate, they should turn on player
		if ( VALIDSTRING( targ->behaviorSet[BSET_FFDEATH] ) )
		{//they're going to run a special friendly fire death script
			//Will make them run the ffdeath script
			targ->NPC->scriptFlags |= SCF_FFDEATH;
		}
		else if ( targ->enemy != attacker )
		{//they were killed by the player without provocation - this way 
			if ( Q_strncmp( "_holo", level.mapname, 5 ) != 0 )
			{//only do this if not on a holodeck
				//after a certain amount of time, end map anyway
				ffireLevel = FFIRE_LEVEL_RETALIATION;
				if ( !killPlayerTimer )
				{//start the kill player timer if we haven't already
					killPlayerTimer = level.time + 10 * 1000;
					if ( Q_irand( 0, 1 ) )
					{
						statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
					}
					else
					{
						statusTextIndex = IGT_KILLEDANINNOCENTCREWMAN;
					}
				}
				//those who are killed after the team turns on the player
				//do not repeat this behavior... and those that may have
				//been spawned by ffdeathscripts do not do this either (like security)
				G_TeamRetaliation( targ, attacker, qtrue );
			}
		}
		else if ( !teamCount[TEAM_STARFLEET] )
		{//killed everyone on your team!
			if ( Q_strncmp( "_holo", level.mapname, 5 ) != 0 )
			{//only do this if not on a holodeck
				//end map now
				ffireLevel = FFIRE_LEVEL_RETALIATION;
				killPlayerTimer = level.time + 500;
				if ( Q_irand( 0, 1 ) )
				{
					statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
				}
				else
				{
					statusTextIndex = IGT_KILLEDANINNOCENTCREWMAN;
				}
			}
		}
		else
		{//Killed a crewmate who was attacking you - start a 30 second timer to end the level one way or another
			gentity_t *brigent = NULL;
			while( (brigent = G_Find(brigent, FOFS(classname), "target_level_change" )) != NULL )
			{
				if(!brigent->message)
					G_Error( "target_level_change without level map name");
				if ( Q_stricmp("_brig", brigent->message) == 0 )
				{
					break;
				}
			}

			if ( brigent )
			{//there is a brig changelevel entity on the map, use it in 30 seconds.
				if ( !loadBrigTimer )
				{
					loadBrigTimer = level.time + 30000;
				}
			}
			else if ( !killPlayerTimer )
			{//start the kill player timer if we haven't already
				if ( Q_strncmp( "_holo", level.mapname, 5 ) != 0 )
				{//only do this if not on a holodeck
					killPlayerTimer = level.time + 30000;
					if ( Q_irand( 0, 1 ) )
					{
						statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
					}
					else
					{
						statusTextIndex = IGT_KILLEDANINNOCENTCREWMAN;
					}
				}
			}
		}

		return;
	}

	if ( inCombat )
	{//we're in combat
		//okay, say something, but doesn't count toward ffire counter
		if ( TIMER_Done( targ, "ffireDebounce" ) )
		{
			TIMER_Set( targ, "ffireDebounce", 1000 );
			G_AddVoiceEvent( targ, Q_irand( EV_FF_1A, EV_FF_2C ), 2000 );
		}

		return;
	}
	//else we are not in combat...

	//shot a teammate, but he's still alive
	if ( targ->enemy == attacker )
	{//Target is already mad at player, just yell at him for continuing to shoot
		if ( TIMER_Done( targ, "ffireDebounce" ) )
		{
			TIMER_Set( targ, "ffireDebounce", 1000 );
			G_AddVoiceEvent( targ, Q_irand( EV_FF_3A, EV_FF_3C ), 2000 );
		}
		return;
	}

	//okay, he's not alreayd mad at player, inc the ffire counter

	//2) Only inc the global ffire counter every 1 sec max
	if ( level.time - ffireForgivenessTimer >= 1000 )
	{
		//NOTE: counter is reset evey 2 minutes that passes without ffire and during combat
		ffireLevel++;
		ffireForgivenessTimer = level.time;
		//3) If the counter >= 10, get team mad
		if ( ffireLevel >= FFIRE_LEVEL_RETALIATION )
		{//we are angry enough to attack
			//if have a ffire script, run that instead
			if ( VALIDSTRING( targ->behaviorSet[BSET_FFIRE] ) )
			{//Got two fair warnings, third strike you're out
				G_ActivateBehavior( targ, BSET_FFIRE );
			}
			else if ( attacker->client->playerTeam == TEAM_STARFLEET )//not in disguise, but this is guaranteed
			{//Okay, we can't just stand here and do nothing...
				//NOTE: this will also make the targ yell at the attacker again
				G_TeamRetaliation( targ, attacker, qtrue );
			}
		}
		else
		{//they're getting pissed
			//1) Only reply and every 1 sec max
			if ( TIMER_Done( targ, "ffireDebounce" ) )
			{
				TIMER_Set( targ, "ffireDebounce", 1000 );
				//reply
				if ( ffireLevel < 3 )
				{
					ffireEvent = Q_irand( EV_FF_1A, EV_FF_1C );
				}
				else if ( ffireLevel < 6 )
				{
					ffireEvent = Q_irand( EV_FF_2A, EV_FF_2C );
				}
				else //must be 9 or higher
				{
					ffireEvent = Q_irand( EV_FF_3A, EV_FF_3C );
				}
				if ( (targ->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_RESTRAINED1 && (targ->client->ps.legsAnim&~ANIM_TOGGLEBIT) != BOTH_RESTRAINED1POINT )
				{
					NPC_TempLookTarget( targ, attacker->s.number, 2000, 4000 );
				}
			}
		}
	}
	
	if ( ffireEvent )
	{//say whatever we were going to say
		G_AddVoiceEvent( targ, ffireEvent, 2000 );
	}
}
/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
	DAMAGE_NO_HIT_LOC		Damage not based on hit location
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod ) 
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			knockback;
	vec3_t		newDir;

	if (!targ->takedamage) {
		return;
	}

	if ( targ->health <= 0 ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	if ( attacker->s.number != 0 && damage >= 2 && targ->s.number != 0 && attacker->client && attacker->client->playerTeam == TEAM_STARFLEET )
	{//player-helpers do only half damage to enemies
		damage = ceil((float)damage/2.0f);
	}

	if ( targ->client )
	{
		if ( ( targ->client->race == RACE_HIROGEN ) && ( targ->s.powerups & ( 1 << PW_HIROGEN_SHIELD ) ) )
		{
			//Shield is up, take no damage
			if ( targ->s.powerups & ( 1 << PW_HIROGEN_SHIELD ) )
				return;
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( dflags&DAMAGE_NO_DAMAGE )
	{
		damage = 0;
	}

	if ( dir == NULL ) 
	{
		dflags |= DAMAGE_NO_KNOCKBACK;
	} 
	else 
	{
		VectorNormalize2( dir, newDir );
	}

	if ( targ->s.number != 0 )
	{//not the player
		if ( (targ->flags&FL_GODMODE) || (targ->flags&FL_UNDYING) )
		{//have god or undying on, so ignore no protection flag
			dflags &= ~DAMAGE_NO_PROTECTION;
		}
	}

	if(PM_InOnGroundAnim(targ))
	{
		dflags |= DAMAGE_NO_KNOCKBACK;
	}

	knockback = damage;

	//Attempt to apply extra knockback
	if ( dflags & DAMAGE_EXTRA_KNOCKBACK )
	{
		knockback *= 2;
	}
	
	if ( knockback > 200 ) {
		knockback = 200;
	}

	if ( targ->flags & FL_NO_KNOCKBACK ) 
	{
		knockback = 0;
	}
	else if ( !g_teamKnockback->integer && targ->client && attacker->client && targ->client->playerTeam == attacker->client->playerTeam )
	{
		knockback = 0;
	}
	else if ( dflags & DAMAGE_NO_KNOCKBACK )
	{
		knockback = 0;
	}
	else if ( mod == MOD_STASIS )
	{//HACK, need a dflags field on ents
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client && !(dflags&DAMAGE_DEATH_KNOCKBACK) ) 
	{
		G_ApplyKnockback( targ, newDir, knockback );
	}

	// check for godmode, completely getting out of the damage
	if ( targ->flags & FL_GODMODE && !(dflags&DAMAGE_NO_PROTECTION) ) 
	{
		if ( targ->client && attacker->client && targ->client->playerTeam == attacker->client->playerTeam )
		{//complain, but don't turn on them
			G_FriendlyFireReaction( targ, attacker, qtrue );
		}
		return;
	}

	// Check for team damage
	if ( targ != attacker && !(dflags&DAMAGE_IGNORE_TEAM) && OnSameTeam (targ, attacker)  ) 
	{//on same team
		if ( !targ->client ) 
		{//a non-player object should never take damage from an ent on the same team
			return;
		}

		if ( attacker->client && attacker->client->playerTeam == targ->noDamageTeam ) 
		{//NPC or player shot an object on his own team
			return;
		}

		if ( attacker->s.number != 0 && targ->s.number != 0 &&//player not involved in any way in this exchange
			attacker->client && targ->client &&//two NPCs
			attacker->client->playerTeam == targ->client->playerTeam ) //on the same team
		{//NPCs on same team don't hurt each other
			return;
		}

		if ( targ->s.number == 0 &&//player was hit
			attacker->client && targ->client &&//by an NPC
			attacker->client->playerTeam == TEAM_STARFLEET ) //on the same team
		{
			if ( attacker->enemy != targ )//by accident
			{//do no damage, no armor loss, no reaction, run no scripts
				return;
			}
		}
	}


	// add to the attacker's hit counter
	if ( attacker->client && targ != attacker && targ->health > 0 ) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
		} else {
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	take = damage;
	save = 0;

	//FIXME: Do not use this method of difficulty changing
	// Scale the amount of damage given to the player based on the skill setting
	/*
	if ( targ->s.number == 0 && targ != attacker )
	{
		take *= ( g_spskill->integer + 1) * 0.75;
	}

	if ( take < 1 ) {
		take = 1;
	}
	*/

	//don't lose armor if on same team
	// save some from armor
	asave = CheckArmor (targ, take, dflags);
	take -= asave;

	if ( ( targ->client != NULL ) && ( targ->client->playerTeam == TEAM_HIROGEN ) ) 
	{
		if ( ( Q_stricmp( targ->NPC_type, "hirogenalpha" ) == 0 ) && !( targ->s.powerups & ( 1 << PW_HIROGEN_SHIELD ) ) )
		{
			take = 1;
			targ->NPC->localState++;
			targ->NPC->squadState = 0;
		}
	}

	if ( g_debugDamage->integer ) {
		gi.Printf( "[%d]client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
			targ->health, take, asave );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		client->ps.persistant[PERS_ATTACKER] = attacker->s.number;	//attack can be the world ent
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {	//can't check newdir since it's local, newdir is dir normalized
			VectorCopy ( newDir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// do the damage
	if ( take || (dflags&DAMAGE_NO_DAMAGE) ) 
	{
		if ( !targ->client || !attacker->client )
		{
			targ->health = targ->health - take;
			if ( (targ->flags&FL_UNDYING) && !(dflags&DAMAGE_NO_PROTECTION) )
			{
				if(targ->health < 1)
				{
					if ( VALIDSTRING( targ->behaviorSet[BSET_DEATH] ) )
					{
						G_ActivateBehavior( targ, BSET_DEATH );
					}

					targ->health = 1;
				}
			}
		}
		else 
		{//two clients
			team_t		targTeam = TEAM_FREE;
			team_t		attackerTeam = TEAM_FREE;

			if ( targ->client ) {
				targTeam = targ->client->playerTeam;
			}
			else {
				targTeam = targ->noDamageTeam;
			}
			if ( targTeam == TEAM_DISGUISE ) {
				targTeam = TEAM_STARFLEET;
			}
			if ( attacker->client ) {
				attackerTeam = attacker->client->playerTeam;
			}
			else {
				attackerTeam = attacker->noDamageTeam;
			}
			if ( attackerTeam == TEAM_DISGUISE ) {
				attackerTeam = TEAM_STARFLEET;
			}

			if ( targTeam != attackerTeam )
			{//on the same team
				targ->health = targ->health - take;

				//MCG - Falling should never kill player- only if a trigger_hurt does so.
				if ( mod == MOD_FALLING && targ->s.number == 0 && targ->health < 1 )
				{
					targ->health = 1;
				}

				if ( (targ->flags&FL_UNDYING) && !(dflags&DAMAGE_NO_PROTECTION) )
				{
					if ( targ->health < 1 )
					{
						if ( VALIDSTRING( targ->behaviorSet[BSET_DEATH] ) )
						{
							G_ActivateBehavior( targ, BSET_DEATH );
						}

						//Turn off Hirogen boss' shield upon "death"
						if ( ( targ->client != NULL ) && ( targ->client->playerTeam == TEAM_HIROGEN ) )
						{
							if ( Q_stricmp( targ->NPC_type, "hirogenalpha" ) == 0 )
							{
								targ->s.powerups &= ~( 1 << PW_HIROGEN_SHIELD );
								targ->client->ps.powerups[ PW_HIROGEN_SHIELD ] = -1;
								targ->NPC->ignorePain = qtrue;
							}
						}

						targ->health = 1;
					}
				}
				else if ( targ->health < 1 && attacker->client )
				{
					// The player or NPC just killed an enemy so increment the kills counter
					attacker->client->ps.persistant[PERS_ENEMIES_KILLED]++;
				}
			}
			else if ( targTeam == TEAM_STARFLEET )
			{//on the same team, and target is a starfleet officer 
				qboolean inCombat = qfalse;
				qboolean takeDamage = qtrue;
				qboolean yellAtAttacker = qtrue;

				if ( level.time - teamLastEnemyTime[TEAM_STARFLEET] < 10000 )
				{//Team is in combat
					inCombat = qtrue;
				}

				//1) player doesn't take damage from teammates unless they're angry at him
				if ( targ->s.number == 0 )
				{//the player
					if ( attacker->enemy != targ && attacker != targ )
					{//an NPC shot the player by accident
						takeDamage = qfalse;
					}
				}
				//2) NPCs don't take any damage from player during combat
				else 
				{//an NPC
					if ( (inCombat || (dflags & DAMAGE_RADIUS)) && !(dflags&DAMAGE_IGNORE_TEAM) )
					{//An NPC got hit by player and this is during combat or it was slash damage
						//NOTE: though it's not realistic to have teammates not take splash damage,
						//		even when not in combat, it feels really bad to have them able to
						//		actually be killed by the player's splash damage
						takeDamage = qfalse;
					}

					if ( inCombat && (dflags & DAMAGE_RADIUS) )
					{//you're fighting and it's just radius damage, so don't even mention it
						yellAtAttacker = qfalse;
					}
				}

				if ( takeDamage )
				{
					targ->health = targ->health - take;
				}

				if ( (targ->flags&FL_UNDYING) && !(dflags&DAMAGE_NO_PROTECTION) && attacker->s.number != 0 )
				{//guy is marked undying and we're not the player or we're in combat
					if ( targ->health < 1 )
					{
						if ( VALIDSTRING( targ->behaviorSet[BSET_DEATH] ) )
						{
							G_ActivateBehavior( targ, BSET_DEATH );
						}

						targ->health = 1;
					}
				}
				
				if ( yellAtAttacker )
				{
					G_FriendlyFireReaction( targ, attacker, inCombat );
				}
			}
		}

		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
			g_lastClientDamaged = targ;
		}
			
		if ( targ->health <= 0 ) 
		{
			if ( knockback && targ->client && (dflags&DAMAGE_DEATH_KNOCKBACK) )
			{//only do knockback on death
				G_ApplyKnockback( targ, newDir, knockback );
			}

			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->infoString = NULL;//Dead things have no ID
			GEntity_DieFunc (targ, inflictor, attacker, take, mod);

			return;
		}
		else 
		{
			GEntity_PainFunc(targ, attacker, take);
			if ( targ->s.number == 0 )
			{//player run painscript
				if ( VALIDSTRING( targ->behaviorSet[BSET_PAIN] ) )
				{
					G_ActivateBehavior( targ, BSET_PAIN );
				}
				if ( targ->health <= 25 )
				{
					if ( VALIDSTRING( targ->behaviorSet[BSET_FLEE] ) )
					{
						G_ActivateBehavior( targ, BSET_FLEE );
					}
				}
			}
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->absmin, targ->absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	gi.trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	gi.trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	gi.trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	gi.trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	gi.trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
void G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	gentity_t	*entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = gi.EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = entityList[ e ];

		if ( ent == ignore )
			continue;
		if ( !ent->takedamage )
			continue;
		if ( !ent->contents )
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->absmin[i] ) {
				v[i] = ent->absmin[i] - origin[i];
			} else if ( origin[i] > ent->absmax[i] ) {
				v[i] = origin[i] - ent->absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if (CanDamage (ent, origin)) {
			VectorSubtract (ent->currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}
}
