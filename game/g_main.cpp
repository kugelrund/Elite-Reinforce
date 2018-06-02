
#include "g_local.h"
#include "g_functions.h"
#include "Q3_Interface.h"
#include "g_nav.h"
#include "boltOns.h"
#include "g_roff.h"
#include "g_navigator.h"
#include "anims.h"

extern CNavigator		navigator;

#define	STEPSIZE		18

level_locals_t	level;
game_import_t	gi;
game_export_t	globals;

gentity_t		*g_entities;
gentity_t		*playerEnt = &g_entities[0];

cvar_t	*g_speed;
cvar_t	*g_gravity;
cvar_t	*g_sex;
cvar_t	*g_spskill;
cvar_t	*g_cheats;
cvar_t	*g_developer;
cvar_t	*g_timescale;
cvar_t	*g_knockback;
cvar_t	*g_inactivity;
cvar_t	*g_debugMove;
cvar_t	*g_debugDamage;
cvar_t	*g_weaponRespawn;
cvar_t	*g_subtitles;
cvar_t	*g_language;
cvar_t	*g_ICARUSDebug;
cvar_t	*com_buildScript;
cvar_t	*g_skippingcin;
cvar_t	*g_virtualVoyager;

// new additions for Elite Reinforce
cvar_t	*g_skipDialogs;
cvar_t	*g_teamKnockback;
cvar_t	*g_fixFreezeBorg1;

qboolean	stop_icarus = qfalse;

extern char *G_GetLocationForEnt( gentity_t *ent );
void G_RunFrame (int levelTime);
void CG_LoadInterface (void);
void ClearNPCGlobals( void );

void ClearPlayerAlertEvents( void );
#define	ALERT_CLEAR_TIME	200
int eventClearTime = 0;

int form_shot_traces = 0;
int form_updateseg_traces = 0;
int form_leaderseg_traces = 0;
int form_closestSP_traces = 0;
int form_clearpath_traces = 0;

extern void NPC_ShowDebugInfo (void);
extern int ffireForgivenessTimer;
extern int ffireLevel;
extern int killPlayerTimer;
extern int loadBrigTimer;
extern const int FFIRE_LEVEL_RETALIATION;
int	teamCount[TEAM_NUM_TEAMS];
int	teamLastEnemyTime[TEAM_NUM_TEAMS];
int	teamEnemyCount[TEAM_NUM_TEAMS];
/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams( void ) {
	gentity_t	*e, *e2;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i=1, e=g_entities,i ; i < globals.num_entities ; i++,e++ ){
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_entities ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				e2->teamchain = e->teamchain;
				e->teamchain = e2;
				e2->teammaster = e;
				e2->flags |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname ) {
					e->targetname = e2->targetname;
					e2->targetname = NULL;
				}
			}
		}
	}

	gi.Printf ("%i teams with %i entities\n", c, c2);
}


/*
============
G_InitCvars

============
*/
void G_InitCvars( void ) {
	// don't override the cheat state set by the system
	g_cheats = gi.cvar ("sv_cheats", "", 0);
	g_developer = gi.cvar ("developer", "", 0);

	// noset vars
	gi.cvar( "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM );
	gi.cvar( "gamedate", __DATE__ , CVAR_ROM );
	g_skippingcin = gi.cvar ("skippingCinematic", "0", CVAR_ROM);

	// latched vars

	// change anytime vars
	g_speed = gi.cvar( "g_speed", "250", 0 );
	g_gravity = gi.cvar( "g_gravity", "800", CVAR_USERINFO );	//using userinfo as savegame flag
	g_sex = gi.cvar ("sex", "male", CVAR_USERINFO | CVAR_ARCHIVE );
	g_spskill = gi.cvar ("g_spskill", "0", CVAR_ARCHIVE | CVAR_USERINFO);	//using userinfo as savegame flag
	g_knockback = gi.cvar( "g_knockback", "1000", 0 );
	g_inactivity = gi.cvar ("g_inactivity", "0", 0);
	g_debugMove = gi.cvar ("g_debugMove", "0", 0);
	g_debugDamage = gi.cvar ("g_debugDamage", "0", 0);
	g_ICARUSDebug = gi.cvar( "g_ICARUSDebug", "0", 0 );
	g_timescale = gi.cvar( "timescale", "1", 0 );
	g_virtualVoyager = gi.cvar( "cg_virtualvoyager", "0", CVAR_NORESTART );

	g_subtitles = gi.cvar ("g_subtitles", "2", CVAR_ARCHIVE);
	g_language = gi.cvar ("g_language", "", CVAR_ARCHIVE | CVAR_NORESTART);
	com_buildScript = gi.cvar ("com_buildscript", "0", 0);

	// new additions for Elite Reinforce
	g_skipDialogs = gi.cvar( "g_skipDialogs", "0", CVAR_ARCHIVE );
	g_teamKnockback = gi.cvar( "g_teamKnockback", "0", CVAR_CHEAT );
	g_fixFreezeBorg1 = gi.cvar( "g_fixFreezeBorg1", "1", CVAR_ARCHIVE );
}

/*
============
InitGame

============
*/
extern void Q3_SetPrecacheFile (const char *file);	//q3_interface

// I'm just declaring a global here which I need to get at in NAV_GenerateSquadPaths for deciding if pre-calc'd
//	data is valid, and this saves changing the proto of G_SpawnEntitiesFromString() to include a checksum param which
//	may get changed anyway if a new nav system is ever used. This way saves messing with g_local.h each time -slc
int giMapChecksum;
SavedGameJustLoaded_e g_eSavedGameJustLoaded;
qboolean g_qbLoadTransition = qfalse;
void InitGame(  const char *mapname, const char *spawntarget, int checkSum, const char *entities, int levelTime, int randomSeed, int globalTime, SavedGameJustLoaded_e eSavedGameJustLoaded, qboolean qbLoadTransition )
{
	int		i;

	giMapChecksum = checkSum;
	g_eSavedGameJustLoaded = eSavedGameJustLoaded;
	g_qbLoadTransition = qbLoadTransition;

	gi.Printf ("------- Game Initialization -------\n");
	gi.Printf ("gamename: %s\n", GAMEVERSION);
	gi.Printf ("gamedate: %s\n", __DATE__);

	srand( randomSeed );

	G_InitCvars();

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.globalTime = globalTime;
	Q_strncpyz( level.mapname, mapname, sizeof(level.mapname) );
	if ( spawntarget != NULL && spawntarget[0] )
	{
		Q_strncpyz( level.spawntarget, spawntarget, sizeof(level.spawntarget) );
	}
	else
	{
		level.spawntarget[0] = 0;
	}


	G_InitWorldSession();

	// initialize all entities for this game
	g_entities =  (struct gentity_s *) G_Alloc( MAX_GENTITIES * sizeof(g_entities[0]) );
	memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	globals.gentities = g_entities;

	// initialize all clients for this game
	level.maxclients = 1;
	level.clients = (struct gclient_s *) G_Alloc( level.maxclients * sizeof(level.clients[0]) );

	// set client fields on player
	g_entities[0].client = level.clients;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	globals.num_entities = MAX_CLIENTS;

	//Load bolt-on list
	G_LoadBoltOns();

	//Sets intial squadpoint data
	G_SquadPathsInit();

	//Set up NPC init data
	NPC_InitGame();

	TIMER_Clear();

	//
	//ICARUS INIT START

	gi.Printf("------ ICARUS Initialization ------\n");
	gi.Printf("ICARUS version : %1.2f\n", ICARUS_VERSION);

	Interface_Init( &interface_export );
	ICARUS_Init();

	gi.Printf ("-----------------------------------\n");

	//ICARUS INIT END
	//

	IT_LoadItemParms ();
	IS_LoadInfoItemParms();

	OBJ_LoadObjectives();

	OBJ_LoadTactical();

	ClearRegisteredItems();

	navCalculatePaths	= ( navigator.Load( mapname, checkSum ) == qfalse );

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString( entities );

	// general initialization
	G_FindTeams();

	SaveRegisteredItems();

	gi.Printf ("-----------------------------------\n");

	//randomize the rand functions
	byte num_calls = (byte)timeGetTime();

	for(i = 0; i < (int)num_calls; i++)
	{
		rand();
	}

	//Calculate all paths
	if ( navCalculatePaths )
	{
		NAV_CalculatePaths( mapname, checkSum );

		navigator.CalculatePaths();

		if ( navigator.Save( mapname, checkSum ) == qfalse )
		{
			gi.Printf("Unable to save navigations data for map \"%s\" (checksum:%d)\n", mapname, checkSum );
		}
	}

	//Precache the auto-built dialogue .pre file
	if (strrchr(mapname,'/')) {
		Q3_SetPrecacheFile (va("%s/behaved",strrchr(mapname,'/')));
	} else {
		Q3_SetPrecacheFile (va("%s/behaved",mapname));
	}

	//Precache the designer-made extras .pre file
	if (strrchr(mapname,'/')) {
		Q3_SetPrecacheFile (va("%s/extra",strrchr(mapname,'/')));
	} else {
		Q3_SetPrecacheFile (va("%s/extra",mapname));
	}
}


/*
=================
ShutdownGame
=================
*/
void ShutdownGame( void ) {
	gi.Printf ("==== ShutdownGame ====\n");

	gi.Printf ("... ICARUS_Shutdown\n");
	ICARUS_Shutdown ();	//Shut ICARUS down

	gi.Printf ("... Reference Tags Cleared\n");
	TAG_Init();	//Clear the reference tags

	gi.Printf ("... Navigation Data Cleared\n");
	NAV_Shutdown();

	// write all the client session data so we can get it back
	G_WriteSessionData();
}



//===================================================================

static void G_Cvar_Create( const char *var_name, const char *var_value, int flags ) {
	gi.cvar( var_name, var_value, flags );
}

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI( game_import_t *import ) {
	gameinfo_import_t	gameinfo_import;

	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;

	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;
	globals.GameAllowedToSaveHere = GameAllowedToSaveHere;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ConsoleCommand = ConsoleCommand;

	globals.gentitySize = sizeof(gentity_t);

	gameinfo_import.FS_FOpenFile = gi.FS_FOpenFile;
	gameinfo_import.FS_Read = gi.FS_Read;
	gameinfo_import.FS_FCloseFile = gi.FS_FCloseFile;
	gameinfo_import.Cvar_Set = gi.cvar_set;
	gameinfo_import.Cvar_VariableStringBuffer = gi.Cvar_VariableStringBuffer;
	gameinfo_import.Cvar_Create = G_Cvar_Create;
	GI_Init( &gameinfo_import );

	return &globals;
}

void QDECL G_Error( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	gi.Error( ERR_DROP, "%s", text);
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link

/*
-------------------------
Com_Error
-------------------------
*/

void Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.Error( level, "%s", text);
}

/*
-------------------------
Com_Printf
-------------------------
*/

void Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.Printf ("%s", text);
}

#endif

/*
========================================================================

MAP CHANGING

========================================================================
*/


/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/

void G_CheckTasksCompleted (gentity_t *ent)
{
	if ( Q3_TaskIDPending( ent, TID_CHAN_VOICE ) )
	{
		if ( !gi.S_Override[ent->s.number] || g_skipDialogs->integer )
		{//not playing a voice sound
			//return task_complete
			Q3_TaskIDComplete( ent, TID_CHAN_VOICE );
		}
	}

	if ( Q3_TaskIDPending( ent, TID_LOCATION ) )
	{
		char	*currentLoc = G_GetLocationForEnt( ent );

		if ( currentLoc && currentLoc[0] && Q_stricmp( ent->message, currentLoc ) == 0 )
		{//we're in the desired location
			Q3_TaskIDComplete( ent, TID_LOCATION );
		}
		//FIXME: else see if were in other trigger_locations?
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink (gentity_t *ent)
{
	float	thinktime;

	/*
	if ( ent->NPC == NULL )
	{
		if ( ent->taskManager && !stop_icarus )
		{
			ent->taskManager->Update( );
		}
	}
	*/

	thinktime = ent->nextthink;
	if ( thinktime <= 0 )
	{
		goto runicarus;
	}

	if ( thinktime > level.time )
	{
		goto runicarus;
	}

	ent->nextthink = 0;
	if ( ent->e_ThinkFunc == thinkF_NULL )	// actually you don't need this if I check for it in the next function -slc
	{
		//gi.Error ( "NULL ent->think");
		goto runicarus;
	}

	GEntity_ThinkFunc( ent );	// ent->think (ent);

runicarus:
	if ( ent->inuse )	// GEntity_ThinkFunc( ent ) can have freed up this ent if it was a type flier_child (stasis1 crash)
	{
		if ( ent->NPC == NULL )
		{
			if ( ent->taskManager && !stop_icarus )
			{
				ent->taskManager->Update( );
			}
		}
	}
}

/*
-------------------------
G_Animate
-------------------------
*/

void G_Animate ( gentity_t *self )
{
	if ( self->s.frame == self->endFrame )
	{
		if ( self->svFlags & SVF_ANIMATING )
		{
			if ( self->loopAnim )
			{
				self->s.frame = self->startFrame;
			}
			else
			{
				self->svFlags &= ~SVF_ANIMATING;
			}

			//Finished sequence - FIXME: only do this once even on looping anims?
			Q3_TaskIDComplete( self, TID_ANIM_BOTH );
		}
		return;
	}

	self->svFlags |= SVF_ANIMATING;

	if ( self->startFrame < self->endFrame )
	{
		if ( self->s.frame < self->startFrame || self->s.frame > self->endFrame )
		{
			self->s.frame = self->startFrame;
		}
		else
		{
			self->s.frame++;
		}
	}
	else if ( self->startFrame > self->endFrame )
	{
		if ( self->s.frame > self->startFrame || self->s.frame < self->endFrame )
		{
			self->s.frame = self->startFrame;
		}
		else
		{
			self->s.frame--;
		}
	}
	else
	{
		self->s.frame = self->endFrame;
	}
}

/*
-------------------------
G_AnimateBoltOn
-------------------------
*/

void G_AnimateBoltOn ( boltOnInfo_t *boltOn )
{
	if ( boltOn->frame == boltOn->endFrame )
	{
		if ( boltOn->loopAnim )
		{
			boltOn->frame = boltOn->startFrame;
		}
		return;
	}

	if ( boltOn->startFrame < boltOn->endFrame )
	{
		if ( boltOn->frame < boltOn->startFrame || boltOn->frame > boltOn->endFrame )
		{
			boltOn->frame = boltOn->startFrame;
		}
		else
		{
			boltOn->frame++;
		}
	}
	else if ( boltOn->startFrame > boltOn->endFrame )
	{
		if ( boltOn->frame > boltOn->startFrame || boltOn->frame < boltOn->endFrame )
		{
			boltOn->frame = boltOn->startFrame;
		}
		else
		{
			boltOn->frame--;
		}
	}
	else
	{
		boltOn->frame = boltOn->endFrame;
	}

/*
-------------------------
G_AnimateBoltOns
-------------------------
*/
}
void G_AnimateBoltOns (gentity_t *self)
{
	//Go through all my boltOns and animate them if needbe
	if ( !self->client )
	{
		if ( self->boltOn.index >= 0 && self->boltOn.index < numBoltOns )
		{
			G_AnimateBoltOn( &self->boltOn );
		}
	}
	else
	{
		for ( int i = 0; i < MAX_BOLT_ONS; i++ )
		{
			if ( self->client->renderInfo.boltOns[i].index >= 0 && self->client->renderInfo.boltOns[i].index < numBoltOns )
			{
				G_AnimateBoltOn( &self->client->renderInfo.boltOns[i] );
			}
		}
	}
}

/*
-------------------------
ResetTeamCounters
-------------------------
*/

void ResetTeamCounters( void )
{
	//clear team enemy counters
	for ( int team = TEAM_FREE; team < TEAM_NUM_TEAMS; team++ )
	{
		teamEnemyCount[team] = 0;
		teamCount[team] = 0;
	}
}

/*
-------------------------
UpdateTeamCounters
-------------------------
*/

void UpdateTeamCounters( gentity_t *ent )
{
	if ( !ent->NPC )
	{
		return;
	}
	if ( !ent->client )
	{
		return;
	}
	if ( ent->health <= 0 )
	{
		return;
	}
	if ( (ent->s.eFlags&EF_NODRAW) )
	{
		return;
	}
	if ( ent->client->playerTeam == TEAM_FREE )
	{
		return;
	}
	//this is an NPC who is alive and visible and is on a specific team

	teamCount[ent->client->playerTeam]++;
	if ( !ent->enemy )
	{
		return;
	}

	//ent has an enemy
	if ( !ent->enemy->client )
	{//enemy is a normal ent
		if ( ent->noDamageTeam == ent->client->playerTeam )
		{//it's on my team, don't count it as an enemy
			return;
		}
	}
	else
	{//enemy is another NPC/player
		if ( ent->enemy->client->playerTeam == ent->client->playerTeam)
		{//enemy is on the same team, don't count it as an enemy
			return;
		}
	}

	//ent's enemy is not on the same team
	teamLastEnemyTime[ent->client->playerTeam] = level.time;
	teamEnemyCount[ent->client->playerTeam]++;
}

extern void NPC_SetAnim(gentity_t	*ent,int type,int anim,int priority);
extern void G_MakeTeamVulnerable( void );
void G_CheckEndLevelTimers( gentity_t *ent )
{
	if ( killPlayerTimer && level.time > killPlayerTimer )
	{
		killPlayerTimer = 0;
		ent->health = 0;
		if ( ent->client && ent->client->ps.stats[STAT_HEALTH] > 0 )
		{
			//simulate death
			ent->client->ps.stats[STAT_HEALTH] = 0;
			//debounce respawn time
			ent->client->respawnTime = level.time + 2000;
			//play the "what have I done?!" anim
			NPC_SetAnim( ent, SETANIM_BOTH, BOTH_GUILT1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			/*
			NPC_SetAnim( ent, SETANIM_TORSO, BOTH_SIT2TO1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			NPC_SetAnim( ent, SETANIM_LEGS, LEGS_KNEELDOWN1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			*/
			ent->client->ps.torsoAnimTimer = -1;
			ent->client->ps.legsAnimTimer = -1;
			//look at yourself
			ent->client->ps.stats[STAT_DEAD_YAW] = ent->client->ps.viewangles[YAW]+180;
			//stop all scripts
			if (Q_stricmpn(level.mapname,"_holo",5)) {
				stop_icarus = qtrue;
			}
			//make the team killable
			G_MakeTeamVulnerable();
		}
	}
	if ( loadBrigTimer && level.time > loadBrigTimer )
	{
		gentity_t *brigent = NULL;

		loadBrigTimer = 0;

		while( (brigent = G_Find(brigent, FOFS(classname), "target_level_change" )) != NULL )
		{
			if ( Q_stricmp("_brig", brigent->message) == 0 )
			{
				break;
			}
		}

		if ( brigent )
		{
			GEntity_UseFunc( brigent, ent, ent );
		}
		else
		{//somehow it was lost, do a manual load
			gi.SendConsoleCommand( "wait;maptransition _brig\n" );
		}

	}
}
/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) {
	int			i;
	gentity_t	*ent;
	int			msec;

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;
	msec = level.time - level.previousTime;

	ResetTeamCounters();

	//remember last waypoint, clear current one
	for ( i = 0, ent = &g_entities[0]; i < globals.num_entities ; i++, ent++)
	{
		if ( ent->waypoint != WAYPOINT_NONE )
		{
			ent->lastWaypoint = ent->waypoint;
			ent->waypoint = WAYPOINT_NONE;
		}
	}

	//Look to clear out old events
	if ( eventClearTime < level.time )
	{
		ClearPlayerAlertEvents();
		eventClearTime = level.time + ALERT_CLEAR_TIME;
	}

	//Run the frame for all entities
	for ( i = 0, ent = &g_entities[0]; i < globals.num_entities ; i++, ent++)
	{
		if ( !ent->inuse )
			continue;

		// clear events that are too old
		if ( level.time - ent->eventTime > EVENT_VALID_MSEC ) {
			if ( ent->s.event ) {
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if ( ent->client ) {
					ent->client->ps.externalEvent = 0;
				}
			}
			if ( ent->freeAfterEvent ) {
				// tempEntities or dropped items completely go away after their event
				G_FreeEntity( ent );
				continue;
			} else if ( ent->unlinkAfterEvent ) {
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent = qfalse;
				gi.unlinkentity( ent );
			}
		}

		// temporary entities don't think
		if ( ent->freeAfterEvent )
			continue;

		G_CheckTasksCompleted(ent);

		G_Roff( ent );

		if( !ent->client )
		{
			if ( !(ent->svFlags & SVF_SELF_ANIMATING) )
			{//FIXME: make sure this is done only for models with frames?
				//Or just flag as animating?
				if ( ent->s.eFlags & EF_ANIM_ONCE )
				{
					ent->s.frame++;
				}
				else if ( !(ent->s.eFlags & EF_ANIM_ALLFAST) )
				{
					G_Animate( ent );
				}
			}
		}
		else
		{
			G_AnimateBoltOns( ent );
		}

		if ( ent->s.eType == ET_MISSILE )
		{
			G_RunMissile( ent );
			continue;
		}

		if ( ent->s.eType == ET_ITEM )
		{
			G_RunItem( ent );
			continue;
		}

		if ( ent->s.eType == ET_MOVER )
		{
			G_RunMover( ent );
			continue;
		}

		//The player
		if ( i == 0 )
		{
			G_CheckEndLevelTimers( ent );
			//Recalculate the nearest waypoint for the coming NPC updates
			NAV_FindPlayerWaypoint();

			if( ent->taskManager && !stop_icarus )
			{
				ent->taskManager->Update();
			}

			continue;	// players are ucmd driven
		}

		G_RunThink( ent );	// be aware that ent may be free after returning from here, at least one func frees them
		ClearNPCGlobals();			//	but these 2 funcs are ok
		UpdateTeamCounters( ent );	//	   to call anyway on a freed ent.
	}

	// perform final fixups on the player
	ent = &g_entities[0];
	if ( ent->inuse ) {
		ClientEndFrame( ent );
	}

	//DEBUG STUFF
	NAV_ShowDebugInfo();
	NPC_ShowDebugInfo();

	//handle the ffire counter
	if ( ffireLevel < FFIRE_LEVEL_RETALIATION )
	{//if we haven't reached the retaliation point, clear the counter
		if ( level.time - teamLastEnemyTime[TEAM_STARFLEET] < 10000 ||//teammates have had an enemy in the last 10 seconds
			level.time - ffireForgivenessTimer > 120000 )//Haven't shot your teammates in the past 2 minutes
		{
			//reset friendly fire counter
			ffireLevel = 0;
		}
	}
}



extern qboolean player_locked;

void G_LoadSave_WriteMiscData(void)
{
	gi.AppendToSaveGame('LCKD', &player_locked, sizeof(player_locked));
}



void G_LoadSave_ReadMiscData(void)
{
	gi.ReadFromSaveGame('LCKD', &player_locked, sizeof(player_locked), NULL);
}

