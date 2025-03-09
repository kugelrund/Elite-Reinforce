//B_local.h
//re-added by MCG
#ifndef __B_LOCAL_H__
#define __B_LOCAL_H__

#include "g_local.h"
#include "say.h"

#include "AI.h"

//
// Navigation susbsystem
//

#define NAVF_DUCK			0x00000001
#define NAVF_JUMP			0x00000002
#define NAVF_HOLD			0x00000004
#define NAVF_SLOW			0x00000008

#define DEBUG_LEVEL_DETAIL	4
#define DEBUG_LEVEL_INFO	3
#define DEBUG_LEVEL_WARNING	2
#define DEBUG_LEVEL_ERROR	1
#define DEBUG_LEVEL_NONE	0

#define MAX_GOAL_REACHED_DIST_SQUARED	256//16 squared
#define MIN_ANGLE_ERROR 0.01f

//
// NPC.cpp
//
// ai debug cvars
void SetNPCGlobals( gentity_t *ent );
void SaveNPCGlobals();
void RestoreNPCGlobals();
extern cvar_t		*debugNPCAI;			// used to print out debug info about the NPC AI
extern cvar_t		*debugNPCFreeze;		// set to disable NPC ai and temporarily freeze them in place
extern cvar_t		*debugNPCAimingBeam;
extern cvar_t		*debugBreak;
extern cvar_t		*debugNoRoam;
extern cvar_t		*debugNPCName;
void NPC_Think ( gentity_t *self);

//NPC_reactions.cpp
void NPC_Pain( gentity_t *self, gentity_t *other, int damage );
void NPC_Touch( gentity_t *self, gentity_t *other, trace_t *trace );
void NPC_Use( gentity_t *self, gentity_t *other, gentity_t *activator );

//
// NPC_misc.cpp
//
void Debug_Printf( cvar_t *cv, int level, char *fmt, ... );
void Debug_NPCPrintf( gentity_t *printNPC, cvar_t *cv, int debugLevel, char *fmt, ... );

//MCG - Begin============================================================
//NPC_ai variables - shared by NPC.cpp andf the following modules
extern gentity_t	*NPC;
extern gNPC_t		*NPCInfo;
extern gclient_t	*client;
extern usercmd_t	ucmd;
extern qboolean		blocked;
extern visibility_t	enemyVisibility;

//NPC_behavior
qboolean NPC_StandTrackAndShoot (gentity_t *NPC);
void NPC_BSIdle( void );
void NPC_BSRoam (void);
void NPC_BSWalk (void);
void NPC_BSCrouch (void);
void NPC_BSRun (void);
void NPC_BSStandGuard (void);
void NPC_BSPatrol (void);
void NPC_BSHuntAndKill (void);
void NPC_BSStandAndShoot (void);
void NPC_BSEvade (void);
void NPC_BSEvadeAndShoot (void);
void NPC_BSRunAndShoot (void);
void NPC_BSDefend (void);
void NPC_BSCombat (void);
void NPC_BSGetAmmo (void);
void NPC_BSAdvanceFight (void);
void NPC_BSTakeCover (void);
void NPC_ClearPatient (void);
void NPC_BSMedic (void);
void NPC_BSMedicHide (void);
void NPC_BSMedicCombat (void);
void NPC_BSWait( void );
void NPC_BSFace (void);
void NPC_BSMove(void);
void NPC_BSWaitHeal(void);
void NPC_BSShoot(void);
void NPC_BSSniper (void);
void NPC_TakePatient (gentity_t *patient);
void NPC_BSPointShoot(qboolean shoot);
void NPC_BSInvestigate (void);
void NPC_BSSleep( void );
void NPC_BSSay (void);
void NPC_BSFlee (void);
void NPC_BSRetreat (void);
void NPC_BSCover (void);
void NPC_BSMelee (void);
qboolean NPC_BSPointCombat (void);
void NPC_BSFollowLeader (void);
qboolean isMedic (gentity_t *self);
void NPC_BSJump (void);
void NPC_BSRemove (void);
void NPC_BSSearch (void);
void NPC_BSSearchStart (int	homeWp, bState_t bState);
void NPC_BSFly ( void );
void NPC_BSWander (void);
void NPC_BSFaceLeader (void);

//NPC_combat
int ChooseBestWeapon( void );
void NPC_ChangeWeapon( int newWeapon );
void ShootThink( void );
void WeaponThink( qboolean inCombat );
qboolean HaveWeapon( int weapon );
qboolean CanShoot ( gentity_t *ent, gentity_t *shooter );
void NPC_CheckPossibleEnemy( gentity_t *other, visibility_t vis );
gentity_t *NPC_PickEnemy (gentity_t *closestTo, int enemyTeam, qboolean checkVis, qboolean findPlayersFirst, qboolean findClosest);
void NPC_CheckEnemy (qboolean findNew, qboolean tooFarOk);
qboolean NPC_CheckAttack (float scale);
qboolean NPC_CheckDefend (float scale);
int NPC_AttackDebounceForWeapon (void);
qboolean EntIsGlass (gentity_t *check);
qboolean ShotThroughGlass (trace_t *tr, gentity_t *target, vec3_t spot, int mask);
qboolean ValidEnemy (gentity_t *ent);
void G_ClearEnemy (gentity_t *self);
void G_SetEnemy (gentity_t *self, gentity_t *enemy);
gentity_t *NPC_PickAlly ( qboolean facingEachOther, float range, qboolean ignoreGroup, qboolean movingOnly );
void NPC_LostEnemyDecideChase(void);

//NPC_formation
qboolean NPC_SlideMoveToGoal (void);
float NPC_FindClosestTeammate (gentity_t *self);
void NPC_CalcClosestFormationSpot(gentity_t *self);
void G_MaintainFormations (gentity_t *self);
void NPC_BSFormation (void);
void NPC_CreateFormation (gentity_t *self);
void NPC_DropFormation (gentity_t *self);
void NPC_ReorderFormation (gentity_t *self);
void NPC_InsertIntoFormation (gentity_t *self);
void NPC_DeleteFromFormation (gentity_t *self);

#define COLLISION_RADIUS 32
#define NUM_POSITIONS 30

//NPC spawnflags
#define SFB_SMALLHULL	1

#define SFB_RIFLEMAN	2
#define SFB_OLDBORG		2//Borg
#define SFB_PHASER		4
#define SFB_GUN			4//Borg
#define	SFB_TRICORDER	8
#define	SFB_TASER		8//Borg
#define	SFB_DRILL		16//Borg

#define	SFB_SILENTSPAWN	32
#define	SFB_NOTSOLID	64
#define	SFB_STARTINSOLID 128

//NPC_goal
void SetGoal( gentity_t *goal, float rating );
void NPC_SetGoal( gentity_t *goal, float rating );
void NPC_ClearGoal( void );
qboolean ReachedGoal( gentity_t *goal );
gentity_t *UpdateGoal( void );


//void NPC_StuckCheck( void );
//void NPC_StuckHousekeeping( void );
//void NPC_Predict( void );
//void NPC_StuckCheck( void );
//void NPC_StuckHousekeeping( void );
qboolean NPC_ClearPathToGoal(vec3_t dir, gentity_t *goal);
qboolean NPC_MoveToGoal( void );

//NPC_reactions
//NPC_senses
#define CHECK_PVS		1
#define CHECK_360		2
#define CHECK_FOV		4
#define CHECK_SHOOT		8
#define CHECK_VISRANGE	16
qboolean CanSee ( gentity_t *ent );

qboolean InFOV ( gentity_t *ent, gentity_t *from, int hFOV, int vFOV );
qboolean InFOV( vec3_t origin, gentity_t *from, int hFOV, int vFOV );
qboolean InFOV( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );

visibility_t NPC_CheckVisibility ( gentity_t *ent, int flags );
qboolean InVisrange ( gentity_t *ent );

//NPC_sounds
void NPC_AngerSound(void);


//NPC_spawn
void NPC_Spawn( gentity_t *self );

//NPC_stats
bool NPC_ShouldShowPath( gentity_t* self );
int NPC_ReactionTime ( void );
qboolean NPC_ParseParms( const char *NPCName, gentity_t *NPC );
void NPC_LoadParms( void );

//NPC_utils
extern int	teamNumbers[TEAM_NUM_TEAMS];
extern int	teamStrength[TEAM_NUM_TEAMS];
extern int	teamCounter[TEAM_NUM_TEAMS];
void CalcEntitySpot ( gentity_t *ent, spot_t spot, vec3_t point );
qboolean NPC_UpdateAngles ( qboolean doPitch, qboolean doYaw );
void NPC_UpdateShootAngles (vec3_t angles, qboolean doPitch, qboolean doYaw );
qboolean NPC_UpdateFiringAngles ( qboolean doPitch, qboolean doYaw );
void SetTeamNumbers (void);
void G_ActivateBehavior (gentity_t *self, int bset );

//g_nav.cpp
int NAV_FindClosestWaypointForEnt (gentity_t *ent, int targWp);
qboolean NAV_CheckAhead( gentity_t *self, vec3_t end, trace_t &trace, int clipmask );

//other modules
extern void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint, float lead_in );

//g_combat
void ExplodeDeath( gentity_t *self );
void ExplodeDeath_Wait( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
void GoExplodeDeath( gentity_t *self, gentity_t *other, gentity_t *activator);
float IdealDistance ( gentity_t *self );

//g_client
qboolean SpotWouldTelefrag( gentity_t *spot, team_t checkteam );

//g_squad
void NPC_SetSayState (gentity_t *self, gentity_t *to, int saying);

//g_utils
extern qboolean G_CheckInSolid (gentity_t *self, qboolean fix);
extern qboolean infront(gentity_t *from, gentity_t *to);

//MCG - End============================================================

// NPC.cpp
void NPC_SetAnim(gentity_t	*ent,int type,int anim,int priority);

qboolean NPC_EnemyTooFar(gentity_t *enemy, float dist, qboolean toShoot);

// ==================================================================

qboolean NPC_ClearLOS( vec3_t start, vec3_t end );
qboolean NPC_ClearLOS( vec3_t start, gentity_t *ent );
qboolean NPC_ClearLOS( gentity_t *ent, vec3_t end );
qboolean NPC_ClearLOS( gentity_t *ent );
qboolean NPC_ClearLOS( vec3_t end );

qboolean NPC_ClearShot( gentity_t *ent );

int NPC_FindCombatPoint( vec3_t position, vec3_t avoidPosition, vec3_t enemyPosition, int flags );
int NPC_FindCombatPoint( int flags );

qboolean NPC_ReserveCombatPoint( int combatPointID );
qboolean NPC_FreeCombatPoint( int combatPointID );
qboolean NPC_SetCombatPoint( int combatPointID );

#define	CP_ANY			0			//No flags
#define	CP_COVER		0x00000001	//The enemy cannot currently shoot this position
#define CP_CLEAR		0x00000002	//This cover point has a clear shot to the enemy
#define CP_FLEE			0x00000004	//This cover point is marked as a flee point
#define CP_DUCK			0x00000008	//This cover point is marked as a duck point
#define CP_NEAREST		0x00000010	//Find the nearest combat point
#define CP_AVOID_ENEMY	0x00000020	//Avoid our enemy
#define CP_INVESTIGATE	0x00000040	//A special point worth enemy investigation if searching
#define	CP_SQUAD		0x00000080	//Squad path
#define	CP_AVOID		0x00000100	//Avoid supplied position

#define CPF_NONE		0
#define	CPF_DUCK		0x00000001
#define	CPF_FLEE		0x00000002
#define	CPF_INVESTIGATE	0x00000004
#define	CPF_SQUAD		0x00000008
#define	CPF_LEAN		0x00000010

#define	MAX_COMBAT_POINT_CHECK	32

int NPC_ValidEnemy( gentity_t *ent );
int NPC_CheckEnemyExt( void );
qboolean NPC_FindPlayer( void );
qboolean NPC_CheckCanAttackExt( void );

int NPC_CheckSoundEvents( void );
int NPC_CheckSightEvents( void );
int NPC_CheckAlertEvents( void );
void G_AlertTeam( gentity_t *victim, gentity_t *attacker, float radius, float soundDist );

void AddSightEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );
void AddSoundEvent( gentity_t *owner, vec3_t position, float radius, alertEventLevel_e alertLevel );

int NPC_FindSquadPoint( vec3_t position );
//int NPC_FindSquadPoint( vec3_t position, vec3_t dirToPlayer, float distToPlayer, vec3_t playerDir );

void ClearPlayerAlertEvents( void );

qboolean G_BoundsOverlap(const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
qboolean NAV_HitNavGoal( vec3_t point, vec3_t mins, vec3_t maxs, vec3_t dest, int radius );

void NPC_SetMoveGoal( gentity_t *ent, vec3_t point, int radius, bool isNavGoal = qfalse );

extern qboolean NAV_ClearPathToPoint(gentity_t *self, vec3_t pmins, vec3_t pmaxs, vec3_t point, int clipmask);
extern void NPC_ApplyWeaponFireDelay(void);

//NPC_FaceXXX suite
qboolean NPC_FacePosition( vec3_t position, qboolean doPitch = qtrue );
qboolean NPC_FaceEntity( gentity_t *ent, qboolean doPitch = qtrue );
qboolean NPC_FaceEnemy( qboolean doPitch = qtrue );

extern BYTE G_AddBoltOn( gentity_t *NPC, const char *boltOnName );

//Skill level cvar
extern cvar_t	*g_spskill;

#define	NIF_NONE		0x00000000
#define	NIF_FAILED		0x00000001	//failed to find a way to the goal
#define	NIF_MACRO_NAV	0x00000002	//using macro navigation
#define	NIF_COLLISION	0x00000004	//resolving collision with an entity
#define NIF_BLOCKED		0x00000008	//blocked from moving

/*
-------------------------
struct navInfo_s
-------------------------
*/

typedef struct navInfo_s
{
	gentity_t	*blocker;
	vec3_t		direction;
	vec3_t		pathDirection;
	float		distance;
	trace_t		trace;
	int			flags;
} navInfo_t;

int	NAV_MoveToGoal( gentity_t *self, navInfo_t &info );
void NAV_GetLastMove( navInfo_t &info );
qboolean NAV_AvoidCollision( gentity_t *self, gentity_t *goal, navInfo_t &info );

#endif
