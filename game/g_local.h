#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__
// g_local.h -- local definitions for game module

// define GAME_INCLUDE so that g_public.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "../ui/gameinfo.h"
#include "g_shared.h"

//NOTENOTE: Only change this to re-point ICARUS to a new script directory
#define Q3_SCRIPT_DIR	"real_scripts"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"baseEF"

#define BODY_QUEUE_SIZE		8

#define Q3_INFINITE			16777216 

#define	FRAMETIME			100					// msec
#define	EVENT_VALID_MSEC	300
#define	CARNAGE_REWARD_TIME	3000

#define	INTERMISSION_DELAY_TIME	1000

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define	FL_DONT_SHOOT			0x00002000	// Can target him, but not shoot him
#define	FL_UNDYING				0x00004000	// Takes damage down to 1 point, but does not die

//Pointer safety utilities
#define VALID( a )		( a != NULL )
#define	VALIDATE( a )	( assert( a ) )

#define	VALIDATEV( a )	if ( a == NULL ) {	assert(0);	return;			}
#define	VALIDATEB( a )	if ( a == NULL ) {	assert(0);	return qfalse;	}
#define VALIDATEP( a )	if ( a == NULL ) {	assert(0);	return NULL;	}

#define VALIDSTRING( a )	( ( a != NULL ) && ( a[0] != NULL ) )

//Interest points

#define MAX_INTEREST_POINTS		64

typedef struct 
{
	vec3_t		origin;
	lookMode_t	lookMode;
	char		*target;
} interestPoint_t;

//Combat points

#define MAX_COMBAT_POINTS		256

typedef struct 
{
	vec3_t		origin;
	int			flags;
//	char		*NPC_targetname;
//	team_t		team;
	qboolean	occupied;
} combatPoint_t;

// Alert events

#define	MAX_ALERT_EVENTS	32

enum alertEventType_e
{
	AET_SIGHT,
	AET_SOUND,
};

enum alertEventLevel_e
{
	AEL_MINOR,			//Enemy responds to the sound, but only by looking
	AEL_SUSPICIOUS,		//Enemy looks at the sound, and will also investigate it
	AEL_DISCOVERED,		//Enemy knows the player is around, and will actively hunt
};

// !!!!!!!!! LOADSAVE-affecting struct !!!!!!!!!!
typedef struct alertEvent_s
{
	vec3_t				position;	//Where the event is located
	float				radius;		//Consideration radius
	alertEventLevel_e	level;		//Priority level of the event
	alertEventType_e	type;		//Event type (sound,sight)
	gentity_t			*owner;		//Who made the sound
} alertEvent_t;

#define MAX_FAILED_EDGES	32
typedef struct failedEdge_e
{
	int	startID;
	int	endID;
	int checkTime;
	int	entID;
} failedEdge_t;

//
// this structure is cleared as each map is entered
//

#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	2048

typedef struct 
{
	gclient_t	*clients;		// [maxclients]

	// store latched cvars here that we want to get at often
	int			maxclients;

	int			framenum;
	int			time;					// in msec
	int			previousTime;			// so movers can back up when blocked

	int			globalTime;				// global time at level initialization

	char		mapname[MAX_QPATH];		// the server name (base1, etc)

	qboolean	locationLinked;			// target_locations get linked
	gentity_t	*locationHead;			// head of the location list

	alertEvent_t	alertEvents[ MAX_ALERT_EVENTS ];
	int				numAlertEvents;

	failedEdge_t	failedEdges[MAX_FAILED_EDGES];

// =====================================
//
// NOTE!!!!!!   The only things beyond this point in the structure should be the ones you do NOT wish to be
//					affected by loading saved-games. Since loading a game first starts the map and then loads
//					over things like entities etc then these fields are usually the ones setup by the map loader.
//					If they ever get modified in-game let me know and I'll include them in the save. -Ste
//
#define LEVEL_LOCALS_T_SAVESTOP logFile	// name of whichever field is next below this in the source

	fileHandle_t	logFile;

	//Interest points- squadmates automatically look at these if standing around and close to them
	interestPoint_t	interestPoints[MAX_INTEREST_POINTS];
	int			numInterestPoints;

	//Combat points- NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point
	combatPoint_t	combatPoints[MAX_COMBAT_POINTS];
	int			numCombatPoints;
	char		spawntarget[MAX_QPATH];		// the targetname of the spawnpoint you want the player to start at

} level_locals_t;

extern	level_locals_t	level;
extern	game_export_t	globals;

extern	cvar_t	*g_gravity;
extern	cvar_t	*g_speed;
extern	cvar_t	*g_cheats;
extern	cvar_t	*g_developer;
extern	cvar_t	*g_knockback;
extern	cvar_t	*g_inactivity;
extern	cvar_t	*g_debugMove;
extern	cvar_t	*g_subtitles;
extern	cvar_t	*g_language;
extern	cvar_t	*g_removeDoors;

extern	cvar_t	*g_ICARUSDebug;

extern	cvar_t	*g_virtualVoyager;

// new additions for Elite Reinforce
extern	cvar_t	*g_teamKnockback;
extern	cvar_t	*g_fixFreezeBorg1;

//
// g_spawn.c
//
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out );
qboolean	G_SpawnVector4( const char *key, const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( const char *entities );

//
// g_cmds.c
//
void Cmd_Score_f (gentity_t *ent);

//
// g_items.c
//
void G_RunItem( gentity_t *ent );
void RespawnItem( gentity_t *ent );

void UseHoldableItem( gentity_t *ent );
void PrecacheItem (gitem_t *it);
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle, qboolean copytarget );
void SetRespawn (gentity_t *ent, float delay);
void G_SpawnItem (gentity_t *ent, gitem_t *item);
void FinishSpawningItem( gentity_t *ent );
void Think_Weapon (gentity_t *ent);
int ArmorIndex (gentity_t *ent);
void	Add_Ammo (gentity_t *ent, int weapon, int count);
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int G_ModelIndex( char *name );
int		G_SoundIndex( char *name );
void	G_TeamCommand( team_t team, char *cmd );
void	G_KillBox (gentity_t *ent);
gentity_t *G_Find (gentity_t *from, int fieldofs, char *match);
int		G_RadiusList ( vec3_t origin, float radius,	gentity_t *ignore, qboolean takeDamage, gentity_t *ent_list[MAX_GENTITIES]);
gentity_t *G_PickTarget (char *targetname);
void	G_UseTargets (gentity_t *ent, gentity_t *activator);
void	G_UseTargets2 (gentity_t *ent, gentity_t *activator, const char *string);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

void	G_InitGentity( gentity_t *e );
gentity_t	*G_Spawn (void);
gentity_t *G_TempEntity( vec3_t origin, int event );
void	G_Sound( gentity_t *ent, int soundIndex );
void	G_FreeEntity( gentity_t *e );

void	G_TouchTriggers (gentity_t *ent);
void	G_TouchTeamClients (gentity_t *ent);
void	G_TouchSolids (gentity_t *ent);

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

float vectoyaw( const vec3_t vec );

void G_AddEvent( gentity_t *ent, int event, int eventParm );
void G_SetOrigin( gentity_t *ent, vec3_t origin );
void G_SetAngles( gentity_t *ent, vec3_t angles );
void G_LanguageFilename(char *baseName,char *baseExtension,char *finalName);


//
// g_combat.c
//
qboolean CanDamage (gentity_t *targ, vec3_t origin);
void G_Damage (gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod);
void G_RadiusDamage (vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod);
void TossClientItems( gentity_t *self );
void ExplodeDeath_Wait( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void ExplodeDeath( gentity_t *self );
void GoExplodeDeath( gentity_t *self, gentity_t *other, gentity_t *activator);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_NO_HIT_LOC		0x00000010	// do not modify damage by hit loc
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_EXTRA_KNOCKBACK	0x00000040	// add extra knockback to this damage
#define DAMAGE_DEATH_KNOCKBACK	0x00000080	// only does knockback on death of target
#define DAMAGE_IGNORE_TEAM		0x00000100	// damage is always done, regardless of teams
#define DAMAGE_NO_DAMAGE		0x00000200	// do no actual damage but react as if damage was taken

//
// g_missile.c
//
void G_RunMissile( gentity_t *ent );

gentity_t *fire_blaster (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_plasma (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir);


//
// g_mover.c
//
void G_RunMover( gentity_t *ent );


//
// g_misc.c
//
#define STARFLEET_TELEPORT_FX	1
#define BORG_TELEPORT_FX		2
#define STASIS_TELEPORT_FX		3

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles, int teleport_fx );


//
// g_weapon.c
//
//void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
//void SnapVectorTowards( vec3_t v, vec3_t to );
//qboolean CheckGauntletAttack( gentity_t *ent );
void WP_LoadWeaponParms (void);

void IT_LoadItemParms( void );
void IS_LoadInfoItemParms( void );

//
// g_client.c
//
team_t PickTeam( int ignoreClientNum );
void SetClientViewAngle( gentity_t *ent, vec3_t angle );
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, team_t team, vec3_t origin, vec3_t angles );
void respawn (gentity_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
qboolean ClientSpawn( gentity_t *ent, SavedGameJustLoaded_e eSavedGameJustLoaded );
void player_die (gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void AddScore( gentity_t *ent, int score );
qboolean SpotWouldTelefrag( gentity_t *spot, team_t checkteam );

//
// g_svcmds.c
//
qboolean	ConsoleCommand( void );

//
// g_weapon.c
//
void FireWeapon( gentity_t *ent, qboolean alt_fire );

//
// p_hud.c
//
void MoveClientToIntermission (gentity_t *client);
void G_SetStats (gentity_t *ent);
void DeathmatchScoreboardMessage (gentity_t *client);

//
// g_cmds.c
//
static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message );

//
// g_pweapon.c
//


//
// g_main.c
//
void G_RunThink (gentity_t *ent);
void QDECL G_Error( const char *fmt, ... );

//
// g_nav.cpp
//
void Svcmd_Nav_f (void);

//
// g_squad.cpp
//
void Svcmd_Comm_f (void);
void Svcmd_Hail_f (void);
void Svcmd_Form_f (void);


//
// g_utils.cpp
//
void Svcmd_Use_f (void);

//
// g_weapons.cpp
//
void UseCharge (int entityNum);

//
// g_client.c
//
char *ClientConnect( int clientNum, qboolean firstTime, SavedGameJustLoaded_e eSavedGameJustLoaded );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum, usercmd_t *cmd, SavedGameJustLoaded_e eSavedGameJustLoaded );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum, usercmd_t *cmd );
void ClientEndFrame (gentity_t *ent);

//
// g_team.c
//
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 );


//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( gclient_t *client );
void G_InitSessionData( gclient_t *client, char *userinfo );

void G_InitWorldSession( void );
void G_WriteSessionData( void );


//
// g_objectives.c
//

void OBJ_LoadObjectives (void);
void OBJ_LoadTactical (void);

//============================================================================


//ICARUS includes

extern	interface_export_t	interface_export;

extern	void Interface_Init( interface_export_t *pe );
extern	int ICARUS_RunScript( gentity_t *ent, const char *name );
extern	bool ICARUS_RegisterScript( const char *name, bool bCalledDuringInterrogate = false);

//Tags

// Reference tags

#define MAX_REFTAGS	128		// Probably more than we'll ever need
#define MAX_REFNAME	32

#define	RTF_NONE	0
#define	RTF_NAVGOAL	0x00000001

typedef struct reference_tag_s
{
	char	name[MAX_REFNAME];
	vec3_t	origin;
	vec3_t	angles;
	int		flags;	//Just in case
	int		radius;	//For nav goals
} reference_tag_t;

extern	void TAG_Init( void );
extern	reference_tag_t	*TAG_Add( const char *name, const char *owner, vec3_t origin, vec3_t angles, int radius, int flags );

extern	int	TAG_GetOrigin( const char *owner, const char *name, vec3_t origin );
extern	int	TAG_GetAngles( const char *owner, const char *name, vec3_t angles );
extern	int TAG_GetRadius( const char *owner, const char *name );
extern	int TAG_GetFlags( const char *owner, const char *name );

void TAG_ShowTags( int flags );

// Reference tags END

typedef struct pscript_s
{
	char	*buffer;
	long	length;
} pscript_t;

typedef	map < string, int, less<string>, allocator<int> >		entlist_t;
typedef map < string, pscript_t*, less<string>, allocator<pscript_t*> >	bufferlist_t;

extern ICARUS_Instance	*iICARUS;
extern bufferlist_t		ICARUS_BufferList;
extern entlist_t		ICARUS_EntList;

//
//	g_ICARUS.cpp
//
void ICARUS_Init( void );
bool ICARUS_ValidEnt( gentity_t *ent );
void ICARUS_InitEnt( gentity_t *ent );
void ICARUS_FreeEnt( gentity_t *ent );
void ICARUS_AssociateEnt( gentity_t *ent );
void ICARUS_Shutdown( void );
void Svcmd_ICARUS_f( void );

extern int		ICARUS_entFilter;

extern char *G_NewString( const char *string );

// some stuff for savegames...
//
void WriteLevel(qboolean qbAutosave);
void ReadLevel(qboolean qbAutosave, qboolean qbLoadTransition);
qboolean GameAllowedToSaveHere(void);

extern void G_ActivateBehavior( gentity_t *ent, int bset );

//Timing information
void TIMER_Clear( void );
void TIMER_Save( void );
void TIMER_Load( void );
void TIMER_Set( gentity_t *ent, const char *identifier, int duration );
int	TIMER_Get( gentity_t *ent, const char *identifier );
qboolean TIMER_Done( gentity_t *ent, const char *identifier );

float NPC_GetHFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float hFOV );
float NPC_GetVFOVPercentage( vec3_t spot, vec3_t from, vec3_t facing, float vFOV );

#endif//#ifndef __G_LOCAL_H__
