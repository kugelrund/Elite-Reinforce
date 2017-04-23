// ICARUS Engine Interface File
//
//	This file is the only section of the ICARUS systems that 
//	is not directly portable from engine to engine.
//
//	-- jweier

#include "g_local.h"
#include "g_functions.h"
#include "Q3_Interface.h"
#include "Q3_Registers.h"
#include "anims.h"
#include "boltOns.h"
#include "b_local.h"
#include "events.h"
#include "g_nav.h"
#include "..\cgame\cg_camera.h"
#include "..\cgame\cg_text.h"
#include "..\game\objectives.h"
#include "g_roff.h"
#include "..\cgame\cg_local.h"
#include "..\game\speakers.h"

extern int ICARUS_LinkEntity( int entID, CSequencer *sequencer, CTaskManager *taskManager );

extern void InitMover( gentity_t *ent );
extern void MatchTeam( gentity_t *teamLeader, int moverState, int time );
//extern void SetMoverState( gentity_t *ent, moverState_t moverState, int time );
extern const char *GetStringForID( stringID_table_t *table, int id );
extern void ChangeWeapon( gentity_t *ent, int newWeapon );
extern char *G_GetLocationForEnt( gentity_t *ent );
extern void NPC_BSSearchStart( int homeWp, bState_t bState );
extern void InitMoverTrData( gentity_t *ent );
extern qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest );
extern cvar_t *g_sex;
extern cvar_t *g_timescale;
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
extern void FX_BorgTeleport( vec3_t org );
static void Q3_SetWeapon (int entID, const char *wp_name);
extern void CG_ChangeWeapon( int num );
extern int	TAG_GetOrigin2( const char *owner, const char *name, vec3_t origin );
extern void G_TeamRetaliation ( gentity_t *targ, gentity_t *attacker, qboolean stopIcarus );
extern void G_PlayDoorLoopSound( gentity_t *ent );
extern void G_PlayDoorSound( gentity_t *ent, int type );
extern qboolean G_WeaponChangeEffect ( gentity_t *ent );
extern void NPC_SetLookTarget( gentity_t *self, int entNum, int clearTime );
extern void NPC_ClearLookTarget( gentity_t *self );
qboolean PM_HasAnimation( gentity_t *ent, int animation );
extern int	BMS_START;
extern int	BMS_MID;
extern int	BMS_END;
extern cvar_t	*g_skippingcin;


extern int	ffireLevel;
extern const int FFIRE_LEVEL_RETALIATION;
extern qboolean	stop_icarus;
void G_MakeTeamVulnerable( void );

#define stringIDExpand(str, strEnum)	str, strEnum, ENUM2STRING(strEnum)
//#define stringIDExpand(str, strEnum)	str,strEnum

/*
stringID_table_t tagsTable [] = 
{
}

stringID_table_t boltOnsTable [] = 
{
}
*/


//FIXME: remove!  For backwards compatibility with old scripts only!
stringID_table_t animTableOld [] = 
{
	//Both
	"death1",		BOTH_DEATH1,
	"dead1",		BOTH_DEAD1,
	"death2",		BOTH_DEATH2,
	"dead2",		BOTH_DEAD2,
	"death3",		BOTH_DEATH3,
	"dead3",		BOTH_DEAD3,
	"death4",		BOTH_DEATH4,
	"dead4",		BOTH_DEAD4,
	"death5",		BOTH_DEATH5,
	"dead5",		BOTH_DEAD5,
	
	//Torso
	"gesture1",		BOTH_GESTURE1,
	"gesture",		BOTH_GESTURE1,
	"torso_idle",	BOTH_STAND1,
	"torso_stand",	BOTH_STAND1,
	"torso_stand2",	BOTH_STAND2,
	"ready",		BOTH_STAND2,
	"drop",			TORSO_DROPWEAP1,
	"raise",		TORSO_RAISEWEAP1,
	"attack",		BOTH_ATTACK1,
	"torso_attack",	BOTH_ATTACK1,
	"torso_attack1",BOTH_ATTACK1,
	"torso_attack2",BOTH_ATTACK2,

	//Legs
	"legs_idle",	BOTH_STAND1,
	"legs_idlecr",	BOTH_CROUCH1IDLE,
	"legs_walkcr",	BOTH_CROUCH1WALK,
	"walk",			BOTH_WALK1,
	"legs_walk",	BOTH_WALK1,
	"run",			BOTH_RUN1,
	"legs_run",		BOTH_RUN1,
	"back",			LEGS_RUNBACK1,
	"legs_back",	LEGS_RUNBACK1,
	"legs_jump",	BOTH_JUMP1,
	"legs_land",	BOTH_LAND1,
	"jumpb",		BOTH_JUMPBACK1,
	"landb",		BOTH_LANDBACK1,
	"turn",			LEGS_TURN1,
	"swim",			BOTH_SWIM1,

//crewanims
	"BOTH_PAIN1",BOTH_PAIN1,
	"BOTH_PAIN2",BOTH_PAIN2,
	"BOTH_PAIN3",BOTH_PAIN3,

	"BOTH_DEATHFORWARD",BOTH_DEATHFORWARD1,
	"BOTH_DEADFORWARD",BOTH_DEADFORWARD1,
	
	"BOTH_DEATHBACKWARD",BOTH_DEATHBACKWARD1,
	"BOTH_DEADBACKWARD",BOTH_DEADBACKWARD1,
	
	"BOTH_DEATHFORWARD2",BOTH_DEATHFORWARD2,
	"BOTH_DEADFORWARD2",BOTH_DEADFORWARD2,
	
	"BOTH_DEATHBACK2",BOTH_DEATHBACKWARD2,
	"BOTH_DEADBACK2",BOTH_DEADBACKWARD2,
	
	"BOTH_WRITHINGDEATH",BOTH_LYINGDEATH1,
	"BOTH_WRITHINGDEAD",BOTH_LYINGDEAD1,

	"BOTH_STUMBLEDEATH",BOTH_STUMBLEDEATH1,
	"BOTH_STUMBLEDEAD",BOTH_STUMBLEDEAD1,
	
	"BOTH_FALLDEATH",BOTH_FALLDEATH1,
	"BOTH_FALLDEAD",BOTH_FALLDEAD1LAND,
	"BOTH_SITTING1",BOTH_SIT3TO1,			//# First sitting anim
	"BOTH_SITTING2",BOTH_SIT1TO2,			//# Second sitting anim
	"BOTH_SITTING3",BOTH_SIT2TO3,			//# Third sitting anim

	"TORSO_RUN",BOTH_RUN1,
	"TORSO_CONSOLE",BOTH_CONSOLE1,
	"TORSO_CONSOLE2",BOTH_CONSOLE2,
	"TORSO_CROUCH",BOTH_CROUCH1,
	"TORSO_WALKCR",BOTH_CROUCH1WALK,
	"TORSO_IDLECR",BOTH_CROUCH1IDLE,
	"TORSO_WALK",BOTH_WALK1,
	"TORSO_PAIN",BOTH_PAIN1,
	"TORSO_PLUGIN",BOTH_PLUGIN1,
	"TORSO_PLUGOUT",BOTH_PLUGOUT1,
	"TORSO_JUMP",BOTH_JUMP1,
	"TORSO_INAIR",BOTH_INAIR1,
	"TORSO_LAND",BOTH_LAND1,
	"TORSO_SITTING",BOTH_SIT3TO1,
	"TORSO_WREADY1",TORSO_WEAPONREADY1,
	"TORSO_WREADY2",TORSO_WEAPONREADY2,
	"TORSO_TRICORDER",TORSO_TRICORDER1,
	"TORSO_MEDICORDER",TORSO_MEDICORDER1,
	"TORSO_INJURED",BOTH_INJURED1,
	"TORSO_WRITHING",BOTH_WRITHING1,
	"TORSO_CRAWLBACK",BOTH_CRAWLBACK1,
	"TORSO_PAIN2WRITHE",BOTH_PAIN2WRITHE1,
	"TORSO_CONSOLE1IDLE",BOTH_CONSOLE1IDLE,
	"TORSO_CONSOLE1RIGHT",BOTH_CONSOLE1RIGHT,
	"TORSO_CONSOLE1LEFT",BOTH_CONSOLE1LEFT,
	"TORSO_SITTING1",BOTH_SIT3TO1,
	"TORSO_SITTING2",BOTH_SIT1TO2,
	"TORSO_SITTING3",BOTH_SIT2TO3,
	"TORSO_GUARD_LOOKAROUND", BOTH_GUARD_LOOKAROUND1,
	"TORSO_GUARD_IDLE", BOTH_GUARD_IDLE1,
	"TORSO_WIDLE",TORSO_WEAPONIDLE1,
	"TORSO_ACTIVATEBELT1",TORSO_RAISEHELMET1,

	"LEGS_ATTACK",BOTH_ATTACK1,
	"LEGS_ATTACK2",BOTH_ATTACK2,
	"LEGS_CONSOLE",BOTH_CONSOLE1,
	"LEGS_CONSOLE2",BOTH_CONSOLE2,
	"LEGS_CROUCH",BOTH_CROUCH1,
	"LEGS_PAIN",BOTH_PAIN1,
	"LEGS_PLUGIN",BOTH_PLUGIN1,
	"LEGS_PLUGOUT",BOTH_PLUGOUT1,
	"LEGS_INAIR",BOTH_INAIR1,
	"LEGS_SITTING",BOTH_SIT3TO1,
	"LEGS_TRICORDER",BOTH_STAND1,
	"LEGS_MEDICORDER",BOTH_STAND1,
	"LEGS_INJURED",BOTH_INJURED1,
	"LEGS_WRITHING",BOTH_WRITHING1,
	"LEGS_CRAWLBACK",BOTH_CRAWLBACK1,
	"LEGS_PAIN2WRITHE",BOTH_PAIN2WRITHE1,
	"LEGS_CONSOLE1IDLE",BOTH_CONSOLE1IDLE,
	"LEGS_CONSOLE1RIGHT",BOTH_CONSOLE1RIGHT,
	"LEGS_CONSOLE1LEFT",BOTH_CONSOLE1LEFT,
	"LEGS_SITTING1",BOTH_SIT3TO1,
	"LEGS_SITTING2",BOTH_SIT1TO2,
	"LEGS_SITTING3",BOTH_SIT2TO3,
	"LEGS_SITPOSE",BOTH_SIT3TO1,
	"LEGS_GUARD_LOOKAROUND", BOTH_GUARD_LOOKAROUND1,
	"LEGS_GUARD_IDLE", BOTH_GUARD_IDLE1,

	//END
	"",	NULL,
};

stringID_table_t BSTable[] =
{
	stringIDExpand("default",		BS_DEFAULT),		//# 0: whatever
	stringIDExpand("idle",			BS_IDLE),			//# 1: Stand around, do abolutely nothing
	stringIDExpand("roam",			BS_ROAM),			//# 2: Roam around, collect stuff
	stringIDExpand("walk",			BS_WALK),			//# 3: Walk toward their goals
	stringIDExpand("run",			BS_RUN),			//# 4: Run toward their goals
	stringIDExpand("standshoot",	BS_STAND_AND_SHOOT),//# 5: Stay in one spot and shoot- duck when neccesary
	stringIDExpand("standguard",	BS_STAND_GUARD),	//# 6: Wait around for an enemy
	stringIDExpand("patrol",		BS_PATROL),			//# 7: Follow a path, looking for enemies
	stringIDExpand("huntkill",		BS_HUNT_AND_KILL),	//# 8: Track down enemies and kill them
	stringIDExpand("evade",			BS_EVADE),			//# 9: Run from enemies
	stringIDExpand("evadeshoot",	BS_EVADE_AND_SHOOT),//# 10: Run from enemies, shoot them if they hit you
	stringIDExpand("runshoot",		BS_RUN_AND_SHOOT),	//# 11: Run to your goal and shoot enemy when possible
	stringIDExpand("defend",		BS_DEFEND),			//# 12: Defend an entity or spot?
	stringIDExpand("combat",		BS_COMBAT),			//# 14: Attack, evade, use cover, move about, etc.  Full combat AI - id BOTH_ code
	stringIDExpand("medic",			BS_MEDIC),			//# 15: Go to each person and heal them fully, return task complete when done
	stringIDExpand("takecover",		BS_TAKECOVER),		//# 16: Find nearest cover from enemies
	stringIDExpand("getammo",		BS_GET_AMMO),		//# 17: Go get some ammo
	stringIDExpand("advancefight",	BS_ADVANCE_FIGHT),	//# 18: Go somewhere and fight along the way
	stringIDExpand("face",			BS_FACE),			//# 19: turn until facing desired angles
	stringIDExpand("wait",			BS_WAIT),			//# 20: do nothing
	stringIDExpand("formation",		BS_FORMATION),		//# 21: Maintain a formation
	stringIDExpand("crouch",		BS_CROUCH),			//# 22: Crouch-Walk toward their goals
	stringIDExpand("move",			BS_MOVE),			//# 23: Move in one dir, face another
	stringIDExpand("waitheal",		BS_WAITHEAL),		//# 24: Do nothing until health > 75
	stringIDExpand("shoot",			BS_SHOOT),			//# 25: Just fire straight ahead
	stringIDExpand("sniper",		BS_SNIPER),			//# 26: Wait for exact hit and fire at anyone exposed.
	stringIDExpand("mediccombat",	BS_MEDIC_COMBAT),	//# 27: Run to and heal people to a certain level then go back to hiding spot
	stringIDExpand("medichide",		BS_MEDIC_HIDE),		//# 28: Stay in hiding spot and wait for others to come to you
	stringIDExpand("pointshoot",	BS_POINT_AND_SHOOT),//# 29: turn until facing enemy and fire
	stringIDExpand("faceenemy",		BS_FACE_ENEMY),		//# 30: turn until facing enemy
	stringIDExpand("investigate",	BS_INVESTIGATE),	//# 31: Walk somewhere and look around, not to be used by scripts, really
	stringIDExpand("sleep",			BS_SLEEP),			//# 32: Asleep, will only play awake script when startled
	//stringIDExpand("flee",			BS_FLEE),			//# 33: Run away!
	//stringIDExpand("retreat",		BS_RETREAT),		//# 34: Back away while still engaging enemy
	//stringIDExpand("cover",			BS_COVER),			//# 35: Watch your coverTarg and shoot any enemy around him, laying down supressing fire
	stringIDExpand("say",			BS_SAY),			//# 36: Turn head to sayTarg, use talk anim, say your sayString (look up string in your sounds table), exit tempBState when sound finished (anim of mouth should be timed to length of sound as well)
	stringIDExpand("aim",			BS_AIM),			//# 37: Turn head and torso to facing, keep feet in place if you can
	stringIDExpand("look",			BS_LOOK),			//# 38: Turn head only to facing, keep torso and head in place if you can
	stringIDExpand("pointcombat",	BS_POINT_COMBAT),	//# 39: Head toward closest empty point_combat and shoot from there
	stringIDExpand("followleader",	BS_FOLLOW_LEADER),	//# 40: Follow your leader and shoot any enemies you come across
	stringIDExpand("jump",			BS_JUMP),			//# 41: Face navgoal and jump to it
	stringIDExpand("remove",		BS_REMOVE),			//# 42: Waits for player to leave PVS then removes itself
	stringIDExpand("search",		BS_SEARCH),			//# 43: Using current waypoint as a base, search the immediate branches of waypoints for enemies
	stringIDExpand("fly",			BS_FLY),			//# 44: Moves around without gravity
	stringIDExpand("noclip",		BS_NOCLIP),			//# 45: Moves through walls, etc.
	stringIDExpand("wander",		BS_WANDER),			//# 46: Wander down random waypoint paths
	stringIDExpand("faceleader",	BS_FACE_LEADER),	//#	47: They will stand still and keep facing whatever ent is set by SET_LEADER until their bState is changed
	"",				-1,
};

stringID_table_t BSETTable[] =
{
	stringIDExpand("spawnscript", BSET_SPAWN),//# script to use when first spawned
	stringIDExpand("idlescript", BSET_IDLE),//# script to use when standing around
	stringIDExpand("touchscript", BSET_TOUCH),//# script to use when touched
	stringIDExpand("usescript", BSET_USE),//# script to use when used
	stringIDExpand("awakescript", BSET_AWAKE),//# script to use when awoken/startled
	stringIDExpand("angerscript", BSET_ANGER),//# script to use when aquire an enemy
	stringIDExpand("attackscript", BSET_ATTACK),//# script to run when you attack
	stringIDExpand("victoryscript", BSET_VICTORY),//# script to run when you kill someone
	stringIDExpand("lostenemyscript", BSET_LOSTENEMY),//# script to run when you can't find your enemy
	stringIDExpand("painscript", BSET_PAIN),//# script to use when take pain
	stringIDExpand("fleescript", BSET_FLEE),//# script to use when take pain below 50% of health
	stringIDExpand("deathscript", BSET_DEATH),//# script to use when killed
	stringIDExpand("delayedscript", BSET_DELAYED),//# script to run when self->delayScriptTime is reached
	stringIDExpand("blockedscript", BSET_BLOCKED),//# script to run when blocked by a friendly NPC or player
	stringIDExpand("bumpedscript", BSET_BUMPED),//# script to run when bumped into a friendly NPC or player (can set bumpRadius)
	stringIDExpand("stuckscript", BSET_STUCK),//# script to run when blocked by a wall
	stringIDExpand("ffirescript", BSET_FFIRE),//# script to run when player shoots their own teammates
	stringIDExpand("ffdeathscript", BSET_FFDEATH),//# script to run when player kills a teammate
	stringIDExpand("", BSET_INVALID),
	"",				-1,
};

stringID_table_t WPTable[] =
{
	"NULL",WP_NONE,
	stringIDExpand("none",WP_NONE),
	stringIDExpand("phaser",WP_PHASER),
	stringIDExpand("prifle",WP_COMPRESSION_RIFLE),
	stringIDExpand("imod",WP_IMOD),
	stringIDExpand("borg",WP_BORG_WEAPON),
	stringIDExpand("borghand",WP_BORG_ASSIMILATOR),
	stringIDExpand("borgdrill",WP_BORG_DRILL),
	stringIDExpand("borgtaser",WP_BORG_TASER),
	stringIDExpand("scavenger",WP_SCAVENGER_RIFLE),
	stringIDExpand("stasis",WP_STASIS),
	stringIDExpand("grenade",WP_GRENADE_LAUNCHER),
	stringIDExpand("tetrion",WP_TETRION_DISRUPTOR),
	stringIDExpand("dreadnought",WP_DREADNOUGHT),
	stringIDExpand("quantum",WP_QUANTUM_BURST),
	stringIDExpand("tricorder",WP_TRICORDER),
	stringIDExpand("botwelder",WP_BOT_WELDER),
	stringIDExpand("chaosgun",WP_CHAOTICA_GUARD_GUN),
	stringIDExpand("botrocket",WP_BOT_ROCKET),
	stringIDExpand("forgeproj",WP_FORGE_PROJ),
	stringIDExpand("forgepsych",WP_FORGE_PSYCH),
	stringIDExpand("parasite",WP_PARASITE),
	stringIDExpand("bluehypo",WP_BLUE_HYPO),		
	stringIDExpand("redhypo",WP_RED_HYPO),
	stringIDExpand("voyhypo",WP_VOYAGER_HYPO),
	stringIDExpand("dktahg",WP_KLINGON_BLADE),
	stringIDExpand("imperial",WP_IMPERIAL_BLADE),
	stringIDExpand("desperado",WP_DESPERADO),
	stringIDExpand("paladin",WP_PALADIN),
	stringIDExpand("proton",WP_PROTON_GUN),
	"", NULL
};

stringID_table_t eventTable[] =
{
	//BOTH_h
	stringIDExpand("beamdown",		EV_BEAM_DOWN),
	stringIDExpand("beamup",		EV_BEAM_UP),
	stringIDExpand("scavteleport",	EV_SCAV_TELEPORT),
	stringIDExpand("stasisbeamin",	EV_STASIS_BEAM_IN),
	stringIDExpand("stasisbeamout",	EV_STASIS_BEAM_OUT),
	stringIDExpand("disintegrate",	EV_DISINTEGRATE),
	stringIDExpand("borgteleport",	EV_BORG_TELEPORT_FX),
	stringIDExpand("8472beamout",	EV_8472_BEAM_OUT),
	//END
	"",				EV_BAD,
};

stringID_table_t setTable[] =
{
	stringIDExpand("spawnscript",	SET_SPAWNSCRIPT),//0
	stringIDExpand("idlescript",	SET_IDLESCRIPT),
	stringIDExpand("touchscript",	SET_TOUCHSCRIPT),
	stringIDExpand("usescript",		SET_USESCRIPT),
	stringIDExpand("awakescript",	SET_AWAKESCRIPT),
	stringIDExpand("angerscript",	SET_ANGERSCRIPT),
	stringIDExpand("attackscript",	SET_ATTACKSCRIPT),
	stringIDExpand("victoryscript",	SET_VICTORYSCRIPT),
	stringIDExpand("painscript",	SET_PAINSCRIPT),
	stringIDExpand("fleescript",	SET_FLEESCRIPT),
	stringIDExpand("deathscript",	SET_DEATHSCRIPT),
	stringIDExpand("delayscript",	SET_DELAYEDSCRIPT),
	stringIDExpand("blockedscript",	SET_BLOCKEDSCRIPT),
	stringIDExpand("ffirescript",	SET_FFIRESCRIPT),
	stringIDExpand("ffiredeath",	SET_FFDEATHSCRIPT),
	stringIDExpand("ORIGIN",		SET_ORIGIN),
	stringIDExpand("teleportdest",	SET_TELEPORT_DEST),
	stringIDExpand("ANGLES",		SET_ANGLES),
	stringIDExpand("VELOCITY",		SET_VELOCITY),
	stringIDExpand("XVELOCITY",		SET_XVELOCITY),
	stringIDExpand("YVELOCITY",		SET_YVELOCITY),
	stringIDExpand("ZVELOCITY",		SET_ZVELOCITY),
	stringIDExpand("AVELOCITY",		SET_AVELOCITY),
	stringIDExpand("ENEMY",			SET_ENEMY),
	stringIDExpand("leader",		SET_LEADER),
	stringIDExpand("NAVGOAL",		SET_NAVGOAL),
	stringIDExpand("ANIM_UPPER",	SET_ANIM_UPPER),
	stringIDExpand("ANIM_LOWER",	SET_ANIM_LOWER),
	stringIDExpand("ANIM_BOTH",		SET_ANIM_BOTH),
	stringIDExpand("anim_holdtime_lower",SET_ANIM_HOLDTIME_LOWER),
	stringIDExpand("anim_holdtime_upper",SET_ANIM_HOLDTIME_UPPER),
	stringIDExpand("anim_holdtime_both",SET_ANIM_HOLDTIME_BOTH),
	stringIDExpand("playerTeam",	SET_PLAYER_TEAM),
	stringIDExpand("enemyTeam",		SET_ENEMY_TEAM),
	stringIDExpand("behaviorState",	SET_BEHAVIOR_STATE),
	stringIDExpand("bState",		SET_BEHAVIOR_STATE),
	stringIDExpand("health",		SET_HEALTH),
	stringIDExpand("armor",			SET_ARMOR),
	stringIDExpand("defaultBState",	SET_DEFAULT_BSTATE),
	stringIDExpand("hideGoal",		SET_HIDING),
	stringIDExpand("captureGoal",	SET_CAPTURE),
	stringIDExpand("desiredPitch",	SET_DPITCH),//FIXME: temp, remove
	stringIDExpand("desiredYaw",	SET_DYAW),//FIXME: temp, remove
	stringIDExpand("event",			SET_EVENT),//FIXME: temp, remove
	stringIDExpand("tempBehavior",	SET_TEMP_BSTATE),
	stringIDExpand("copyorigin",	SET_COPY_ORIGIN),
	stringIDExpand("viewtarget",	SET_VIEWTARGET),
	stringIDExpand("weapon",		SET_WEAPON),
	stringIDExpand("walkspeed",		SET_WALKSPEED),
	stringIDExpand("runspeed",		SET_RUNSPEED),
	stringIDExpand("YAWSPEED",		SET_YAWSPEED),
	stringIDExpand("aggression",	SET_AGGRESSION),
	stringIDExpand("aim",			SET_AIM),
	stringIDExpand("friction",		SET_FRICTION),
	stringIDExpand("gravity",		SET_GRAVITY),
	stringIDExpand("ignorepain",	SET_IGNOREPAIN),
	stringIDExpand("ignoreenemies",	SET_IGNOREENEMIES),
	stringIDExpand("ignorealerts",	SET_IGNOREALERTS),
	stringIDExpand("straighttogoal",SET_STRAIGHTTOGOAL),
	stringIDExpand("DONTSHOOT",		SET_DONTSHOOT),
	stringIDExpand("DONTFIRE",		SET_DONTFIRE),
	stringIDExpand("NOSLOWDOWN",	SET_NOSLOWDOWN),
	stringIDExpand("lockedEnemy",	SET_LOCKED_ENEMY),
	stringIDExpand("NOTARGET",		SET_NOTARGET),
	stringIDExpand("lean",			SET_LEAN),
	stringIDExpand("crouched",		SET_CROUCHED),
	stringIDExpand("walking", 		SET_WALKING),
	stringIDExpand("running", 		SET_RUNNING),
	stringIDExpand("careful", 		SET_CAREFUL),
	stringIDExpand("altfire", 		SET_ALT_FIRE),
	stringIDExpand("noresponse", 	SET_NO_RESPONSE),
	stringIDExpand("nocombattalk", 	SET_NO_COMBAT_TALK),
	stringIDExpand("undying", 		SET_UNDYING),
	stringIDExpand("treasoned", 	SET_TREASONED),
	stringIDExpand("invincible", 	SET_INVINCIBLE),
//	stringIDExpand("missionstatusactive",SET_MISSIONSTATUSACTIVE),
	stringIDExpand("noavoid",		SET_NOAVOID),
	stringIDExpand("flashlight", 	SET_BEAM),
	stringIDExpand("shootdist",		SET_SHOOTDIST),
	stringIDExpand("squadname",		SET_SQUADNAME),
	stringIDExpand("targetname",	SET_TARGETNAME),
	stringIDExpand("target",		SET_TARGET),
	stringIDExpand("target2",		SET_TARGET2),
	stringIDExpand("location",		SET_LOCATION),
	stringIDExpand("paintarget",	SET_PAINTARGET),
	stringIDExpand("timescale",		SET_TIMESCALE),
	stringIDExpand("visrange",		SET_VISRANGE),
	stringIDExpand("earshot",		SET_EARSHOT),
	stringIDExpand("vigilance",		SET_VIGILANCE),
	stringIDExpand("hfov",			SET_HFOV),
	stringIDExpand("vfov",			SET_VFOV),
	stringIDExpand("precache",		SET_PRECACHE),
	stringIDExpand("createformation",SET_CREATEFORMATION),
	stringIDExpand("delayscripttime",SET_DELAYSCRIPTTIME),
	stringIDExpand("detpack",		SET_DETPACK),
	stringIDExpand("forwardmove",	SET_FORWARDMOVE),
	stringIDExpand("rightmove",		SET_RIGHTMOVE),
	stringIDExpand("lockAngle",		SET_LOCKYAW),
	stringIDExpand("solid",			SET_SOLID),
	stringIDExpand("cameraGroup",	SET_CAMERA_GROUP),
	stringIDExpand("lookTarget",	SET_LOOK_TARGET),
	stringIDExpand("playerWidth",	SET_PLAYER_WIDTH),
	stringIDExpand("faceaux",		SET_FACEAUX),
	stringIDExpand("faceblink",		SET_FACEBLINK),
	stringIDExpand("faceblinkfrown",SET_FACEBLINKFROWN),
	stringIDExpand("facefrown",		SET_FACEFROWN),
	stringIDExpand("facenormal",	SET_FACENORMAL),
	stringIDExpand("scrolltext",	SET_SCROLLTEXT),
	stringIDExpand("lcarstext",		SET_LCARSTEXT),
	stringIDExpand("scrolltextcolor",	SET_SCROLLTEXTCOLOR),
	stringIDExpand("captiontextcolor",	SET_CAPTIONTEXTCOLOR),
	stringIDExpand("centertextcolor",	SET_CENTERTEXTCOLOR),
	stringIDExpand("usable",		SET_PLAYER_USABLE),
	stringIDExpand("startframe",	SET_STARTFRAME),
	stringIDExpand("endframe",		SET_ENDFRAME),
	stringIDExpand("loopAnim",		SET_LOOP_ANIM),
	stringIDExpand("interface",		SET_INTERFACE),
	stringIDExpand("shields",		SET_SHIELDS),
	stringIDExpand("noKnockback",	SET_NO_KNOCKBACK),
	stringIDExpand("invisible",		SET_INVISIBLE),
	stringIDExpand("greet",			SET_GREET_ALLIES),
	stringIDExpand("player_locked",	SET_PLAYER_LOCKED),
	stringIDExpand("width",			SET_WIDTH),
	stringIDExpand("height",		SET_HEIGHT),
	stringIDExpand("crouchheight",	SET_CROUCHHEIGHT),
	stringIDExpand("parm1",			SET_PARM1),
	stringIDExpand("parm2",			SET_PARM2),
	stringIDExpand("parm3",			SET_PARM3),
	stringIDExpand("parm4",			SET_PARM4),
	stringIDExpand("parm5",			SET_PARM5),
	stringIDExpand("parm6",			SET_PARM6),
	stringIDExpand("parm7",			SET_PARM7),
	stringIDExpand("parm8",			SET_PARM8),
	stringIDExpand("parm9",			SET_PARM9),
	stringIDExpand("parm10",		SET_PARM10),
	stringIDExpand("parm11",		SET_PARM11),
	stringIDExpand("parm12",		SET_PARM12),
	stringIDExpand("parm13",		SET_PARM13),
	stringIDExpand("parm14",		SET_PARM14),
	stringIDExpand("parm15",		SET_PARM15),
	stringIDExpand("parm16",		SET_PARM16),
	stringIDExpand("defendTarget",	SET_DEFEND_TARGET),
	stringIDExpand("wait",			SET_WAIT),
	stringIDExpand("count",			SET_COUNT),
	stringIDExpand("shotspacing",	SET_SHOT_SPACING),
	stringIDExpand("inGameCinematic",	SET_VIDEO_PLAY),
	stringIDExpand("cl_VidFadeUp",		SET_VIDEO_FADE_IN),
	stringIDExpand("cl_VidFadeDown",	SET_VIDEO_FADE_OUT),
	stringIDExpand("+boltOn",		SET_BOLTON_ON),
	stringIDExpand("-boltOn",		SET_BOLTON_OFF),
	stringIDExpand("+beamBoltOn",	SET_BEAMIN_BOLTON),	
	stringIDExpand("-beamBoltOn",	SET_BEAMOUT_BOLTON),	
	stringIDExpand("!boltOn",		SET_BOLTON_DROP),
	stringIDExpand("@boltOn",		SET_BOLTON_ACTIVE),
	stringIDExpand("boltOnStartFrame",	SET_BOLTON_STARTFRAME),
	stringIDExpand("boltOnEndFrame",	SET_BOLTON_ENDFRAME),
	stringIDExpand("boltOnLoopAnim",	SET_BOLTON_ANIMLOOP),
	stringIDExpand("plugtarget",		SET_PLUG_TARGET),
	stringIDExpand("removetarget",		SET_REMOVE_TARGET),
	stringIDExpand("loadgame",			SET_LOADGAME),
	stringIDExpand("concommand",		SET_CON_COMMAND),
	stringIDExpand("menuscreen",		SET_MENU_SCREEN),
	stringIDExpand("objectiveshow",		SET_OBJECTIVE_SHOW),
	stringIDExpand("objectivehide",		SET_OBJECTIVE_HIDE),
	stringIDExpand("objectivesucceed",	SET_OBJECTIVE_SUCCEEDED),
	stringIDExpand("objectivefailed",	SET_OBJECTIVE_FAILED),
	stringIDExpand("tacticalshow",		SET_TACTICAL_SHOW),
	stringIDExpand("tacticalhide",		SET_TACTICAL_HIDE),
	stringIDExpand("followdist",		SET_FOLLOWDIST),
	stringIDExpand("objectiveclear",	SET_OBJECTIVE_CLEARALL),
	stringIDExpand("objectivefoster",	SET_OBJECTIVEFOSTER),
	stringIDExpand("objectiveodell",	SET_OBJECTIVEODELL),
	stringIDExpand("objectivecsatlos",	SET_OBJECTIVECSATLOS),
	stringIDExpand("objectiveisodesium1",SET_OBJECTIVEISODESIUM1),
	stringIDExpand("objectiveisodesium2",SET_OBJECTIVEISODESIUM2),
	stringIDExpand("objectiveisodesium3",SET_OBJECTIVEISODESIUM3),
	stringIDExpand("objectiveisodesiumall",SET_OBJECTIVEISODESIUMALL),
	stringIDExpand("objectivesecuritycode",SET_OBJECTIVESECURITYCODE),

	stringIDExpand("objectivecollectedactionfigures",SET_OBJECTIVE_COLLECTEDACTIONFIGURES),
	stringIDExpand("objectiveharvesterleg",SET_OBJECTIVE_HARVESTER_LEG),
	stringIDExpand("objectiveefposter",SET_OBJECTIVE_EF_POSTER),
	stringIDExpand("objectivesevenplate",SET_OBJECTIVE_SEVEN_PLATE),
	stringIDExpand("objectivefireflies",SET_OBJECTIVE_FIREFLIES),
	stringIDExpand("objectivearcwelder",SET_OBJECTIVE_ARC_WELDER),
	stringIDExpand("objectivegrenadelauncher",SET_OBJECTIVE_GRENADE_LAUNCHER),
	stringIDExpand("objectivebiessmanmorgue",SET_OBJECTIVE_BIESSMAN_MORGUE),
	stringIDExpand("objectiveborgslayer",SET_OBJECTIVE_BORG_SLAYER),
	stringIDExpand("objectiveravenplaque",SET_OBJECTIVE_RAVEN_PLAQUE),
	stringIDExpand("objectivehexeniibox",SET_OBJECTIVE_HERETIC_II_BOX),
	stringIDExpand("objectivesofbox",SET_OBJECTIVE_SOF_BOX),

	stringIDExpand("objectivedollmunro",SET_OBJECTIVE_DOLL_MUNRO),
	stringIDExpand("objectivedollfoster",SET_OBJECTIVE_DOLL_FOSTER),
	stringIDExpand("objectivedolltelsia",SET_OBJECTIVE_DOLL_TELSIA),
	stringIDExpand("objectivedollalexandria",SET_OBJECTIVE_DOLL_ALEXANDRIA),
	stringIDExpand("objectivedollbiessman",SET_OBJECTIVE_DOLL_BIESSMAN),
	stringIDExpand("objectivedollchell",SET_OBJECTIVE_DOLL_CHELL),
	stringIDExpand("objectivedollchang",SET_OBJECTIVE_DOLL_CHANG),
	stringIDExpand("objectivedolljurot",SET_OBJECTIVE_DOLL_JUROT),
	stringIDExpand("objectivedollborgfoster",SET_OBJECTIVE_DOLL_BORG_FOSTER),

	stringIDExpand("objectivephotonburst",SET_OBJECTIVE_PHOTONBURST),
	stringIDExpand("objectivetetryondisruptor",SET_OBJECTIVE_TETRYONDISRUPTOR),
	stringIDExpand("objectivetoursuccessful",SET_OBJECTIVE_TOURSUCCESSFUL),

	stringIDExpand("missionstatustext",	SET_MISSIONSTATUSTEXT),
	stringIDExpand("closingcredits",	SET_CLOSINGCREDITS),
	stringIDExpand("skill",				SET_SKILL),
	stringIDExpand("sex",				SET_SEX),
	stringIDExpand("missionstatustime",	SET_MISSIONSTATUSTIME),
	stringIDExpand("maxammo",			SET_MAX_AMMO),


	stringIDExpand("clearborgadapthits",SET_CLEAR_BORG_ADAPT),
	stringIDExpand("fullName",			SET_FULLNAME),

//FIXME: add BOTH_ attributes here too
	"",	SET_,
};

qboolean G_ParseString( char **data, char **s ); 

//=======================================================================

interface_export_t	interface_export;


vec4_t textcolor_caption;
vec4_t textcolor_center;
vec4_t textcolor_scroll;


/*
============
Q3_ReadScript
  Description	: Reads in a file and attaches the script directory properly
  Return type	: static int 
  Argument		: const char *name
  Argument		: void **buf
============
*/
extern int ICARUS_GetScript( const char *name, char **buf );	//g_icarus.cpp
static int Q3_ReadScript( const char *name, void **buf )
{
	return ICARUS_GetScript( va( "%s/%s", Q3_SCRIPT_DIR, name ), (char**)buf );	//get a (hopefully) cached file
}

/*
============
Q3_CenterPrint 
  Description	: Prints a message in the center of the screen
  Return type	: static void 
  Argument		:  const char *format
  Argument		: ...
============
*/
static void Q3_CenterPrint ( const char *format, ... )
{

	va_list		argptr;
	char		text[1024];

	va_start (argptr, format);
	vsprintf (text, format, argptr);
	va_end (argptr);

	if (text[0] == '@')	// It's a key
	{
		gi.SendServerCommand( NULL, "cp \"%s\"", text );
		return;
	}

	Q3_DebugPrint( WL_VERBOSE, "%s\n", text); 	// Just a developers note

	return;
}

/*
-------------------------
SetTextColor
-------------------------
*/

static void SetTextColor ( vec4_t textcolor,const char *color)
{

	if (Q_stricmp(color,"BLACK") == 0)
	{
		Vector4Copy( colorTable[CT_BLACK], textcolor );
	}
	else if (Q_stricmp(color,"RED") == 0)
	{
		Vector4Copy( colorTable[CT_RED], textcolor );
	}
	else if (Q_stricmp(color,"GREEN") == 0)
	{
		Vector4Copy( colorTable[CT_GREEN], textcolor );
	}
	else if (Q_stricmp(color,"YELLOW") == 0)
	{
		Vector4Copy( colorTable[CT_YELLOW], textcolor );
	}
	else if (Q_stricmp(color,"BLUE") == 0)
	{
		Vector4Copy( colorTable[CT_BLUE], textcolor );
	}
	else if (Q_stricmp(color,"CYAN") == 0)
	{
		Vector4Copy( colorTable[CT_CYAN], textcolor );
	}
	else if (Q_stricmp(color,"MAGENTA") == 0)
	{
		Vector4Copy( colorTable[CT_MAGENTA], textcolor );
	}
	else if (Q_stricmp(color,"WHITE") == 0)
	{
		Vector4Copy( colorTable[CT_WHITE], textcolor );
	}
	else 
	{
		Vector4Copy( colorTable[CT_WHITE], textcolor );
	}

	return;
}

/*
-------------------------
void Q3_ClearTaskID( int *taskID )

WARNING: Clearing a taskID will make that task never finish unless you intend to
			return the same taskID from somewhere else.
-------------------------
*/
void Q3_TaskIDClear( int *taskID )
{
	*taskID = -1;
}

/*
-------------------------
qboolean Q3_TaskIDPending( gentity_t *ent, taskID_t taskType )
-------------------------
*/
qboolean Q3_TaskIDPending( gentity_t *ent, taskID_t taskType )
{
	if ( !ent->sequencer || !ent->taskManager )
	{
		return qfalse;
	}

	if ( taskType < TID_CHAN_VOICE || taskType >= NUM_TIDS )
	{
		return qfalse;
	}

	if ( ent->taskID[taskType] >= 0 )//-1 is none
	{
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
void Q3_TaskIDComplete( gentity_t *ent, taskID_t taskType )
-------------------------
*/
void Q3_TaskIDComplete( gentity_t *ent, taskID_t taskType )
{
	if ( taskType < TID_CHAN_VOICE || taskType >= NUM_TIDS )
	{
		return;
	}

	if ( ent->taskManager && Q3_TaskIDPending( ent, taskType ) )
	{//Complete it
		ent->taskManager->Completed( ent->taskID[taskType] );

		//See if any other tasks have the name number and clear them so we don't complete more than once
		int	clearTask = ent->taskID[taskType];
		for ( int tid = 0; tid < NUM_TIDS; tid++ )
		{
			if ( ent->taskID[tid] == clearTask )
			{
				Q3_TaskIDClear( &ent->taskID[tid] );
			}
		}

		//clear it - should be cleared in for loop above
		//Q3_TaskIDClear( &ent->taskID[taskType] );
	}
	//otherwise, wasn't waiting for a task to complete anyway
}

/*
-------------------------
void Q3_SetTaskID( gentity_t *ent, taskID_t taskType, int taskID )
-------------------------
*/

static void Q3_TaskIDSet( gentity_t *ent, taskID_t taskType, int taskID )
{
	if ( taskType < TID_CHAN_VOICE || taskType >= NUM_TIDS )
	{
		return;
	}

	//Might be stomping an old task, so complete and clear previous task if there was one
	Q3_TaskIDComplete( ent, taskType );

	ent->taskID[taskType] = taskID;
}


/*
============
Q3_CheckStringCounterIncrement
  Description	: 
  Return type	: static float 
  Argument		: const char *string
============
*/
static float Q3_CheckStringCounterIncrement( const char *string )
{
	char	*numString;
	float	val = 0.0f;

	if ( string[0] == '+' )
	{//We want to increment whatever the value is by whatever follows the +
		if ( string[1] )
		{
			numString = (char *)&string[1];
			val = atof( numString );
		}
	}
	else if ( string[0] == '-' )
	{//we want to decrement
		if ( string[1] )
		{
			numString = (char *)&string[1];
			val = atof( numString ) * -1;
		}
	}

	return val;
}

/*
-------------------------
Q3_GetAnimLower
-------------------------
*/
static char *Q3_GetAnimLower( gentity_t *ent )
{
	if ( ent->client == NULL )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_GetAnimLower: attempted to read animation state off non-client!\n" );
		return NULL;
	}

	int anim = ent->client->ps.legsAnim & ~ANIM_TOGGLEBIT;

	return (char *) GetStringForID( animTable, anim );
}

/*
-------------------------
Q3_GetAnimUpper
-------------------------
*/
static char *Q3_GetAnimUpper( gentity_t *ent )
{
	if ( ent->client == NULL )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_GetAnimUpper: attempted to read animation state off non-client!\n" );
		return NULL;
	}

	int anim = ent->client->ps.torsoAnim & ~ANIM_TOGGLEBIT;

	return (char *) GetStringForID( animTable, anim );
}

/*
-------------------------
Q3_GetAnimBoth
-------------------------
*/
static char *Q3_GetAnimBoth( gentity_t *ent )
{
 	char	*lowerName, *upperName;

	lowerName = Q3_GetAnimLower( ent );
	upperName = Q3_GetAnimUpper( ent );

	if ( VALIDSTRING( lowerName ) == false )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_GetAnimBoth: NULL legs animation string found!\n" );
		return NULL;
	}

	if ( VALIDSTRING( upperName ) == false )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_GetAnimBoth: NULL torso animation string found!\n" );
		return NULL;
	}

	if ( _stricmp( lowerName, upperName ) )
	{
#ifdef _DEBUG	// sigh, cut down on tester reports that aren't important
		Q3_DebugPrint( WL_WARNING, "Q3_GetAnimBoth: legs and torso animations did not match : returning legs\n" );
#endif
	}

	return lowerName;
}

/*
-------------------------
Q3_SetPlugTarget
-------------------------
*/
static void Q3_SetPlugTarget( int entID, const char *targetname )
{
	gentity_t	*self = &g_entities[ entID ];
	gentity_t	*ent = G_Find( NULL, FOFS(targetname), (char *) targetname );

	if ( ent == NULL )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetPlugTarget: cannot find target %s\n", targetname );

		//FIXME: This won't work if they try and use this name...
		self->target = "!INVALID!";

		return;
	}

	//Set the target
	self->target = ent->targetname;
}


/*
-------------------------
Q3_SetTactical
-------------------------
*/
static void Q3_SetTactical(const char *TactEnum, int status)
{
	int tacticalID;

	tacticalID = GetIDForString( tacticalTable, TactEnum );

	switch (status)
	{
	case SET_TACTICAL_ON :
		tactical_info[tacticalID] = qtrue;
		break;
	case SET_TACTICAL_OFF :
		tactical_info[tacticalID] = qfalse;
		break;
	}
}

#define MAX_OBJ_CHECK	17

int obj_checks[] =
{
OBJ_REDALERT,
OBJ_TRANSPORTER,
OBJ_DECK15,
OBJ_ASTROMETRICS,
OBJ_SECURITYCODE,
OBJ_LAUNCHSHUTTLE,
OBJ_COLLECTEDACTIONFIGURES,
OBJ_HARVESTER_LEG,
OBJ_EF_POSTER,
OBJ_SEVEN_PLATE,
OBJ_FIREFLIES,
OBJ_ARC_WELDER,
OBJ_GRENADE_LAUNCHER,
OBJ_BIESSMAN_MORGUE,
OBJ_BORG_SLAYER,
OBJ_RAVEN_PLAQUE,
OBJ_PHOTONBURST
};

/*
-------------------------
Q3_CheckTourSuccess
-------------------------
*/
static void Q3_CheckTourSuccess(void)
{
	int i,figureCount,figureTotal;
	gclient_t	*client;
	objectives_t	*objective;

	// Not in tour mode
	if (cg_virtualVoyager.value!=1)
	{
		return;
	}

	client = &level.clients[0];

	figureCount = 0;
	figureTotal = 9;	// Change this if more Dolls are added.
	if (client->tourSess.tour_objectives[OBJ_DOLL_MUNRO].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}

	if (client->tourSess.tour_objectives[OBJ_DOLL_FOSTER].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_TELSIA].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_ALEXANDRIA].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_BIESSMAN].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_CHELL].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_CHANG].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_JUROT].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (client->tourSess.tour_objectives[OBJ_DOLL_BORG_FOSTER].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}

	// Count number of action figures collected
	if (client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].status != OBJECTIVE_STAT_SUCCEEDED)
	{
		if (figureCount)
		{
			tactical_info[TACT_HOWMANY_DOLLS] = qtrue;	// Tell them how many dolls they have.

			// Collected all action figures so don't print count
			if (figureCount==figureTotal)
			{
				client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].status = OBJECTIVE_STAT_SUCCEEDED;

				// If TOURSUCCESS (all items have been gathered) don't show COLLECTEDACTIONFIGURES
				if (client->tourSess.tour_objectives[OBJ_TOURSUCCESS].display == OBJECTIVE_SHOW)
				{
					client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].display = OBJECTIVE_HIDE;
					tactical_info[TACT_EASTEREGG4] = qfalse;
				}
				else
				{
					client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].display = OBJECTIVE_SHOW;
					tactical_info[TACT_EASTEREGG4] = qtrue;
				}
				tactical_info[TACT_HOWMANY_DOLLS] = qfalse;	// They have them all so use TACT_EASTEREGG4 (unless the tour is successful)
			}
		}
	}

	for (i=0;i<MAX_OBJ_CHECK;i++)
	{
		objective = &client->tourSess.tour_objectives[obj_checks[i]];

		// Any unsuccessful mean get out of here.
		if (objective->status != OBJECTIVE_STAT_SUCCEEDED)
		{
			return;
		}
	}


	// They all succeeded so give the player the succeeded objective
	for (i=0;i<MAX_OBJ_CHECK;i++)
	{
		objective = &client->tourSess.tour_objectives[obj_checks[i]];

		// There's always a special case.
		if ((obj_checks[i] != OBJ_EF_POSTER) && (obj_checks[i] != OBJ_RAVEN_PLAQUE))
		{
			objective->status = SET_OBJ_SUCCEEDEDTOUR;
		}
		objective->display = OBJECTIVE_HIDE;
	}

	// Hide this one
	objective = &client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES];
	objective->display = OBJECTIVE_HIDE;

	// Hide this one
	objective = &client->tourSess.tour_objectives[OBJ_ACCESSCODES];
	objective->display = OBJECTIVE_HIDE;

	objective = &client->tourSess.tour_objectives[OBJ_TOURSUCCESS];
	objective->status = OBJECTIVE_STAT_SUCCEEDED;
	objective->display = OBJECTIVE_SHOW;

	tactical_info[TACT_EASTEREGG4] = qfalse;
	tactical_info[TACT_HOWMANY_DOLLS] = qfalse;

	if (Q_stricmpn(level.mapname,"deck02",6)) 
	{
		G_UseTargets2( &g_entities[0], &g_entities[0], "objectivecheck" );
	}

}

/*
-------------------------
Q3_SetObjective
-------------------------
*/
static void Q3_SetObjective(const char *ObjEnum, int status)
{
	int objectiveID;
	gclient_t	*client;
	objectives_t	*objective;
	int				*objectivesShown;

	client = &level.clients[0];

	objectiveID = GetIDForString( objectiveTable, ObjEnum );

	if (objectiveID<0)
	{
		objectiveID = GetIDForString( tourObjectiveTable, ObjEnum );
		objective = &client->tourSess.tour_objectives[objectiveID];
		objectivesShown = &client->tourSess.tourObjectivesShown;
	}
	else
	{
		objectiveID = GetIDForString( objectiveTable, ObjEnum );
		objective = &client->sess.mission_objectives[objectiveID];
		objectivesShown = &client->sess.missionObjectivesShown;
	}


	switch (status)
	{
	case SET_OBJ_HIDE :
		objective->display = OBJECTIVE_HIDE;
		break;
	case SET_OBJ_SHOW :
		objective->display = OBJECTIVE_SHOW;
		objectivesShown++;
		missionInfo_Updated = qtrue;	// Activate flashing text
		break;
	case SET_OBJ_PENDING :
		objective->status = OBJECTIVE_STAT_PENDING;
		if (objective->display != OBJECTIVE_HIDE)
		{
			objectivesShown++;
			missionInfo_Updated = qtrue;	// Activate flashing text
		}
		break;
	case SET_OBJ_SUCCEEDED :
		objective->status = OBJECTIVE_STAT_SUCCEEDED;
		if (objective->display != OBJECTIVE_HIDE)
		{
			objectivesShown++;
			missionInfo_Updated = qtrue;	// Activate flashing text
		}

		if (g_virtualVoyager->integer)	// When in tour mode, flash text if tactical is updated
		{
			switch (objectiveID)
			{
			case OBJ_DOLL_MUNRO	:
			case OBJ_DOLL_FOSTER	:
			case OBJ_DOLL_TELSIA	:
			case OBJ_DOLL_ALEXANDRIA	:
			case OBJ_DOLL_BIESSMAN	:
			case OBJ_DOLL_CHELL	:
			case OBJ_DOLL_CHANG	:
			case OBJ_DOLL_JUROT	:
			case OBJ_DOLL_BORG_FOSTER	:
				missionInfo_Updated = qtrue;	// Activate flashing text
			}
		}
		break;
	case SET_OBJ_FAILED :
		objective->status = OBJECTIVE_STAT_FAILED;
		if (objective->display != OBJECTIVE_HIDE)
		{
			objectivesShown++;
			missionInfo_Updated = qtrue;	// Activate flashing text
		}
		break;
	}
}


/*
-------------------------
Q3_SetStatusText
-------------------------
*/
static void Q3_SetStatusText(const char *StatusTextEnum)
{
	int statusTextID;

	statusTextID = GetIDForString( statusTextTable, StatusTextEnum );

	switch (statusTextID)
	{
	case STAT_INSUBORDINATION:
		statusTextIndex = IGT_INSUBORDINATION;
		break;
	case STAT_YOUCAUSEDDEATHOFTEAMMATE:
		statusTextIndex = IGT_YOUCAUSEDDEATHOFTEAMMATE;
		break;
	case STAT_DIDNTPROTECTTECH:
		statusTextIndex = IGT_DIDNTPROTECTTECH;
		break;
	case STAT_DIDNTPROTECT7OF9:
		statusTextIndex = IGT_DIDNTPROTECT7OF9;
		break;
	case STAT_NOTSTEALTHYENOUGH:
		statusTextIndex = IGT_NOTSTEALTHYENOUGH;
		break;
	case STAT_STEALTHTACTICSNECESSARY:
		statusTextIndex = IGT_STEALTHTACTICSNECESSARY;
		break;
	case STAT_WATCHYOURSTEP:
		statusTextIndex = IGT_WATCHYOURSTEP;
		break;
	case STAT_JUDGEMENTMUCHDESIRED:
		statusTextIndex = IGT_JUDGEMENTMUCHDESIRED;
		break;
	default:
		assert(0);
		break;
	}
}


/*
-------------------------
Q3_ObjectiveClearAll
-------------------------
*/
static void Q3_ObjectiveClearAll(void)
{
	client = &level.clients[0];
	memset(client->sess.mission_objectives,0,sizeof(client->sess.mission_objectives));
	memset(client->tourSess.tour_objectives,0,sizeof(client->tourSess.tour_objectives));
}

/*
-------------------------
Q3_ResetBorgAdaptHits
-------------------------
*/
static void Q3_ResetBorgAdaptHits( void )
{
	gentity_t *ent;

	ent = &g_entities[0];

	if ( ent && ent->client )
	{
		for ( int i=0; i < MAX_WEAPONS; i++ )
		{
			ent->client->ps.borgAdaptHits[ i ] = 0;
		}
	}
}

/*
=============
Q3_SetCaptionTextColor

Change color text prints in
=============
*/
static void Q3_SetCaptionTextColor ( const char *color)
{
	SetTextColor(textcolor_caption,color);
}

/*
=============
Q3_SetCenterTextColor

Change color text prints in
=============
*/
static void Q3_SetCenterTextColor ( const char *color)
{
	SetTextColor(textcolor_center,color);
}

/*
=============
Q3_SetScrollTextColor

Change color text prints in
=============
*/
static void Q3_SetScrollTextColor ( const char *color)
{
	SetTextColor(textcolor_scroll,color);
}

/*
=============
Q3_ScrollText

Prints a message in the center of the screen
=============
*/
static void Q3_ScrollText ( const char *id)
{
	gi.SendServerCommand( NULL, "st \"%s\"", id);

	return;
}

/*
=============
Q3_LCARSText

Prints a message in the center of the screen giving it an LCARS frame around it
=============
*/
static void Q3_LCARSText ( const char *id)
{
	gi.SendServerCommand( NULL, "lt \"%s\"", id);

	return;
}

/*
=============
Q3_GetEntityByName

Returns the sequencer of the entity by the given name
=============
*/
static CSequencer *Q3_GetEntityByName( const char *name )
{
	gentity_t				*ent;
	entlist_t::iterator		ei;
	char					temp[1024];

	if ( name == NULL || name[0] == NULL )
		return NULL;

	strncpy( (char *) temp, name, sizeof(temp) );
	temp[sizeof(temp)-1] = 0;

	ei = ICARUS_EntList.find( _strupr( (char *) temp ) );

	if ( ei == ICARUS_EntList.end() )
		return NULL;

	ent = &g_entities[(*ei).second];

	if (ent == NULL)
		return NULL;

	return ent->sequencer;
}

/*
=============
Q3_GetTime

Get the current game time
=============
*/
static DWORD Q3_GetTime( void )
{
	return level.time;
}

/*
=============
G_AddSexToMunroString

Take any string, look for "munro/" replace with "alexa/" based on "sex"
And: Take any string, look for "/mr_" replace with "/ms_" based on "sex" 
returns qtrue if changed to ms
=============
*/
qboolean G_AddSexToMunroString ( char *string, qboolean qDoBoth )
{
	char *start;

	if VALIDSTRING( string ) {
		if ( g_sex->string[0] == 'f' ) {
			start = strstr( string, "munro/" );
			if ( start != NULL ) {
				strncpy( start, "alexa", 5 );
				return qtrue;
			} else {
				start = strrchr( string, '/' );		//get the last slash before the wav
				if (start != NULL) {
					if (!strncmp( start, "/mr_", 4) ) {
						if (qDoBoth) {	//we want to change mr to ms
							start[2] = 's';	//change mr to ms
							return qtrue;
						} else {	//IF qDoBoth
							return qfalse;	//don't want this one
						}
					}
				}	//IF found slash
			}
		}	//IF Female
		else {	//i'm male
			start = strrchr( string, '/' );		//get the last slash before the wav
			if (start != NULL) {
				if (!strncmp( start, "/ms_", 4) ) {
					return qfalse;	//don't want this one
				}
			}	//IF found slash
		}
	}	//if VALIDSTRING
	return qtrue;
}


/*
============
HeadText
  Description	: 
  Return type	: void 
  Argument		: int index
  Argument		: int entID
============
*/
static void HeadText(int index,int entID,char *finalName)
{
	int index2;
	index2 = G_SoundIndex( (char *) finalName );

	//FIXME: what do you do if more than one person is talking at a time?
	//		only override if closer?
	if ( entID == 0 && precacheWav[index].speaker != SP_MUNRO )
	{//Not Munro speaking, someone speaking on player channel
		gi.SendServerCommand( NULL, "gt %s %i %i %i", precacheWav[index].textKey, precacheWav[index].speaker, -1, index2 );
	}
	else
	{
		gi.SendServerCommand( NULL, "gt %s %i %i %i", precacheWav[index].textKey, precacheWav[index].speaker, entID, index2 );
	}
}

/*
=============
Q3_PlaySound

Plays a sound from an entity
=============
*/
extern int USE_ENT_NUM;
extern void G_SoundOnEnt (gentity_t *ent, soundChannel_t channel, const char *soundPath);
extern void G_SoundBroadcast( gentity_t *ent, int soundIndex );
static int Q3_PlaySound( int taskID, int entID, const char *name, const char *channel )
{
	gentity_t		*ent = &g_entities[entID];
	gentity_t		*ent2 = &g_entities[0];
	int				i;
	char			finalName[MAX_QPATH];
	soundChannel_t	voice_chan = CHAN_VOICE; // set a default so the compiler doesn't bitch
	qboolean		type_voice = qfalse;

	Q_strncpyz( finalName, name, MAX_QPATH, 0 );
	_strlwr(finalName);
	G_AddSexToMunroString( finalName, qtrue );

	COM_StripExtension( (const char *)finalName, finalName );
	COM_DefaultExtension( finalName, MAX_QPATH, ".wav" );
	int soundHandle = G_SoundIndex( (char *) finalName );
	bool bBroadcast = false;

	if ( ( _stricmp( channel, "CHAN_ANNOUNCER" ) == 0 ) || (ent->classname && Q_stricmp("target_scriptrunner", ent->classname ) == 0) ) {
		bBroadcast = true;
	}


	// moved here from further down so I can easily check channel-type without code dup...
	//
	if ( _stricmp( channel, "CHAN_VOICE" ) == 0 )
	{
		voice_chan = CHAN_VOICE;
		type_voice = qtrue;
	}
	else if ( _stricmp( channel, "CHAN_VOICE_ATTEN" ) == 0 )
	{
		voice_chan = CHAN_VOICE_ATTEN;
		type_voice = qtrue;
	}

	
	// Is it in the list of precached wavs???

	// if we're in-camera, check for skipping cinematic and ifso, no subtitle print (since screen is not being
	//	updated anyway during skipping). This stops leftover subtitles being left onscreen after unskipping.
	//
	if (!in_camera ||
		(!g_skippingcin || !g_skippingcin->integer)
		)	// paranoia towards project end <g>
	{
		for ( i=0;i<precacheWav_i;i++ )
		{
			if ( strcmp( finalName, precacheWav[i].wavFile ) == 0 )
			{
				if ( precacheWav[i].textKey[0] )	// There's text with it
				{
					// Text off
					if (g_subtitles->integer == 0) // Show only talking head text
					{
						if (!in_camera && (precacheWav[i].speaker!=SP_NONE))
						{
							HeadText(i,entID,finalName);
						}
					}
					// Text on
					else if (g_subtitles->integer == 1) // Show all text
					{
						if ( in_camera)	// Cinematic
						{					
							gi.SendServerCommand( NULL, "ct \"%s\" %i", precacheWav[i].textKey, soundHandle );
						}
						else if (precacheWav[i].speaker==SP_NONE)	//  lower screen text
						{
							// the numbers in here were either the original ones Bob entered (350), or one arrived at from checking the distance Chell stands at in stasis2 by the computer core that was submitted as a bug report...
							//
							if (bBroadcast || (DistanceSquared(ent->currentOrigin, ent2->currentOrigin) < ((voice_chan == CHAN_VOICE_ATTEN)?(350 * 350):(1200 * 1200)) ) )
							{
								gi.SendServerCommand( NULL, "ct \"%s\" %i", precacheWav[i].textKey, soundHandle );
							}
						}
						else if (precacheWav[i].speaker!=SP_NONE)	// Head text
						{
							HeadText( i, entID,finalName);
						}
					}
					// Cinematic only
					else if (g_subtitles->integer == 2) // Show only talking head text and CINEMATIC
					{
						if ( in_camera)	// Cinematic text
						{							
							gi.SendServerCommand( NULL, "ct \"%s\" %i", precacheWav[i].textKey, soundHandle);
						}
						else if (precacheWav[i].speaker!=SP_NONE)	// Head text
						{
							HeadText( i, entID,finalName);
						}
					}
				}
				break;
			}
		}

		if (i == precacheWav_i)
		{
			Q3_DebugPrint(WL_WARNING, "Script sound not in .pre files: %s!\n", finalName );
		}
	}

	if ( type_voice )
	{
		if ( g_timescale->value > 1.0f )
		{//Skip the damn sound!
			return qtrue;
		}
		else
		{
			//This the voice channel
			G_SoundOnEnt( ent, voice_chan, ((char *) finalName) );
		}
		//Remember we're waiting for this
		Q3_TaskIDSet( ent, TID_CHAN_VOICE, taskID );
		//do not task_return complete
		return qfalse;
	}

	if ( bBroadcast )
	{//Broadcast the sound
		G_SoundBroadcast( ent, soundHandle );
	}
	else
	{
		G_Sound( ent, soundHandle );
	}

	return qtrue;
}


/*
=============
Q3_SetAngles

Sets the angles of an entity directly
=============
*/
static void Q3_SetDYaw( int entID, float data );
static void Q3_SetAngles( int entID, vec3_t angles )
{
	gentity_t	*ent = &g_entities[entID];


	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetAngles: bad ent %d\n", entID);
		return;
	}

	if (ent->client)
	{
		SetClientViewAngle( ent, angles );
		if ( ent->NPC )
		{
			Q3_SetDYaw( entID, angles[YAW] );
		}
	}
	else
	{
		VectorCopy( angles, ent->s.angles );
	}
	gi.linkentity( ent );
}

/*
=============
Q3_SetOrigin

Sets the origin of an entity directly
=============
*/
static void Q3_SetOrigin( int entID, vec3_t origin )
{
	gentity_t	*ent = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetOrigin: bad ent %d\n", entID);
		return;
	}

	gi.unlinkentity (ent);

	if(ent->client)
	{
		VectorCopy(origin, ent->client->ps.origin);
		VectorCopy(origin, ent->currentOrigin);
		ent->client->ps.origin[2] += 1;

		VectorClear (ent->client->ps.velocity);
		ent->client->ps.pm_time = 160;		// hold time
		ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		
		ent->client->ps.eFlags ^= EF_TELEPORT_BIT;

//		G_KillBox (ent);
	}
	else
	{
		G_SetOrigin( ent, origin );
	}

	gi.linkentity( ent );
}


/*
============
MoveOwner
  Description	: 
  Return type	: void 
  Argument		: gentity_t *self
============
*/
void MoveOwner( gentity_t *self )
{
	self->nextthink = level.time + FRAMETIME;
	self->e_ThinkFunc = thinkF_G_FreeEntity;

	if ( !self->owner || !self->owner->inuse )
	{
		return;
	}

	if ( SpotWouldTelefrag2( self->owner, self->currentOrigin ) )
	{
		self->e_ThinkFunc = thinkF_MoveOwner;
	}
	else
	{
		if ( self->owner->NPC )
		{
			self->owner->NPC->sPCurSegPoint1 = self->owner->NPC->sPCurSegPoint2 = -1;
		}
		G_SetOrigin( self->owner, self->currentOrigin );
		Q3_TaskIDComplete( self->owner, TID_MOVE_NAV );
	}
}


/*
=============
Q3_SetTeleportDest

Copies passed origin to ent running script once there is nothing there blocking the spot
=============
*/
static qboolean Q3_SetTeleportDest( int entID, vec3_t org )
{
	gentity_t	*teleEnt = &g_entities[entID];

	if ( teleEnt )
	{
		if ( SpotWouldTelefrag2( teleEnt, org ) )
		{
			gentity_t *teleporter = G_Spawn();

			G_SetOrigin( teleporter, org );
			teleporter->owner = teleEnt;

			teleporter->e_ThinkFunc = thinkF_MoveOwner;
			teleporter->nextthink = level.time + FRAMETIME;
			
			return qfalse;
		}
		else
		{
			if ( teleEnt->NPC )
			{
				teleEnt->NPC->sPCurSegPoint1 = teleEnt->NPC->sPCurSegPoint2 = -1;
			}
			G_SetOrigin( teleEnt, org );
		}
	}

	return qtrue;
}

/*
=============
Q3_SetCopyOrigin

Copies origin of found ent into ent running script
=============`
*/
static void Q3_SetCopyOrigin( int entID, const char *name )
{
	gentity_t	*found = G_Find( NULL, FOFS(targetname), (char *) name);

	if(found)
	{
		Q3_SetOrigin( entID, found->currentOrigin );
		SetClientViewAngle( &g_entities[entID], found->s.angles );
	}
	else
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCopyOrigin: ent %s not found!\n", name);
	}
}

/*
=============
Q3_SetVelocity

Set the velocity of an entity directly
=============
*/
static void Q3_SetVelocity( int entID, int axis, float speed )
{
	gentity_t	*found = &g_entities[entID];
	//FIXME: Not supported
	if(!found)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetVelocity invalid entID %d\n", entID);
		return;
	}

	if(!found->client)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetVelocity: not a client %d\n", entID);
		return;
	}

	//FIXME: add or set?
	found->client->ps.velocity[axis] += speed;

	found->client->ps.pm_time = 500;
	found->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
}

/*
=============
moverCallback

Utility function
=============
*/
void moverCallback( gentity_t *ent )
{
	//complete the task
	Q3_TaskIDComplete( ent, TID_MOVE_NAV );
	
	// play sound
	ent->s.loopSound = 0;//stop looping sound
	G_PlayDoorSound( ent, BMS_END );//play end sound

	if ( ent->moverState == MOVER_1TO2 ) 
	{//reached open
		// reached pos2
		MatchTeam( ent, MOVER_POS2, level.time );
		//SetMoverState( ent, MOVER_POS2, level.time );
	} 
	else if ( ent->moverState == MOVER_2TO1 ) 
	{//reached closed
		MatchTeam( ent, MOVER_POS1, level.time );
		//SetMoverState( ent, MOVER_POS1, level.time );
		//close the portal
		gi.AdjustAreaPortalState( ent, qfalse );
	}

	if ( ent->e_BlockedFunc == blockedF_Blocked_Mover )
	{
		ent->e_BlockedFunc = blockedF_NULL;
	}
}

/*
=============
anglerCallback

Utility function
=============
*/
void anglerCallback( gentity_t *ent )
{
	//Complete the task
	Q3_TaskIDComplete( ent, TID_ANGLE_FACE );

	//Set the currentAngles, clear all movement
	VectorMA( ent->s.apos.trBase, (ent->s.apos.trDuration*0.001f), ent->s.apos.trDelta, ent->currentAngles );
	VectorCopy( ent->currentAngles, ent->s.apos.trBase );
	VectorClear( ent->s.apos.trDelta );
	ent->s.apos.trDuration = 1;
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = level.time;

	//Stop thinking
	ent->e_ReachedFunc = reachedF_NULL;
	if ( ent->e_ThinkFunc == thinkF_anglerCallback )
	{
		ent->e_ThinkFunc = thinkF_NULL;
	}

	//link
	gi.linkentity( ent );
}

/*
=============
moveAndRotateCallback

Utility function
=============
*/
void moveAndRotateCallback( gentity_t *ent )
{
	//stop turning
	anglerCallback( ent );
	//stop moving
	moverCallback( ent );
}

void Blocked_Mover( gentity_t *ent, gentity_t *other ) {
	// remove anything other than a client
	if ( !other->client ) {
		G_FreeEntity( other );
		return;
	}

	if ( ent->damage ) {
		G_Damage( other, ent, ent, NULL, NULL, ent->damage, 0, MOD_CRUSH );
	}
}

/*
=============
Q3_Lerp2Start

Lerps the origin of an entity to its starting position
=============
*/
static void Q3_Lerp2Start( int entID, int taskID, float duration )
{
	gentity_t	*ent = &g_entities[entID];

	if(!ent)
	{	
		Q3_DebugPrint( WL_WARNING, "Q3_Lerp2Start: invalid entID %d\n", entID);
		return;
	}
	
	if ( ent->client || ent->NPC || Q_stricmp(ent->classname, "target_scriptrunner") == 0 )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Lerp2Start: ent %d is NOT a mover!\n", entID);
		return;
	}

	if ( ent->s.eType != ET_MOVER )
	{
		ent->s.eType = ET_MOVER;
	}

	//FIXME: set up correctly!!!
	ent->moverState = MOVER_2TO1;
	ent->s.eType = ET_MOVER;
	ent->e_ReachedFunc = reachedF_moverCallback;		//Callsback the the completion of the move
	if ( ent->damage )
	{
		ent->e_BlockedFunc = blockedF_Blocked_Mover;
	}

	ent->s.pos.trDuration = duration * 10;	//In seconds
	ent->s.pos.trTime = level.time;
	
	Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
	// starting sound
	G_PlayDoorLoopSound( ent );
	G_PlayDoorSound( ent, BMS_START );	//??

	gi.linkentity( ent );
}

/*
=============
Q3_Lerp2End

Lerps the origin of an entity to its ending position
=============
*/
static void Q3_Lerp2End( int entID, int taskID, float duration )
{
	gentity_t	*ent = &g_entities[entID];

	if(!ent)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Lerp2End: invalid entID %d\n", entID);
		return;
	}
	
	if ( ent->client || ent->NPC || Q_stricmp(ent->classname, "target_scriptrunner") == 0 )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Lerp2End: ent %d is NOT a mover!\n", entID);
		return;
	}

	if ( ent->s.eType != ET_MOVER )
	{
		ent->s.eType = ET_MOVER;
	}

	if ( ent->moverState == MOVER_POS1 )
	{//open the portal
		gi.AdjustAreaPortalState( ent, qtrue );
	}

	//FIXME: set up correctly!!!
	ent->moverState = MOVER_1TO2;
	ent->s.eType = ET_MOVER;
	ent->e_ReachedFunc = reachedF_moverCallback;		//Callsback the the completion of the move
	if ( ent->damage )
	{
		ent->e_BlockedFunc = blockedF_Blocked_Mover;
	}

	ent->s.pos.trDuration = duration * 10;	//In seconds
	ent->s.time = level.time;
	
	Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
	// starting sound
	G_PlayDoorLoopSound( ent );
	G_PlayDoorSound( ent, BMS_START );	//??

	gi.linkentity( ent );
}

/*
=============
Q3_Lerp2Pos

Lerps the origin and angles of an entity to the destination values

=============
*/
static void Q3_Lerp2Pos( int taskID, int entID, vec3_t origin, vec3_t angles, float duration )
{
	gentity_t	*ent = &g_entities[entID];
	vec3_t		ang;
	int			i;

	if(!ent)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Lerp2Pos: invalid entID %d\n", entID);
		return;
	}
	
	if ( ent->client || ent->NPC || Q_stricmp(ent->classname, "target_scriptrunner") == 0 )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Lerp2Pos: ent %d is NOT a mover!\n", entID);
		return;
	}

	if ( ent->s.eType != ET_MOVER )
	{
		ent->s.eType = ET_MOVER;
	}

	//Don't allow a zero duration
	if ( duration == 0 )
		duration = 1;

	//
	// Movement

	moverState_t moverState = ent->moverState;

	if ( moverState == MOVER_POS1 || moverState == MOVER_2TO1 )
	{
		VectorCopy( ent->currentOrigin, ent->pos1 );
		VectorCopy( origin, ent->pos2 );

		if ( moverState == MOVER_POS1 )
		{//open the portal
			gi.AdjustAreaPortalState( ent, qtrue );
		}

		moverState = MOVER_1TO2;
	}
	else /*if ( moverState == MOVER_POS2 || moverState == MOVER_1TO2 )*/
	{
		VectorCopy( ent->currentOrigin, ent->pos2 );
		VectorCopy( origin, ent->pos1 );

		moverState = MOVER_2TO1;
	}

	InitMoverTrData( ent );

	ent->s.pos.trDuration = duration;

	// start it going
	MatchTeam( ent, moverState, level.time );
	//SetMoverState( ent, moverState, level.time );

	//Only do the angles if specified
	if ( angles != NULL )
	{
		//
		// Rotation

		for ( i = 0; i < 3; i++ )
		{
			ang[i] = AngleDelta( angles[i], ent->currentAngles[i] );
			ent->s.apos.trDelta[i] = ( ang[i] / ( duration * 0.001f ) );
		}

		VectorCopy( ent->currentAngles, ent->s.apos.trBase );

		ent->s.apos.trType = TR_LINEAR_STOP;
		ent->s.apos.trDuration = duration;

		ent->s.apos.trTime = level.time;

		ent->e_ReachedFunc = reachedF_moveAndRotateCallback;
		Q3_TaskIDSet( ent, TID_ANGLE_FACE, taskID );
	}
	else
	{
		//Setup the last bits of information
		ent->e_ReachedFunc  = reachedF_moverCallback;
	}

	if ( ent->damage )
	{
		ent->e_BlockedFunc = blockedF_Blocked_Mover;
	}

	Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
	// starting sound
	G_PlayDoorLoopSound( ent );
	G_PlayDoorSound( ent, BMS_START );	//??

	gi.linkentity( ent );

}

/*
=============
Q3_Lerp2Origin

Lerps the origin to the destination value
=============
*/
static void Q3_Lerp2Origin( int taskID, int entID, vec3_t origin, float duration )
{
	gentity_t	*ent = &g_entities[entID];

	if(!ent)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Lerp2Origin: invalid entID %d\n", entID);
		return;
	}
	
	if ( ent->client || ent->NPC || Q_stricmp(ent->classname, "target_scriptrunner") == 0 )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Lerp2Origin: ent %d is NOT a mover!\n", entID);
		return;
	}

	if ( ent->s.eType != ET_MOVER )
	{
		ent->s.eType = ET_MOVER;
	}

	moverState_t moverState = ent->moverState;

	if ( moverState == MOVER_POS1 || moverState == MOVER_2TO1 )
	{
		VectorCopy( ent->currentOrigin, ent->pos1 );
		VectorCopy( origin, ent->pos2 );

		if ( moverState == MOVER_POS1 )
		{//open the portal
			gi.AdjustAreaPortalState( ent, qtrue );
		}

		moverState = MOVER_1TO2;
	}
	else if ( moverState == MOVER_POS2 || moverState == MOVER_1TO2 )
	{
		VectorCopy( ent->currentOrigin, ent->pos2 );
		VectorCopy( origin, ent->pos1 );

		moverState = MOVER_2TO1;
	}

	InitMoverTrData( ent );	//FIXME: This will probably break normal things that are being moved...

	ent->s.pos.trDuration = duration;

	// start it going
	MatchTeam( ent, moverState, level.time );
	//SetMoverState( ent, moverState, level.time );

	ent->e_ReachedFunc  = reachedF_moverCallback;
	if ( ent->damage )
	{
		ent->e_BlockedFunc = blockedF_Blocked_Mover;
	}
	Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
	// starting sound
	G_PlayDoorLoopSound( ent );//start looping sound
	G_PlayDoorSound( ent, BMS_START );	//play start sound

	gi.linkentity( ent );
}

/*
=============
Q3_LerpAngles

Lerps the angles to the destination value
=============
*/
static void Q3_Lerp2Angles( int taskID, int entID, vec3_t angles, float duration )
{
	gentity_t	*ent = &g_entities[entID];
	vec3_t		ang;
	int			i;

	if(!ent)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Lerp2Angles: invalid entID %d\n", entID);
		return;
	}
	
	if ( ent->client || ent->NPC || Q_stricmp(ent->classname, "target_scriptrunner") == 0 )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Lerp2Angles: ent %d is NOT a mover!\n", entID);
		return;
	}

	//If we want an instant move, don't send 0...
	ent->s.apos.trDuration = (duration>0) ? duration : 1;

	for ( i = 0; i < 3; i++ )
	{
		ang [i] = AngleSubtract( angles[i], ent->currentAngles[i]);
		ent->s.apos.trDelta[i] = ( ang[i] / ( ent->s.apos.trDuration * 0.001f ) );
	}

	VectorCopy( ent->currentAngles, ent->s.apos.trBase );

	ent->s.apos.trType = TR_LINEAR_STOP;

	ent->s.apos.trTime = level.time;
	
	Q3_TaskIDSet( ent, TID_ANGLE_FACE, taskID );

	ent->e_ReachedFunc = reachedF_NULL;
	ent->e_ThinkFunc = thinkF_anglerCallback;
	ent->nextthink = level.time + duration;

	gi.linkentity( ent );
}

/*
=============
Q3_GetTag

Gets the value of a tag by the give name
=============
*/
static int	Q3_GetTag( int entID, const char *name, int lookup, vec3_t info )
{
	gentity_t	*ent = &g_entities[ entID ];

	VALIDATEB( ent );

	switch ( lookup )
	{
	case TYPE_ORIGIN:
		//return TAG_GetOrigin( ent->targetname, name, info );
		return TAG_GetOrigin( ent->ownername, name, info );
		break;

	case TYPE_ANGLES:
		//return TAG_GetAngles( ent->targetname, name, info );
		return TAG_GetAngles( ent->ownername, name, info );
		break;	
	}

	return false;
}

/*
=============
Q3_SetNavGoal

Sets the navigational goal of an entity
=============
*/
static void Q3_SetNavGoal( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[ entID ];
	vec3_t		goalPos;

	//Get the position of the goal
	if ( TAG_GetOrigin2( NULL, name, goalPos ) == false )
	{
		gentity_t	*targ = G_Find(NULL, FOFS(targetname), (char*)name);
		if ( !targ )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetNavGoal: can't find NAVGOAL \"%s\"\n", name );
			return;
		}
		else if ( ent->NPC )
		{
			ent->NPC->goalEntity = targ;
			ent->NPC->goalRadius = sqrt(ent->maxs[0]+ent->maxs[0]) + sqrt(targ->maxs[0]+targ->maxs[0]);
			ent->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
			return;
		}
	}
	else
	{
		int	goalRadius = TAG_GetRadius( NULL, name );

		if ( ent->NPC )
		{
			NPC_SetMoveGoal( ent, goalPos, goalRadius, qtrue );
			//We know we want to clear the lastWaypoint here
			ent->NPC->goalEntity->lastWaypoint = WAYPOINT_NONE;
			ent->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
			return;
		}
	}

	Q3_DebugPrint( WL_ERROR, "Q3_SetNavGoal: tried to set a navgoal (\"%s\") on a non-NPC: \"%s\"\n", name, ent->script_targetname );
}

//-----------------------------------------------

/*
============
SetLowerAnim
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int animID
============
*/
static void SetLowerAnim( int entID, int animID)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "SetLowerAnim: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "SetLowerAnim: ent %d is NOT a player or NPC!\n", entID);
		return;
	}

	NPC_SetAnim(ent,SETANIM_LEGS,animID,SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE);
}


/*
============
SetUpperAnim 
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int animID
============
*/
static void SetUpperAnim ( int entID, int animID)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "SetUpperAnim: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "SetLowerAnim: ent %d is NOT a player or NPC!\n", entID);
		return;
	}

	NPC_SetAnim(ent,SETANIM_TORSO,animID,SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE);
}

//-----------------------------------------------

/*
=============
Q3_SetAnimUpper

Sets the upper animation of an entity
=============
*/
static qboolean Q3_SetAnimUpper( int entID, const char *anim_name )
{
	int			animID = 0;

	animID = GetIDForString( animTable, anim_name );

	if( animID == -1 )
	{
		animID = GetIDForString( animTableOld, anim_name );
		if( animID == -1 )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_SetAnimUpper: unknown animation sequence '%s'\n", anim_name );
			return qfalse;
		}
		else
		{//tell them the right string!
			Q3_DebugPrint( WL_WARNING, "WARNING!  %s is an outdated enum name, please update your script to read %s!\n", anim_name, ENUM2STRING(animID) );
		}
	}
	
	if ( !PM_HasAnimation( &g_entities[entID], animID ) )
	{
		return qfalse;
	}

	SetUpperAnim( entID, animID );
	return qtrue;
}


/*
=============
Q3_SetAnimLower

Sets the lower animation of an entity
=============
*/
static qboolean Q3_SetAnimLower( int entID, const char *anim_name )
{
	int			animID = 0;

	//FIXME: Setting duck anim does not actually duck!

	animID = GetIDForString( animTable, anim_name );

	if( animID == -1 )
	{
		animID = GetIDForString( animTableOld, anim_name );
		if( animID == -1 )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_SetAnimLower: unknown animation sequence '%s'\n", anim_name );
			return qfalse;
		}
		else
		{//FIXME: tell them the right string!
			Q3_DebugPrint( WL_WARNING, "WARNING!  %s is an outdated enum name, please update your script!\n", anim_name );
		}
	}
	
	if ( !PM_HasAnimation( &g_entities[entID], animID ) )
	{
		return qfalse;
	}

	SetLowerAnim( entID, animID );
	return qtrue;
}

/*
============
Q3_SetAnimHoldTime
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int int_data
  Argument		: qboolean lower
============
*/
extern void PM_SetTorsoAnimTimer( gentity_t *ent, int *torsoAnimTimer, int time );
extern void PM_SetLegsAnimTimer( gentity_t *ent, int *legsAnimTimer, int time );
static void Q3_SetAnimHoldTime( int entID, int int_data, qboolean lower )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetAnimHoldTime: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetAnimHoldTime: ent %d is NOT a player or NPC!\n", entID);
		return;
	}
	
	if(lower)
	{
		PM_SetLegsAnimTimer( ent, &ent->client->ps.legsAnimTimer, int_data );
	}
	else
	{
		PM_SetTorsoAnimTimer( ent, &ent->client->ps.torsoAnimTimer, int_data );
	}
}

/*
=============
Q3_SetEnemy

Sets the enemy of an entity
=============
*/
static void Q3_SetEnemy( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEnemy: invalid entID %d\n", entID);
		return;
	}

	if( !Q_stricmp("NONE", name) || !Q_stricmp("NULL", name))
	{
		if(ent->NPC)
		{
			G_ClearEnemy(ent);
		}
		else
		{
			ent->enemy = NULL;
		}
	}
	else
	{
		gentity_t	*enemy = G_Find( NULL, FOFS(targetname), (char *) name);

		if(enemy == NULL)
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEnemy: no such enemy: '%s'\n", name );
			return;
		}
		/*else if(enemy->health <= 0)
		{
			//Q3_DebugPrint( WL_ERROR, "Q3_SetEnemy: ERROR - desired enemy has health %d\n", enemy->health );
			return;
		}*/
		else
		{
			if(ent->NPC)
			{
				G_SetEnemy( ent, enemy );
				ent->cantHitEnemyCounter = 0;
			}
			else
			{
				G_SetEnemy(ent, enemy);
			}
		}
	}
}

/*
=============
Q3_SetEnemy

Sets the enemy of an entity
=============
*/
/*
static void Q3_SetDefendTarget( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[entID];
	gentity_t	*defendEnt  = NULL;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDefendTarget: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDefendTarget: ent %d is NOT an NPC!\n", entID);
		return;
	}

	defendEnt = G_Find(NULL, FOFS(targetname), (char *)name);
	if ( defendEnt != NULL )
	{
		ent->NPC->defendEnt = defendEnt;
	}
	else
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDefendTarget: %s NOT found!\n", name);
	}
}
*/

/*
=============
Q3_SetLeader

Sets the leader of an NPC
=============
*/
static void Q3_SetLeader( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLeader: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetLeader: ent %d is NOT a player or NPC!\n", entID);
		return;
	}

	if( !Q_stricmp("NONE", name) || !Q_stricmp("NULL", name))
	{
		ent->client->leader = NULL;
	}
	else
	{
		gentity_t	*leader = G_Find( NULL, FOFS(targetname), (char *) name);

		if(leader == NULL)
		{
			//Q3_DebugPrint( WL_ERROR,"Q3_SetEnemy: unable to locate enemy: '%s'\n", name );
			return;
		}
		else if(leader->health <= 0)
		{
			//Q3_DebugPrint( WL_ERROR,"Q3_SetEnemy: ERROR - desired enemy has health %d\n", enemy->health );
			return;
		}
		else
		{
			ent->client->leader = leader;
		}
	}
}

stringID_table_t teamTable [] = 
{
	ENUM2STRING(TEAM_FREE),
	ENUM2STRING(TEAM_STARFLEET),
	ENUM2STRING(TEAM_BORG),
	ENUM2STRING(TEAM_PARASITE),
	ENUM2STRING(TEAM_SCAVENGERS),
	ENUM2STRING(TEAM_KLINGON),
	ENUM2STRING(TEAM_MALON),
	ENUM2STRING(TEAM_HIROGEN),
	ENUM2STRING(TEAM_IMPERIAL),
	ENUM2STRING(TEAM_STASIS),
	ENUM2STRING(TEAM_8472),
	ENUM2STRING(TEAM_BOTS),
	ENUM2STRING(TEAM_FORGE),
	ENUM2STRING(TEAM_DISGUISE),
	"", TEAM_FREE,
};


/*
============
Q3_SetPlayerTeam
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: const char *teamName
============
*/
static void Q3_SetPlayerTeam( int entID, const char *teamName )
{
	gentity_t	*ent  = &g_entities[entID];
	team_t		newTeam;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetPlayerTeam: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetPlayerTeam: ent %d is NOT a player or NPC!\n", entID);
		return;
	}

	newTeam = (team_t)GetIDForString( teamTable, teamName );

	if ( ent->s.number == 0 )
	{//Disguise hack
		if ( ent->client->playerTeam == TEAM_STARFLEET && newTeam == TEAM_DISGUISE )
		{
			//need to set an anim too!  Else when change models current frame is out of range
			SetUpperAnim( 0, TORSO_WEAPONIDLE3 );
			SetLowerAnim( 0, BOTH_STAND2 );

			if ( g_sex->string[0] == 'f' )
			{
				gi.SendConsoleCommand( "headModel alexascav/default; torsoModel impfem/default; legsModel impfem/default\n");
			}
			else
			{
				gi.SendConsoleCommand( "headModel munroscav/default; torsoModel imperial/raider; legsModel imperial/raider\n");
			}
			
			//need to set the weapon too
			Q3_SetWeapon( 0, "scavenger" );
		}
		else if ( ent->client->playerTeam == TEAM_DISGUISE && newTeam == TEAM_STARFLEET )
		{
			//need to set an anim too!  Else when change models current frame is out of range
			SetUpperAnim( 0, TORSO_WEAPONIDLE3 );
			SetLowerAnim( 0, BOTH_STAND2 );

			if ( g_sex->string[0] == 'f' )
			{
				gi.SendConsoleCommand( "headModel alexa/default; torsoModel hazardfemale/default; legsModel hazardfemale/default\n");
			}
			else
			{
				gi.SendConsoleCommand( "headModel munro/default; torsoModel hazard/default; legsModel hazard/default\n");
			}
		}
	}

	ent->client->playerTeam = newTeam;
}


/*
============
Q3_SetEnemyTeam
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: const char *teamName
============
*/
static void Q3_SetEnemyTeam( int entID, const char *teamName )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEnemyTeam: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetEnemyTeam: ent %d is NOT a player or NPC!\n", entID);
		return;
	}
	
	ent->client->enemyTeam = (team_t)GetIDForString( teamTable, teamName );
}


/*
============
Q3_SetHealth
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int data
============
*/
static void Q3_SetHealth( int entID, int data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetHealth: invalid entID %d\n", entID);
		return;
	}
	
	ent->health = data;

	if(!ent->client)
	{
		return;
	}

	ent->client->ps.stats[STAT_HEALTH] = data;
	if ( ent->s.number == 0 )
	{//clamp health to max		
		if ( ent->client->ps.stats[STAT_HEALTH] > ent->client->ps.stats[STAT_MAX_HEALTH] )
		{
			ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( data <= 0 )
		{//artificially "killing" the player", don't let him respawn right away
			ent->client->ps.pm_type = PM_DEAD;
			//delay respawn for 2 seconds
			ent->client->respawnTime = level.time + 2000;
			//stop all scripts
			if (Q_stricmpn(level.mapname,"_holo",5)) {
				stop_icarus = qtrue;
			}
			//make the team killable
			G_MakeTeamVulnerable();
		}
	}
}

/*
============
Q3_SetArmor
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int data
============
*/
static void Q3_SetArmor( int entID, int data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetArmor: invalid entID %d\n", entID);
		return;
	}
	
	if(!ent->client)
	{
		return;
	}

	ent->client->ps.stats[STAT_ARMOR] = data;
	if ( ent->s.number == 0 )
	{//clamp armor to max_health
		if ( ent->client->ps.stats[STAT_ARMOR] > ent->client->ps.stats[STAT_MAX_HEALTH] )
		{
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
	}
}

/*
============
Q3_SetBState
  Description	: 
  Return type	: static qboolean 
  Argument		:  int entID
  Argument		: const char *bs_name
FIXME: this should be a general NPC wrapper function 
	that is called ANY time	a bState is changed...
============
*/
static qboolean Q3_SetBState( int entID, const char *bs_name )
{
	gentity_t	*ent  = &g_entities[entID];
	bState_t	bSID;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetBState: invalid entID %d\n", entID);
		return qtrue;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetBState: '%s' is not an NPC\n", ent->targetname );
		return qtrue;//ok to complete
	}

	bSID = (bState_t)(GetIDForString( BSTable, bs_name ));
	if ( bSID > -1 )
	{
		if ( bSID == BS_FORMATION )
		{
			ent->NPC->aiFlags &= ~NPCAI_FORM_TELE_NAV;
			VectorClear( ent->NPC->leaderTeleportSpot );
		}
		else
		{
			ent->NPC->scriptFlags &= ~SCF_CAREFUL;//So they don't keep using walk2, stand2, run2, etc.
		}

		if ( bSID == BS_SEARCH || bSID == BS_WANDER )
		{
			//FIXME: Reimplement
			
			if( ent->waypoint != WAYPOINT_NONE )
			{
				NPC_BSSearchStart( ent->waypoint, bSID );
			}
			else
			{
				ent->waypoint = NAV_FindClosestWaypointForEnt( ent, WAYPOINT_NONE );

				if( ent->waypoint != WAYPOINT_NONE )
				{
					NPC_BSSearchStart( ent->waypoint, bSID );
				}
				/*else if( ent->lastWaypoint >=0 && ent->lastWaypoint < num_waypoints )
				{
					NPC_BSSearchStart( ent->lastWaypoint, bSID );
				}
				else if( ent->lastValidWaypoint >=0 && ent->lastValidWaypoint < num_waypoints )
				{
					NPC_BSSearchStart( ent->lastValidWaypoint, bSID );
				}*/
				else
				{
					Q3_DebugPrint( WL_ERROR, "Q3_SetBState: '%s' is not in a valid waypoint to search from!\n", ent->targetname );
					return qtrue;
				}
			}
		}
		

		ent->NPC->tempBehavior = BS_DEFAULT;//need to clear any temp behaviour
		if ( ent->NPC->behaviorState == BS_NOCLIP && bSID != BS_NOCLIP )
		{//need to rise up out of the floor after noclipping
			ent->currentOrigin[2] += 0.125;
			G_SetOrigin( ent, ent->currentOrigin );
		}
		ent->NPC->behaviorState = bSID;
	}

	ent->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;

	if ( bSID == BS_FLY )
	{//FIXME: need a set bState wrapper
		ent->NPC->stats.moveType = MT_FLYSWIM;
	}
	else
	{
		//FIXME: these are presumptions!
		//Q3_SetGravity( entID, g_gravity->value );
		//ent->NPC->stats.moveType = MT_RUNJUMP;
	}

	if ( bSID == BS_NOCLIP )
	{
		ent->client->noclip = qtrue;
	}
	else
	{
		ent->client->noclip = qfalse;
	}

	if ( bSID == BS_FACE || bSID == BS_POINT_AND_SHOOT || bSID == BS_FACE_ENEMY )
	{
		ent->NPC->aimTime = level.time + 5 * 1000;//try for 5 seconds
		return qfalse;//need to wait for task complete message
	}

	if ( bSID == BS_WAITHEAL || bSID == BS_SNIPER || bSID == BS_MEDIC || bSID == BS_ADVANCE_FIGHT )
	{
		return qfalse;//need to wait for task complete message
	}

	if ( bSID == BS_SHOOT || bSID == BS_POINT_AND_SHOOT )
	{//Let them shoot right NOW
		ent->NPC->shotTime = ent->attackDebounceTime = level.time;
	}

	if ( bSID == BS_JUMP )
	{
		ent->NPC->jumpState = JS_FACING;
	}

	return qtrue;//ok to complete
}


/*
============
Q3_SetTempBState
  Description	: 
  Return type	: static qboolean 
  Argument		:  int entID
  Argument		: const char *bs_name
============
*/
static qboolean Q3_SetTempBState( int entID, const char *bs_name )
{
	gentity_t	*ent  = &g_entities[entID];
	bState_t	bSID;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetTempBState: invalid entID %d\n", entID);
		return qtrue;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetTempBState: '%s' is not an NPC\n", ent->targetname );
		return qtrue;//ok to complete
	}

	bSID = (bState_t)(GetIDForString( BSTable, bs_name ));
	if ( bSID > -1 )
	{
		ent->NPC->tempBehavior = bSID;
	}

	if ( bSID == BS_FACE || bSID == BS_POINT_AND_SHOOT || bSID == BS_FACE_ENEMY )
	{
		ent->NPC->aimTime = level.time + 5 * 1000;//try for 5 seconds
		return qfalse;//need to wait for task complete message
	}

	if ( bSID == BS_SHOOT || bSID == BS_POINT_AND_SHOOT )
	{//Let them shoot right NOW
		ent->NPC->shotTime = ent->attackDebounceTime = level.time;
	}

	return qtrue;//ok to complete
}


/*
============
Q3_SetDefaultBState
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: const char *bs_name
============
*/
static void Q3_SetDefaultBState( int entID, const char *bs_name )
{
	gentity_t	*ent  = &g_entities[entID];
	bState_t	bSID;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDefaultBState: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetDefaultBState: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	bSID = (bState_t)(GetIDForString( BSTable, bs_name ));
	if ( bSID > -1 )
	{
		ent->NPC->defaultBehavior = bSID;
	}
}


/*
============
Q3_SetDPitch
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetDPitch( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDPitch: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC || !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetDPitch: '%s' is not an NPC\n", ent->targetname );
		return;
	}
	
	int pitchMin = -ent->client->renderInfo.headPitchRangeUp + 1;
	int pitchMax = ent->client->renderInfo.headPitchRangeDown - 1;

	//clamp angle to -180 -> 180
	data = AngleNormalize180( data );

	//Clamp it to my valid range
	if ( data < -1 )
	{
		if ( data < pitchMin )
		{
			data = pitchMin;
		}
	}
	else if ( data > 1 )
	{
		if ( data > pitchMax )
		{
			data = pitchMax;
		}
	}

	ent->NPC->lockedDesiredPitch = ent->NPC->desiredPitch = data;
}


/*
============
Q3_SetDYaw
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetDYaw( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];
	
	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDYaw: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetDYaw: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	if(!ent->enemy)
	{//don't mess with this if they're aiming at someone
		ent->NPC->lockedDesiredYaw = ent->NPC->desiredYaw = data;
	}
	else
	{
		Q3_DebugPrint( WL_WARNING, "Could not set DYAW: '%s' has an enemy (%s)!\n", ent->targetname, ent->enemy->targetname );
	}
}


/*
============
Q3_SetShootDist
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetShootDist( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetShootDist: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetShootDist: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.shootDistance = data;
}


/*
============
Q3_SetVisrange
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetVisrange( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetVisrange: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetVisrange: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.visrange = data;
}


/*
============
Q3_SetEarshot
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetEarshot( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEarshot: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetEarshot: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.earshot = data;
}


/*
============
Q3_SetVigilance
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: float data
============
*/
static void Q3_SetVigilance( int entID, float data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetVigilance: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetVigilance: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.vigilance = data;
}


/*
============
Q3_SetVFOV
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int data
============
*/
static void Q3_SetVFOV( int entID, int data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetVFOV: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetVFOV: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.vfov = data;
}


/*
============
Q3_SetHFOV
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int data
============
*/
static void Q3_SetHFOV( int entID, int data )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetHFOV: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetHFOV: '%s' is not an NPC\n", ent->targetname );
		return;
	}

	ent->NPC->stats.hfov = data;
}


/*
============
Q3_GetTimeScale
  Description	: 
  Return type	: static DWORD 
  Argument		: void
============
*/
static DWORD Q3_GetTimeScale( void )
{
	//return	Q3_TIME_SCALE;
	return g_timescale->value;
}


/*
============
Q3_SetTimeScale
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: const char *data
============
*/
static void Q3_SetTimeScale( int entID, const char *data )
{
	gi.cvar_set("timescale", data);
}


/*
============
Q3_SetInvisible
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: qboolean invisible
============
*/
static void Q3_SetInvisible( int entID, qboolean invisible )
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetInvisible: invalid entID %d\n", entID);
		return;
	}
	
	if ( invisible )
	{
		self->s.eFlags |= EF_NODRAW;
		if ( self->client )
		{
			self->client->ps.eFlags |= EF_NODRAW;
		}
		self->contents = 0;
	}
	else
	{
		self->s.eFlags &= ~EF_NODRAW;
		if ( self->client )
		{
			self->client->ps.eFlags &= ~EF_NODRAW;
		}
	}
}


/*
============
Q3_SetGreetAllies
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: qboolean greet
============
*/
static void Q3_SetGreetAllies( int entID, qboolean greet )
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetGreetAllies: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetGreetAllies: ent %s is not an NPC!\n", self->targetname );
		return;
	}

	if ( greet )
	{
		self->NPC->aiFlags |= NPCAI_GREET_ALLIES;
	}
	else
	{
		self->NPC->aiFlags &= ~NPCAI_GREET_ALLIES;
	}
}


/*
============
Q3_SetViewTarget 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *name
============
*/
static void Q3_SetViewTarget (int entID, const char *name)
{
	gentity_t	*self  = &g_entities[entID];
	gentity_t	*viewtarget = G_Find( NULL, FOFS(targetname), (char *) name);
	vec3_t		viewspot, selfspot, viewvec, viewangles;

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetViewTarget: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetViewTarget: '%s' is not a player/NPC!\n", self->targetname );
		return;
	}
	
	//FIXME: Exception handle here
	if (viewtarget == NULL)
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetViewTarget: can't find ViewTarget: '%s'\n", name );
		return;
	}
	
	//FIXME: should we set behavior to BS_FACE and keep facing this ent as it moves
	//around for a script-specified length of time...?
	VectorCopy ( self->currentOrigin, selfspot );
	selfspot[2] += self->client->ps.viewheight;

	if ( viewtarget->client && (!g_skippingcin || !g_skippingcin->integer ) ) 
	{
		VectorCopy ( viewtarget->client->renderInfo.eyePoint, viewspot );
	}
	else
	{
		VectorCopy ( viewtarget->currentOrigin, viewspot );
	}
	
	VectorSubtract( viewspot, selfspot, viewvec );
	
	vectoangles( viewvec, viewangles );

	Q3_SetDYaw( entID, viewangles[YAW] );
	if ( !g_skippingcin || !g_skippingcin->integer )
	{
		Q3_SetDPitch( entID, viewangles[PITCH] );
	}
}


/*
============
Q3_SetWeapon 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *wp_name
============
*/
void TossClientItems( gentity_t *self );
static void Q3_SetWeapon (int entID, const char *wp_name)
{
	gentity_t	*self  = &g_entities[entID];
	int		wp;

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetWeapon: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetWeapon: '%s' is not a player/NPC!\n", self->targetname );
		return;
	}

	if(!Q_stricmp("drop", wp_name))
	{//no weapon, drop it
		TossClientItems( self );
		self->client->ps.weapon = WP_NONE;
		return;
	}

	wp = GetIDForString( WPTable, wp_name );
	if(wp == WP_NONE)
	{//no weapon
		self->client->ps.weapon = WP_NONE;
		return;
	}

	/*
	if(wp < WP_PHASER || wp > WP_TRICORDER)
	{
		return;
	}
	*/
	gitem_t *item = FindItemForWeapon( (weapon_t) wp);
	RegisterItem( item );	//make sure the weapon is cached in case this runs at startup

	if(self->NPC)
	{//Should NPCs have only 1 weapon at a time?
		self->client->ps.stats[STAT_WEAPONS] = ( 1 << wp );
		self->client->ps.ammo[weaponData[wp].ammoIndex] = 999;

		ChangeWeapon( self, wp );
		self->client->ps.weapon = wp;
		self->client->ps.weaponstate = WEAPON_READY;//WEAPON_RAISING;
		if ( G_WeaponChangeEffect( self ) )
		{//we did do the weapon change effect sound
			// This sound should already be pre-cached
			G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/change.wav" ));
		}
	}
	else
	{
		//Take away the hypos
		self->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_BLUE_HYPO );
		self->client->ps.stats[STAT_WEAPONS] &= ~( 1 << WP_RED_HYPO );

		self->client->ps.stats[STAT_WEAPONS] |= ( 1 << wp );
		self->client->ps.ammo[weaponData[wp].ammoIndex] = ammoData[weaponData[wp].ammoIndex].max;

		G_AddEvent( self, EV_ITEM_PICKUP, (item - bg_itemlist) );
		//force it to change
		CG_ChangeWeapon( wp );

		if ( G_WeaponChangeEffect( self ) )
		{
			G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/change.wav" ));
		}
	}
}


/*
============
Q3_SetWalkSpeed 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: int int_data
============
*/
static void Q3_SetWalkSpeed (int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetWalkSpeed: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetWalkSpeed: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	if(int_data == 0)
	{
		self->NPC->stats.walkSpeed = self->client->ps.speed = 1;
	}

	self->NPC->stats.walkSpeed = self->client->ps.speed = int_data;
}


/*
============
Q3_SetRunSpeed 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: int int_data
============
*/
static void Q3_SetRunSpeed (int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetRunSpeed: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetRunSpeed: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	if(int_data == 0)
	{
		self->NPC->stats.runSpeed = self->client->ps.speed = 1;
	}

	self->NPC->stats.runSpeed = self->client->ps.speed = int_data;
}


/*
============
Q3_SetYawSpeed 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: float float_data
============
*/
static void Q3_SetYawSpeed (int entID, float float_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetYawSpeed: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetYawSpeed: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	self->NPC->stats.yawSpeed = float_data;
}


/*
============
Q3_SetAggression
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: int int_data
============
*/
static void Q3_SetAggression(int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];


	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetAggression: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetAggression: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	if(int_data < 1 || int_data > 5)
		return;

	self->NPC->stats.aggression = int_data;
}


/*
============
Q3_SetAim
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: int int_data
============
*/
static void Q3_SetAim(int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetAim: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetAim: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	if(int_data < 1 || int_data > 5)
		return;

	self->NPC->stats.aim = int_data;
}


/*
============
Q3_SetFriction
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: int int_data
============
*/
static void Q3_SetFriction(int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetFriction: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetFriction: '%s' is not an NPC/player!\n", self->targetname );
		return;
	}

	self->client->ps.friction = int_data;
}


/*
============
Q3_SetGravity
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: float float_data
============
*/
static void Q3_SetGravity(int entID, float float_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetGravity: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetGravity: '%s' is not an NPC/player!\n", self->targetname );
		return;
	}

	//FIXME: what if we want to return them to normal global gravity?
	self->svFlags |= SVF_CUSTOM_GRAVITY;
	self->client->ps.gravity = float_data;
}


/*
============
Q3_SetWait
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: float float_data
============
*/
static void Q3_SetWait(int entID, float float_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetWait: invalid entID %d\n", entID);
		return;
	}
	
	self->wait = float_data;
}


static void Q3_SetShotSpacing(int entID, int int_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetShotSpacing: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetShotSpacing: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	self->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
	self->NPC->burstSpacing = int_data;
}

/*
============
Q3_SetFollowDist
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: float float_data
============
*/
static void Q3_SetFollowDist(int entID, float float_data)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetFollowDist: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client || !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetFollowDist: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	self->NPC->followDist = float_data;
}


/*
============
Q3_SetCount
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *data
============
*/
static void Q3_SetCount(int entID, const char *data)
{
	gentity_t	*self  = &g_entities[entID];
	float		val = 0.0f;

	//FIXME: use FOFS() stuff here to make a generic entity field setting?
	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCount: invalid entID %d\n", entID);
		return;
	}
	
	if ( (val = Q3_CheckStringCounterIncrement( data )) )
	{
		self->count += (int)(val);
	}
	else
	{
		self->count = atoi((char *) data);
	}
}


/*
============
Q3_SetSquadName 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *squadname
============
*/
static void Q3_SetSquadName (int entID, const char *squadname)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetSquadName: invalid entID %d\n", entID);
		return;
	}
	
	if ( !self->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetSquadName: '%s' is not an NPC/player!\n", self->targetname );
		return;
	}

	if(!Q_stricmp("NULL", ((char *)squadname)))
	{
		self->client->squadname = NULL;
	}
	else
	{
		self->client->squadname = G_NewString(squadname);
	}
}


/*
============
Q3_SetTargetName 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *targetname
============
*/
static void Q3_SetTargetName (int entID, const char *targetname)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetTargetName: invalid entID %d\n", entID);
		return;
	}

	if(!Q_stricmp("NULL", ((char *)targetname)))
	{
		self->targetname = NULL;
	}
	else
	{
		self->targetname = G_NewString( targetname );
	}

	/*
	if ( self->NPC )
	{//uhh, this will may screw up ICARUS' associated ent list
		self->script_targetname = self->targetname;
	}
	*/
}


/*
============
Q3_SetTarget 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *target
============
*/
static void Q3_SetTarget (int entID, const char *target)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetTarget: invalid entID %d\n", entID);
		return;
	}

	if(!Q_stricmp("NULL", ((char *)target)))
	{
		self->target = NULL;
	}
	else
	{
		self->target = G_NewString( target );
	}
}

/*
============
Q3_SetTarget2
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *target
============
*/
static void Q3_SetTarget2 (int entID, const char *target2)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetTarget2: invalid entID %d\n", entID);
		return;
	}

	if(!Q_stricmp("NULL", ((char *)target2)))
	{
		self->target2 = NULL;
	}
	else
	{
		self->target2 = G_NewString( target2 );
	}
}
/*
============
Q3_SetRemoveTarget 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *target
============
*/
static void Q3_SetRemoveTarget (int entID, const char *target)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetRemoveTarget: invalid entID %d\n", entID);
		return;
	}

	if ( !self->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetRemoveTarget: '%s' is not an NPC!\n", self->targetname );
		return;
	}

	if( !Q_stricmp("NULL", ((char *)target)) )
	{
		self->target3 = NULL;
	}
	else
	{
		self->target3 = G_NewString( target );
	}
}


/*
============
Q3_SetPainTarget 
  Description	: 
  Return type	: void 
  Argument		: int entID
  Argument		: const char *targetname
============
*/
static void Q3_SetPainTarget (int entID, const char *targetname)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetPainTarget: invalid entID %d\n", entID);
		return;
	}

	if(Q_stricmp("NULL", ((char *)targetname)) == 0)
	{
		self->paintarget = NULL;
	}
	else
	{
		self->paintarget = G_NewString((char *)targetname);
	}
}

/*
============
Q3_SetFullName 
  Description	: 
  Return type	: static void 
  Argument		: int entID
  Argument		: const char *fullName
============
*/
static void Q3_SetFullName (int entID, const char *fullName)
{
	gentity_t	*self  = &g_entities[entID];

	if ( !self )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetFullName: invalid entID %d\n", entID);
		return;
	}

	if(!Q_stricmp("NULL", ((char *)fullName)))
	{
		self->fullName = NULL;
	}
	else
	{
		self->fullName = G_NewString( fullName );
	}
}

/*
============
Q3_SetParm
  Description	: 
  Return type	: void 
  Argument		: int entID
  Argument		: int parmNum
  Argument		: const char *parmValue
============
*/
void Q3_SetParm (int entID, int parmNum, const char *parmValue)
{
	gentity_t	*ent = &g_entities[entID];
	float		val;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetParm: invalid entID %d\n", entID);
		return;
	}

	if ( parmNum < 0 || parmNum >= MAX_PARMS )
	{
		Q3_DebugPrint( WL_WARNING, "SET_PARM: parmNum %d out of range!\n", parmNum );
		return;
	}

	if( !ent->parms )
	{
		ent->parms = (parms_t *)G_Alloc( sizeof(parms_t) );
		memset( ent->parms, 0, sizeof(parms_t) );
	}

	if ( (val = Q3_CheckStringCounterIncrement( parmValue )) )
	{
		val += atof( ent->parms->parm[parmNum] );
		sprintf( ent->parms->parm[parmNum], "%f", val );
	}
	else
	{//Just copy the string
		//copy only 16 characters
		strncpy( ent->parms->parm[parmNum], parmValue, sizeof(ent->parms->parm[0]) );
		//set the last charcter to null in case we had to truncate their passed string
		if ( ent->parms->parm[parmNum][sizeof(ent->parms->parm[0]) - 1] != 0 )
		{//Tried to set a string that is too long
			ent->parms->parm[parmNum][sizeof(ent->parms->parm[0]) - 1] = 0;
			Q3_DebugPrint( WL_WARNING, "SET_PARM: parm%d string too long, truncated to '%s'!\n", parmNum, ent->parms->parm[parmNum] );
		}
	}
}


/*
=============
Q3_SetHidingGoal

Sets the hiding spot goal of an entity
=============
*/

static void Q3_SetHidingGoal( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[entID];
	gentity_t	*goal = G_Find( NULL, FOFS(targetname), (char *) name);

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetHidingGoal: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetHidingGoal: '%s' is not an NPC!\n", ent->targetname );
		return;
	}
	
	//FIXME: Exception handle here
	if (goal == NULL)
	{
		G_Error(  "Q3_SetHidingGoal: can't find HidingGoal target: '%s'\n", name );
		return;
	}

	if(ent->NPC)
	{
		ent->NPC->hidingGoal = goal;
	}
}

/*
=============
Q3_SetCaptureGoal

Sets the capture spot goal of an entity
=============
*/
static void Q3_SetCaptureGoal( int entID, const char *name )
{
	gentity_t	*ent  = &g_entities[entID];
	gentity_t	*goal = G_Find( NULL, FOFS(targetname), (char *) name);

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCaptureGoal: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCaptureGoal: '%s' is not an NPC!\n", ent->targetname );
		return;
	}
	
	//FIXME: Exception handle here
	if (goal == NULL)
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCaptureGoal: can't find CaptureGoal target: '%s'\n", name );
		return;
	}

	if(ent->NPC)
	{
		ent->NPC->captureGoal = goal;
		ent->NPC->goalEntity = goal;
		ent->NPC->goalTime = level.time + 100000;
//		NAV_ClearLastRoute(ent);
	}
}

/*
=============
Q3_SetEvent

?
=============
*/
static void Q3_SetEvent( int entID, const char *event_name )
{
	gentity_t	*ent  = &g_entities[entID];
	gentity_t	*tent  = NULL;
	int			event;
	vec3_t		spot;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEvent: invalid entID %d\n", entID);
		return;
	}
	
	event = GetIDForString( eventTable, event_name );
	switch( event )
	{
	case EV_BEAM_DOWN:
		/*
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}
		*/
 
		if ( ent->s.number != 0 )
		{//we don't want to remove the player
			tent = G_Spawn();
			tent->owner = ent;
			tent->e_ThinkFunc = thinkF_RemoveOwner;
			tent->nextthink = level.time + 4000;
		}

		if ( !ent->client )
		{
			tent = G_TempEntity( ent->currentOrigin, EV_PLAYER_TELEPORT_OUT );
		}
		else
		{
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		}
		tent->owner = ent;
		break;

	case EV_BEAM_UP:
		/*
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}
		*/

		if ( !ent->client )
		{
			tent = G_TempEntity( ent->currentOrigin, EV_PLAYER_TELEPORT_IN );
		}
		else
		{
			tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		}
		tent->owner = ent;
		break;

	case EV_SCAV_TELEPORT:
		if( VectorCompare( ent->currentOrigin, vec3_origin ) )
		{//Brush with no origin
			VectorSubtract( ent->absmax, ent->absmin, spot );
			VectorMA( ent->absmin, 0.5, spot, spot );
		}
		else
		{
			VectorCopy( ent->currentOrigin, spot );
		}
		tent = G_TempEntity( spot, EV_SCAV_BEAMING );
		tent->owner = ent;
		break;

	case EV_DISINTEGRATE:
		if( VectorCompare( ent->currentOrigin, vec3_origin ) )
		{//Brush with no origin
			VectorSubtract( ent->absmax, ent->absmin, spot );
			VectorMA( ent->absmin, 0.5, spot, spot );
		}
		else
		{
			VectorCopy( ent->currentOrigin, spot );
			spot[2] += ent->maxs[2]/2;
		}
		tent = G_TempEntity( spot, EV_DISINTEGRATION );
		tent->s.eventParm = PW_REGEN;
		tent->owner = ent;
		break;

	case EV_STASIS_BEAM_IN:
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}

		tent = G_TempEntity( ent->client->ps.origin, EV_STASIS_TELEPORT_IN );
		tent->owner = ent;
		break;

	case EV_STASIS_BEAM_OUT:
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}

		tent = G_TempEntity( ent->client->ps.origin, EV_STASIS_TELEPORT_OUT );
		tent->owner = ent;
		break;

	case EV_BORG_TELEPORT_FX:
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}
		FX_BorgTeleport( ent->client->ps.origin );
		break;

	case EV_8472_BEAM_OUT:
		if ( !ent->client )
		{
			Q3_DebugPrint( WL_ERROR, "Q3_SetEvent: '%s' is not an NPC/player!\n", ent->targetname );
			return;
		}

		ent->s.eFlags |= EF_SCALE_DOWN;
		ent->client->ps.eFlags |= EF_SCALE_DOWN;
		ent->fx_time = level.time;
		tent = G_TempEntity( ent->client->ps.origin, EV_SPECIES_TELEPORT );
		tent->s.eventParm = 0;
		tent->owner = ent;
		break;

	case EV_BAD:
	default:
		//Q3_DebugPrint( WL_ERROR,"Q3_SetEvent: Invalid Event %d\n", event );
		return;
		break;
	}
}

/*
============
Q3_Use

Uses an entity
============
*/
static void Q3_Use( int entID, const char *target )
{
	gentity_t	*ent  = &g_entities[entID];
	
	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Use: invalid entID %d\n", entID);
		return;
	}

	if( !target || !target[0] )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Use: string is NULL!\n" );
		return;
	}

	G_UseTargets2(ent, ent, target);
}


/*
============
Q3_SetBehaviorSet

?
============
*/
static qboolean Q3_SetBehaviorSet( int entID, int toSet, const char *scriptname)
{
	gentity_t	*ent  = &g_entities[entID];
	bSet_t		bSet = BSET_INVALID;

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetBehaviorSet: invalid entID %d\n", entID);
		return qfalse;
	}

	switch(toSet)
	{
	case SET_SPAWNSCRIPT:
		bSet = BSET_SPAWN;
		break;
	case SET_IDLESCRIPT:
		bSet = BSET_IDLE;
		break;
	case SET_TOUCHSCRIPT:
		bSet = BSET_TOUCH;
		break;
	case SET_USESCRIPT:
		bSet = BSET_USE;
		break;
	case SET_AWAKESCRIPT:
		bSet = BSET_AWAKE;
		break;
	case SET_ANGERSCRIPT:
		bSet = BSET_ANGER;
		break;
	case SET_ATTACKSCRIPT:
		bSet = BSET_ATTACK;
		break;
	case SET_VICTORYSCRIPT:
		bSet = BSET_VICTORY;
		break;
	case SET_LOSTENEMYSCRIPT:
		bSet = BSET_LOSTENEMY;
		break;
	case SET_PAINSCRIPT:
		bSet = BSET_PAIN;
		break;
	case SET_FLEESCRIPT:
		bSet = BSET_FLEE;
		break;
	case SET_DEATHSCRIPT:
		bSet = BSET_DEATH;
		break;
	case SET_DELAYEDSCRIPT:
		bSet = BSET_DELAYED;
		break;
	case SET_BLOCKEDSCRIPT:
		bSet = BSET_BLOCKED;
		break;
	case SET_FFIRESCRIPT:
		bSet = BSET_FFIRE;
		break;
	case SET_FFDEATHSCRIPT:
		bSet = BSET_FFDEATH;
		break;
	}

	if(bSet < BSET_SPAWN || bSet >= NUM_BSETS)
	{
		return qfalse;
	}

	if(!Q_stricmp("NULL", scriptname))
	{
		if ( ent->behaviorSet[bSet] != NULL )
		{
//			gi.TagFree( ent->behaviorSet[bSet] );
		}

		ent->behaviorSet[bSet] = NULL;
		//memset( &ent->behaviorSet[bSet], 0, sizeof(ent->behaviorSet[bSet]) );
	}
	else
	{
		if ( scriptname )
		{
			if ( ent->behaviorSet[bSet] != NULL )
			{
//				gi.TagFree( ent->behaviorSet[bSet] );
			}
			
			ent->behaviorSet[bSet] = G_NewString( (char *) scriptname );	//FIXME: This really isn't good...
		}

		//ent->behaviorSet[bSet] = scriptname;
		//strncpy( (char *) &ent->behaviorSet[bSet], scriptname, MAX_BSET_LENGTH );
	}
	return qtrue;
}

/*
============
Q3_SetDelayScriptTime

?
============
*/
static void Q3_SetDelayScriptTime(int entID, int delayTime)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDelayScriptTime: invalid entID %d\n", entID);
		return;
	}

	ent->delayScriptTime = level.time + delayTime;
}


/*
============
Q3_SetPrecacheFile

Precache the sounds in the given file and load the text to go with
============
*/
extern	cvar_t	*com_buildScript;
void Q3_SetPrecacheFile (const char *file)
{
	char	*buffer,*holdBuf;
	char	*tokenStr;
	int		len,i;
	char	*holdText;
	char	filename[MAX_QPATH];
	char	finalName[MAX_QPATH];

	G_LanguageFilename(va( "%s/%s", Q3_SCRIPT_DIR, file),"pre",(char *) &finalName);

	len = gi.FS_ReadFile(finalName,(void **) &buffer);
//	len = gi.FS_ReadFile(va( "%s/%s%s", Q3_SCRIPT_DIR, file,".pre"),(void **) &buffer);

	if (len < 1)
	{
		Q3_DebugPrint( WL_ERROR, "Invalid Precache file %s!\n", file );
		return;
	}

	holdBuf = buffer;
	COM_BeginParseSession();
	
	while ( holdBuf ) 
	{
		// Parse line
		tokenStr = COM_ParseExt(&holdBuf, qtrue);

		if (!holdBuf)	// No more data
			break;

		holdText = G_NewString(tokenStr);
		
		G_ParseString(&holdBuf,&tokenStr); 

		if (holdText)
		{
			if ( !( _stricmp( holdText, "ROF") ))
			{
				// The token says we should expect a rof file next
				if ( tokenStr )
				{
					G_LoadRoff( tokenStr );
				}
				else
				{
					Q3_DebugPrint( WL_ERROR, "ROF token found in %s.pre but no rof file was specified!\n", file );
				}
			}
			else if (holdText[0] == '@')	// It's just a key so send it straight to precacheText[]
			{
				precacheText[precacheText_i].key = holdText;
				precacheText[precacheText_i].text = G_NewString(tokenStr);

				precacheText_i++;

				if (precacheText_i >= MAX_PRECACHETEXT)	// Past the array limit???
				{
					precacheText_i = MAX_PRECACHETEXT - 1;
					Com_Printf(S_COLOR_RED"Exceeded max text in Precache Array!\n");
					break;
				}
			}
			else	// It must be a wav file
			{		
				COM_StripExtension((const char *)holdText, holdText);
				COM_DefaultExtension( holdText, MAX_QPATH, ".wav");
				precacheWav[precacheWav_i].wavFile = _strlwr(holdText);	// Store wav name

				if (com_buildScript->integer) {	//grab the default wav when building
					G_SoundIndex(holdText);
				}
				//replace "munro/" with "alexa/" but NOT handle mr/ms
				qboolean qbMatches= G_AddSexToMunroString( holdText, qfalse );

				if (qbMatches)
					G_SoundIndex(holdText);	//grab the converted wav unless it's female and we're not

				if (!Q_stricmp(tokenStr,"NOTEXT"))	// No text with sound so be on your merry way
				{
					precacheWav[precacheWav_i].textKey[0] = NULL;
					precacheWav[precacheWav_i].speaker = SP_NONE;
				}
				else	// There is text with the sound
				{

					if (tokenStr[0] == NULL)
					{
						Q3_DebugPrint( WL_ERROR, "You forgot to put NOTEXT or text after wav '%s'!\n",precacheWav[precacheWav_i].wavFile);
					}
					else
					{
						Com_sprintf (precacheWav[precacheWav_i].textKey, sizeof(precacheWav[precacheWav_i].textKey), "@wav%d",precacheText_i);	// Create a wave file key

						precacheText[precacheText_i].key = precacheWav[precacheWav_i].textKey;

						precacheText[precacheText_i].text = G_NewString(tokenStr);

						G_ParseString(&holdBuf,&tokenStr); 
						// Ignore the computer entity 
						if ((tokenStr[0]) && (strcmp(tokenStr,"COMPUTER")))
						{

							for (i=0;i<SP_MAX;++i)	// Find the speaker
							{
								if (!strcmp(tokenStr,speakerTable[i].stringID))
								{
									precacheWav[precacheWav_i].speaker = i;
									if (!speakerTable[i].headModel)	// Precache speakers head model
									{
										if ( speakerTable[i].headModelFile[0] )
										{//not a null model
											Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", speakerTable[i].headModelFile );
											speakerTable[i].headModel = G_ModelIndex( filename );
										}
									}
									break;
								}
							}

							if (i == SP_MAX)	// No match
							{
								precacheWav[precacheWav_i].speaker = SP_NONE;
								Q3_DebugPrint( WL_WARNING, "Precache unable to locate speaker '%s'!\n",tokenStr);
							}

						}
						else	// No speaker specified
						{
							precacheWav[precacheWav_i].speaker = SP_NONE;
						}
						precacheText_i++;

					}
				}

				precacheWav_i++;

				if (precacheWav_i >= MAX_PRECACHEWAV)	// Past the array limit???
				{
					precacheWav_i = MAX_PRECACHEWAV - 1;
					Com_Printf(S_COLOR_RED"Exceeded max sounds in Precache Array!\n");
				}
			}
		}

	}

	gi.FS_FreeFile( buffer );	//let go of the buffer
}
	
/*
============
Q3_SetIgnorePain

?
============
*/
static void Q3_SetIgnorePain( int entID, qboolean data)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetIgnorePain: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetIgnorePain: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	ent->NPC->ignorePain = data;
}

/*
============
Q3_SetIgnoreEnemies

?
============
*/
static void Q3_SetIgnoreEnemies( int entID, qboolean data)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetIgnoreEnemies: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetIgnoreEnemies: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(data)
	{
		ent->svFlags |= SVF_IGNORE_ENEMIES;
	}
	else
	{
		ent->svFlags &= ~SVF_IGNORE_ENEMIES;
	}
}

/*
============
Q3_SetIgnoreAlerts

?
============
*/
static void Q3_SetIgnoreAlerts( int entID, qboolean data)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetIgnoreAlerts: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetIgnoreAlerts: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(data)
	{
		ent->NPC->aiFlags |= NPCAI_IGNORE_ALERTS;
	}
	else
	{
		ent->NPC->aiFlags &= ~NPCAI_IGNORE_ALERTS;
	}
}
/*
============

Q3_SetStraightToGoal

?
============
*/
static void Q3_SetStraightToGoal( int entID, qboolean data)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetStraightToGoal: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetStraightToGoal: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	ent->NPC->straightToGoal = data;
}

/*
============
Q3_SetNoTarget

?
============
*/
static void Q3_SetNoTarget( int entID, qboolean data)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetNoTarget: invalid entID %d\n", entID);
		return;
	}

	if(data)
		ent->flags |= FL_NOTARGET;
	else
		ent->flags &= ~FL_NOTARGET;
}

/*
============
Q3_SetDontShoot

?
============
*/
static void Q3_SetDontShoot( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDontShoot: invalid entID %d\n", entID);
		return;
	}

	if(add)
	{
		ent->flags |= FL_DONT_SHOOT;
	}
	else
	{
		ent->flags &= ~FL_DONT_SHOOT;
	}
}

/*
============
Q3_SetDontFire

?
============
*/
static void Q3_SetDontFire( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetDontFire: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetDontFire: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->aiFlags |= NPCAI_DONT_FIRE;
	}
	else
	{
		ent->NPC->aiFlags &= ~NPCAI_DONT_FIRE;
	}
}

/*
============
Q3_SetNoSlowDown

?
============
*/
static void Q3_SetNoSlowDown( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetNoSlowDown: invalid entID %d\n", entID);
		return;
	}
	
	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetNoSlowDown: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->aiFlags |= NPCAI_NO_SLOWDOWN;
	}
	else
	{
		ent->NPC->aiFlags &= ~NPCAI_NO_SLOWDOWN;
	}
}

/*
============
Q3_SetLockedEnemy

?
============
*/
static void Q3_SetLockedEnemy ( int entID, qboolean locked)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLockedEnemy: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetLockedEnemy: '%s' is not an NPC!\n", ent->targetname );
		return;
	}
	
	//FIXME: make an NPCAI_FLAG
	if(locked)
	{
		ent->svFlags |= SVF_LOCKEDENEMY;
	}
	else
	{
		ent->svFlags &= ~SVF_LOCKEDENEMY;
	}
}

/*
============
Q3_SetCrouched

?
============
*/
static void Q3_SetCrouched( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCrouched: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCrouched: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_CROUCHED;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_CROUCHED;
	}
}

/*
============
Q3_SetWalking

?
============
*/
static void Q3_SetWalking( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetWalking: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetWalking: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_WALKING;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_WALKING;
	}
}

/*
============
Q3_SetRunning

?
============
*/
static void Q3_SetRunning( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetRunning: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetRunning: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_RUNNING;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_RUNNING;
	}
}

/*
============
Q3_SetAltFire

?
============
*/
static void Q3_SetAltFire( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetAltFire: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetAltFire: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_ALT_FIRE;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_ALT_FIRE;
	}
}

/*
============
Q3_SetNoResponse

?
============
*/
static void Q3_SetNoResponse( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetNoResponse: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetNoResponse: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_NO_RESPONSE;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_NO_RESPONSE;
	}
}

/*
============
Q3_SetCombatTalk

?
============
*/
static void Q3_SetCombatTalk( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCombatTalk: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCombatTalk: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if ( add )
	{
		ent->NPC->scriptFlags |= SCF_NO_COMBAT_TALK;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_NO_COMBAT_TALK;
	}
}

/*
============
Q3_SetCareful

?
============
*/
static void Q3_SetCareful( int entID, qboolean add)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCareful: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCareful: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(add)
	{
		ent->NPC->scriptFlags |= SCF_CAREFUL;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_CAREFUL;
	}
}

/*
============
Q3_SetUndying

?
============
*/
static void Q3_SetUndying( int entID, qboolean undying)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetUndying: invalid entID %d\n", entID);
		return;
	}

	if(undying)
	{
		ent->flags |= FL_UNDYING;
	}
	else
	{
		ent->flags &= ~FL_UNDYING;
	}
}

/*
============
Q3_SetInvincible

?
============
*/
static void Q3_SetInvincible( int entID, qboolean invincible)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetInvincible: invalid entID %d\n", entID);
		return;
	}

	if ( invincible )
	{
		ent->flags |= FL_GODMODE;
	}
	else
	{
		ent->flags &= ~FL_GODMODE;
	}
}

/*
============
Q3_SetNoAvoid

?
============
*/
static void Q3_SetNoAvoid( int entID, qboolean noAvoid)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetNoAvoid: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetNoAvoid: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(noAvoid)
	{
		ent->NPC->aiFlags |= NPCAI_NO_COLL_AVOID;
	}
	else
	{
		ent->NPC->aiFlags &= ~NPCAI_NO_COLL_AVOID;
	}
}

/*
============
SolidifyOwner
  Description	: 
  Return type	: void 
  Argument		: gentity_t *self
============
*/
void SolidifyOwner( gentity_t *self )
{
	self->nextthink = level.time + FRAMETIME;
	self->e_ThinkFunc = thinkF_G_FreeEntity;

	if ( !self->owner || !self->owner->inuse )
	{
		return;
	}

	int oldContents = self->owner->contents;
	self->owner->contents = CONTENTS_BODY;
	if ( SpotWouldTelefrag2( self->owner, self->owner->currentOrigin ) )
	{
		self->owner->contents = oldContents;
		self->e_ThinkFunc = thinkF_SolidifyOwner;
	}
	else
	{
		if ( self->owner->NPC && !(self->owner->spawnflags & SFB_NOTSOLID) )
		{
			self->owner->clipmask |= CONTENTS_BODY;
		}
		Q3_TaskIDComplete( self->owner, TID_RESIZE );
	}
}


/*
============
Q3_SetSolid

?
============
*/
static qboolean Q3_SetSolid( int entID, qboolean solid)
{
	gentity_t	*ent  = &g_entities[entID];
	
	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetSolid: invalid entID %d\n", entID);
		return qtrue;
	}

	if ( solid )
	{//FIXME: Presumption
		int oldContents = ent->contents;
		ent->contents = CONTENTS_BODY;
		if ( SpotWouldTelefrag2( ent, ent->currentOrigin ) )
		{
			gentity_t *solidifier = G_Spawn();

			solidifier->owner = ent;

			solidifier->e_ThinkFunc = thinkF_SolidifyOwner;
			solidifier->nextthink = level.time + FRAMETIME;
			
			ent->contents = oldContents;
			return qfalse;
		}
		ent->clipmask |= CONTENTS_BODY;
	}
	else
	{//FIXME: Presumption
		if ( ent->s.eFlags & EF_NODRAW )
		{//We're invisible too, so set contents to none
			ent->contents = 0;
		}
		else
		{
			ent->contents = CONTENTS_CORPSE;
		}
		if ( ent->NPC )
		{
			if(!(ent->spawnflags & SFB_NOTSOLID))
			{
				ent->clipmask &= ~CONTENTS_BODY;
			}
		}
	}
	return qtrue;
}
/*
============
Q3_SetLean

?
============
*/
#define LEAN_NONE	0
#define LEAN_RIGHT	1
#define LEAN_LEFT	2
static void Q3_SetLean( int entID, int lean)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLean: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetLean: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if(lean == LEAN_RIGHT)
	{
		ent->NPC->scriptFlags |= SCF_LEAN_RIGHT;
		ent->NPC->scriptFlags &= ~SCF_LEAN_LEFT;
	}
	else if(lean == LEAN_LEFT)
	{
		ent->NPC->scriptFlags |= SCF_LEAN_LEFT;
		ent->NPC->scriptFlags &= ~SCF_LEAN_RIGHT;
	}
	else
	{
		ent->NPC->scriptFlags &= ~SCF_LEAN_LEFT;
		ent->NPC->scriptFlags &= ~SCF_LEAN_RIGHT;
	}
}

/*
============
Q3_SetBeam

?
============
*/
static void Q3_SetBeam( int entID, qboolean beamOn)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetBeam: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		//Q3_DebugPrint( WL_WARNING, "Q3_SetBeam: '%s' is not an NPC/player!\n", ent->targetname );
		if(beamOn)
		{
			ent->s.eFlags |= EF_EYEBEAM;
		}
		else
		{
			ent->s.eFlags &= ~EF_EYEBEAM;
		}
		return;
	}

	if(beamOn)
	{
		ent->client->ps.eFlags |= EF_EYEBEAM;
	}
	else
	{
		ent->client->ps.eFlags &= ~EF_EYEBEAM;
	}
}

/*
============
Q3_SetForwardMove

?
============
*/
static void Q3_SetForwardMove( int entID, int fmoveVal)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetForwardMove: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetForwardMove: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}

	ent->client->forced_forwardmove = fmoveVal;
}

/*
============
Q3_SetRightMove

?
============
*/
static void Q3_SetRightMove( int entID, int rmoveVal)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetRightMove: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetRightMove: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}

	ent->client->forced_rightmove = rmoveVal;
}

/*
============
Q3_SetLockAngle

?
============
*/
static void Q3_SetLockAngle( int entID, const char *lockAngle)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLockAngle: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetLockAngle: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}

	if(Q_stricmp("off", lockAngle) == 0)
	{//free it
		ent->client->renderInfo.renderFlags &= ~RF_LOCKEDANGLE;
	}
	else
	{
		ent->client->renderInfo.renderFlags |= RF_LOCKEDANGLE;
		if(Q_stricmp("auto", lockAngle) == 0)
		{//use current yaw
			ent->client->renderInfo.lockYaw = ent->client->ps.viewangles[YAW];
		}
		else
		{//specified yaw
			ent->client->renderInfo.lockYaw = atof((char *)lockAngle);
		}
	}
}

/*
============
Q3_SetFormation

?
============
*/
extern void G_CreateFormation (gentity_t *self);
static void Q3_SetFormation( int entID, qboolean inForm)
{
	if(inForm)
	{
		G_CreateFormation(&g_entities[0]);
	}
	else
	{//FIXME: dissolve formation		
	}
}

/*
============
Q3_CameraGroup

?
============
*/
static void Q3_CameraGroup( int entID, char *camG)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_CameraGroup: invalid entID %d\n", entID);
		return;
	}

	ent->cameraGroup = G_NewString(camG);
}

/*
============
Q3_LookTarget

?
============
*/
static void Q3_LookTarget( int entID, char *targetName)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_LookTarget: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_LookTarget: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}

	if(Q_stricmp("none", targetName) == 0 || Q_stricmp("NULL", targetName) == 0)
	{//clearing look target
		NPC_ClearLookTarget( ent );
		return;
	}

	gentity_t	*targ  = G_Find(NULL, FOFS(targetname), targetName);
	if(!targ)
	{
		Q3_DebugPrint( WL_ERROR, "Q3_LookTarget: Can't find ent %s\n", targetName );
		return;
	}

	NPC_SetLookTarget( ent, targ->s.number, 0 );
}

/*
============
Q3_Face

?
============
*/
static void Q3_Face( int entID,int expression, float holdtime)
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Face: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Face: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}
	
	//FIXME: change to milliseconds to be consistant!
	holdtime *= 1000;

	switch(expression)
	{
	case SET_FACEAUX:
		ent->client->facial_aux = -(level.time + holdtime);

		ent->client->facial_blink = level.time + holdtime + Q_flrand(3000.0, 5000.0);
		ent->client->facial_frown = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		break;
	case SET_FACEBLINK:
		ent->client->facial_blink = -(level.time + holdtime);

		ent->client->facial_aux = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		ent->client->facial_frown = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		break;

	case SET_FACEBLINKFROWN:
		ent->client->facial_blink = -(level.time + holdtime);
		ent->client->facial_frown = -(level.time + holdtime);

		ent->client->facial_aux = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		break;

	case SET_FACEFROWN:
		ent->client->facial_frown = -(level.time + holdtime);

		ent->client->facial_aux = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		ent->client->facial_blink = level.time + holdtime + Q_flrand(3000.0, 5000.0);
		break;

	case SET_FACENORMAL:
		ent->client->facial_aux = level.time + Q_flrand(6000.0, 10000.0);
		ent->client->facial_blink = level.time + Q_flrand(3000.0, 5000.0);
		ent->client->facial_frown = level.time + holdtime + Q_flrand(6000.0, 10000.0);
		break;
	}

}


/*
============
Q3_SetPlayerUsable
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean usable
============
*/
static void Q3_SetPlayerUsable( int entID, qboolean usable )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetPlayerUsable: invalid entID %d\n", entID);
		return;
	}

	if(usable)
	{
		ent->svFlags |= SVF_PLAYER_USABLE;
	}
	else
	{
		ent->svFlags &= ~SVF_PLAYER_USABLE;
	}
}


/*
============
Q3_SetStartFrame
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int startFrame
============
*/
static void Q3_SetStartFrame( int entID, int startFrame )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetStartFrame: invalid entID %d\n", entID);
		return;
	}

	if ( ent->client )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLoopAnim: command not valid on players/NPCs!\n" );
		return;
	}

	if ( startFrame >= 0 )
	{
		ent->startFrame = startFrame;
	}
}


/*
============
Q3_SetEndFrame
  Description	: 
  Return type	: static void 
  Argument		:  int entID
  Argument		: int endFrame
============
*/
static void Q3_SetEndFrame( int entID, int endFrame )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEndFrame: invalid entID %d\n", entID);
		return;
	}

	if ( ent->client )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetEndFrame: command not valid on players/NPCs!\n" );
		return;
	}

	if ( endFrame >= 0 )
	{
		ent->endFrame = endFrame;
	}
}


void InflateOwner( gentity_t *self )
{
	self->nextthink = level.time + FRAMETIME;
	self->e_ThinkFunc = thinkF_G_FreeEntity;

	if ( !self->owner || !self->owner->inuse )
	{
		return;
	}

	trace_t	trace;

	gi.trace( &trace, self->currentOrigin, self->mins, self->maxs, self->currentOrigin, self->owner->s.number, self->owner->clipmask&~(CONTENTS_SOLID|CONTENTS_MONSTERCLIP) );
	if ( trace.allsolid || trace.startsolid )
	{
		self->e_ThinkFunc = thinkF_InflateOwner;
		return;
	}

	if ( Q3_TaskIDPending( self->owner, TID_RESIZE ) )
	{
		Q3_TaskIDComplete( self->owner, TID_RESIZE );

		VectorCopy( self->mins, self->owner->mins );
		VectorCopy( self->maxs, self->owner->maxs );
		gi.linkentity( self->owner );
	}
}
/*
============
Q3_SetWidth
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: int width
============
*/
static qboolean Q3_SetWidth( int entID, int width )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetWidth: invalid entID %d\n", entID);
		return qtrue;
	}

	if ( width < 0 )
	{
		return qtrue;
	}

	if ( ent->client && width > (ent->maxs[0] - ent->mins[0]) || width > (ent->maxs[1] - ent->mins[1]) )
	{//trying to inflate
		vec3_t	start, mins, maxs;

		VectorCopy( ent->currentOrigin, start );
		VectorCopy( ent->mins, mins );
		VectorCopy( ent->maxs, maxs );
		mins[0] = mins[1] = -width;
		maxs[0] = maxs[1] = width;

		trace_t	trace;
		gi.trace( &trace, start, mins, maxs, start, ent->s.number, ent->clipmask&~(CONTENTS_SOLID|CONTENTS_MONSTERCLIP) );
		if ( trace.allsolid || trace.startsolid )
		{
			gentity_t *inflater = G_Spawn();
			
			VectorCopy( start, inflater->currentOrigin );
			VectorCopy( mins, inflater->mins );
			VectorCopy( maxs, inflater->maxs );

			inflater->owner = ent;
			inflater->e_ThinkFunc = thinkF_InflateOwner;
			inflater->nextthink = level.time + FRAMETIME;

			return qfalse;
		}
	}

	ent->mins[0] = ent->mins[1] = -width;
	ent->maxs[0] = ent->maxs[1] = width;

	return qtrue;
}

/*
============
Q3_SetHeight
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: int height
============
*/
static qboolean Q3_SetHeight( int entID, int height )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetHeight: invalid entID %d\n", entID);
		return qtrue;
	}

	if ( height < 0 )
	{
		return qtrue;
	}

	if ( ent->client && height > (ent->maxs[2] - ent->mins[2]) )
	{//trying to inflate, make sure we don't clip into another NPC
		vec3_t	start, mins, maxs;

		VectorCopy( ent->currentOrigin, start );
		VectorCopy( ent->mins, mins );
		VectorCopy( ent->maxs, maxs );
		maxs[2] = height + mins[2];
		trace_t	trace;
		gi.trace( &trace, start, mins, maxs, start, ent->s.number, ent->clipmask&~(CONTENTS_SOLID|CONTENTS_MONSTERCLIP) );
		if ( trace.allsolid || trace.startsolid )
		{
			gentity_t *inflater = G_Spawn();
			
			VectorCopy( start, inflater->currentOrigin );
			VectorCopy( mins, inflater->mins );
			VectorCopy( maxs, inflater->maxs );

			inflater->owner = ent;
			inflater->e_ThinkFunc = thinkF_InflateOwner;
			inflater->nextthink = level.time + FRAMETIME;

			return qfalse;
		}
	}

	ent->maxs[2] = height + ent->mins[2];

	if(ent->client)
	{
		ent->client->standheight = ent->maxs[2];
	}

	return qtrue;
}


/*
============
Q3_SetCrouchHeight
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: int crouchheight
============
*/
static void Q3_SetCrouchHeight( int entID, int crouchheight )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetCrouchHeight: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->client )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetCrouchHeight: '%s' is not an NPC/player!\n", ent->targetname );
		return;
	}

	if ( crouchheight < 0 )
	{
		return;
	}

	ent->client->crouchheight = crouchheight + ent->mins[2];
}


/*
============
Q3_SetLoopAnim
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean loopAnim
============
*/
static void Q3_SetLoopAnim( int entID, qboolean loopAnim )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLoopAnim: invalid entID %d\n", entID);
		return;
	}

	if ( ent->client )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetLoopAnim: command not valid on players/NPCs!\n" );
		return;
	}

	ent->loopAnim = loopAnim;
}


/*
============
Q3_SetShields
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean shields
============
*/
static void Q3_SetShields( int entID, qboolean shields )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetShields: invalid entID %d\n", entID);
		return;
	}

	if ( !ent->NPC )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_SetShields: '%s' is not an NPC!\n", ent->targetname );
		return;
	}

	if ( shields )
	{
		ent->NPC->aiFlags |= NPCAI_SHIELDS;
	}
	else
	{
		ent->NPC->aiFlags &= ~NPCAI_SHIELDS;
	}
}


/*
============
Q3_SetNoKnockback
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean noKnockback
============
*/
static void Q3_SetNoKnockback( int entID, qboolean noKnockback )
{
	gentity_t	*ent  = &g_entities[entID];

	if ( !ent )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_SetNoKnockback: invalid entID %d\n", entID);
		return;
	}

	if ( noKnockback )
	{
		ent->flags |= FL_NO_KNOCKBACK;
	}
	else
	{
		ent->flags &= ~FL_NO_KNOCKBACK;
	}
}


/*
============
Q3_SetInterface
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *data
============
*/
static void Q3_SetInterface( int entID, const char *data )
{
	gi.cvar_set("cg_drawStatus", data);
}


/*
============
Q3_AddBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_AddBoltOn( int entID, const char *boltOnName )
{
	gentity_t	*ent = &g_entities[entID];
	if ( !ent )
	{
		//FIXME: error message
		return;
	}

	//Add it
	ent->activeBoltOn = G_AddBoltOn( ent, boltOnName );
}


/*
============
Q3_BeamInBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_BeamInBoltOn( int entID, const char *boltOnName )
{
	gentity_t	*ent = &g_entities[entID];
	if ( !ent )
	{
		//FIXME: error message
		return;
	}

	//Add it
	ent->activeBoltOn = G_AddBoltOn( ent, boltOnName );
	ent->client->renderInfo.boltOns[ent->activeBoltOn].fxFlags = 1;	// BOLTON_BEAMIN
	ent->client->renderInfo.boltOns[ent->activeBoltOn].startTime = level.time;

	// This sound should already be pre-cached
	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/change.wav" ));
}


/*
============
Q3_BeamOutBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_BeamOutBoltOn( int entID, const char *boltOnName )
{
	gentity_t	*ent = &g_entities[entID];

	if ( !ent )
	{
		// FIXME: error message
		return;
	}

	// Remove it
	G_RemoveBoltOn( ent, boltOnName );

	// This sound should already be pre-cached
	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/change.wav" ));
}


/*
============
Q3_RemoveBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_RemoveBoltOn( int entID, const char *boltOnName )
{
	G_RemoveBoltOn( &g_entities[entID], boltOnName );
}


/*
============
Q3_DropBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_DropBoltOn( int entID, const char *boltOnName )
{
	G_DropBoltOn( &g_entities[entID], boltOnName );
}


/*
============
Q3_SetActiveBoltOn
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *boltOnName
============
*/
static void Q3_SetActiveBoltOn( int entID, const char *boltOnName )
{
	gentity_t	*ent = &g_entities[entID];

	if ( !ent )
	{
		//FIXME: error message
		return;
	}

	byte	boltOn;

	boltOn = G_BoltOnNumberForName( ent, boltOnName );

	if ( boltOn >= 0 && boltOn < MAX_BOLT_ONS )
	{
		ent->activeBoltOn = boltOn;
	}
}


/*
============
Q3_SetActiveBoltOnStartFrame
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: int startFrame
============
*/
void Q3_SetActiveBoltOnStartFrame( int entID, int startFrame )
{
	gentity_t	*ent = &g_entities[entID];

	if ( !ent )
	{
		return;
	}

	if ( ent->activeBoltOn < 0 || ent->activeBoltOn >= MAX_BOLT_ONS )
	{
		return;
	}

	if ( !ent->client )
	{
		ent->boltOn.startFrame = startFrame;
	}
	else
	{
		ent->client->renderInfo.boltOns[ent->activeBoltOn].startFrame = startFrame;
	}
}


/*
============
Q3_SetActiveBoltOnEndFrame
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: int endFrame
============
*/
void Q3_SetActiveBoltOnEndFrame( int entID, int endFrame )
{
	gentity_t *ent = &g_entities[entID];

	if ( !ent )
	{
		return;
	}

	if ( ent->activeBoltOn < 0 || ent->activeBoltOn >= MAX_BOLT_ONS )
	{
		return;
	}

	if ( !ent->client )
	{
		ent->boltOn.endFrame = endFrame;
	}
	else
	{
		ent->client->renderInfo.boltOns[ent->activeBoltOn].endFrame = endFrame;
	}
}


/*
============
Q3_SetActiveBoltOnAnimLoop
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean loopAnim
============
*/
static void Q3_SetActiveBoltOnAnimLoop( int entID, qboolean loopAnim )
{
	gentity_t *ent = &g_entities[entID];

	if ( !ent )
	{
		return;
	}

	if ( ent->activeBoltOn < 0 || ent->activeBoltOn >= MAX_BOLT_ONS )
	{
		return;
	}

	if ( !ent->client )
	{
		ent->boltOn.loopAnim = loopAnim;
	}
	else
	{
		NPC->client->renderInfo.boltOns[ent->activeBoltOn].loopAnim = loopAnim;
	}
}


/*
============
Q3_SetLocation
  Description	: 
  Return type	: qboolean 
  Argument		:  int entID
  Argument		: const char *location
============
*/
static qboolean Q3_SetLocation( int entID, const char *location )
{
	gentity_t	*ent = &g_entities[entID];
	char		*currentLoc;
	
	if ( !ent )
	{
		return qtrue;
	}

	currentLoc = G_GetLocationForEnt( ent );
	if ( currentLoc && currentLoc[0] && Q_stricmp( location, currentLoc ) == 0 )
	{
		return qtrue;
	}

	ent->message = G_NewString( location );
	return qfalse;
}

/*
============
Q3_SetPlayerLocked
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: qboolean locked
============
*/
qboolean	player_locked = qfalse;
static void Q3_SetPlayerLocked( int entID, qboolean locked )
{
	gentity_t	*ent = &g_entities[0];

	player_locked = locked;
	if ( ent && ent->client )
	{//stop him too
		VectorClear(ent->client->ps.velocity);
	}
}

extern void CG_CameraAutoAim( const char *name );
extern void CG_CameraAutoTrack( const char *name );

/*
============
Q3_SetVar
  Description	: 
  Return type	: static void 
  Argument		:  int taskID
  Argument		: int entID
  Argument		: const char *type_name
  Argument		: const char *data
============
*/
void Q3_SetVar( int taskID, int entID, const char *type_name, const char *data )
{
	int	vret = Q3_VariableDeclared( type_name ) ;
	float	float_data;
	float	val = 0.0f;

	
	if ( vret != VTYPE_NONE )
	{
		switch ( vret )
		{
		case VTYPE_FLOAT:
			//Check to see if increment command
			if ( (val = Q3_CheckStringCounterIncrement( data )) )
			{
				Q3_GetFloatVariable( type_name, &float_data );
				float_data += val;
			}
			else
			{
				float_data = atof((char *) data);
			}
			Q3_SetFloatVariable( type_name, float_data );
			break;

		case VTYPE_STRING:
			Q3_SetStringVariable( type_name, data );
			break;

		case VTYPE_VECTOR:
			Q3_SetVectorVariable( type_name, (char *) data );
			break;
		}

		return;
	}

	Q3_DebugPrint( WL_ERROR, "%s variable or field not found!\n", type_name );
}


/*
============
Q3_Set
  Description	: 
  Return type	: void 
  Argument		:  int taskID
  Argument		: int entID
  Argument		: const char *type_name
  Argument		: const char *data
============
*/
static void Q3_Set( int taskID, int entID, const char *type_name, const char *data )
{
	gentity_t	*ent = &g_entities[entID];
	float		float_data;
	int			int_data, toSet;
	vec3_t		vector_data;

	//Set this for callbacks
	toSet = GetIDForString( setTable, type_name );

	//TODO: Throw in a showscript command that will list each command and what they're doing...
	//		maybe as simple as printing that line of the script to the console preceeded by the person's name?
	//		showscript can take any number of targetnames or "all"?  Groupname?
	switch ( toSet )
	{
	case SET_ORIGIN:
		sscanf( data, "%f %f %f", &vector_data[0], &vector_data[1], &vector_data[2] );
		G_SetOrigin( ent, vector_data );
		if ( strncmp( "NPC_", ent->classname, 4 ) == 0 )
		{//hack for moving spawners
			VectorCopy( vector_data, ent->s.origin);
		}
		break;

	case SET_TELEPORT_DEST:
		sscanf( data, "%f %f %f", &vector_data[0], &vector_data[1], &vector_data[2] );
		if ( !Q3_SetTeleportDest( entID, vector_data ) )
		{
			Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
			return;
		}
		break;

	case SET_COPY_ORIGIN:
		Q3_SetCopyOrigin( entID, (char *) data );
		break;

	case SET_ANGLES:
		//Q3_SetAngles( entID, *(vec3_t *) data);
		sscanf( data, "%f %f %f", &vector_data[0], &vector_data[1], &vector_data[2] );
		Q3_SetAngles( entID, vector_data);
		break;

	case SET_XVELOCITY:
		float_data = atof((char *) data);
		Q3_SetVelocity( entID, 0, float_data);
		break;
	
	case SET_YVELOCITY:
		float_data = atof((char *) data);
		Q3_SetVelocity( entID, 1, float_data);
		break;

	case SET_ZVELOCITY:
		float_data = atof((char *) data);
		Q3_SetVelocity( entID, 2, float_data);
		break;

	case SET_AVELOCITY:
		break;

	case SET_ENEMY:
		Q3_SetEnemy( entID, (char *) data );
		break;

	case SET_LEADER:
		Q3_SetLeader( entID, (char *) data );
		break;

	case SET_NAVGOAL:
		Q3_SetNavGoal( entID, (char *) data );
		Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
		return;	//Don't call it back
		break;

	case SET_ANIM_UPPER:
		if ( Q3_SetAnimUpper( entID, (char *) data ) )
		{
			Q3_TaskIDClear( &ent->taskID[TID_ANIM_BOTH] );//We only want to wait for the top
			Q3_TaskIDSet( ent, TID_ANIM_UPPER, taskID );
			return;	//Don't call it back
		}
		break;

	case SET_ANIM_LOWER:
		if ( Q3_SetAnimLower( entID, (char *) data ) )
		{
			Q3_TaskIDClear( &ent->taskID[TID_ANIM_BOTH] );//We only want to wait for the bottom
			Q3_TaskIDSet( ent, TID_ANIM_LOWER, taskID );
			return;	//Don't call it back
		}
		break;

	case SET_ANIM_BOTH:
		{
			int	both = 0;
			if ( Q3_SetAnimUpper( entID, (char *) data ) )
			{
				Q3_TaskIDSet( ent, TID_ANIM_UPPER, taskID );
				both++;
			}
			else
			{
				Q3_DebugPrint( WL_ERROR, "Q3_SetAnimUpper: %s does not have anim %s!\n", ent->targetname, (char *)data );
			}
			if ( Q3_SetAnimLower( entID, (char *) data ) )
			{
				Q3_TaskIDSet( ent, TID_ANIM_LOWER, taskID );
				both++;
			}
			else
			{
				Q3_DebugPrint( WL_ERROR, "Q3_SetAnimLower: %s does not have anim %s!\n", ent->targetname, (char *)data );
			}
			if ( both >= 2 )
			{
				Q3_TaskIDSet( ent, TID_ANIM_BOTH, taskID );
			}
			if ( both )
			{
				return;	//Don't call it back
			}
		}
		break;
	
	case SET_ANIM_HOLDTIME_LOWER:
		int_data = atoi((char *) data);
		Q3_SetAnimHoldTime( entID, int_data, qtrue );
		Q3_TaskIDClear( &ent->taskID[TID_ANIM_BOTH] );//We only want to wait for the bottom
		Q3_TaskIDSet( ent, TID_ANIM_LOWER, taskID );
		return;	//Don't call it back
		break;

	case SET_ANIM_HOLDTIME_UPPER:
		int_data = atoi((char *) data);
		Q3_SetAnimHoldTime( entID, int_data, qfalse );
		Q3_TaskIDClear( &ent->taskID[TID_ANIM_BOTH] );//We only want to wait for the top
		Q3_TaskIDSet( ent, TID_ANIM_UPPER, taskID );
		return;	//Don't call it back
		break;

	case SET_ANIM_HOLDTIME_BOTH:
		int_data = atoi((char *) data);
		Q3_SetAnimHoldTime( entID, int_data, qfalse );
		Q3_SetAnimHoldTime( entID, int_data, qtrue );
		Q3_TaskIDSet( ent, TID_ANIM_BOTH, taskID );
		Q3_TaskIDSet( ent, TID_ANIM_UPPER, taskID );
		Q3_TaskIDSet( ent, TID_ANIM_LOWER, taskID );
		return;	//Don't call it back
		break;

	case SET_PLAYER_TEAM:
		Q3_SetPlayerTeam( entID, (char *) data );
		break;

	case SET_ENEMY_TEAM:
		Q3_SetEnemyTeam( entID, (char *) data );
		break;

	case SET_HEALTH:
		int_data = atoi((char *) data);
		Q3_SetHealth( entID, int_data );
		break;

	case SET_ARMOR:
		int_data = atoi((char *) data);
		Q3_SetArmor( entID, int_data );
		break;

	case SET_BEHAVIOR_STATE:
		if( !Q3_SetBState( entID, (char *) data ) )
		{
			Q3_TaskIDSet( ent, TID_BSTATE, taskID );
			return;//don't complete
		}
		break;

	case SET_DEFAULT_BSTATE:
		Q3_SetDefaultBState( entID, (char *) data );
		break;

	case SET_TEMP_BSTATE:
		if( !Q3_SetTempBState( entID, (char *) data ) )
		{
			Q3_TaskIDSet( ent, TID_BSTATE, taskID );
			return;//don't complete
		}
		break;

	case SET_HIDING:
		//Q3_DebugPrint( WL_WARNING, "Q3_SetHidingGoal unimplemented\n", entID );
		Q3_SetHidingGoal( entID, (char *) data );//unimplemented
		break;

	case SET_CAPTURE:
		Q3_SetCaptureGoal( entID, (char *) data );
		break;

	case SET_DPITCH://FIXME: make these set tempBehavior to BS_FACE and await completion?  Or set lockedDesiredPitch/Yaw and aimTime?
		float_data = atof((char *) data);
		Q3_SetDPitch( entID, float_data );
		break;

	case SET_DYAW:
		float_data = atof((char *) data);
		Q3_SetDYaw( entID, float_data );
		break;

	case SET_EVENT:
		Q3_SetEvent( entID, (char *) data );
		break;

	case SET_VIEWTARGET:
		Q3_SetViewTarget( entID, (char *) data );
		break;

	case SET_WEAPON:
		Q3_SetWeapon ( entID, (char *) data);
		break;

	case SET_WALKSPEED:
		int_data = atoi((char *) data);
		Q3_SetWalkSpeed ( entID, int_data);
		break;

	case SET_RUNSPEED:
		int_data = atoi((char *) data);
		Q3_SetRunSpeed ( entID, int_data);
		break;

	case SET_YAWSPEED:
		float_data = atof((char *) data);
		Q3_SetYawSpeed ( entID, float_data);
		break;

	case SET_AGGRESSION:
		int_data = atoi((char *) data);
		Q3_SetAggression ( entID, int_data);
		break;

	case SET_AIM:
		int_data = atoi((char *) data);
		Q3_SetAim ( entID, int_data);
		break;

	case SET_FRICTION:
		int_data = atoi((char *) data);
		Q3_SetFriction ( entID, int_data);
		break;

	case SET_GRAVITY:
		float_data = atof((char *) data);
		Q3_SetGravity ( entID, float_data);
		break;

	case SET_WAIT:
		float_data = atof((char *) data);
		Q3_SetWait( entID, float_data);
		break;

	case SET_FOLLOWDIST:
		float_data = atof((char *) data);
		Q3_SetFollowDist( entID, float_data);
		break;

	case SET_COUNT:
		Q3_SetCount( entID, (char *) data);
		break;

	case SET_SHOT_SPACING:
		int_data = atoi((char *) data);
		Q3_SetShotSpacing( entID, int_data );
		break;

	case SET_IGNOREPAIN:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetIgnorePain( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetIgnorePain( entID, qfalse);
		break;

	case SET_IGNOREENEMIES:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetIgnoreEnemies( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetIgnoreEnemies( entID, qfalse);
		break;

	case SET_IGNOREALERTS:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetIgnoreAlerts( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetIgnoreAlerts( entID, qfalse);
		break;

	case SET_STRAIGHTTOGOAL:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetStraightToGoal( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetStraightToGoal( entID, qfalse);
		break;
		
	case SET_DONTSHOOT:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetDontShoot( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetDontShoot( entID, qfalse);
		break;
	
	case SET_DONTFIRE:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetDontFire( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetDontFire( entID, qfalse);
		break;

	case SET_NOSLOWDOWN:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetNoSlowDown( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetNoSlowDown( entID, qfalse);
		break;

	case SET_LOCKED_ENEMY:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetLockedEnemy( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetLockedEnemy( entID, qfalse);
		break;

	case SET_NOTARGET:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetNoTarget( entID, qtrue);
		else if(!_stricmp("false", ((char *)data)))
			Q3_SetNoTarget( entID, qfalse);
		break;

	case SET_LEAN:
		if(!_stricmp("right", ((char *)data)))
			Q3_SetLean( entID, LEAN_RIGHT);
		else if(!_stricmp("left", ((char *)data)))
			Q3_SetLean( entID, LEAN_LEFT);
		else
			Q3_SetLean( entID, LEAN_NONE);
		break;

	case SET_SHOOTDIST:
		float_data = atof((char *) data);
		Q3_SetShootDist( entID, float_data );
		break;

	case SET_TIMESCALE:
		Q3_SetTimeScale( entID, (char *) data );
		break;

	case SET_VISRANGE:
		float_data = atof((char *) data);
		Q3_SetVisrange( entID, float_data );
		break;
	
	case SET_EARSHOT:
		float_data = atof((char *) data);
		Q3_SetEarshot( entID, float_data );
		break;
	
	case SET_VIGILANCE:
		float_data = atof((char *) data);
		Q3_SetVigilance( entID, float_data );
		break;
	
	case SET_VFOV:
		int_data = atoi((char *) data);
		Q3_SetVFOV( entID, int_data );
		break;

	case SET_HFOV:
		int_data = atoi((char *) data);
		Q3_SetHFOV( entID, int_data );
		break;

	case SET_SQUADNAME:
		Q3_SetSquadName( entID, (char *) data );
		break;

	case SET_TARGETNAME:
		Q3_SetTargetName( entID, (char *) data );
		break;

	case SET_TARGET:
		Q3_SetTarget( entID, (char *) data );
		break;

	case SET_TARGET2:
		Q3_SetTarget2( entID, (char *) data );
		break;
	
	case SET_LOCATION:
		if ( !Q3_SetLocation( entID, (char *) data ) )
		{
			Q3_TaskIDSet( ent, TID_LOCATION, taskID );
			return;
		}
		break;

	case SET_PAINTARGET:
		Q3_SetPainTarget( entID, (char *) data );
		break;

	case SET_DEFEND_TARGET:
		Q3_DebugPrint( WL_WARNING, "Q3_SetDefendTarget unimplemented\n", entID );
		//Q3_SetEnemy( entID, (char *) data);
		break;

	case SET_PARM1:
	case SET_PARM2:
	case SET_PARM3:
	case SET_PARM4:
	case SET_PARM5:
	case SET_PARM6:
	case SET_PARM7:
	case SET_PARM8:
	case SET_PARM9:
	case SET_PARM10:
	case SET_PARM11:
	case SET_PARM12:
	case SET_PARM13:
	case SET_PARM14:
	case SET_PARM15:
	case SET_PARM16:
		Q3_SetParm( entID, (toSet-SET_PARM1), (char *) data );
		break;

	case SET_SPAWNSCRIPT:
	case SET_IDLESCRIPT:
	case SET_TOUCHSCRIPT:
	case SET_USESCRIPT:
	case SET_AWAKESCRIPT:
	case SET_ANGERSCRIPT:
	case SET_ATTACKSCRIPT:
	case SET_VICTORYSCRIPT:
	case SET_PAINSCRIPT:
	case SET_FLEESCRIPT:
	case SET_DEATHSCRIPT:
	case SET_DELAYEDSCRIPT:
	case SET_BLOCKEDSCRIPT:
	case SET_FFIRESCRIPT:
	case SET_FFDEATHSCRIPT:
		if( !Q3_SetBehaviorSet(entID, toSet, (char *) data) )
			Q3_DebugPrint( WL_ERROR, "Q3_SetBehaviorSet: Invalid bSet %s\n", type_name );
		break;

	case SET_DELAYSCRIPTTIME:
		int_data = atoi((char *) data);
		Q3_SetDelayScriptTime( entID, int_data );
		break;

	case SET_PRECACHE:
		Q3_DebugPrint(WL_ERROR,"Precache cmd is OBSOLETE! Remove this now!\n");
		//Q3_SetPrecacheFile((char *) data);
		break;

	case SET_CROUCHED:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetCrouched( entID, qtrue);
		else
			Q3_SetCrouched( entID, qfalse);
		break;

	case SET_WALKING:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetWalking( entID, qtrue);	
		else
			Q3_SetWalking( entID, qfalse);	
		break;

	case SET_RUNNING:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetRunning( entID, qtrue);	
		else
			Q3_SetRunning( entID, qfalse);	
		break;

	case SET_ALT_FIRE:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetAltFire( entID, qtrue);	
		else
			Q3_SetAltFire( entID, qfalse);	
		break;

	case SET_NO_RESPONSE:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetNoResponse( entID, qtrue);	
		else
			Q3_SetNoResponse( entID, qfalse);	
		break;

	case SET_NO_COMBAT_TALK:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetCombatTalk( entID, qtrue);	
		else
			Q3_SetCombatTalk( entID, qfalse);	
		break;

	case SET_TREASONED:
		G_TeamRetaliation( NULL, &g_entities[0], qfalse );
		ffireLevel = FFIRE_LEVEL_RETALIATION;
		break;

	case SET_CAREFUL:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetCareful( entID, qtrue);	
		else
			Q3_SetCareful( entID, qfalse);	
		break;

	case SET_UNDYING:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetUndying( entID, qtrue);	
		else
			Q3_SetUndying( entID, qfalse);	
		break;

	case SET_INVINCIBLE:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetInvincible( entID, qtrue);	
		else
			Q3_SetInvincible( entID, qfalse);	
		break;

	case SET_NOAVOID:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetNoAvoid( entID, qtrue);	
		else
			Q3_SetNoAvoid( entID, qfalse);	
		break;

	case SET_SOLID:
		if(!_stricmp("true", ((char *)data)))
		{
			if ( !Q3_SetSolid( entID, qtrue) )
			{
				Q3_TaskIDSet( ent, TID_RESIZE, taskID );
				return;
			}
		}
		else
		{
			Q3_SetSolid( entID, qfalse);
		}
		break;

	case SET_BEAM:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetBeam( entID, qtrue);	
		else
			Q3_SetBeam( entID, qfalse);	
		break;

	case SET_CREATEFORMATION:
		if(!_stricmp("true", ((char *)data)))
			Q3_SetFormation( entID, qtrue);	
		else
			Q3_SetFormation( entID, qfalse);	
		break;

//	case SET_CINEMATIC:
//		Q3_DebugPrint(WL_WARNING,"Q3_SetCinematic:  Obsolete command! Please remove.\n");
//		break;
	
	case SET_INVISIBLE:
		if( !_stricmp("true", ((char *)data)) )
			Q3_SetInvisible( entID, qtrue );
		else
			Q3_SetInvisible( entID, qfalse );
		break;

	case SET_GREET_ALLIES:
		if( !_stricmp("true", ((char *)data)) )
			Q3_SetGreetAllies( entID, qtrue );
		else
			Q3_SetGreetAllies( entID, qfalse );
		break;

	case SET_PLAYER_LOCKED:
		if( !_stricmp("true", ((char *)data)) )
			Q3_SetPlayerLocked( entID, qtrue );
		else
			Q3_SetPlayerLocked( entID, qfalse );
		break;

	case SET_DETPACK:
extern void UseCharge (int entityNum);
		UseCharge( entID );	
		break;

	case SET_FORWARDMOVE:
		int_data = atoi((char *) data);
		Q3_SetForwardMove( entID, int_data);	
		break;

	case SET_RIGHTMOVE:
		int_data = atoi((char *) data);
		Q3_SetRightMove( entID, int_data);	
		break;

	case SET_LOCKYAW:
		Q3_SetLockAngle( entID, data);	
		break;
	
	case SET_CAMERA_GROUP:
		Q3_CameraGroup(entID, (char *)data);
		break;

	//FIXME: put these into camera commands
	case SET_LOOK_TARGET:
		Q3_LookTarget(entID, (char *)data);
		break;

	case SET_FACEAUX:
	case SET_FACEBLINK:
	case SET_FACEBLINKFROWN:
	case SET_FACEFROWN:
	case SET_FACENORMAL:
		float_data = atof((char *) data);
		Q3_Face(entID, toSet, float_data);
		break;

	case SET_SCROLLTEXT:
		Q3_ScrollText( (char *)data );	
		break;

	case SET_LCARSTEXT:
		Q3_LCARSText( (char *)data );	
		break;

	case SET_CAPTIONTEXTCOLOR:
		Q3_SetCaptionTextColor ( (char *)data );	
		break;
	case SET_CENTERTEXTCOLOR:
		Q3_SetCenterTextColor ( (char *)data );	
		break;
	case SET_SCROLLTEXTCOLOR:
		Q3_SetScrollTextColor ( (char *)data );	
		break;

	case SET_PLAYER_USABLE:
		if(!_stricmp("true", ((char *)data)))
		{
			Q3_SetPlayerUsable(entID, qtrue);
		}
		else
		{
			Q3_SetPlayerUsable(entID, qfalse);
		}
		break;

	case SET_STARTFRAME:
		int_data = atoi((char *) data);
		Q3_SetStartFrame(entID, int_data);
		break;
	
	case SET_ENDFRAME:
		int_data = atoi((char *) data);
		Q3_SetEndFrame(entID, int_data);

		Q3_TaskIDSet( ent, TID_ANIM_BOTH, taskID );
		return;
		break;
	
	case SET_LOOP_ANIM:
		if(!_stricmp("true", ((char *)data)))
		{
			Q3_SetLoopAnim(entID, qtrue);
		}
		else
		{
			Q3_SetLoopAnim(entID, qfalse);
		}
		break;

	case SET_INTERFACE:
		if(!_stricmp("true", ((char *)data)))
		{
			Q3_SetInterface(entID, "1");
		}
		else
		{
			Q3_SetInterface(entID, "0");
		}

		break;

	case SET_SHIELDS:
		if(!_stricmp("true", ((char *)data)))
		{
			Q3_SetShields(entID, qtrue);
		}
		else
		{
			Q3_SetShields(entID, qfalse);
		}
		break;
	
	case SET_NO_KNOCKBACK:
		if(!_stricmp("true", ((char *)data)))
		{
			Q3_SetNoKnockback(entID, qtrue);
		}
		else
		{
			Q3_SetNoKnockback(entID, qfalse);
		}
		break;

	case SET_PLAYER_WIDTH:
	case SET_WIDTH:
		int_data = atoi((char *) data);
		if ( !Q3_SetWidth ( entID, int_data) )
		{
			Q3_TaskIDSet( ent, TID_RESIZE, taskID );
			return;
		}
		break;

	case SET_HEIGHT:
		int_data = atoi((char *) data);
		if ( !Q3_SetHeight( entID, int_data) )
		{
			Q3_TaskIDSet( ent, TID_RESIZE, taskID );
			return;
		}
		break;

	case SET_CROUCHHEIGHT:
		int_data = atoi((char *) data);
		Q3_SetCrouchHeight ( entID, int_data);
		break;

	case SET_VIDEO_PLAY:
		// don't do this check now, James doesn't want a scripted cinematic to also skip any Bink cinematics as well,
		//	the "timescale" and "skippingCinematic" cvars will be set back to normal in the Bink code, so doing a
		//	skip will now only skip one section of a multiple-part story (eg VOY1 bridge sequence)
		//
//		if ( g_timescale->value <= 1.0f )
		{
			gi.SendConsoleCommand( va("inGameCinematic %s\n", (char *)data) );
		}
		break;

	case SET_VIDEO_FADE_IN:
		if(!_stricmp("true", ((char *)data)))
		{
			gi.cvar_set("cl_VidFadeUp", "1");
		}
		else
		{
			gi.cvar_set("cl_VidFadeUp", "0");
		}
		break;

	case SET_VIDEO_FADE_OUT:
		if(!_stricmp("true", ((char *)data)))
		{
			gi.cvar_set("cl_VidFadeDown", "1");
		}
		else
		{
			gi.cvar_set("cl_VidFadeDown", "0");
		}
		break;

	case SET_BOLTON_ON:
		Q3_AddBoltOn( entID, (char *)data );
		break;

	case SET_BEAMIN_BOLTON:
		Q3_BeamInBoltOn( entID, (char *)data );
		break;

	case SET_BEAMOUT_BOLTON:
		Q3_BeamOutBoltOn( entID, (char *)data );
		break;

	case SET_BOLTON_OFF:
		Q3_RemoveBoltOn( entID, (char *)data );
		break;

	case SET_BOLTON_DROP:
		Q3_DropBoltOn( entID, (char *)data );
		break;

	case SET_BOLTON_ACTIVE:
		Q3_SetActiveBoltOn( entID, (char *)data );
		break;

	case SET_BOLTON_STARTFRAME:
		int_data = atoi((char *) data);
		Q3_SetActiveBoltOnStartFrame( entID, int_data );
		break;

	case SET_BOLTON_ENDFRAME:
		int_data = atoi((char *) data);
		Q3_SetActiveBoltOnEndFrame( entID, int_data );
		break;

	case SET_BOLTON_ANIMLOOP:
		Q3_SetActiveBoltOnAnimLoop( entID, ( Q_stricmp( "true", (char *)data ) ==0 ) ? qtrue : qfalse );
		break;

	case SET_PLUG_TARGET:
		Q3_SetPlugTarget( entID, (const char *) data );
		Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );
		return;
		break;

	case SET_REMOVE_TARGET:
		Q3_SetRemoveTarget( entID, (const char *) data );
		break;

	case SET_LOADGAME:
		if ( g_virtualVoyager->integer && Q_strncmp( "_holo", level.mapname, 5 ) == 0 && ( Q_stricmp( "exitholodeck", (const char *)data) == 0 || Q_stricmp( "virtual", (const char *)data) == 0 ) )
		{//Oh, this is a hack... yes it is
			//if in a holodeck in tour mode and trying to leave, what you REALLY want to do is bring up the holodeck menu!!!  This way you can leave if desired or you can jump to any other available holodeck program
			gi.SendConsoleCommand( "genericmenu holodeck\n");
			//This hack is only necessary because we don't want to edit the maps/scripts for the holodeck maps.  Fortunately, "Star Wars" doesn't have holodecks.
		}
		else
		{
			gi.SendConsoleCommand( va("load %s\n", (const char *) data ) );
		}
		break;

	case SET_CON_COMMAND:
		gi.SendConsoleCommand( va("%s\n", (const char *) data ) );
		break;

	case SET_MENU_SCREEN:
		//ue.UI_SetActiveMenu( (const char *) data );
		break;

	case SET_OBJECTIVE_SHOW:
		Q3_SetObjective((const char *) data ,SET_OBJ_SHOW);
		Q3_SetObjective((const char *) data ,SET_OBJ_PENDING);
		break;
	case SET_OBJECTIVE_HIDE:
		Q3_SetObjective((const char *) data ,SET_OBJ_HIDE);
		break;
	case SET_OBJECTIVE_SUCCEEDED:
		Q3_SetObjective((const char *) data ,SET_OBJ_SUCCEEDED);
		Q3_CheckTourSuccess();
		break;
	case SET_OBJECTIVE_FAILED:
		Q3_SetObjective((const char *) data ,SET_OBJ_FAILED);
		break;

	case SET_TACTICAL_SHOW:
		Q3_SetTactical((const char *) data ,SET_TACTICAL_ON);

		if (g_virtualVoyager->integer)	// When in tour mode, flash text if tactical is updated
		{
			missionInfo_Updated = qtrue;	// Activate flashing text
		}
		break;

	case SET_TACTICAL_HIDE:
		Q3_SetTactical((const char *) data ,SET_TACTICAL_OFF);
		break;

	case SET_OBJECTIVE_CLEARALL:
		Q3_ObjectiveClearAll();
		break;

	case SET_CLEAR_BORG_ADAPT:
		Q3_ResetBorgAdaptHits();
		break;

	case SET_MISSIONSTATUSTEXT:
		Q3_SetStatusText((const char *) data);
		break;
		
/*
	case SET_MISSIONSTATUSACTIVE:
		Q3_TaskIDSet( ent, TID_MISSIONSTATUS, taskID );
		cg.missionStatusShow = 1;
		return;
		break;
*/
	case SET_MISSIONSTATUSTIME:
		int_data = atoi((char *) data);
		cg.missionStatusDeadTime = level.time + int_data;
		break;

	case SET_MAX_AMMO:
		int_data = atoi((char *) data);
		ammoData[AMMO_STARFLEET].max	= int_data;
		ammoData[AMMO_ALIEN].max		= int_data;
		break;

	case SET_CLOSINGCREDITS:
		gi.SendConsoleCommand( "ui_closingcredits 1\n" );
		break;

	case SET_SKILL:
	case SET_SEX:
		//can never be set
		break;

	case SET_FULLNAME:
		Q3_SetFullName( entID, (char *) data );
		break;

	default:
		//Q3_DebugPrint( WL_ERROR, "Q3_Set: '%s' is not a valid set field\n", type_name );
		Q3_SetVar( taskID, entID, type_name, data );
		break;
	}

	ent->taskManager->Completed( taskID );
}



/*
============
Q3_Kill
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *name
============
*/
static void Q3_Kill( int entID, const char *name )
{
	gentity_t	*ent = &g_entities[entID];
	gentity_t	*victim = NULL;
	int			o_health;

	if( !_stricmp( name, "self") )
	{
		victim = ent;
	}
	else if( !_stricmp( name, "enemy" ) )
	{
		victim = ent->enemy;
	}
	else
	{
		victim = G_Find (NULL, FOFS(targetname), (char *) name );
	}

	if ( !victim )
	{
		Q3_DebugPrint( WL_WARNING, "Q3_Kill: can't find %s\n", name);
		return;
	}

	if ( victim == ent )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Kill: entity %s trying to kill self - not allowed!\n", name);
		return;
	}

	o_health = victim->health;
	victim->health = 0;
	if ( victim->client )
	{
		victim->flags |= FL_NO_KNOCKBACK;
	}
	G_SetEnemy(victim, ent);
	if( victim->e_DieFunc != dieF_NULL )	// check can be omitted
	{
		GEntity_DieFunc(victim, ent, ent, o_health, MOD_UNKNOWN);
	}
}


/*
============
Q3_RemoveEnt
  Description	: 
  Return type	: void 
  Argument		: gentity_t *victim
============
*/
static void Q3_RemoveEnt( gentity_t *victim )
{
	if( victim->client )
	{
		//ClientDisconnect(ent);
		victim->s.eFlags |= EF_NODRAW;
		victim->s.eFlags &= ~EF_NPC;
		victim->svFlags &= ~SVF_NPC;
		victim->s.eType = ET_INVISIBLE;
		victim->contents = 0;
		victim->health = 0;
		victim->targetname = NULL;

		//Disappear in half a second
		victim->e_ThinkFunc = thinkF_G_FreeEntity;
		victim->nextthink = level.time + 500;
		return;
	}
	else
	{
		victim->e_ThinkFunc = thinkF_G_FreeEntity;
		victim->nextthink = level.time + 100;
	}
}


/*
============
Q3_Remove
  Description	: 
  Return type	: void 
  Argument		:  int entID
  Argument		: const char *name
============
*/
static void Q3_Remove( int entID, const char *name )
{
	gentity_t *ent = &g_entities[entID];
	gentity_t	*victim = NULL;

	if( !Q_stricmp( "self", name ) )
	{
		victim = ent;
		if ( !victim )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_Remove: can't find %s\n", name );
			return;
		}
		Q3_RemoveEnt( victim );
	}
	else if( !Q_stricmp( "enemy", name ) )
	{
		victim = ent->enemy;
		if ( !victim )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_Remove: can't find %s\n", name );
			return;
		}
		Q3_RemoveEnt( victim );
	}
	else
	{
		victim = G_Find( NULL, FOFS(targetname), (char *) name );
		if ( !victim )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_Remove: can't find %s\n", name );
			return;
		}

		while ( victim )
		{
			Q3_RemoveEnt( victim );
			victim = G_Find( victim, FOFS(targetname), (char *) name );
		}
	}
}

/*
============
MakeOwnerInvis
  Description	: 
  Return type	: void 
  Argument		: gentity_t *self
============
*/
void MakeOwnerInvis(gentity_t *self)
{
	if(self->owner && self->owner->client)
	{
		self->owner->client->ps.powerups[PW_INVIS] = level.time + 500;
	}

	//HACKHGACLHACK!! - MCG
	self->e_ThinkFunc = thinkF_RemoveOwner;
	self->nextthink = level.time + 400;
}


/*
============
MakeOwnerEnergy
  Description	: 
  Return type	: void 
  Argument		: gentity_t *self
============
*/
void MakeOwnerEnergy(gentity_t *self)
{
	if(self->owner && self->owner->client)
	{
		self->owner->client->ps.powerups[PW_QUAD] = level.time + 1000;
	}

	G_FreeEntity(self);
}

/*
============
RemoveOwner
  Description	: 
  Return type	: void 
  Argument		: gentity_t *self
============
*/
void RemoveOwner (gentity_t *self)
{
	if ( self->owner && self->owner->inuse )
	{//I have an owner and they heavn't been freed yet
		Q3_Remove( self->owner->s.number, "self" );
	}

	G_FreeEntity( self );
}

/*
=================================================

  Get / Set Functions

=================================================
*/

/*
============
Q3_GetFloat
  Description	: 
  Return type	: int 
  Argument		:  int entID
  Argument		: int type
  Argument		: const char *name
  Argument		: float *value
============
*/
static int Q3_GetFloat( int entID, int type, const char *name, float *value )
{
	gentity_t	*ent = &g_entities[entID];
	gclient_t	*client;

	if ( !ent )
	{
		return false;
	}

	int toGet = GetIDForString( setTable, name );	//FIXME: May want to make a "getTable" as well
	//FIXME: I'm getting really sick of these huge switch statements!

	//NOTENOTE: return true if the value was correctly obtained
	switch ( toGet )
	{
	case SET_PARM1:
	case SET_PARM2:
	case SET_PARM3:
	case SET_PARM4:
	case SET_PARM5:
	case SET_PARM6:
	case SET_PARM7:
	case SET_PARM8:
	case SET_PARM9:
	case SET_PARM10:
	case SET_PARM11:
	case SET_PARM12:
	case SET_PARM13:
	case SET_PARM14:
	case SET_PARM15:
	case SET_PARM16:
		if (ent->parms == NULL)
		{
			Q3_DebugPrint( WL_ERROR, "GET_PARM: %s %s did not have any parms set!\n", ent->classname, ent->targetname );
			return false;	// would prefer qfalse, but I'm fitting in with what's here <sigh>
		}
		*value = atof( ent->parms->parm[toGet - SET_PARM1] );
		break;
	
	case SET_COUNT:
		*value = ent->count;
		break;

	case SET_HEALTH:
		*value = ent->health;
		break;

	case SET_OBJECTIVEFOSTER:
		client = &level.clients[0];
//		memset(&client->sess,0,sizeof(client->sess));
		*value = client->sess.mission_objectives[OBJ_FOSTERLIVES].status;
		break;

	case SET_OBJECTIVEODELL:
		client = &level.clients[0];
//		memset(&client->sess,0,sizeof(client->sess));
		*value = client->sess.mission_objectives[OBJ_ODELLLIVES].status;
		break;

	case SET_OBJECTIVECSATLOS:
		client = &level.clients[0];
//		memset(&client->sess,0,sizeof(client->sess));
		*value = client->sess.mission_objectives[OBJ_CSATLOSLIVES].status;
		break;

	case SET_OBJECTIVEISODESIUM1:
		client = &level.clients[0];
		*value = client->sess.mission_objectives[OBJ_ISODESIUM1].status;
		break;

	case SET_OBJECTIVEISODESIUM2:
		client = &level.clients[0];
		*value = client->sess.mission_objectives[OBJ_ISODESIUM2].status;
		break;

	case SET_OBJECTIVEISODESIUM3:
		client = &level.clients[0];
		*value = client->sess.mission_objectives[OBJ_ISODESIUM3].status;
		break;

	case SET_OBJECTIVEISODESIUMALL:
		client = &level.clients[0];
		*value = client->sess.mission_objectives[OBJ_ISODESIUMALL].status;
		break;	

	case SET_OBJECTIVESECURITYCODE:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_SECURITYCODE].status;
		break;

	case SET_OBJECTIVE_COLLECTEDACTIONFIGURES:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].status;
		break;

	case SET_OBJECTIVE_HARVESTER_LEG:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_HARVESTER_LEG].status;
		break;

	case SET_OBJECTIVE_EF_POSTER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_EF_POSTER].status;
		break;

	case SET_OBJECTIVE_SEVEN_PLATE:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_SEVEN_PLATE].status;
		break;

	case SET_OBJECTIVE_FIREFLIES:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_FIREFLIES].status;
		break;

	case SET_OBJECTIVE_ARC_WELDER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_ARC_WELDER].status;
		break;

	case SET_OBJECTIVE_GRENADE_LAUNCHER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_GRENADE_LAUNCHER].status;
		break;

	case SET_OBJECTIVE_BIESSMAN_MORGUE:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_BIESSMAN_MORGUE].status;
		break;

	case SET_OBJECTIVE_BORG_SLAYER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_BORG_SLAYER].status;
		break;

	case SET_OBJECTIVE_RAVEN_PLAQUE:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_RAVEN_PLAQUE].status;
		break;

	case SET_OBJECTIVE_HERETIC_II_BOX:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_HERETIC_II_BOX].status;
		break;

	case SET_OBJECTIVE_SOF_BOX:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_SOF_BOX].status;
		break;

	case SET_OBJECTIVE_DOLL_MUNRO:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_MUNRO].status;
		break;

	case SET_OBJECTIVE_DOLL_FOSTER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_FOSTER].status;
		break;

	case SET_OBJECTIVE_DOLL_TELSIA:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_TELSIA].status;
		break;

	case SET_OBJECTIVE_DOLL_ALEXANDRIA:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_ALEXANDRIA].status;
		break;

	case SET_OBJECTIVE_DOLL_BIESSMAN:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_BIESSMAN].status;
		break;

	case SET_OBJECTIVE_DOLL_CHELL:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_CHELL].status;
		break;

	case SET_OBJECTIVE_DOLL_CHANG:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_CHANG].status;
		break;

	case SET_OBJECTIVE_DOLL_JUROT:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_JUROT].status;
		break;

	case SET_OBJECTIVE_DOLL_BORG_FOSTER:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_DOLL_BORG_FOSTER].status;
		break;

	case SET_OBJECTIVE_PHOTONBURST:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_PHOTONBURST].status;
		break;

	case SET_OBJECTIVE_TETRYONDISRUPTOR:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_TETRYONDISRUPTOR].status;
		break;

	case SET_OBJECTIVE_TOURSUCCESSFUL:
		client = &level.clients[0];
		*value = client->tourSess.tour_objectives[OBJ_TOURSUCCESS].status;
		break;

	case SET_SKILL:
		*value = g_spskill->integer;
		break;

	case SET_XVELOCITY://## %f="0.0" # Velocity along X axis
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_XVELOCITY, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.velocity[0];
		break;

	case SET_YVELOCITY://## %f="0.0" # Velocity along Y axis
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_YVELOCITY, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.velocity[1];
		break;

	case SET_ZVELOCITY://## %f="0.0" # Velocity along Z axis
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ZVELOCITY, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.velocity[2];
		break;

	case SET_DPITCH://## %f="0.0" # Pitch for NPC to turn to
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_DPITCH, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->desiredPitch;
		break;

	case SET_DYAW://## %f="0.0" # Yaw for NPC to turn to
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_DYAW, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->desiredPitch;
		break;

	case SET_TIMESCALE://## %f="0.0" # Speed-up slow down game (0 - 1.0)
		*value = g_timescale->value;
		break;

	case SET_VISRANGE://## %f="0.0" # How far away NPC can see
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_VISRANGE, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.visrange;
		break;

	case SET_EARSHOT://## %f="0.0" # How far an NPC can hear
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_EARSHOT, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.earshot;
		break;

	case SET_VIGILANCE://## %f="0.0" # How often to look for enemies (0 - 1.0)
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_VIGILANCE, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.vigilance;
		break;

	case SET_GRAVITY://## %f="0.0" # Change this ent's gravity - 800 default
		if ( (ent->svFlags&SVF_CUSTOM_GRAVITY) && ent->client )
		{
			*value = ent->client->ps.gravity;
		}
		else
		{
			*value = g_gravity->value;
		}
		break;

	case SET_PLAYER_WIDTH://## %f="0.0" # 24 default: applies to player and all NPCs
		*value = (ent->maxs[0] - ent->mins[0]);
		break;
	case SET_FACEAUX:		//## %f="0.0" # Set face to Aux expression for number of seconds
	case SET_FACEBLINK:		//## %f="0.0" # Set face to Blink expression for number of seconds
	case SET_FACEBLINKFROWN:	//## %f="0.0" # Set face to Blinkfrown expression for number of seconds
	case SET_FACEFROWN:		//## %f="0.0" # Set face to Frown expression for number of seconds
	case SET_FACENORMAL:		//## %f="0.0" # Set face to Normal expression for number of seconds
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_FACE___ not implemented\n" );
		return false;
		break;
	case SET_WAIT:		//## %f="0.0" # Change an entity's wait field
		*value = ent->wait;
		break;
	case SET_FOLLOWDIST:		//## %f="0.0" # How far away to stay from leader in BS_FOLLOW_LEADER
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_FOLLOWDIST, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->followDist;
		break;
	//# #sep ints
	case SET_ANIM_HOLDTIME_LOWER://## %d="0" # Hold lower anim for number of milliseconds
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ANIM_HOLDTIME_LOWER, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.legsAnimTimer;
		break;
	case SET_ANIM_HOLDTIME_UPPER://## %d="0" # Hold upper anim for number of milliseconds
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ANIM_HOLDTIME_UPPER, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.torsoAnimTimer;
		break;
	case SET_ANIM_HOLDTIME_BOTH://## %d="0" # Hold lower and upper anims for number of milliseconds
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ANIM_HOLDTIME_BOTH not implemented\n" );
		return false;
		break;
	case SET_ARMOR://## %d="0" # Change armor
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ARMOR, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.stats[STAT_ARMOR];
		break;
	case SET_WALKSPEED://## %d="0" # Change walkSpeed
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_WALKSPEED, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.walkSpeed;
		break;
	case SET_RUNSPEED://## %d="0" # Change runSpeed
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_RUNSPEED, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.runSpeed;
		break;
	case SET_YAWSPEED://## %d="0" # Change yawSpeed
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_YAWSPEED, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.yawSpeed;
		break;
	case SET_AGGRESSION://## %d="0" # Change aggression 1-5
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_AGGRESSION, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.aggression;
		break;
	case SET_AIM://## %d="0" # Change aim 1-5
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_AIM, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.aim;
		break;
	case SET_FRICTION://## %d="0" # Change ent's friction - 6 default
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_FRICTION, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->ps.friction;
		break;
	case SET_SHOOTDIST://## %d="0" # How far the ent can shoot - 0 uses weapon
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_SHOOTDIST, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.shootDistance;
		break;
	case SET_HFOV://## %d="0" # Horizontal field of view
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_HFOV, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.hfov;
		break;
	case SET_VFOV://## %d="0" # Vertical field of view
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_VFOV, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->stats.vfov;
		break;
	case SET_DELAYSCRIPTTIME://## %d="0" # How many seconds to wait before running delayscript
		*value = ent->delayScriptTime - level.time;
		break;
	case SET_FORWARDMOVE://## %d="0" # NPC move forward -127(back) to 127
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_FORWARDMOVE, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->forced_forwardmove;
		break;
	case SET_RIGHTMOVE://## %d="0" # NPC move right -127(left) to 127
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_RIGHTMOVE, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->forced_rightmove;
		break;
	case SET_STARTFRAME:	//## %d="0" # frame to start animation sequence on
		*value = ent->startFrame;
		break;
	case SET_ENDFRAME:	//## %d="0" # frame to end animation sequence on
		*value = ent->endFrame;
		break;
	case SET_WIDTH://## %d="0" # width of player/NPC
		*value = (ent->maxs[0] - ent->mins[0]);
		break;
	case SET_HEIGHT://## %d="0" # standing height of of player/NPC
		*value = (ent->maxs[2] - ent->mins[2]);
		break;
	case SET_CROUCHHEIGHT://## %d="0" # crouching height of of player/NPC
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_CROUCHHEIGHT, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->crouchheight;
		break;
	case SET_SHOT_SPACING://## %d="1000" # Time between shots for an NPC - reset to defaults when changes weapon
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_SHOT_SPACING, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->burstSpacing;
		break;
	case SET_MISSIONSTATUSTIME://## %d="0" # Amount of time until Mission Status should be shown after death
		*value = cg.missionStatusDeadTime - level.time;
		break;
	//# #sep booleans
	case SET_IGNOREPAIN://## %t="BOOL_TYPES" # Do not react to pain
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_IGNOREPAIN, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->ignorePain;
		break;
	case SET_IGNOREENEMIES://## %t="BOOL_TYPES" # Do not acquire enemies
		*value = (ent->svFlags&SVF_IGNORE_ENEMIES);
		break;
	case SET_IGNOREALERTS://## Do not get enemy set by allies in area(ambush)
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_IGNOREALERTS, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_IGNORE_ALERTS);
	case SET_STRAIGHTTOGOAL://## %t="BOOL_TYPES" # Don't use waypoint network to get to goal
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_STRAIGHTTOGOAL, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = ent->NPC->straightToGoal;
		break;
	case SET_DONTSHOOT://## %t="BOOL_TYPES" # Others won't shoot you
		*value = (ent->flags&FL_DONT_SHOOT);
		break;
	case SET_NOTARGET://## %t="BOOL_TYPES" # Others won't pick you as enemy
		*value = (ent->flags&FL_NOTARGET);
		break;
	case SET_DONTFIRE://## %t="BOOL_TYPES" # Don't fire your weapon
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_DONTFIRE, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_DONT_FIRE);
		break;
	case SET_NOSLOWDOWN://## %t="BOOL_TYPES" # Don't slow down approaching navgoals
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_NOSLOWDOWN, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_NO_SLOWDOWN);
		break;
	case SET_LOCKED_ENEMY://## %t="BOOL_TYPES" # Keep current enemy until dead
		*value = (ent->svFlags&SVF_LOCKEDENEMY);
		break;
	case SET_CROUCHED://## %t="BOOL_TYPES" # Force NPC to crouch
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_CROUCHED, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_CROUCHED);
		break;
	case SET_WALKING://## %t="BOOL_TYPES" # Force NPC to move at walkSpeed
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_WALKING, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_WALKING);
		break;
	case SET_RUNNING://## %t="BOOL_TYPES" # Force NPC to move at runSpeed
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_RUNNING, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_RUNNING);
		break;
	case SET_CAREFUL://## %t="BOOL_TYPES" # Force NPC to use careful: weapon ready anims
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_CAREFUL, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_CAREFUL);
		break;
	case SET_UNDYING://## %t="BOOL_TYPES" # Can take damage down to 1 but not die
		*value = (ent->flags&FL_UNDYING);
		break;
	case SET_NOAVOID://## %t="BOOL_TYPES" # Will not avoid other NPCs or architecture
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_NOAVOID, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_NO_COLL_AVOID);
		break;
	case SET_BEAM://## %t="BOOL_TYPES" # Turn on eyebeam: flashlight: etc.
		*value = (ent->s.eFlags&EF_EYEBEAM);
		break;
	case SET_CREATEFORMATION://## %t="BOOL_TYPES" # Form the player's squad
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_CREATEFORMATION not implemented\n" );
		return false;
		break;
	case SET_SOLID://## %t="BOOL_TYPES" # Make yourself notsolid or solid
		*value = ent->contents;
		break;
	case SET_PLAYER_USABLE://## %t="BOOL_TYPES" # Can be activateby the player's "use" button
		*value = (ent->svFlags&SVF_PLAYER_USABLE);
		break;
	case SET_LOOP_ANIM://## %t="BOOL_TYPES" # For non-NPCs: loop your animation sequence
		*value = ent->loopAnim;
		break;
	case SET_INTERFACE://## %t="BOOL_TYPES" # Player interface on/off
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_INTERFACE not implemented\n" );
		return false;
		break;
	case SET_SHIELDS://## %t="BOOL_TYPES" # NPC has no shields (Borg do not adapt)
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_SHIELDS, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_SHIELDS);
		break;
	case SET_INVISIBLE://## %t="BOOL_TYPES" # Makes an NPC not solid and not visible
		*value = (ent->s.eFlags&EF_NODRAW);
		break;
	case SET_GREET_ALLIES://## %t="BOOL_TYPES" # Makes an NPC greet teammates
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_GREET_ALLIES, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->aiFlags&NPCAI_GREET_ALLIES);
		break;
	case SET_VIDEO_FADE_IN://## %t="BOOL_TYPES" # Makes video playback fade in
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_VIDEO_FADE_IN not implemented\n" );
		return false;
		break;
	case SET_VIDEO_FADE_OUT://## %t="BOOL_TYPES" # Makes video playback fade out
		Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_VIDEO_FADE_OUT not implemented\n" );
		return false;
		break;
	case SET_PLAYER_LOCKED://## %t="BOOL_TYPES" # Makes it so player cannot move
		*value = player_locked;
		break;
	case SET_NO_KNOCKBACK://## %t="BOOL_TYPES" # Stops this ent from taking knockback from weapons
		*value = (ent->flags&FL_NO_KNOCKBACK);
		break;
	case SET_ALT_FIRE://## %t="BOOL_TYPES" # Force NPC to use altfire when shooting
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_ALT_FIRE, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_ALT_FIRE);
		break;
	case SET_NO_RESPONSE://## %t="BOOL_TYPES" # NPCs will do generic responses when this is on (usescripts override generic responses as well)
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_NO_RESPONSE, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_NO_RESPONSE);
		break;
	case SET_INVINCIBLE://## %t="BOOL_TYPES" # Completely unkillable
		*value = (ent->flags&FL_GODMODE);
		break;
	case SET_MISSIONSTATUSACTIVE:	//# Turns on Mission Status Screen
		*value = cg.missionStatusShow;
		break;
	case SET_NO_COMBAT_TALK://## %t="BOOL_TYPES" # NPCs will not do their combat talking noises when this is on
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetFloat: SET_NO_COMBAT_TALK, %s not an NPC\n", ent->targetname );
			return false;
		}
		*value = (ent->NPC->scriptFlags&SCF_NO_COMBAT_TALK);
		break;
	case SET_TREASONED://## %t="BOOL_TYPES" # Player has turned on his own- scripts will stop: NPCs will turn on him and level changes load the brig
		*value = (ffireLevel>=FFIRE_LEVEL_RETALIATION);
		break;

	default:
		if ( Q3_VariableDeclared( name ) != VTYPE_FLOAT )
			return false;

		return Q3_GetFloatVariable( name, value );
	}

	return true;
}


/*
============
Q3_GetVector
  Description	: 
  Return type	: int 
  Argument		:  int entID
  Argument		: int type
  Argument		: const char *name
  Argument		: vec3_t value
============
*/
static int Q3_GetVector( int entID, int type, const char *name, vec3_t value )
{
	gentity_t	*ent = &g_entities[entID];
	if ( !ent )
	{
		return false;
	}

	int toGet = GetIDForString( setTable, name );	//FIXME: May want to make a "getTable" as well
	//FIXME: I'm getting really sick of these huge switch statements!

	//NOTENOTE: return true if the value was correctly obtained
	switch ( toGet )
	{
	case SET_PARM1:
	case SET_PARM2:
	case SET_PARM3:
	case SET_PARM4:
	case SET_PARM5:
	case SET_PARM6:
	case SET_PARM7:
	case SET_PARM8:
	case SET_PARM9:
	case SET_PARM10:
	case SET_PARM11:
	case SET_PARM12:
	case SET_PARM13:
	case SET_PARM14:
	case SET_PARM15:
	case SET_PARM16:
		sscanf( ent->parms->parm[toGet - SET_PARM1], "%f %f %f", &value[0], &value[1], &value[2] );
		break;

	case SET_ORIGIN:
		VectorCopy(ent->currentOrigin, value);
		break;

	case SET_ANGLES:
		VectorCopy(ent->currentAngles, value);
		break;

	case SET_VELOCITY://## %v="0.0 0.0 0.0"  # NOT IMPLEMENTED
		if ( ent->client )
		{
			VectorCopy(ent->client->ps.velocity, value);
		}
		else
		{
			VectorCopy(ent->s.pos.trDelta, value);
		}
		break;

	case SET_AVELOCITY://## %v="0.0 0.0 0.0"  # NOT IMPLEMENTED
		if ( ent->client )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetVector: SET_AVELOCITY not implemented on clients\n" );
			return false;
		}
		else
		{
			VectorCopy(ent->s.apos.trDelta, value);
		}
		break;
	
	case SET_TELEPORT_DEST://## %v="0.0 0.0 0.0" # Set origin here as soon as the area is clear
		Q3_DebugPrint( WL_WARNING, "Q3_GetVector: SET_TELEPORT_DEST not implemented\n" );
		return false;
		break;

	default:

		if ( Q3_VariableDeclared( name ) != VTYPE_VECTOR )
			return false;

		return Q3_GetVectorVariable( name, value );
	}

	return true;
}

/*
============
Q3_GetString
  Description	: 
  Return type	: int 
  Argument		:  int entID
  Argument		: int type
  Argument		: const char *name
  Argument		: char **value
============
*/
static int Q3_GetString( int entID, int type, const char *name, char **value )
{
	gentity_t	*ent = &g_entities[entID];
	if ( !ent )
	{
		return false;
	}

	int toGet = GetIDForString( setTable, name );	//FIXME: May want to make a "getTable" as well

	switch ( toGet )
	{
	case SET_ANIM_BOTH:
		*value = (char *) Q3_GetAnimBoth( ent );

		if ( VALIDSTRING( value ) == false )
			return false;

		break;

	case SET_PARM1:
	case SET_PARM2:
	case SET_PARM3:
	case SET_PARM4:
	case SET_PARM5:
	case SET_PARM6:
	case SET_PARM7:
	case SET_PARM8:
	case SET_PARM9:
	case SET_PARM10:
	case SET_PARM11:
	case SET_PARM12:
	case SET_PARM13:
	case SET_PARM14:
	case SET_PARM15:
	case SET_PARM16:
		if ( ent->parms )
		{
			*value = (char *) ent->parms->parm[toGet - SET_PARM1];
		}
		else
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: invalid ent %s has no parms!\n", ent->targetname );
			return false;
		}
		break;

	case SET_TARGET:
		*value = (char *) ent->target;
		break;

	case SET_LOCATION:
		*value = G_GetLocationForEnt( ent );
		if ( !value || !value[0] )
		{
			return false;
		}
		break;

	case SET_SEX:
		*value = (char *) g_sex->string;
		break;

	//# #sep Scripts and other file paths
	case SET_SPAWNSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when spawned //0 - do not change these, these are equal to BSET_SPAWN, etc
		*value = ent->behaviorSet[BSET_SPAWN];
		break;
	case SET_IDLESCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # NOT IMPLEMENTED
		*value = ent->behaviorSet[BSET_IDLE];
		break;
	case SET_TOUCHSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # NOT IMPLEMENTED
		*value = ent->behaviorSet[BSET_TOUCH];
		break;
	case SET_USESCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when used
		*value = ent->behaviorSet[BSET_USE];
		break;
	case SET_AWAKESCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when startled
		*value = ent->behaviorSet[BSET_AWAKE];
		break;
	case SET_ANGERSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script run when find an enemy for the first time
		*value = ent->behaviorSet[BSET_ANGER];
		break;
	case SET_ATTACKSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when you shoot
		*value = ent->behaviorSet[BSET_ATTACK];
		break;
	case SET_VICTORYSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when killed someone
		*value = ent->behaviorSet[BSET_VICTORY];
		break;
	case SET_LOSTENEMYSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when you can't find your enemy
		*value = ent->behaviorSet[BSET_LOSTENEMY];
		break;
	case SET_PAINSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when hit
		*value = ent->behaviorSet[BSET_PAIN];
		break;
	case SET_FLEESCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when hit and low health
		*value = ent->behaviorSet[BSET_FLEE];
		break;
	case SET_DEATHSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when killed
		*value = ent->behaviorSet[BSET_DEATH];
		break;
	case SET_DELAYEDSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run after a delay
		*value = ent->behaviorSet[BSET_DELAYED];
		break;
	case SET_BLOCKEDSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when blocked by teammate
		*value = ent->behaviorSet[BSET_BLOCKED];
		break;
	case SET_FFIRESCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when player has shot own team repeatedly
		*value = ent->behaviorSet[BSET_FFIRE];
		break;
	case SET_FFDEATHSCRIPT://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.txt" # Script to run when player kills a teammate
		*value = ent->behaviorSet[BSET_FFDEATH];
		break;

	//# #sep Standard strings
	case SET_ENEMY://## %s="NULL" # Set enemy by targetname
		if ( ent->enemy != NULL )
		{
			*value = ent->enemy->targetname;
		}
		else return false;
		break;
	case SET_LEADER://## %s="NULL" # Set for BS_FOLLOW_LEADER
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_LEADER, %s not a client\n", ent->targetname );
			return false;
		}
		else if ( ent->client->leader )
		{
			*value = ent->client->leader->targetname;
		}
		else return false;
		break;
	case SET_CAPTURE://## %s="NULL" # Set captureGoal by targetname
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_CAPTURE, %s not an NPC\n", ent->targetname );
			return false;
		}
		else if ( ent->NPC->captureGoal != NULL )
		{
			*value = ent->NPC->captureGoal->targetname;
		}
		else return false;
		break;
	case SET_HIDING://## %s="NULL" # Set hidingGoal by targetname
		if ( ent->NPC == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_HIDING, %s not an NPC\n", ent->targetname );
			return false;
		}
		else if ( ent->NPC->hidingGoal != NULL )
		{
			*value = ent->NPC->hidingGoal->targetname;
		}
		else return false;
		break;
	case SET_SQUADNAME://## %s="NULL" # Who's squad you're in
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_SQUADNAME, %s not a client\n", ent->targetname );
			return false;
		}
		*value = ent->client->squadname;
		break;
	case SET_TARGETNAME://## %s="NULL" # Set/change your targetname
		*value = ent->targetname;
		break;
	case SET_PAINTARGET://## %s="NULL" # Set/change what to use when hit
		*value = ent->paintarget;
		break;
	case SET_CAMERA_GROUP://## %s="NULL" # all ents with this cameraGroup will be focused on
		*value = ent->cameraGroup;
		break;
	case SET_LOOK_TARGET://## %s="NULL" # object for NPC to look at
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_LOOK_TARGET, %s not a client\n", ent->targetname );
			return false;
		}
		else
		{
			gentity_t *lookTarg = &g_entities[ent->client->renderInfo.lookTarget];
			if ( lookTarg != NULL )
			{
				*value = lookTarg->targetname;
			}
			else return false;
		}
		break;
	case SET_TARGET2://## %s="NULL" # Set/change your target2: on NPC's: this fires when they're knocked out by the red hypo
		*value = ent->target2;
		break;
	case SET_PLUG_TARGET://## %s="INVALID" # A target for a head bot to plug into - will only return when it gets there
		*value = ent->target;
		break;
	case SET_REMOVE_TARGET://## %s="NULL" # Target that is fired when someone completes the BS_REMOVE behaviorState
		*value = ent->target3;
		break;
	case SET_WEAPON:
		if ( ent->client == NULL )
		{
			Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_WEAPON, %s not a client\n", ent->targetname );
			return false;
		}
		else
		{
			*value = (char *)GetStringForID( WPTable, ent->client->ps.weapon );
		}
		break;

	//The below cannot be gotten
	case SET_NAVGOAL://## %s="NULL" # *Move to this navgoal then continue script
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_NAVGOAL not implemented\n" );
		return false;
		break;
	case SET_VIEWTARGET://## %s="NULL" # Set angles toward ent by targetname
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_VIEWTARGET not implemented\n" );
		return false;
		break;
	case SET_CAPTIONTEXTCOLOR:	//## %s=""  # Color of text RED:WHITE:BLUE: YELLOW
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_CAPTIONTEXTCOLOR not implemented\n" );
		return false;
		break;
	case SET_CENTERTEXTCOLOR:	//## %s=""  # Color of text RED:WHITE:BLUE: YELLOW
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_CENTERTEXTCOLOR not implemented\n" );
		return false;
		break;
	case SET_SCROLLTEXTCOLOR:	//## %s=""  # Color of text RED:WHITE:BLUE: YELLOW
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_SCROLLTEXTCOLOR not implemented\n" );
		return false;
		break;
	case SET_COPY_ORIGIN://## %s="targetname"  # Copy the origin of the ent with targetname to your origin
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_COPY_ORIGIN not implemented\n" );
		return false;
		break;
	case SET_DEFEND_TARGET://## %s="targetname"  # This NPC will attack the target NPC's enemies
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_COPY_ORIGIN not implemented\n" );
		return false;
		break;
	case SET_PRECACHE://## %s="NULL" !!"Q:\quake\baseEF\real_scripts\!!#*.pre" # DISABLED!
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_PRECACHE not implemented\n" );
		return false;
		break;
	case SET_VIDEO_PLAY://## %s="filename" !!"Q:\quake\baseEF\video\!!#*.bik" # Play a bink video (inGame)
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_VIDEO_PLAY not implemented\n" );
		return false;
		break;
	case SET_LOADGAME://## %s="exitholodeck" # Load the savegame that was auto-saved when you started the holodeck
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_LOADGAME not implemented\n" );
		return false;
		break;
	case SET_CON_COMMAND://## %s="disconnect" # DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!  Stuff a console command - default is disconnect
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_CON_COMMAND not implemented\n" );
		return false;
		break;
	case SET_LOCKYAW://## %s="off"  # Lock legs to a certain yaw angle (or "off" or "auto" uses current)
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_LOCKYAW not implemented\n" );
		return false;
		break;
	case SET_SCROLLTEXT:	//## %s="" # key of text string to print
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_SCROLLTEXT not implemented\n" );
		return false;
		break;
	case SET_LCARSTEXT:	//## %s="" # key of text string to print in LCARS frame
		Q3_DebugPrint( WL_WARNING, "Q3_GetString: SET_LCARSTEXT not implemented\n" );
		return false;
		break;

	case SET_FULLNAME://## %s="NULL" # Set/change your targetname
		*value = ent->fullName;
		break;
	default:

		if ( Q3_VariableDeclared( name ) != VTYPE_STRING )
			return false;

		return Q3_GetStringVariable( name, (const char **) value );
	}

	return true;
}

/*
============
Q3_Evaluate
  Description	: 
  Return type	: int 
  Argument		:  int p1Type
  Argument		: const char *p1
  Argument		: int p2Type
  Argument		: const char *p2
  Argument		: int operatorType
============
*/
static int Q3_Evaluate( int p1Type, const char *p1, int p2Type, const char *p2, int operatorType )
{
	float	f1=0, f2=0;
	vec3_t	v1, v2;
	char	*c1=0, *c2=0;
	int		i1=0, i2=0;

	//Always demote to int on float to integer comparisons
	if ( ( ( p1Type == TK_FLOAT ) && ( p2Type == TK_INT ) ) || ( ( p1Type == TK_INT ) && ( p2Type == TK_FLOAT ) ) )
	{
		p1Type = TK_INT;
		p2Type = TK_INT;
	}

	//Cannot compare two disimilar types
	if ( p1Type != p2Type )
	{
		Q3_DebugPrint( WL_ERROR, "Q3_Evaluate comparing two disimilar types!\n");
		return false;
	}

	//Format the parameters
	switch ( p1Type )
	{
	case TK_FLOAT:
		sscanf( p1, "%f", &f1 );
		sscanf( p2, "%f", &f2 );
		break;

	case TK_INT:
		sscanf( p1, "%d", &i1 );
		sscanf( p2, "%d", &i2 );
		break;

	case TK_VECTOR:
		sscanf( p1, "%f %f %f", &v1[0], &v1[1], &v1[2] );
		sscanf( p2, "%f %f %f", &v2[0], &v2[1], &v2[2] );
		break;

	case TK_STRING:
	case TK_IDENTIFIER:
		c1 = (char *) p1;
		c2 = (char *) p2;
		break;

	default:
		Q3_DebugPrint( WL_WARNING, "Q3_Evaluate unknown type used!\n");
		return false;
	}

	//Compare them and return the result

	//FIXME: YUCK!!!  Better way to do this?

	switch ( operatorType )
	{

	//
	//	EQUAL TO
	//

	case TK_EQUALS:

		switch ( p1Type )
		{
		case TK_FLOAT:
			return (int) ( f1 == f2 );
			break;

		case TK_INT:
			return (int) ( i1 == i2 );
			break;

		case TK_VECTOR:
			return (int) VectorCompare( v1, v2 );
			break;

		case TK_STRING:
		case TK_IDENTIFIER:
			return (int) !_stricmp( c1, c2 );	//NOTENOTE: The script uses proper string comparison logic (ex. ( a == a ) == true )
			break;

		default:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate unknown type used!\n");
			return false;
		}

		break;

	//
	//	GREATER THAN
	//

	case TK_GREATER_THAN:

		switch ( p1Type )
		{
		case TK_FLOAT:
			return (int) ( f1 > f2 );
			break;

		case TK_INT:
			return (int) ( i1 > i2 );
			break;

		case TK_VECTOR:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate vector comparisons of type GREATER THAN cannot be performed!");
			return false;
			break;

		case TK_STRING:
		case TK_IDENTIFIER:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate string comparisons of type GREATER THAN cannot be performed!");
			return false;
			break;

		default:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate unknown type used!\n");
			return false;
		}

		break;

	//
	//	LESS THAN
	//

	case TK_LESS_THAN:

		switch ( p1Type )
		{
		case TK_FLOAT:
			return (int) ( f1 < f2 );
			break;

		case TK_INT:
			return (int) ( i1 < i2 );
			break;

		case TK_VECTOR:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate vector comparisons of type LESS THAN cannot be performed!");
			return false;
			break;

		case TK_STRING:
		case TK_IDENTIFIER:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate string comparisons of type LESS THAN cannot be performed!");
			return false;
			break;

		default:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate unknown type used!\n");
			return false;
		}

		break;

	//
	//	NOT
	//

	case TK_NOT:	//NOTENOTE: Implied "NOT EQUAL TO"

		switch ( p1Type )
		{
		case TK_FLOAT:
			return (int) ( f1 != f2 );
			break;

		case TK_INT:
			return (int) ( i1 != i2 );
			break;

		case TK_VECTOR:
			return (int) !VectorCompare( v1, v2 );
			break;

		case TK_STRING:
		case TK_IDENTIFIER:
			return (int) _stricmp( c1, c2 );
			break;

		default:
			Q3_DebugPrint( WL_ERROR, "Q3_Evaluate unknown type used!\n");
			return false;
		}

		break;
	
	default:
		Q3_DebugPrint( WL_ERROR, "Q3_Evaluate unknown operator used!\n");
		break;
	}

	return false;
}

/*
-------------------------
Q3_CameraFade
-------------------------
*/
static void Q3_CameraFade( float sr, float sg, float sb, float sa, float dr, float dg, float db, float da, float duration )
{
	vec4_t	src, dst;

	src[0] = sr;
	src[1] = sg;
	src[2] = sb;
	src[3] = sa;

	dst[0] = dr;
	dst[1] = dg;
	dst[2] = db;
	dst[3] = da;

	CGCam_Fade( src, dst, duration );
}

/*
-------------------------
Q3_CameraPath
-------------------------
*/
static void Q3_CameraPath( const char *name )
{
	CGCam_StartRoff( G_NewString( name ) );
}

/*
-------------------------
Q3_DebugPrint
-------------------------
*/
void Q3_DebugPrint( int level, const char *format, ... )
{
	//Don't print messages they don't want to see
	if ( g_ICARUSDebug->integer < level )
		return;

	va_list		argptr;
	char		text[1024];

	va_start (argptr, format);
	vsprintf (text, format, argptr);
	va_end (argptr);

	//Add the color formatting
	switch ( level )
	{
		case WL_ERROR:
			Com_Printf ( S_COLOR_RED"ERROR: %s", text );
			break;
		
		case WL_WARNING:
			Com_Printf ( S_COLOR_YELLOW"WARNING: %s", text );
			break;
		
		case WL_DEBUG:
			{
				int		entNum;
				char	*buffer;

				sscanf( text, "%d", &entNum );

				if ( ( ICARUS_entFilter >= 0 ) && ( ICARUS_entFilter != entNum ) )
					return;

				buffer = (char *) text;
				buffer += 5;

				if ( ( entNum < 0 ) || ( entNum > MAX_GENTITIES ) )
					entNum = 0;

				Com_Printf ( S_COLOR_BLUE"DEBUG: %s(%d): %s\n", g_entities[entNum].script_targetname, entNum, buffer );
				break;
			}
		default:
		case WL_VERBOSE:
			Com_Printf ( S_COLOR_GREEN"INFO: %s", text );
			break;
	}
}

/*
-------------------------
Q3_Play
-------------------------
*/
static void Q3_Play( int taskID, int entID, const char *type, const char *name )
{
	gentity_t *ent = &g_entities[entID];

	if ( !_stricmp( type, "PLAY_ROFF" ) )
	{
		// Try to load the requested ROFF
		if ( G_LoadRoff( name ) )
		{
			ent->roff = G_NewString( name );

			// Start the roff from the beginning
			ent->roff_ctr = 0;

			//Save this off for later
			Q3_TaskIDSet( ent, TID_MOVE_NAV, taskID );

			// Let the ROFF playing start.
			ent->next_roff_time = level.time;

			// These need to be initialised up front...
			VectorCopy( ent->currentOrigin, ent->pos1 );
			VectorCopy( ent->currentAngles, ent->pos2 );
			
			gi.linkentity( ent );
		}
	}
}

/*
============
Interface_Init
  Description	: Inits the interface for the game
  Return type	: void 
  Argument		: interface_export_t *pe
============
*/
void Interface_Init( interface_export_t *pe )
{
	//TODO: This is where you link up all your functions to the engine

	//General
	pe->I_LoadFile				=	Q3_ReadScript;
	pe->I_CenterPrint			=	Q3_CenterPrint;
	pe->I_DPrintf				=	Q3_DebugPrint;
	pe->I_GetEntityByName		=	Q3_GetEntityByName;
	pe->I_GetTime				=	Q3_GetTime;
	pe->I_GetTimeScale			=	Q3_GetTimeScale;
	pe->I_PlaySound				=	Q3_PlaySound;
	pe->I_Lerp2Pos				=	Q3_Lerp2Pos;
	pe->I_Lerp2Origin			=	Q3_Lerp2Origin;
	pe->I_Lerp2Angles			=	Q3_Lerp2Angles;
	pe->I_GetTag				=	Q3_GetTag;
	pe->I_Lerp2Start			=	Q3_Lerp2Start;
	pe->I_Lerp2End				=	Q3_Lerp2End;	
	pe->I_Use					=	Q3_Use;
	pe->I_Kill					=	Q3_Kill;
	pe->I_Remove				=	Q3_Remove;
	pe->I_Set					=	Q3_Set;
	pe->I_Random				=	Q_flrand;
	pe->I_Play					=	Q3_Play;

	//Camera functions
	pe->I_CameraEnable			=	CGCam_Enable;
	pe->I_CameraDisable			=	CGCam_Disable;
	pe->I_CameraZoom			=	CGCam_Zoom;
	pe->I_CameraMove			=	CGCam_Move;
	pe->I_CameraPan				=	CGCam_Pan;
	pe->I_CameraRoll			=	CGCam_Roll;
	pe->I_CameraTrack			=	CGCam_Track;
	pe->I_CameraFollow			=	CGCam_Follow;
	pe->I_CameraDistance		=	CGCam_Distance;
	pe->I_CameraShake			=	CGCam_Shake;
	pe->I_CameraFade			=	Q3_CameraFade;
	pe->I_CameraPath			=	Q3_CameraPath;

	//Variable information
	pe->I_GetFloat				=	Q3_GetFloat;
	pe->I_GetVector				=	Q3_GetVector;
	pe->I_GetString				=	Q3_GetString;

	pe->I_Evaluate				=	Q3_Evaluate;

	pe->I_DeclareVariable		=	Q3_DeclareVariable;
	pe->I_FreeVariable			=	Q3_FreeVariable;

	//Save / Load functions
	pe->I_WriteSaveData			=	gi.AppendToSaveGame;
	pe->I_ReadSaveData			=	gi.ReadFromSaveGame;
	pe->I_LinkEntity			=	ICARUS_LinkEntity;

	precacheWav_i=0;	// For the sound precache array
	precacheText_i=0;	// For the text precache array

	gclient_t	*client;
	client = &level.clients[0];
	memset(&client->sess,0,sizeof(client->sess));
	memset(&client->tourSess,0,sizeof(client->tourSess));
}
