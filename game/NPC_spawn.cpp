//b_spawn.cpp
//added by MCG

#include "b_local.h"
#include "anims.h"
#include "g_functions.h"
#include "boltOns.h"

extern cvar_t *g_sex;

extern qboolean G_CheckInSolid (gentity_t *self, qboolean fix);
extern void ClientUserinfoChanged( int clientNum );
extern qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest );

extern void FX_BorgTeleport( vec3_t org );

extern void Q3_SetParm (int entID, int parmNum, const char *parmValue);
extern char	showSPaths[MAX_QPATH];
extern team_t TranslateTeamName( const char *name );
extern char	*TeamNames[TEAM_NUM_TEAMS];
extern void G_InitBoltOnData ( gentity_t *ent );

extern void CG_ShimmeryThing_Spawner( vec3_t start, vec3_t end, float radius, qboolean taper, int duration );
extern void Q3_DebugPrint( int level, const char *format, ... );

extern void NPC_StasisSpawnEffect( gentity_t *ent );

extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );

#define	NSF_DROP_TO_FLOOR	16

void HirogenAlpha_Precache( void );

/*
-------------------------
NPC_PainFunc
-------------------------
*/

painFunc_t NPC_PainFunc( gentity_t *ent )
{
	painFunc_t	func;

	switch ( ent->client->playerTeam )
	{
	case TEAM_SCAVENGERS:
		func = painF_NPC_Scav_Pain;
		break;

	default:
		func = painF_NPC_Pain;
		break;
	}

	return func;
}


/*
-------------------------
NPC_TouchFunc
-------------------------
*/

touchFunc_t NPC_TouchFunc( gentity_t *ent )
{
	return touchF_NPC_Touch;
}

/*
-------------------------
NPC_SetMiscDefaultData
-------------------------
*/

void NPC_SetMiscDefaultData( gentity_t *ent )
{
	switch(ent->client->playerTeam)
	{
	case TEAM_BORG:
		//FIXME: Is NPC always pointing at the same as ent here?
		ent->NPC->aiFlags |= NPCAI_SHIELDS;
		ent->physicsBounce = 400;//higher mass for less knockback
		break;

	case TEAM_STARFLEET:
		ent->flags |= FL_NO_KNOCKBACK;
		break;

	case TEAM_BOTS:
		if ( Q_stricmp( ent->NPC_type, "hunterseeker" ) == 0 ) 
		{		
			ent->client->ps.gravity = 0;
			ent->svFlags |= SVF_CUSTOM_GRAVITY;
		}

		if ( Q_stricmp( ent->NPC_type, "headbot" ) == 0 ) 
		{
			ent->client->ps.gravity = 0;
			ent->svFlags |= SVF_CUSTOM_GRAVITY;

			//See if he should start plugged in
			if ( ent->spawnflags & 2 )
			{
				NPC_SetAnim(ent, SETANIM_BOTH, BOTH_SLEEP1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				client->ps.legsAnimTimer = -1;
			}
		}

		if ( ( Q_stricmp( ent->NPC_type, "warriorbot" ) == 0 ) || ( Q_stricmp( ent->NPC_type, "warriorbot_boss" ) == 0 ) )
		{
			ent->physicsBounce = 500;//higher mass for less knockback

			//Add the head unless flagged not to
			if ( ( ent->spawnflags & 2 ) == qfalse )
				G_AddBoltOn( ent, "headbot_warriorbot" );
			else
				ent->flags |= FL_NOTARGET;
		}

		if ( Q_stricmp( ent->NPC_type, "scoutbot" ) == 0 )
		{
			ent->physicsBounce = 400;//higher mass for less knockback

			//Add the head unless flagged not to
			if ( ( ent->spawnflags & 2 ) == qfalse )
				G_AddBoltOn( ent, "headbot_scoutbot" );
			else
				ent->flags |= FL_NOTARGET;
		}

		break;

	case TEAM_FORGE:

		if ( Q_stricmp( ent->NPC_type, "vohrsoth" ) == 0 )
		{
			VectorSet( ent->mins, -180, -70, -28 );
			VectorSet( ent->maxs,  20,  70,  150 );
		}

		break;
/*
	case TEAM_SCAVENGERS:

		if ( Q_stricmp( ent->NPC_type, "hirogenalpha" ) == 0 )
		{
		}

		break;
*/
	default:
		break;
	}

	if ( Q_stricmp( "satan", ent->NPC_type ) == 0 )
	{
		ent->spawnflags |= SFB_TASER;
	}
}

/*
-------------------------
NPC_WeaponForTeam
-------------------------
*/

weapon_t NPC_WeaponForTeam( team_t team, int spawnflags, const char *NPC_type )
{
	switch(team)
	{
	case TEAM_BORG:
		
		if(spawnflags & SFB_GUN)
			return WP_BORG_WEAPON;

		if(spawnflags & SFB_TASER)
			return WP_BORG_TASER;
		
		if(spawnflags & SFB_DRILL)
			return WP_BORG_DRILL;
		
		return WP_BORG_ASSIMILATOR;
		break;

	case TEAM_HIROGEN:
		if( Q_stricmp( "hirogenalpha", NPC_type ) == 0 )
			return WP_TETRION_DISRUPTOR;

		if ( Q_stricmp( "satan", NPC_type ) == 0 )
			return WP_BORG_TASER;
		//Falls through

	case TEAM_KLINGON:

		if ( spawnflags & 1 )
			return WP_KLINGON_BLADE;
		//NOTENOTE: Falls through if not a knife user

	case TEAM_IMPERIAL:
		if ( spawnflags & 1 )
			return WP_IMPERIAL_BLADE;
		//NOTENOTE: Falls through if not a knife user

	case TEAM_SCAVENGERS:
	case TEAM_MALON:

		if( Q_stricmp( "desperado", NPC_type ) == 0 )
			return WP_DESPERADO;

		if( Q_stricmp( "paladin", NPC_type ) == 0 )
			return WP_PALADIN;

		if ( Q_stricmp( "chaoticaguard", NPC_type ) == 0 || Q_stricmp( "chaotica", NPC_type ) == 0 )
			return WP_CHAOTICA_GUARD_GUN;

		return WP_SCAVENGER_RIFLE;
		break;

	case TEAM_STARFLEET:
		
		if(spawnflags & SFB_TRICORDER)
			return WP_TRICORDER;
		
		if(spawnflags & SFB_RIFLEMAN)
			return WP_COMPRESSION_RIFLE;
		
		if(spawnflags & SFB_PHASER)
			return WP_PHASER;

		if ( Q_stricmp( "buster", NPC_type ) == 0 || Q_stricmp( "proton", NPC_type ) == 0 )
			return WP_PROTON_GUN;

		break;

	case TEAM_BOTS:
		
		if ( ( Q_stricmp( "warriorbot", NPC_type ) == 0 ) || ( Q_stricmp( "warriorbot_boss", NPC_type ) == 0 ) )
			return WP_MELEE;

		if ( Q_stricmp( "hunterseeker", NPC_type ) == 0 )
			return WP_BOT_ROCKET;

		if ( Q_stricmp( "scoutbot", NPC_type ) == 0 )
			return WP_BOT_WELDER;
	
		break;
	
	case TEAM_FORGE:

		if( Q_stricmp( "harvester", NPC_type ) == 0 || Q_stricmp( "biohulk", NPC_type ) == 0 )
			return WP_MELEE;

		if( Q_stricmp( "avatar", NPC_type ) == 0 )
			return WP_FORGE_PSYCH;

		if( Q_stricmp( "reaver", NPC_type ) == 0 )
			return WP_FORGE_PROJ;

		break;

	case TEAM_STASIS:
		
		//Are we marked as a shooter?
		if ( spawnflags & 2 )
			return WP_STASIS_ATTACK;		

		return WP_MELEE;
		break;

	case TEAM_PARASITE:
		return WP_PARASITE;
		break;

	case TEAM_8472:
		return WP_MELEE;
		break;

	default:
		break;
	}

	return WP_NONE;
}

void ChangeWeapon( gentity_t *ent, int newWeapon );

/*
-------------------------
NPC_SetWeapons
-------------------------
*/

void NPC_SetWeapons( gentity_t *ent )
{
	weapon_t	weapon = NPC_WeaponForTeam( ent->client->playerTeam, ent->spawnflags, ent->NPC_type );

	if ( weapon == WP_BORG_TASER )
	{
		ent->NPC->stats.aim = 5;
	}

	switch ( ent->client->playerTeam )
	{
	case TEAM_KLINGON:
	case TEAM_MALON:
	case TEAM_HIROGEN:
	case TEAM_IMPERIAL:
		ent->client->playerTeam = TEAM_SCAVENGERS;
		break;
	}

	if ( weapon != WP_NONE )
	{
		ent->client->ps.stats[STAT_WEAPONS] = ( 1 << weapon );
		RegisterItem( FindItemForWeapon( weapon) );	//precache the weapon
		ent->NPC->currentAmmo = ent->client->ps.ammo[weaponData[weapon].ammoIndex] = 100;
		ent->client->ps.weapon = weapon;
		ent->client->ps.weaponstate = WEAPON_IDLE;
	}
	
	ChangeWeapon( ent, weapon );
}

/*
-------------------------
NPC_SpawnEffect

  NOTE:  Make sure any effects called here have their models, tga's and sounds precached in
			CG_RegisterNPCEffects in cg_player.cpp
-------------------------
*/

void NPC_SpawnEffect (gentity_t *ent)
{
	if ( ent->spawnflags&SFB_SILENTSPAWN )
	{
		return;
	}

	gentity_t	*tent;

	// NOTE:  Make sure any effects called here have their models, tga's and sounds precached in
	//		CG_RegisterNPCEffects in cg_player.cpp
	switch( ent->client->playerTeam )
	{
	case TEAM_BORG:
		FX_BorgTeleport( ent->client->ps.origin );
		break;

	case TEAM_SCAVENGERS:
	case TEAM_KLINGON:
	case TEAM_MALON:
	case TEAM_HIROGEN:
	case TEAM_IMPERIAL:
		tent = G_TempEntity( ent->client->ps.origin, EV_SCAV_BEAMING );
		tent->owner = ent;
		break;

	case TEAM_STARFLEET:
		tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->owner = ent;
		break;

	case TEAM_STASIS:
		NPC_StasisSpawnEffect( ent );
		tent = G_TempEntity( ent->client->ps.origin, EV_STASIS_TELEPORT_IN );
		tent->count = 1;	// flag to play stasis appear sound
		tent->owner = ent;
		break;

	case TEAM_8472:
		tent = G_TempEntity( ent->client->ps.origin, EV_SPECIES_TELEPORT );
		tent->s.eventParm = 0;
		tent->owner = ent;
		break;

	case TEAM_FORGE:
		G_AddEvent( ent, EV_FORGE_FADE, 2 );
		break;

	case TEAM_PARASITE:
	case TEAM_BOTS:
	default:
		break;
	}
}

//--------------------------------------------------------------
// NPC_SetFX_SpawnStates
//
// Set up any special parms for spawn effects
//--------------------------------------------------------------
void NPC_SetFX_SpawnStates( gentity_t *ent )
{
	// -Etherians -------
	if ( ent->client->playerTeam == TEAM_STASIS )
	{
		ent->svFlags |= SVF_CUSTOM_GRAVITY;
		ent->client->ps.gravity = 300;

		if ( !(ent->spawnflags & SFB_SILENTSPAWN) )
		{
			// The spawn effect can happen, so it's ok to do this extra setup stuff for the effect.
			ent->fx_time = level.time;
			ent->s.eFlags |= EF_SCALE_UP;
			ent->client->ps.eFlags |= EF_SCALE_UP;
			// Make it really small to start with
			VectorSet( ent->client->renderInfo.scaleXYZ, 1,1,1 );
		}
	}
	else
	{
		ent->client->ps.gravity = g_gravity->value;
	}

	// -Hunterseeker -------
	if ( !_stricmp( ent->NPC_type, "hunterseeker" ) )
	{
		// Set the custom banking flag
		ent->s.eFlags |= EF_BANK_STRAFE;
		ent->client->ps.eFlags |= EF_BANK_STRAFE;
	}

	// -8472 -------
	if ( ent->client->playerTeam == TEAM_8472 )
	{
		if ( !(ent->spawnflags & SFB_SILENTSPAWN) )
		{
			// The spawn effect can happen, so it's ok to do this extra setup stuff for the effect.
			ent->fx_time = level.time;
			ent->s.eFlags |= EF_SCALE_UP;
			ent->client->ps.eFlags |= EF_SCALE_UP;
			// Make it really small to start with
			VectorSet( ent->client->renderInfo.scaleXYZ, 1,1,100 );
		}
	}

	// -Forge -----
	if ( ent->client->playerTeam == TEAM_FORGE )
	{
		if ( !(ent->spawnflags & SFB_SILENTSPAWN) )
		{
			// The spawn effect can happen, so it's ok to do this extra setup stuff for the effect.
			ent->fx_time = level.time;
			ent->s.eFlags |= EF_SCALE_UP;
			ent->client->ps.eFlags |= EF_SCALE_UP;
			// Make it really small to start with
			VectorSet( ent->client->renderInfo.scaleXYZ, 1,1,100 );
		}
	}
}

//--------------------------------------------------------------
void NPC_Begin (gentity_t *ent)
{
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	usercmd_t	ucmd;
	gentity_t	*spawnPoint = NULL;

	memset( &ucmd, 0, sizeof( ucmd ) );

	if ( !(ent->spawnflags & SFB_NOTSOLID) )
	{//No NPCs should telefrag
		if( SpotWouldTelefrag( ent, TEAM_FREE ) )//(team_t)(ent->client->playerTeam)
		{
			if ( ent->wait < 0 )
			{//remove yourself
				Q3_DebugPrint( WL_DEBUG, "NPC %s could not spawn, firing target3 (%s) and removing self\n", ent->targetname, ent->target3 );
				//Fire off our target3
				G_UseTargets2( ent, ent, ent->target3 );

				//Kill us
				ent->e_ThinkFunc = thinkF_G_FreeEntity;
				ent->nextthink = level.time + 100;
			}
			else
			{
				Q3_DebugPrint( WL_DEBUG, "NPC %s could not spawn, waiting %4.2 secs to try again\n", ent->targetname, ent->wait/1000.0f );
				ent->e_ThinkFunc = thinkF_NPC_Begin;
				ent->nextthink = level.time + ent->wait;//try again in half a second
			}
			return;
		}
	}
	//Spawn effect
	NPC_SpawnEffect( ent );

	VectorCopy( ent->client->ps.origin, spawn_origin);
	VectorCopy( ent->s.angles, spawn_angles);
	spawn_angles[YAW] = ent->NPC->desiredYaw;

	client = ent->client;

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;

	client->airOutTime = level.time + 12000;

	client->ps.clientNum = ent->s.number;
	// clear entity values
	ent->max_health = client->pers.maxHealth = client->ps.stats[STAT_MAX_HEALTH] = 100;
	ent->s.groundEntityNum = -1;
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "NPC";
	if ( ent->client->race == RACE_HOLOGRAM )
	{//can shoot through holograms, but not walk through them
		ent->contents = CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_ITEM;//contents_corspe to make them show up in ID and use traces
		ent->clipmask = MASK_NPCSOLID;
	}
	else if(!(ent->spawnflags & SFB_NOTSOLID))
	{
		ent->contents = CONTENTS_BODY;
		ent->clipmask = MASK_NPCSOLID;
	}
	else
	{
		ent->contents = 0;
		ent->clipmask = MASK_NPCSOLID&~CONTENTS_BODY;
	}
	if(!ent->NPC->stats.moveType)//Static?
	{
		ent->NPC->stats.moveType = MT_RUNJUMP;
	}
	ent->e_DieFunc = dieF_player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	
	//visible to player and NPCs
	ent->flags &= ~FL_NOTARGET;
	ent->s.eFlags &= ~EF_NODRAW;

	NPC_SetFX_SpawnStates( ent );
	
	client->ps.friction = 6;

	NPC_SetWeapons(ent);

//MCG - Begin: HACK: no health if already have some
	if(ent->health <= 0)
	{
		//ORIGINAL ID: health will count down towards max_health
		ent->health = client->ps.stats[STAT_HEALTH] = ent->max_health * 1.25;
	}
	else
		client->ps.stats[STAT_HEALTH] = ent->max_health = ent->health;
//MCG - End

	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	// clear entity state values
	ent->s.eType = ET_PLAYER;
	ent->s.eFlags |= EF_NPC;
//	ent->s.skinNum = ent - g_entities - 1;	// used as index to get custom models

	VectorCopy (spawn_origin, ent->s.origin);
//	ent->s.origin[2] += 1;	// make sure off ground

	SetClientViewAngle( ent, spawn_angles );
	client->renderInfo.lookTarget = ENTITYNUM_NONE;

	if(!(ent->spawnflags & 64))
	{
		G_KillBox( ent );
		gi.linkentity (ent);
	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity->value * 1000;
	client->latched_buttons = 0;

	// set default animations
	NPC_SetAnim( ent, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_NORMAL );

	if( spawnPoint )
	{
		// fire the targets of the spawn point
		G_UseTargets( spawnPoint, ent );
	}

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	memset( &ucmd, 0, sizeof( ucmd ) );
	VectorCopy( client->pers.cmd_angles, ucmd.angles );
	
	ClientThink(ent->s.number, &ucmd);

	//ICARUS include
	ICARUS_InitEnt( ent );

//==NPC initialization
	SetNPCGlobals( ent );
	memset( &ucmd, 0, sizeof( ucmd ) );

	ent->enemy = NULL;
	NPCInfo->timeOfDeath = 0;
	NPCInfo->shotTime = 0;
	NPC_ClearGoal();
	NPC_ChangeWeapon( ent->client->ps.weapon );

//==Final NPC initialization
	ent->e_PainFunc  = NPC_PainFunc( ent ); //painF_NPC_Pain;
	ent->e_TouchFunc = NPC_TouchFunc( ent ); //touchF_NPC_Touch;
//	ent->NPC->side = 1;

	ent->client->ps.ping = ent->NPC->stats.reactions * 50;

	//MCG - Begin: NPC hacks
	//FIXME: Set the team correctly
	ent->client->ps.persistant[PERS_TEAM] = ent->client->playerTeam;

	//Run a script if you have one assigned to you
	if ( ent->behaviorSet[BSET_SPAWN] )
	{
		G_ActivateBehavior( ent, BSET_SPAWN );
	}

	ent->client->ps.eFlags |= EF_NPC;
	ent->e_UseFunc   = useF_NPC_Use;
	ent->e_ThinkFunc = thinkF_NPC_Think;
	ent->nextthink = level.time + FRAMETIME + Q_irand(0, 100);

	NPC_SetMiscDefaultData( ent );

	if ( !(ent->spawnflags & SFB_STARTINSOLID) )
	{//Not okay to start in solid
		G_CheckInSolid( ent, qtrue );
	}
	VectorClear( ent->NPC->lastClearOrigin );
	gi.linkentity( ent );
}

gNPC_t *New_NPC_t()
{
	gNPC_t *ptr = (gNPC_t *)G_Alloc (sizeof(gNPC_t));

	if (ptr)
	{
		// clear it...
		//
		memset(ptr, 0, sizeof( *ptr ) );

		// specific inits...
		//
		ptr->iSquadPathIndex = -1;	// -1 is treated as NULL ptr for above would be
		ptr->iSquadRouteIndex= -1;
	}

	return ptr;
}

/*
-------------------------
NPC_StasisSpawn_Go
-------------------------
*/
	
qboolean NPC_StasisSpawn_Go( gentity_t *ent )
{
	//Setup an owner pointer if we need it
	if VALIDSTRING( ent->ownername )
	{
		ent->owner = G_Find( NULL, FOFS( targetname ), ent->ownername );
		
		if ( ( ent->owner ) && ( ent->owner->health <= 0 ) )
		{//our spawner thing is broken
			if ( ent->target2 && ent->target2[0] )
			{
				//Fire off our target2
				G_UseTargets2( ent, ent, ent->target2 );

				//Kill us
				ent->e_ThinkFunc = thinkF_G_FreeEntity;
				ent->nextthink = level.time + 100;
			}
			else
			{
				//Try to spawn again in one second
				ent->e_ThinkFunc = thinkF_NPC_Spawn_Go;
				ent->nextthink = level.time + 1000;
			}
			return qfalse;
		}
	}

	//Test for an entity blocking the spawn
	trace_t	tr;
	gi.trace( &tr, ent->currentOrigin, ent->mins, ent->maxs, ent->currentOrigin, ent->s.number, MASK_NPCSOLID );

	//Can't have anything in the way
	if ( tr.allsolid || tr.startsolid )
	{
		ent->nextthink = level.time + 1000;
		return qfalse;
	}

	return qtrue;
}

/*
-------------------------
NPC_Spawn_Go
-------------------------
*/

void NPC_Spawn_Go( gentity_t *ent )
{
	gentity_t	*newent;
	int			index;
	vec3_t		saveOrg;

	//Do extra code for stasis spawners
	if ( Q_stricmp( ent->classname, "NPC_Stasis" ) == 0 )
	{
		if ( NPC_StasisSpawn_Go( ent ) == qfalse )
			return;
	}

	//Test for drop to floor
	if ( ent->spawnflags & NSF_DROP_TO_FLOOR )
	{
		trace_t		tr;
		vec3_t		bottom;

		VectorCopy( ent->currentOrigin, saveOrg );
		VectorCopy( ent->currentOrigin, bottom );
		bottom[2] -= 4096;
		gi.trace( &tr, ent->currentOrigin, ent->mins, ent->maxs, bottom, ent->s.number, MASK_NPCSOLID );
		if ( !tr.allsolid && !tr.startsolid && tr.fraction < 1.0 )
		{
			G_SetOrigin( ent, tr.endpos );
		}
	}

	//Check the spawner's count
	if( ent->count != -1 )
	{
		ent->count--;
		
		if( ent->count <= 0 )
		{
			ent->e_UseFunc = useF_NULL;//never again
		}
	}

	//Check for default health
	if( ent->health == 0 )
	{
		ent->health = 100;
	}

	newent = G_Spawn();

	if ( newent == NULL ) 
	{
		gi.Printf ( S_COLOR_RED"ERROR: NPC G_Spawn failed\n" );
		
		goto finish;
		return;
	}
	
	newent->svFlags |= SVF_NPC;
	newent->fullName = ent->fullName;

	newent->NPC = New_NPC_t();	
	if ( newent->NPC == NULL ) 
	{		
		gi.Printf ( S_COLOR_RED"ERROR: NPC G_Alloc NPC failed\n" );		
		goto finish;
		return;
	}	

	newent->NPC->tempGoal = G_Spawn();
	
	if ( newent->NPC->tempGoal == NULL ) 
	{
		newent->NPC = NULL;
		goto finish;
		return;
	}

	newent->NPC->tempGoal->classname = "NPC_goal";
	newent->NPC->tempGoal->owner = newent;

	newent->client = (gclient_s *)G_Alloc (sizeof(gclient_s));
	
	if ( newent->client == NULL ) 
	{
		gi.Printf ( S_COLOR_RED"ERROR: NPC G_Alloc client failed\n" );
		goto finish;
		return;
	}
	
	memset ( newent->client, 0, sizeof(*newent->client) );

//==NPC_Connect( newent, net_name );===================================

	if ( ent->NPC_type == NULL ) 
	{
		ent->NPC_type = "random";
	}

	if ( ent->svFlags & SVF_NO_BASIC_SOUNDS )
	{
		newent->svFlags |= SVF_NO_BASIC_SOUNDS;
	}
	if ( ent->svFlags & SVF_NO_COMBAT_SOUNDS )
	{
		newent->svFlags |= SVF_NO_COMBAT_SOUNDS;
	}
	if ( ent->svFlags & SVF_NO_EXTRA_SOUNDS )
	{
		newent->svFlags |= SVF_NO_EXTRA_SOUNDS;
	}
	if ( ent->svFlags & SVF_NO_SCAV_SOUNDS )
	{
		newent->svFlags |= SVF_NO_SCAV_SOUNDS;
	}
	
	G_InitBoltOnData( newent );
	if ( !NPC_ParseParms( ent->NPC_type, newent ) )
	{
		gi.Printf ( S_COLOR_RED "ERROR: Couldn't spawn NPC %s\n", ent->NPC_type );
		G_FreeEntity( newent );
		goto finish;
		return;
	}

	if ( ent->NPC_type )
	{
		if ( strstr( ent->NPC_type, "munro" ) != NULL || strstr( ent->NPC_type, "alexa" ) != NULL )
		{
			newent->NPC->aiFlags |= NPCAI_MATCHPLAYERWEAPON;
		}
		else if ( !Q_stricmp( ent->NPC_type, "test" ) )
		{
			int	n;
			for ( n = 0; n < 1 ; n++) 
			{
				if ( !(g_entities[n].svFlags & SVF_NPC) && g_entities[n].client) 
				{
					VectorCopy(g_entities[n].s.origin, newent->s.origin);
					newent->client->playerTeam = g_entities[n].client->playerTeam;
					break;
				}
			}
			newent->NPC->defaultBehavior = newent->NPC->behaviorState = BS_WAIT;
			newent->classname = "NPC";
	//		newent->svFlags |= SVF_NOPUSH;
		}
	}
//=====================================================================
	//set the info we want
	newent->script_targetname = ent->NPC_targetname;
	newent->targetname = ent->NPC_targetname;
	newent->target = ent->NPC_target;
	newent->target2 = ent->target2;
	newent->target3 = ent->target3;
	newent->wait = ent->wait;
	newent->infoString = ent->infoString;
	
	for( index = BSET_FIRST; index < NUM_BSETS; index++)
	{
		if ( ent->behaviorSet[index] )
		{
			newent->behaviorSet[index] = ent->behaviorSet[index];
		}
	}

	newent->classname = "NPC";
	newent->NPC_type = ent->NPC_type;
	gi.unlinkentity(newent);
	
	VectorCopy(ent->s.origin, newent->s.origin);
	VectorCopy(ent->s.origin, newent->client->ps.origin);
	VectorCopy(ent->s.origin, newent->currentOrigin);
	G_SetOrigin(newent, ent->s.origin);//just to be sure!

	VectorCopy(ent->s.angles, newent->s.angles);
	VectorCopy(ent->s.angles, newent->currentAngles);
	VectorCopy(ent->s.angles, newent->client->ps.viewangles);
	newent->NPC->desiredYaw =ent->s.angles[YAW];
	
	gi.linkentity(newent);
	newent->health = ent->health;
	newent->spawnflags = ent->spawnflags;

	if(ent->paintarget)
	{	//safe to point at owner's string since memory is never freed during game
		newent->paintarget = ent->paintarget;
	}
	if(ent->opentarget)
	{
		newent->opentarget = ent->opentarget;
	}

//==New stuff=====================================================================
	newent->s.eType	= ET_PLAYER;
	
	//FIXME: Call CopyParms
	if ( ent->parms )
	{
		for ( int parmNum = 0; parmNum < MAX_PARMS; parmNum++ )
		{
			if ( ent->parms->parm[parmNum] && ent->parms->parm[parmNum][0] )
			{
				Q3_SetParm( newent->s.number, parmNum, ent->parms->parm[parmNum] );
			}
		}
	}
	//FIXME: copy cameraGroup, store mine in message or other string field

	// allow to ride movers
	newent->s.pos.trType = TR_GRAVITY;
	
	newent->NPC->combatPoint = -1;

	newent->flags |= FL_NOTARGET;//So he's ignored until he's fully spawned
	newent->s.eFlags |= EF_NODRAW;//So he's ignored until he's fully spawned

	newent->e_ThinkFunc = thinkF_NPC_Begin;
	newent->nextthink = level.time + FRAMETIME;

	gi.linkentity (newent);

	if(ent->e_UseFunc == useF_NULL)
	{
		if( ent->target )
		{//use any target we're pointed at
			G_UseTargets ( ent, ent );
		}
		if(ent->closetarget)
		{//last guy should fire this target when he dies
			newent->target = ent->closetarget;
		}
		ent->targetname = NULL;
	}

finish:
	if ( ent->spawnflags & NSF_DROP_TO_FLOOR )
	{
		G_SetOrigin( ent, saveOrg );
	}
}

/*
-------------------------
NPC_StasisSpawnEffect
-------------------------
*/

void NPC_StasisSpawnEffect( gentity_t *ent )
{
	vec3_t		start, end, forward;
	qboolean	taper;

	//Floor or wall?
	if ( ent->spawnflags & 1 )
	{
		AngleVectors( ent->s.angles, forward, NULL, NULL );
		VectorMA( ent->currentOrigin,  24, forward, end );
		VectorMA( ent->currentOrigin, -20, forward, start );
		
		start[2] += 64;
		
		taper = qtrue;
	}
	else
	{
		VectorCopy( ent->currentOrigin, start );
		VectorCopy( start, end );
		end[2] += 48;
		taper = qfalse;
	}

	//Add the effect
	CG_ShimmeryThing_Spawner( start, end, 32, qtrue, 1000 );
}

/*
-------------------------
NPC_ShySpawn
-------------------------
*/

#define SHY_THINK_TIME			1000
#define SHY_SPAWN_DISTANCE		128
#define SHY_SPAWN_DISTANCE_SQR	( SHY_SPAWN_DISTANCE * SHY_SPAWN_DISTANCE )

void NPC_ShySpawn( gentity_t *ent )
{
	ent->nextthink = level.time + SHY_THINK_TIME;
	ent->e_ThinkFunc = thinkF_NPC_ShySpawn;

	if ( DistanceSquared( g_entities[0].currentOrigin, ent->currentOrigin ) <= SHY_SPAWN_DISTANCE_SQR )
		return;

	if ( (InFOV( ent, &g_entities[0], 80, 64 )) ) // FIXME: hardcoded fov
		if ( (NPC_ClearLOS( &g_entities[0], ent->currentOrigin )) )
			return;

	ent->e_ThinkFunc = thinkF_NULL;
	ent->nextthink = 0;

	NPC_Spawn_Go( ent );
}

/*
-------------------------
NPC_Spawn
-------------------------
*/

void NPC_Spawn(gentity_t *ent, gentity_t *other, gentity_t *activator)
{
	//delay before spawning NPC
	if( ent->delay )
	{
		//Stasis does an extra step
		if ( Q_stricmp( ent->classname, "NPC_Stasis" ) == 0 )
		{
			if ( NPC_StasisSpawn_Go( ent ) == qfalse )
				return;
		}

		if ( ent->spawnflags & 2048 )  // SHY
			ent->e_ThinkFunc = thinkF_NPC_ShySpawn;
		else
			ent->e_ThinkFunc = thinkF_NPC_Spawn_Go;

		ent->nextthink = level.time + ent->delay;
	}
	else
	{
		if ( ent->spawnflags & 2048 )  // SHY
			NPC_ShySpawn( ent );
		else
			NPC_Spawn_Go( ent );
	}
}

/*QUAKED NPC_spawner (1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)
target2 - Fired by stasis spawners when they try to spawn while their spawner model is broken
target3 - Fired by spawner if they try to spawn and are blocked and have a wait < 0 (removes them)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1
delay - how long to wait to spawn after used
wait - if trying to spawn and blocked, how many seconds to wait before trying again (default = 0.5, < 0 = never try again and fire target2)

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
NPC_target2 - NPC's target to fire when knocked out
NPC_type - type of NPC ("Borg" (default), "Xian", etc)
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is none)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)
These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
//void NPC_PrecacheModels ( char *NPCName );
void SP_NPC_spawner( gentity_t *self)
{
	extern void NPC_PrecacheAnimationCFG( const char *NPC_type );

	if ( !self->fullName || !self->fullName[0] )
	{
		//FIXME: make an index into an external string table for localization

		if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
		{
			self->fullName = "Humanoide Lebensform";
		}
		else
		{
			self->fullName = "Humanoid Lifeform";
		}
	}

	//register/precache the models needed for this NPC, not anymore
	//self->classname = "NPC_spawner";

	if(!self->count)
	{
		self->count = 1;
	}

	{//Stop loading of certain extra sounds
		static	int	garbage;

		if ( G_SpawnInt( "noBasicSounds", "0", &garbage ) )
		{
			self->svFlags |= SVF_NO_BASIC_SOUNDS;
		}
		if ( G_SpawnInt( "noCombatSounds", "0", &garbage ) )
		{
			self->svFlags |= SVF_NO_COMBAT_SOUNDS;
		}
		if ( G_SpawnInt( "noExtraSounds", "0", &garbage ) )
		{
			self->svFlags |= SVF_NO_EXTRA_SOUNDS;
		}
		if ( G_SpawnInt( "noScavSounds", "0", &garbage ) )
		{
			self->svFlags |= SVF_NO_SCAV_SOUNDS;
		}
	}

	if ( !self->wait )
	{
		self->wait = 500;
	}
	else
	{
		self->wait *= 1000;//1 = 1 msec, 1000 = 1 sec
	}

	self->delay *= 1000;//1 = 1 msec, 1000 = 1 sec

	if ( self->delay > 0 )
	{
		self->svFlags |= SVF_NPC_PRECACHE;
	}

	//We have to load the animation.cfg now because spawnscripts are going to want to set anims and we need to know their length and if they're valid
	NPC_PrecacheAnimationCFG( self->NPC_type );

	if(self->targetname)
	{//Wait for triggering
		self->e_UseFunc = useF_NPC_Spawn;
		self->svFlags |= SVF_NPC_PRECACHE;
		//NPC_PrecacheModels( self->NPC_type );
	}
	else
	{
		NPC_Spawn (self, self, self);
	}

	//FIXME: store cameraGroup somewhere else and apply to spawned NPCs' cameraGroup
	//Or just don't include NPC_spawners in cameraGroupings
}

/*QUAKED NPC_starfleet (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

RIFLEMAN - Will use Phaser Compression Rifle (default is normal phaser)
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Star Fleet NPC

Helmeted Hazard guy by default, override by setting:
NPC_type:
(Hazard)
	Kenn
	Odell
(Crew)
	Pasty
	Inya

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_starfleet( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "Security";
	}

	self->classname = "NPC_starfleet";

	SP_NPC_spawner (self);
}

/*QUAKED NPC_starfleet_random (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

RIFLEMAN - Will use Phaser Compression Rifle (default is normal phaser)
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Star Fleet NPC

Randomly picks a generic male or female crewmember.
For a random male only crewmember, set NPC_type to "male".
For a random female only crewmember, set NPC_type to "female".
For a random gold only crewmember, set NPC_type to "gold".
For a random red only crewmember, set NPC_type to "red".
For a random blue only crewmember, set NPC_type to "blue".

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_starfleet_random( gentity_t *self)
{
	int random;

	//FIXME: randomize scale some, +- 3?
	if ( !self->NPC_type || !self->NPC_type[0] )
	{
		random = Q_irand(0, 1);
		switch(random)
		{
		default:
		case 0:
			self->NPC_type = "male";
			break;
		case 1:
			self->NPC_type = "female";
			break;
		}
	}

	if ( Q_stricmp("female", self->NPC_type) == 0 )
	{//Pick a random female crew config
		random = Q_irand( 0, 8 );

		switch(random)
		{
		case 0:
			self->NPC_type = "GoldF1";
			break;
		case 1:
			self->NPC_type = "GoldF2";
			break;
		case 2:
			self->NPC_type = "GoldF3";
			break;
		case 3:
			self->NPC_type = "redF1";
			break;
		case 4:
			self->NPC_type = "redF2";
			break;
		case 5:
			self->NPC_type = "redF3";
			break;
		case 6:
			self->NPC_type = "blueF1";
			break;
		case 7:
			self->NPC_type = "blueF2";
			break;
		default:
		case 8:
			self->NPC_type = "blueF3";
			break;
		}
	}
	else if ( Q_stricmp("male", self->NPC_type) == 0 )
	{//Pick a random male crew config
		random = Q_irand( 0, 23 );

		switch(random)
		{
		case 0:
			self->NPC_type = "goldm1";
			break;
		case 1:
			self->NPC_type = "goldm2";
			break;
		case 2:
			self->NPC_type = "goldm3";
			break;
		case 3:
			self->NPC_type = "goldm4";
			break;
		case 4:
			self->NPC_type = "goldm5";
			break;
		case 5:
			self->NPC_type = "goldm6";
			break;
		case 6:
			self->NPC_type = "goldm7";
			break;
		case 7:
			self->NPC_type = "goldm8";
			break;
		case 8:
			self->NPC_type = "bluem1";
			break;
		case 9:
			self->NPC_type = "bluem2";
			break;
		case 10:
			self->NPC_type = "bluem3";
			break;
		case 11:
			self->NPC_type = "bluem4";
			break;
		case 12:
			self->NPC_type = "bluem5";
			break;
		case 13:
			self->NPC_type = "bluem6";
			break;
		case 14:
			self->NPC_type = "bluem7";
			break;
		case 15:
			self->NPC_type = "bluem8";
			break;
		case 16:
			self->NPC_type = "redm1";
			break;
		case 17:
			self->NPC_type = "redm2";
			break;
		case 18:
			self->NPC_type = "redm3";
			break;
		case 19:
			self->NPC_type = "redm4";
			break;
		case 20:
			self->NPC_type = "redm5";
			break;
		case 21:
			self->NPC_type = "redm6";
			break;
		case 22:
			self->NPC_type = "redm7";
			break;
		default:
		case 23:
			self->NPC_type = "redm8";
			break;
		}
	}
	else if ( Q_stricmp("gold", self->NPC_type) == 0 )
	{//Pick a random female crew config
		random = Q_irand( 0, 10 );

		switch(random)
		{
		case 0:
			self->NPC_type = "GoldF1";
			break;
		case 1:
			self->NPC_type = "GoldF2";
			break;
		case 2:
			self->NPC_type = "GoldF3";
			break;
		case 3:
			self->NPC_type = "GoldM1";
			break;
		case 4:
			self->NPC_type = "GoldM2";
			break;
		case 5:
			self->NPC_type = "GoldM3";
			break;
		case 6:
			self->NPC_type = "GoldM4";
			break;
		case 7:
			self->NPC_type = "GoldM5";
			break;
		case 8:
			self->NPC_type = "GoldM6";
			break;
		case 9:
			self->NPC_type = "GoldM7";
			break;
		default:
		case 10:
			self->NPC_type = "GoldM8";
			break;
		}
	}
	else if ( Q_stricmp("red", self->NPC_type) == 0 )
	{//Pick a random female crew config
		random = Q_irand( 0, 10 );

		switch(random)
		{
		case 0:
			self->NPC_type = "redF1";
			break;
		case 1:
			self->NPC_type = "redF2";
			break;
		case 2:
			self->NPC_type = "redF3";
			break;
		case 3:
			self->NPC_type = "redM1";
			break;
		case 4:
			self->NPC_type = "redM2";
			break;
		case 5:
			self->NPC_type = "redM3";
			break;
		case 6:
			self->NPC_type = "redM4";
			break;
		case 7:
			self->NPC_type = "redM5";
			break;
		case 8:
			self->NPC_type = "redM6";
			break;
		case 9:
			self->NPC_type = "redM7";
			break;
		default:
		case 10:
			self->NPC_type = "redM8";
			break;
		}
	}
	else //must be blue: if ( Q_stricmp("gold", self->NPC_type) == 0 )
	{//Pick a random female crew config
		random = Q_irand( 0, 10 );

		switch(random)
		{
		case 0:
			self->NPC_type = "blueF1";
			break;
		case 1:
			self->NPC_type = "blueF2";
			break;
		case 2:
			self->NPC_type = "blueF3";
			break;
		case 3:
			self->NPC_type = "blueM1";
			break;
		case 4:
			self->NPC_type = "blueM2";
			break;
		case 5:
			self->NPC_type = "blueM3";
			break;
		case 6:
			self->NPC_type = "blueM4";
			break;
		case 7:
			self->NPC_type = "blueM5";
			break;
		case 8:
			self->NPC_type = "blueM6";
			break;
		case 9:
			self->NPC_type = "blueM7";
			break;
		default:
		case 10:
			self->NPC_type = "blueM8";
			break;
		}
	}
	//self->NPC_type = "random";

	SP_NPC_starfleet( self );
}
//Characters

/*QUAKED NPC_Tuvok (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Tuvok in gold

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Tuvok( gentity_t *self)
{
	if ( !self->NPC_type || !self->NPC_type[0] )
	{
		self->NPC_type = "Tuvok";
	}

	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Kim (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Kim( gentity_t *self)
{
	self->NPC_type = "Kim";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Doctor (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Doctor( gentity_t *self)
{
	self->NPC_type = "Doctor";
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Paris (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Paris( gentity_t *self)
{
	self->NPC_type = "Paris";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Torres (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN  NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Torres( gentity_t *self)
{
	self->NPC_type = "Torres";
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Janeway (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN  NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Janeway( gentity_t *self)
{
	self->NPC_type = "Janeway";
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Seven (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN  NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Seven in her red uniform
For different uniforms, use these 'NPC_type's:
sevenblue - blue/silver uniform
sevenbrown - brown uniform
sevenblack - black uniform
sevenhazard - in Hazard Gear, 7 of 9 style

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Seven( gentity_t *self)
{
	if ( !self->NPC_type || !self->NPC_type[0] )
	{
		self->NPC_type = "Seven";
	}
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Chakotay (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN  NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Chakotay( gentity_t *self)
{
	self->NPC_type = "Chakotay";
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Neelix(1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Neelix( gentity_t *self)
{
	self->NPC_type = "Neelix";
	SP_NPC_starfleet(self);
}
/*QUAKED NPC_Vorik (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Vorik( gentity_t *self)
{
	self->NPC_type = "Vorik";
	SP_NPC_starfleet(self);
}
//HazTeam
/*QUAKED NPC_Foster (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Foster( gentity_t *self)
{
	self->NPC_type = "Foster";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Munro (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

NPC_type - "lt" will make the male/female Munro NPC a lt.

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Munro( gentity_t *self)
{
	if ( g_sex->string[0] == 'f' )
	{
		if ( self->NPC_type && self->NPC_type[0] && Q_stricmp( "lt", self->NPC_type ) == 0 )
		{
			self->NPC_type = "alexandria_lt";
		}
		else
		{
			self->NPC_type = "alexandria";
		}
	}
	else
	{
		if ( self->NPC_type && self->NPC_type[0] && Q_stricmp( "lt", self->NPC_type ) == 0 )
		{
			self->NPC_type = "munro_lt";
		}
		else
		{
			self->NPC_type = "munro";
		}
	}
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_MunroScav (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Munro in his/her Scavenger disguise.

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_MunroScav( gentity_t *self)
{
	if ( g_sex->string[0] == 'f' )
	{
		self->NPC_type = "alexascav";
	}
	else
	{
		self->NPC_type = "munroscav";
	}
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Telsia (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Telsia( gentity_t *self)
{
	self->NPC_type = "Telsia";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Biessman (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Biessman( gentity_t *self)
{
	self->NPC_type = "Biessman";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Chang (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Chang( gentity_t *self)
{
	self->NPC_type = "Chang";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Chell (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Chell( gentity_t *self)
{
	self->NPC_type = "Chell";
	SP_NPC_starfleet(self);
}

/*QUAKED NPC_Jurot (1 0 0) (-12 -12 -24) (12 12 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_STARFLEET)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Jurot( gentity_t *self)
{
	self->NPC_type = "Jurot";
	SP_NPC_starfleet(self);
}
//ENEMIES

/*QUAKED NPC_borg (1 0 0) (-12 -12 -24) (12 12 40) x OLDBORG GUN TASER DRILL SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

There are different Borg configurations (hoses, textures, heads, etc) you can use by changing the NPC_type:
borgthin
borgthin2
borgthin3
borgthin4
borgbig
borgbig2
borgbig3
borgbig4

The default configuration randomly chooses one of these.

Default Borg is aggressive, using assimilator only
OLDBORG - Non-aggressive, uses assimilator
GUN - Aggressive, uses standard Borg projectile
TASER - Aggressive, uses taser (medium-range lightning weapon)

Borg NPC

FIXME: use the same legs and torso, different heads and skins if necc...

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/

#define	BORG_DEFAULT_HEALTH	30

void SP_NPC_borg( gentity_t *self)
{
	if(!self->NPC_type)
	{
		int	which = Q_irand(0, 7);
		switch( which )
		{
		case 0:
			self->NPC_type = "borgthin";
			break;
		case 1:
			self->NPC_type = "borgthin2";
			break;
		case 2:
			self->NPC_type = "borgthin3";
			break;
		case 3:
			self->NPC_type = "borgthin4";
			break;
		case 4:
			self->NPC_type = "borgbig";
			break;
		case 5:
			self->NPC_type = "borgbig2";
			break;
		case 6:
			self->NPC_type = "borgbig3";
			break;
		case 7:
			self->NPC_type = "borgbig4";
			break;
		}
	}

	// Set the default weapon
	weapon_t weap = WP_BORG_ASSIMILATOR;

	if ( self->spawnflags & SFB_GUN )
		weap = WP_BORG_WEAPON;
	else if ( self->spawnflags & SFB_TASER )
		weap = WP_BORG_TASER;
	else if ( self->spawnflags & SFB_DRILL )
		weap = WP_BORG_DRILL;

	// Now register the weapon
	RegisterItem( FindItemForWeapon( weap ) );

	// Precaching sounds for this model
	G_SoundIndex("sound/enemies/borg/borgshield.wav");

	if ( !self->health )
	{
		self->health = BORG_DEFAULT_HEALTH + ( random() * 15 );
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Parasite(1 0 0) (-13 -13 -24) (13 13 -8) x x SPIT AGGRESSIVE DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Parasite NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 25)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Parasite( gentity_t *self)
{
	G_SpawnInt( "health", "25", &self->health );

	if(!self->NPC_type)
	{
		self->NPC_type = "Parasite";
	}

	// For some reason, they don't always have their weapon precached unless you do this right away?
	RegisterItem( FindItemForWeapon( WP_PARASITE ) );

	if( Q_irand(0, 1) )
	{
		self->spawnflags |= 4;//setting this flag will make these spitters, not clawers
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_klingon(1 0 0) (-12 -12 -24) (12 12 40) KNIFE x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

KNIFE - spawns as a melee fighter with a knife
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Klingon Scavenger NPC
NPC_type - "KlingonFem" is the female klingon

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_klingon( gentity_t *self)
{
	if(!self->NPC_type)
	{
		if ( Q_irand(0, 1) )
		{
			self->NPC_type = "Klingon";
		}
		else
		{
			self->NPC_type = "Klingon2";
		}
	}
	else if ( Q_stricmp( "KlingonFem", self->NPC_type ) == 0 )
	{
		if ( Q_irand(0, 2) )
		{
			self->NPC_type = "KlingonFem2";
		}
		else if ( Q_irand(0, 1) )
		{
			self->NPC_type = "KlingonFem3";
		}
	}

	if ( (self->spawnflags & 1) ) // knife
	{
		RegisterItem( FindItemForWeapon( WP_KLINGON_BLADE ) );	//precache the weapon
	}

	SP_NPC_spawner( self );
}

/*QUAKED NPC_Malon(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Malon Scavenger NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Malon( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "Malon";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Hirogen(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Hirogen Scavenger NPC

NPC_type - "Hirogen2" has a helmet

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Hirogen( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "Hirogen";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Hirogen_Alpha(1 0 0) (-12 -12 -24) (12 12 32) x x x x x SILENTSPAWN NOTSOLID STARTINSOLID SHY

SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Hirogen Scavenger Alpha

NPC_type - "Hirogen2" has a helmet

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)
opentarget - it's dropped weapon will fire this tartet when it's picked up

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/

#define	HIROGEN_ALPHA_HEALTH	8	//NOTENOTE: This is hits, NOT total health!  The boss only takes 1 point of damage for any hit

void SP_NPC_Hirogen_Alpha( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "HirogenAlpha";
	}

	self->health = HIROGEN_ALPHA_HEALTH;

	SP_NPC_spawner (self);

	HirogenAlpha_Precache();
}

/*QUAKED NPC_Imperial(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Imperial Scavenger NPC, Red shirt

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
/*QUAKED NPC_Imperial_Blue(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Imperial Scavenger NPC, Blue

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
/*QUAKED NPC_Imperial_Gold(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Imperial Scavenger NPC, Gold

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
/*QUAKED NPC_Imperial_Raider(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Imperial Scavenger NPC, Raider

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Imperial( gentity_t *self)
{
	if(!self->NPC_type)
	{
		if( !Q_stricmp("NPC_Imperial_Blue", self->classname ) )
		{
			self->NPC_type = "ImperialBlue";
		}
		else if( !Q_stricmp("NPC_Imperial_Gold", self->classname ) )
		{
			self->NPC_type = "ImperialGold";
		}
		else if( !Q_stricmp("NPC_Imperial_Raider", self->classname ) )
		{
			self->NPC_type = "ImperialRaider";
		}
		else
		{
			self->NPC_type = "Imperial";
		}
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Stasis(1 0 0) (-12 -12 -24) (12 12 32) WALL_SPAWNER SHOOTER x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

WALL_SPAWNER - Use spawn effect from the wall, not the floor (default)
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Etherian (Stasis alien) NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Stasis( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	int	health = 40;

	//Determine the type of stasis alien
	if ( self->spawnflags & 2 )
	{
		//Shooter
		if( !self->NPC_type )
		{
			self->NPC_type = "StasisShooter";
		}
		
		self->health = 60;
	}
	else
	{
		//Common
		if( !self->NPC_type )
		{
			self->NPC_type = "Stasis";
		}
	}

	//See if we should set the health
	if ( self->health == 0 )
	{
		self->health = health;
	}

	// Precache the sounds...
	G_SoundIndex("sound/movers/stasistransporter.wav");
	G_SoundIndex("sound/ambience/stasis/intostasis.wav");

	RegisterItem( FindItemForWeapon( WP_STASIS_ATTACK ) );

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Species8472(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Species 8472 NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Species8472( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Flüssigraum-Lebensform";
	}
	else
	{
		self->fullName = "Fluidic Space Lifeform";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "Species8472";
	}

	// Now register the weapon
	RegisterItem( FindItemForWeapon( WP_MELEE ) );

	SP_NPC_spawner (self);
	G_SoundIndex ("sound/enemies/species8472/hit5.wav");

	if ( !self->health )
	{
		self->health = 50;
	}
}

/*QUAKED NPC_Headbot(1 0 0) (-4 -4 -24) (4 4 8) x START_PLUGGED_IN x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Headbot NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Headbot( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Mechanisches Objekt";
	}
	else
	{
		self->fullName = "Mechanized Object";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "Headbot";
	}

	if ( !self->health )
	{
		self->health = 25;
	}

	SP_NPC_spawner( self );
}

/*QUAKED NPC_scoutbot(1 0 0) (-10 -10 -24) (10 10 16) x NO_HEAD x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Scoutbot NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_scoutbot( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Mechanisches Objekt";
	}
	else
	{
		self->fullName = "Mechanized Object";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "scoutbot";
	}

	if ( !self->health )
	{
		self->health = 100;
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_HKbot(1 0 0) (-10 -10 -24) (10 10 -16) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Hunter-Seeker bot NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_HKbot( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Mechanisches Objekt";
	}
	else
	{
		self->fullName = "Mechanized Object";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "HKbot";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_warriorbot(1 0 0) (-20 -20 -24) (20 20 56) BOSS NO_HEAD x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Warbot NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_warriorbot( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Mechanisches Objekt";
	}
	else
	{
		self->fullName = "Mechanized Object";
	}

	if ( self->spawnflags & 1 )
	{
		if( !self->NPC_type )
		{
			self->NPC_type = "warriorbot_boss";
		}
	
		if ( self->health == 0 )
		{
			self->health = 1500;
		}
	}
	else
	{
		if(!self->NPC_type)
		{
			self->NPC_type = "warriorbot";
		}

		if ( self->health == 0 )
		{
			self->health = 300;
		}
	}

	SP_NPC_spawner (self);
	G_SoundIndex ("sound/enemies/warbot/stabhit.wav");
}

/*QUAKED NPC_Harvester(1 0 0) (-24 -24 -12) (24 24 36) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Harvester NPC

NPC_type "harvesterboss" - bigger, tougher, slower, meaner Harvesters

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 50)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Harvester( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if ( !self->NPC_type )
	{
		self->NPC_type = "Harvester";
	}

	G_SpawnInt( "health", "60", &self->health );
	RegisterItem( FindItemForWeapon( WP_MELEE ) );

	SP_NPC_spawner( self );
	G_SoundIndex ("sound/enemies/harvester/stab3.wav");
}

/*QUAKED NPC_Reaver(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Reaver NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Reaver( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "reaver";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_ReaverGuard(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Reaver Guard NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_ReaverGuard( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "reaverGuard";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Avatar(1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY 

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Avatar NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Avatar( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "avatar";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Vohrsoth(1 0 0) (-180 -70 -28) (20 70 150) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Vohrsoth NPC

NPC_type - "vohrsothWall" - the Vohrsoth plugged in

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BOTS for now)
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Vohrsoth( gentity_t *self)
{
	//FIXME: make an index into an external string table for localization
	if (g_language && Q_stricmp("DEUTSCH",g_language->string)==0)
	{
		self->fullName = "Fremdartige Lebensform";
	}
	else
	{
		self->fullName = "Alien Lifeform";
	}

	if(!self->NPC_type)
	{
		self->NPC_type = "vohrsoth";
	}

	self->health = 10000;

	SP_NPC_spawner( self );

	// Now register the weapon
	RegisterItem( FindItemForWeapon( WP_FORGE_PROJ ) );
	RegisterItem( FindItemForWeapon( WP_BOT_ROCKET ) );

	for ( int i = 1; i < 5; i++ )
		G_SoundIndex( va( "sound/weapons/explosions/explode%d.wav", i ) );

	G_SoundIndex( "sound/weapons/explosions/fireball.wav" );
	G_SoundIndex( "sound/weapons/hunter_seeker/fire.wav" );
	G_SoundIndex( "sound/enemies/vorhsoth/beamhit.wav" );
	G_SoundIndex( "sound/enemies/vorhsoth/beamfire.wav" );
}

//============ HOLODECK ENEMIES ===============

/*QUAKED NPC_Desperado (1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Desperado

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Desperado( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "desperado";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_Paladin (1 0 0) (-12 -12 -24) (12 12 32) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Paladin

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Paladin( gentity_t *self)
{
	if(!self->NPC_type)
	{
		self->NPC_type = "paladin";
	}

	SP_NPC_spawner (self);
}

/*QUAKED NPC_BioHulk(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

BioHulk NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_BioHulk( gentity_t *self)
{
	self->NPC_type = "biohulk";

	SP_NPC_spawner( self );
}
/*QUAKED NPC_ChaoticaGuard(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Chaotica's Guard NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_ChaoticaGuard( gentity_t *self)
{
	self->NPC_type = "chaoticaguard";

	if(self->health <= 0)
	{
		self->health = 40;
	}

	SP_NPC_spawner( self );
}
/*QUAKED NPC_Chaotica(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Doctor Chaotica NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Chaotica( gentity_t *self)
{
	self->NPC_type = "chaotica";

	SP_NPC_spawner( self );
}
/*QUAKED NPC_CaptainProton (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Captain Proton NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_CaptainProton( gentity_t *self)
{
	self->NPC_type = "proton";

	SP_NPC_spawner( self );
}
/*QUAKED NPC_SatansRobot(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Satan's Robot NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_SatansRobot( gentity_t *self)
{
	self->NPC_type = "satan";

	if ( self->health <= 0 )
	{
		self->health = 250;
	}

	SP_NPC_spawner( self );
}
/*QUAKED NPC_Buster(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Buster Kincaide NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Buster( gentity_t *self)
{
	self->NPC_type = "buster";

	SP_NPC_spawner( self );
}
/*QUAKED NPC_Goodheart(1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR SILENTSPAWN NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
SILENTSPAWN - Will not make a spawn noise or effect
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Constance Goodheart NPC

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
health - starting health (default = 100)
playerTeam - Who not to shoot! (default is TEAM_BORG for now)
	TEAM_FREE (none) = 0
	TEAM_RED = 1
	TEAM_BLUE = 2
	TEAM_GOLD = 3
	TEAM_GREEN = 4
	TEAM_STARFLEET = 5
	TEAM_BORG = 6
	TEAM_SCAVENGERS = 7
	TEAM_STASIS = 8
	TEAM_NPCS = 9
	TEAM_HARVESTER, = 10
	TEAM_FORGE = 11
enemyTeam - Who to shoot (all but own if not set)

spawnscript - default script to run once spawned (none by default)
idlescript - default script to run when idle (none by default)
touchscript - default script to run if NPC bumps into someone (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)

These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
    default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC
*/
void SP_NPC_Goodheart( gentity_t *self)
{
	self->NPC_type = "goodheart";

	SP_NPC_spawner( self );
}
//NPC console commands
/*
NPC_Spawn_f
*/

static void NPC_Spawn_f(void) 
{
	gentity_t		*NPCspawner = G_Spawn();
	vec3_t			forward, end;
	trace_t			trace;
	
	if(!NPCspawner)
	{
		gi.Printf( S_COLOR_RED"NPC_Spawn Error: Out of entities!\n" );
		return;
	}

	//Spawn it at spot of first player
	//FIXME: will gib them!
	AngleVectors(g_entities[0].client->ps.viewangles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorMA(g_entities[0].currentOrigin, 64, forward, end);
	gi.trace(&trace, g_entities[0].currentOrigin, NULL, NULL, end, 0, MASK_SOLID);
	VectorCopy(trace.endpos, end);
	end[2] -= 24;
	gi.trace(&trace, trace.endpos, NULL, NULL, end, 0, MASK_SOLID);
	VectorCopy(trace.endpos, end);
	end[2] += 24;
	G_SetOrigin(NPCspawner, end);
	VectorCopy(NPCspawner->currentOrigin, NPCspawner->s.origin);

	gi.linkentity(NPCspawner);

	NPCspawner->NPC_type = G_NewString(gi.argv( 2 ));
	NPCspawner->NPC_targetname = G_NewString(gi.argv( 3 ));

	NPCspawner->count = 1;

	NPCspawner->delay = 0;

	//NPCspawner->spawnflags |= SFB_NOTSOLID;

	//NPCspawner->playerTeam = TEAM_FREE;
	NPCspawner->behaviorSet[BSET_SPAWN] = "common/guard";

	NPC_Spawn (NPCspawner, NPCspawner, NPCspawner);
	
	NPCspawner->e_ThinkFunc = thinkF_G_FreeEntity;
	NPCspawner->nextthink = level.time + FRAMETIME;
}

/*
NPC_Kill_f
*/

void NPC_Kill_f( void ) 
{
	int			n;
	gentity_t	*player;
	char		*name;
	team_t		killTeam = TEAM_FREE;
	qboolean	killNonSF = qfalse;

	name = gi.argv( 2 );

	if ( !*name || !name[0] )
	{
		gi.Printf( S_COLOR_RED"NPC_Kill Error: Expected:\n");
		gi.Printf( S_COLOR_RED"NPC kill '[NPC targetname]' or 'all' - kills NPCs with certain targetname\n" );
		gi.Printf( S_COLOR_RED"or\n" );
		gi.Printf( S_COLOR_RED"NPC kill 'all' - kills all NPCs\n" );
		gi.Printf( S_COLOR_RED"or\n" );
		gi.Printf( S_COLOR_RED"NPC team '[teamname]' - kills all NPCs of a certain team\n" );
		return;
	}

	if ( Q_stricmp( "team", name ) == 0 )
	{
		name = gi.argv( 3 );

		if ( !*name || !name[0] )
		{
			gi.Printf( S_COLOR_RED"NPC_Kill Error: 'npc kill team' requires a team name!\n" );
			gi.Printf( S_COLOR_RED"Valid team names are:\n");
			for ( n = (TEAM_FREE + 1); n < TEAM_NUM_TEAMS; n++ )
			{
				gi.Printf( S_COLOR_RED"%s\n", TeamNames[n] );
			}
			gi.Printf( S_COLOR_RED"nonstarfleet - kills all but your teammates\n" );
			return;
		}

		if ( Q_stricmp( "nonstarfleet", name ) == 0 )
		{
			killNonSF = qtrue;
		}
		else
		{
			killTeam = TranslateTeamName( name );

			if ( killTeam == TEAM_FREE )
			{
				gi.Printf( S_COLOR_RED"NPC_Kill Error: team '%s' not recognized\n", name );
				gi.Printf( S_COLOR_RED"Valid team names are:\n");
				for ( n = (TEAM_FREE + 1); n < TEAM_NUM_TEAMS; n++ )
				{
					gi.Printf( S_COLOR_RED"%s\n", TeamNames[n] );
				}
				gi.Printf( S_COLOR_RED"nonstarfleet - kills all but your teammates\n" );
				return;
			}
		}
	}

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		player = &g_entities[n];
		if (!player->inuse) {
			continue;
		}
		if ( killNonSF )
		{
			if ( player )
			{
				if ( player->client )
				{
					if ( player->client->playerTeam != TEAM_STARFLEET )
					{
						gi.Printf( S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname );
						player->health = 0;
						GEntity_DieFunc(player, player, player, player->max_health, MOD_UNKNOWN);
					}
				}
				else if ( player->NPC_type && player->classname && player->classname[0] && Q_stricmp( "NPC_starfleet", player->classname ) != 0 )
				{//A spawner, remove it
					gi.Printf( S_COLOR_GREEN"Removing NPC spawner %s with NPC named %s\n", player->NPC_type, player->NPC_targetname );
					G_FreeEntity( player );
					//FIXME: G_UseTargets2(player, player, player->NPC_target & player->target);?
				}
			}
		}
		else if ( player && player->NPC && player->client )
		{
			if ( killTeam != TEAM_FREE )
			{
				if ( player->client->playerTeam == killTeam )
				{
					gi.Printf( S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname );
					player->health = 0;
					GEntity_DieFunc(player, player, player, player->max_health, MOD_UNKNOWN);
				}
			}
			else if( (player->targetname && Q_stricmp( name, player->targetname ) == 0)
				|| Q_stricmp( name, "all" ) == 0 )
			{
				gi.Printf( S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname );
				player->health = 0;
				player->client->ps.stats[STAT_HEALTH] = 0;
				GEntity_DieFunc(player, player, player, 100, MOD_UNKNOWN);
			}
		}
	}
}

void NPC_PrintScore( gentity_t *ent )
{
	gi.Printf( "%s: %d\n", ent->targetname, ent->client->ps.persistant[PERS_SCORE] );
}

/*
Svcmd_NPC_f

parse and dispatch bot commands
*/
qboolean	showBBoxes = qfalse;
void Svcmd_NPC_f( void ) 
{
	char	*cmd;

	cmd = gi.argv( 1 );

	if ( !*cmd ) 
	{
		gi.Printf( "Valid NPC commands are:\n" );
		gi.Printf( " spawn [NPC type (from NCPCs.cfg)]\n" );
		gi.Printf( " kill [NPC targetname] or [all(kills all NPCs)] or 'team [teamname]'\n" );
		gi.Printf( " showbounds (draws exact bounding boxes of NPCs)\n" );
		gi.Printf( " squadpath [NPC targetname] (squadpoint and path of NPCs)\n" );
		gi.Printf( " score [NPC targetname] (prints number of kills per NPC)\n" );
	}
	else if ( Q_stricmp( cmd, "spawn" ) == 0 ) 
	{
		NPC_Spawn_f();
	}
	else if ( Q_stricmp( cmd, "kill" ) == 0 ) 
	{
		NPC_Kill_f();
	}
	else if ( Q_stricmp( cmd, "showbounds" ) == 0 )
	{//Toggle on and off
		showBBoxes = showBBoxes ? qfalse : qtrue;
	}
	else if ( Q_stricmp ( cmd, "squadpath" ) == 0 )
	{//Draw squadpaths, also check a cmd2 for a specific NPC's squadpath
		char		*cmd2 = gi.argv(2);

		if ( !cmd2 )
		{
			showSPaths[0] = 0;
		}
		else if ( Q_stricmp( "all", cmd2 ) == 0 )
		{//Show them all
			Q_strncpyz( showSPaths, "all", sizeof(showSPaths), qtrue );
		}
		else
		{
			Q_strncpyz( showSPaths, cmd2, sizeof(showSPaths), qtrue );
		}
	}
	else if ( Q_stricmp ( cmd, "score" ) == 0 )
	{//Draw squadpaths, also check a cmd2 for a specific NPC's squadpath
		char		*cmd2 = gi.argv(2);
		gentity_t *ent = NULL;

		if ( !cmd2 || !cmd2[0] )
		{//Show the score for all NPCs
			gi.Printf( "SCORE LIST:\n" );
			for ( int i = 0; i < ENTITYNUM_WORLD; i++ )
			{
				ent = &g_entities[i];
				if ( !ent || !ent->client )
				{
					continue;
				}
				NPC_PrintScore( ent );
			}
		}
		else
		{
			if ( (ent = G_Find( NULL, FOFS(targetname), cmd2 )) != NULL && ent->client )
			{
				NPC_PrintScore( ent );
			}
			else
			{
				gi.Printf( "ERROR: NPC score - no such NPC %s\n", cmd2 );
			}
		}
	}
}
