
#include "g_local.h"
#include "g_functions.h"
#include "boltOns.h"
#include "g_infostrings.h"

extern cvar_t *g_spskill;
extern void NAV_GenerateSquadRoutes (int squadPathNum);
// these vars I moved here out of the level_locals_t struct simply because it's pointless to try saving them,
//	and the level_locals_t struct is included in the save process... -slc
//
qboolean	spawning = qfalse;				// the G_Spawn*() functions are valid  (only turned on during one function)
int			numSpawnVars;
char		*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
int			numSpawnVarChars;
char		spawnVarChars[MAX_SPAWN_VARS_CHARS];

//NOTENOTE: Be sure to change the mirrored code in cgmain.cpp
typedef	std::map< std::string, unsigned char, std::less<std::string>, std::allocator< unsigned char >  >	namePrecache_m;
namePrecache_m	as_preCacheMap;

qboolean	G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	if ( !spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < numSpawnVars ; i++ ) {
		if ( !strcmp( key, spawnVars[i][0] ) ) {
			*out = spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

qboolean	G_SpawnVector4( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3] );
	return present;
}

qboolean	G_SpawnFlag( const char *key, int flag, int *out )
{
	//find that key
	for ( int i = 0 ; i < numSpawnVars ; i++ ) 
	{
		if ( !strcmp( key, spawnVars[i][0] ) ) 
		{
			//found the key
			if ( atoi( spawnVars[i][1] ) != 0 )
			{//if it's non-zero, and in the flag
				*out |= flag;
			}
			else
			{//if it's zero, or out the flag
				*out &= ~flag;
			}
			return qtrue;
		}
	}

	return qfalse;
}

stringID_table_t flagTable [] =
{
	"noTED", EF_NO_TED,
	//stringID_table_t Must end with a null entry
	"", NULL
};

//
// fields are needed for spawning from the entity string
//
typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_VECTOR4,
	F_ANGLEHACK,
	F_ENTITY,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_PARM1,			// Special case for parms
	F_PARM2,			// Special case for parms
	F_PARM3,			// Special case for parms
	F_PARM4,			// Special case for parms
	F_PARM5,			// Special case for parms
	F_PARM6,			// Special case for parms
	F_PARM7,			// Special case for parms
	F_PARM8,			// Special case for parms
	F_PARM9,			// Special case for parms
	F_PARM10,			// Special case for parms
	F_PARM11,			// Special case for parms
	F_PARM12,			// Special case for parms
	F_PARM13,			// Special case for parms
	F_PARM14,			// Special case for parms
	F_PARM15,			// Special case for parms
	F_PARM16,			// Special case for parms
	F_FLAG,				// special case for flags
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;

field_t fields[] = {
	//Fields for benefit of Radiant only
	{"autobound", FOFS(classname), F_IGNORE},
	{"groupname", FOFS(classname), F_IGNORE},
	{"noBasicSounds", FOFS(classname), F_IGNORE},//will be looked at separately
	{"noCombatSounds", FOFS(classname), F_IGNORE},//will be looked at separately
	{"noExtraSounds", FOFS(classname), F_IGNORE},//will be looked at separately

	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"mins", FOFS(mins), F_VECTOR},
	{"maxs", FOFS(maxs), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"model2", FOFS(model2), F_LSTRING},
	{"model3", FOFS(target), F_LSTRING},//for misc_replicator_item only!!!
	{"model4", FOFS(target2), F_LSTRING},//for misc_replicator_item only!!!
	{"model5", FOFS(target3), F_LSTRING},//for misc_replicator_item only!!!
	{"model6", FOFS(target4), F_LSTRING},//for misc_replicator_item only!!!
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"duration", FOFS(speed), F_FLOAT},//for psycho jism
	{"gravity", FOFS(speed), F_FLOAT},//For target_gravity_change
	{"leadDist", FOFS(speed), F_FLOAT},//For waypoint_squadpath
	{"interest", FOFS(health), F_INT},//For target_interest
	{"target", FOFS(target), F_LSTRING},
	{"target2", FOFS(target2), F_LSTRING},
	{"target3", FOFS(target3), F_LSTRING},
	{"target4", FOFS(target4), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"material", FOFS(material), F_INT},
	{"message", FOFS(message), F_LSTRING},
	{"infoString", FOFS(infoString), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"mapname", FOFS(message), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"finaltime", FOFS(wait), F_FLOAT},//For dlight
	{"random", FOFS(random), F_FLOAT},
	{"FOV", FOFS(random), F_FLOAT},//for ref_tags and trigger_visibles
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"friction", FOFS(health), F_INT},//For target_friction_change
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(damage), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"cameraGroup", FOFS(cameraGroup), F_LSTRING},
	{"radius", FOFS(radius), F_FLOAT},
	{"hiderange", FOFS(radius), F_FLOAT},//for triggers only
	{"starttime", FOFS(radius), F_FLOAT},//for dlight
	{"type", FOFS(count), F_FLOAT},//for fx_crew_beam_in
	{"fullName", FOFS(fullName), F_LSTRING},

//Script parms - will this handle clamping to 16 or whatever length of parm[0] is?
	{"parm1", 0, F_PARM1},
	{"parm2", 0, F_PARM2},
	{"parm3", 0, F_PARM3},
	{"parm4", 0, F_PARM4},
	{"parm5", 0, F_PARM5},
	{"parm6", 0, F_PARM6},
	{"parm7", 0, F_PARM7},
	{"parm8", 0, F_PARM8},
	{"parm9", 0, F_PARM9},
	{"parm10", 0, F_PARM10},
	{"parm11", 0, F_PARM11},
	{"parm12", 0, F_PARM12},
	{"parm13", 0, F_PARM13},
	{"parm14", 0, F_PARM14},
	{"parm15", 0, F_PARM15},
	{"parm16", 0, F_PARM16},
	{"noTED", FOFS(s.eFlags), F_FLAG},

//MCG - Begin
	//extra fields for ents
	{"delay", FOFS(delay), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"closetarget", FOFS(closetarget), F_LSTRING},//for doors
	{"opentarget", FOFS(opentarget), F_LSTRING},//for doors
	{"paintarget", FOFS(paintarget), F_LSTRING},//for doors
	{"soundGroup", FOFS(paintarget), F_LSTRING},//for target_speakers
	{"backwardstarget", FOFS(paintarget), F_LSTRING},//for trigger_bidirectional
	{"splashDamage", FOFS(splashDamage), F_INT},
	{"splashRadius", FOFS(splashRadius), F_INT},
	//Script stuff
	{"spawnscript", FOFS(behaviorSet[BSET_SPAWN]), F_LSTRING},//name of script to run
	{"idlescript", FOFS(behaviorSet[BSET_IDLE]), F_LSTRING},//name of script to run
	{"touchscript", FOFS(behaviorSet[BSET_TOUCH]), F_LSTRING},//name of script to run
	{"usescript", FOFS(behaviorSet[BSET_USE]), F_LSTRING},//name of script to run
	{"awakescript", FOFS(behaviorSet[BSET_AWAKE]), F_LSTRING},//name of script to run
	{"angerscript", FOFS(behaviorSet[BSET_ANGER]), F_LSTRING},//name of script to run
	{"attackscript", FOFS(behaviorSet[BSET_ATTACK]), F_LSTRING},//name of script to run
	{"victoryscript", FOFS(behaviorSet[BSET_VICTORY]), F_LSTRING},//name of script to run
	{"lostenemyscript", FOFS(behaviorSet[BSET_LOSTENEMY]), F_LSTRING},//name of script to run
	{"painscript", FOFS(behaviorSet[BSET_PAIN]), F_LSTRING},//name of script to run
	{"fleescript", FOFS(behaviorSet[BSET_FLEE]), F_LSTRING},//name of script to run
	{"deathscript", FOFS(behaviorSet[BSET_DEATH]), F_LSTRING},//name of script to run
	{"delayscript", FOFS(behaviorSet[BSET_DELAYED]), F_LSTRING},//name of script to run
	{"delayscripttime", FOFS(delayScriptTime), F_INT},//name of script to run
	{"blockedscript", FOFS(behaviorSet[BSET_BLOCKED]), F_LSTRING},//name of script to run
	{"ffirescript", FOFS(behaviorSet[BSET_FFIRE]), F_LSTRING},//name of script to run
	{"ffdeathscript", FOFS(behaviorSet[BSET_FFDEATH]), F_LSTRING},//name of script to run
	{"script_targetname", FOFS(script_targetname), F_LSTRING},//scripts look for this when "affecting"
	//For NPCs
	//{"playerTeam", FOFS(playerTeam), F_INT},
	//{"enemyTeam", FOFS(enemyTeam), F_INT},
	{"NPC_targetname", FOFS(NPC_targetname), F_LSTRING},
	{"NPC_target", FOFS(NPC_target), F_LSTRING},
	{"NPC_target2", FOFS(target2), F_LSTRING},//NPC_spawner only
	{"NPC_type", FOFS(NPC_type), F_LSTRING},
	//formation
	//{"squadname", FOFS(squadname), F_LSTRING},
	{"ownername", FOFS(ownername), F_LSTRING},
	{"squadtarget", FOFS(paintarget), F_LSTRING},//For squadpoints
	//freaky camera shit
	{"startRGBA", FOFS(startRGBA), F_VECTOR4},
	{"finalRGBA", FOFS(finalRGBA), F_VECTOR4},
//MCG - End

	{"soundSet", FOFS(soundSet), F_LSTRING},


	{NULL}
};


typedef struct {
	char	*name;
	void	(*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start (gentity_t *ent);
void SP_info_player_deathmatch (gentity_t *ent);
void SP_info_player_intermission (gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);

void SP_func_plat (gentity_t *ent);
void SP_func_static (gentity_t *ent);
void SP_func_rotating (gentity_t *ent);
void SP_func_bobbing (gentity_t *ent);
void SP_func_breakable (gentity_t *self);
void SP_func_pendulum( gentity_t *ent );
void SP_func_button (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_train (gentity_t *ent);
void SP_func_timer (gentity_t *self);
void SP_func_wall (gentity_t *ent);
void SP_func_usable( gentity_t *self );
void SP_func_stasis_door( gentity_t *self );

void SP_trigger_always (gentity_t *ent);
void SP_trigger_multiple (gentity_t *ent);
void SP_trigger_once (gentity_t *ent);
void SP_trigger_push (gentity_t *ent);
void SP_trigger_teleport (gentity_t *ent);
void SP_trigger_hurt (gentity_t *ent);
void SP_trigger_formation (gentity_t *ent);
void SP_trigger_bidirectional (gentity_t *ent);
void SP_trigger_entdist (gentity_t *self);
void SP_trigger_location( gentity_t *ent );
void SP_trigger_visible( gentity_t *self );

void SP_target_give (gentity_t *ent);
void SP_target_delay (gentity_t *ent);
void SP_target_speaker (gentity_t *ent);
void SP_target_print (gentity_t *ent);
void SP_target_laser (gentity_t *self);
void SP_target_character (gentity_t *ent);
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay (gentity_t *ent);
void SP_target_kill (gentity_t *ent);
void SP_target_position (gentity_t *ent);
void SP_target_location (gentity_t *ent);
void SP_target_push (gentity_t *ent);
void SP_target_random (gentity_t *self);
void SP_target_counter (gentity_t *self);
void SP_target_create_formation (gentity_t *self);
void SP_target_scriptrunner (gentity_t *self);
void SP_target_interest (gentity_t *self);
void SP_target_activate (gentity_t *self);
void SP_target_deactivate (gentity_t *self);
void SP_target_gravity_change( gentity_t *self );
void SP_target_friction_change( gentity_t *self );
void SP_target_level_change( gentity_t *self );
void SP_target_change_parm( gentity_t *self );
void SP_target_play_music( gentity_t *self );
void SP_target_autosave( gentity_t *self );
void SP_target_interface( gentity_t *self );

void SP_light (gentity_t *self);
void SP_info_null (gentity_t *self);
void SP_info_notnull (gentity_t *self);
void SP_path_corner (gentity_t *self);

void SP_misc_teleporter (gentity_t *self);
void SP_misc_teleporter_dest (gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_turret (gentity_t *base);
void SP_laser_arm (gentity_t *base);

void SP_misc_model_borg_ammo( gentity_t *ent );
void SP_misc_model_borg_health( gentity_t *ent );
void SP_misc_model_borg_disnode( gentity_t *ent );

void SP_misc_model_stasis_ammo( gentity_t *ent );
void SP_misc_model_stasis_ammo_floor( gentity_t *ent );
void SP_misc_model_stasis_health( gentity_t *ent );
void SP_misc_model_stasis_health_floor( gentity_t *ent );

void SP_misc_model_scavenger_ammo( gentity_t *ent );
void SP_misc_model_scavenger_health( gentity_t *ent );

void SP_misc_model_forge_ammo( gentity_t *ent );
void SP_misc_model_forge_health( gentity_t *ent );
void SP_misc_model_forge_panel( gentity_t *ent );
void SP_misc_model_forge_defence_grid( gentity_t *ent );
void SP_misc_model_forge_reaver_container( gentity_t *ent );
void SP_misc_model_forge_mega_ammo( gentity_t *ent );

void SP_misc_model_dreadnought_ammo( gentity_t *ent );
void SP_misc_model_dreadnought_health( gentity_t *ent );
void SP_misc_model_stasis_control_switch( gentity_t *ent );

void SP_misc_model_proton_ammo( gentity_t *ent );
void SP_misc_model_proton_health( gentity_t *ent );

void SP_misc_model_plasma_filter( gentity_t *ent );

void SP_misc_model_stasis_crystal_station( gentity_t *ent );
void SP_misc_model_borg_crystal_station( gentity_t *ent );
void SP_misc_model_scav_crystal_station( gentity_t *ent );
void SP_misc_model_dn_crystal_station( gentity_t *ent );

void SP_misc_model_breakable(gentity_t *ent);//stays as an ent
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);
void SP_misc_camera_focus (gentity_t *self);
void SP_misc_camera_track (gentity_t *self);
void SP_misc_dlight (gentity_t *ent);

void SP_misc_stasis_shooter( gentity_t *self );
void SP_shooter_rocket( gentity_t *ent );
void SP_shooter_plasma( gentity_t *ent );
void SP_shooter_grenade( gentity_t *ent );
void SP_misc_replicator_item( gentity_t *ent );


//New spawn functions
void SP_reference_tag ( gentity_t *ent );

void SP_NPC_spawner( gentity_t *self );
void SP_NPC_starfleet( gentity_t *self );
void SP_NPC_starfleet_random( gentity_t *self );

void SP_NPC_Tuvok( gentity_t *self );
void SP_NPC_Kim( gentity_t *self );
void SP_NPC_Doctor( gentity_t *self);
void SP_NPC_Paris( gentity_t *self);
void SP_NPC_Torres( gentity_t *self);
void SP_NPC_Janeway( gentity_t *self);
void SP_NPC_Seven( gentity_t *self);
void SP_NPC_Chakotay( gentity_t *self);
void SP_NPC_Neelix( gentity_t *self);
void SP_NPC_Vorik( gentity_t *self);

void SP_NPC_Foster( gentity_t *self);
void SP_NPC_Munro( gentity_t *self);
void SP_NPC_MunroScav( gentity_t *self);
void SP_NPC_Telsia( gentity_t *self);
void SP_NPC_Biessman( gentity_t *self);
void SP_NPC_Chang( gentity_t *self);
void SP_NPC_Chell( gentity_t *self);
void SP_NPC_Jurot( gentity_t *self);

void SP_NPC_Parasite( gentity_t *self );
void SP_NPC_borg( gentity_t *self );
void SP_NPC_Stasis( gentity_t *self );
void SP_NPC_klingon( gentity_t *self );
void SP_NPC_Malon( gentity_t *self );
void SP_NPC_Hirogen( gentity_t *self );
void SP_NPC_Hirogen_Alpha( gentity_t *self);
void SP_NPC_Imperial( gentity_t *self );
void SP_NPC_Species8472( gentity_t *self );
void SP_NPC_Headbot( gentity_t *self );
void SP_NPC_scoutbot( gentity_t *self );
void SP_NPC_HunterSeeker( gentity_t *self );
void SP_NPC_HKbot( gentity_t *self );	//FIXME: Obsolete
void SP_NPC_warriorbot( gentity_t *self );
void SP_NPC_Harvester( gentity_t *self );
void SP_NPC_Reaver( gentity_t *self );
void SP_NPC_ReaverGuard( gentity_t *self );
void SP_NPC_Avatar( gentity_t *self );
void SP_NPC_Vohrsoth( gentity_t *self );

void SP_NPC_Desperado( gentity_t *self );
void SP_NPC_Paladin( gentity_t *self );
void SP_NPC_BioHulk( gentity_t *self );
void SP_NPC_ChaoticaGuard( gentity_t *self );
void SP_NPC_Chaotica( gentity_t *self );
void SP_NPC_SatansRobot( gentity_t *self );
void SP_NPC_CaptainProton( gentity_t *self );
void SP_NPC_Buster( gentity_t *self );
void SP_NPC_Goodheart( gentity_t *self );

void SP_ambient_etherian_mine( gentity_t *self );
void SP_ambient_etherian_fixits( gentity_t *self );
void SP_ambient_etherian_fliers (gentity_t *self);

void SP_waypoint (gentity_t *ent);
void SP_waypoint_small (gentity_t *ent);
void SP_waypoint_squadpath (gentity_t *ent);
void SP_waypoint_navgoal (gentity_t *ent);
void SP_waypoint_navgoal_8 (gentity_t *ent);
void SP_waypoint_navgoal_4 (gentity_t *ent);
void SP_waypoint_navgoal_2 (gentity_t *ent);
void SP_waypoint_navgoal_1 (gentity_t *ent);

void SP_fx_spark ( gentity_t *ent );
void SP_fx_steam ( gentity_t *ent );
void SP_fx_bolt	( gentity_t *ent );
void SP_fx_drip ( gentity_t *ent );
void SP_fx_plasma ( gentity_t *ent );
void SP_fx_electricfire ( gentity_t *ent );
void SP_fx_surface_explosion( gentity_t *ent );
void SP_fx_electrical_explosion( gentity_t *ent );
void SP_fx_smoke ( gentity_t *ent );
void SP_fx_teleporter ( gentity_t *ent );
void SP_fx_stream ( gentity_t *ent );
void SP_fx_psycho_jism( gentity_t *ent );
void SP_fx_cooking_steam( gentity_t *ent );
void SP_fx_laser( gentity_t *ent );
void SP_fx_transporter_stream( gentity_t *ent );
void SP_fx_explosion_trail( gentity_t *ent );
void SP_fx_blow_chunks( gentity_t *ent );
void SP_fx_borg_energy_beam( gentity_t *ent );
void SP_fx_shimmery_thing( gentity_t *ent );
void SP_fx_borg_bolt ( gentity_t *ent );
void SP_fx_crew_beam_in( gentity_t *ent );
void SP_fx_forge_bolt( gentity_t *ent );
void SP_fx_stasis_teleporter( gentity_t *ent );
void SP_fx_stasis_mushrooms( gentity_t *ent );
void SP_fx_dn_beam_glow( gentity_t *ent );
void SP_fx_garden_fountain( gentity_t *ent );

void SP_object_cargo_barrel1( gentity_t *ent );

void SP_point_combat (gentity_t *self);

spawn_t	spawns[] = {
	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_static", SP_func_static},
	{"func_rotating", SP_func_rotating},
	{"func_bobbing", SP_func_bobbing},
	{"func_breakable", SP_func_breakable},
	{"func_pendulum", SP_func_pendulum},
	{"func_train", SP_func_train},
	{"func_timer", SP_func_timer},			// rename trigger_timer?
	{"func_wall", SP_func_wall},
	{"func_usable", SP_func_usable},
	{"func_stasis_door", SP_func_stasis_door},

	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_once", SP_trigger_once},
	{"trigger_push", SP_trigger_push},
	{"trigger_teleport", SP_trigger_teleport},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_formation", SP_trigger_formation},
	{"trigger_bidirectional", SP_trigger_bidirectional},
	{"trigger_entdist", SP_trigger_entdist},
	{"trigger_location", SP_trigger_location},
	{"trigger_visible", SP_trigger_visible},

	{"target_give", SP_target_give},
	{"target_delay", SP_target_delay},
	{"target_speaker", SP_target_speaker},
	{"target_print", SP_target_print},
	{"target_laser", SP_target_laser},
	{"target_score", SP_target_score},
	{"target_teleporter", SP_target_teleporter},
	{"target_relay", SP_target_relay},
	{"target_kill", SP_target_kill},
	{"target_position", SP_target_position},
	{"target_location", SP_target_location},
	{"target_push", SP_target_push},
	{"target_random", SP_target_random},
	{"target_counter", SP_target_counter},
	{"target_create_formation", SP_target_create_formation},
	{"target_scriptrunner", SP_target_scriptrunner},
	{"target_interest", SP_target_interest},
	{"target_activate", SP_target_activate},
	{"target_deactivate", SP_target_deactivate},
	{"target_gravity_change", SP_target_gravity_change},
	{"target_friction_change", SP_target_friction_change},
	{"target_level_change", SP_target_level_change},
	{"target_change_parm", SP_target_change_parm},
	{"target_play_music", SP_target_play_music},
	{"target_autosave", SP_target_autosave},
	{"target_interface", SP_target_interface},

	{"light", SP_light},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},		// use target_position instead
	{"path_corner", SP_path_corner},

	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_model", SP_misc_model},
	{"misc_turret", SP_misc_turret},
	{"misc_laser_arm", SP_laser_arm},

	// Borg misc models
	{"misc_model_borg_ammo", SP_misc_model_borg_ammo},
	{"misc_model_borg_health", SP_misc_model_borg_health},
	{"misc_model_borg_disnode", SP_misc_model_borg_disnode},

	// Stasis misc models
	{"misc_model_stasis_ammo", SP_misc_model_stasis_ammo},
	{"misc_model_stasis_health", SP_misc_model_stasis_health},
	{"misc_model_stasis_ammo_floor", SP_misc_model_stasis_ammo_floor},
	{"misc_model_stasis_health_floor", SP_misc_model_stasis_health_floor},

	{"misc_model_forge_ammo", SP_misc_model_forge_ammo},
	{"misc_model_forge_health", SP_misc_model_forge_health},
	{"misc_model_forge_panel", SP_misc_model_forge_panel},
	{"misc_model_forge_defence_grid", SP_misc_model_forge_defence_grid},
	{"misc_model_forge_reaver_container", SP_misc_model_forge_reaver_container},
	{"misc_model_forge_mega_ammo", SP_misc_model_forge_mega_ammo},

	{"misc_model_dreadnought_ammo", SP_misc_model_dreadnought_ammo},
	{"misc_model_dreadnought_health", SP_misc_model_dreadnought_health},

	{"misc_model_proton_ammo", SP_misc_model_proton_ammo},
	{"misc_model_proton_health", SP_misc_model_proton_health},

	{"misc_model_stasis_control_switch", SP_misc_model_stasis_control_switch},

	{"misc_model_plasma_filter", SP_misc_model_plasma_filter},

	{"misc_model_stasis_crystal_station", SP_misc_model_stasis_crystal_station},
	{"misc_model_borg_crystal_station", SP_misc_model_borg_crystal_station},
	{"misc_model_dn_crystal_station", SP_misc_model_dn_crystal_station},
	{"misc_model_scav_crystal_station", SP_misc_model_scav_crystal_station},
	{"misc_model_scavenger_ammo", SP_misc_model_scavenger_ammo},
	{"misc_model_scavenger_health", SP_misc_model_scavenger_health},

	{"misc_model_breakable", SP_misc_model_breakable},
	{"misc_portal_surface", SP_misc_portal_surface},
	{"misc_portal_camera", SP_misc_portal_camera},
	{"misc_camera_focus", SP_misc_camera_focus},
	{"misc_camera_track", SP_misc_camera_track},
	{"misc_dlight", SP_misc_dlight},
	{"misc_stasis_shooter", SP_misc_stasis_shooter},
	{"misc_replicator_item", SP_misc_replicator_item},
	
	{"shooter_rocket", SP_shooter_rocket},
	{"shooter_grenade", SP_shooter_grenade},
	{"shooter_plasma", SP_shooter_plasma},

	{"ref_tag",	SP_reference_tag},

	//new NPC ents
	{"NPC_spawner", SP_NPC_spawner},
	{"NPC_starfleet", SP_NPC_starfleet},
	{"NPC_starfleet_random", SP_NPC_starfleet_random},
	{"NPC_Tuvok", SP_NPC_Tuvok },
	{"NPC_Kim", SP_NPC_Kim },
	{"NPC_Doctor", SP_NPC_Doctor},
	{"NPC_Paris", SP_NPC_Paris},
	{"NPC_Torres", SP_NPC_Torres},
	{"NPC_Janeway", SP_NPC_Janeway},
	{"NPC_Seven", SP_NPC_Seven},
	{"NPC_Chakotay", SP_NPC_Chakotay},
	{"NPC_Neelix", SP_NPC_Neelix},
	{"NPC_Vorik", SP_NPC_Vorik},
	{"NPC_Foster", SP_NPC_Foster},
	{"NPC_Munro", SP_NPC_Munro},
	{"NPC_MunroScav", SP_NPC_MunroScav},
	{"NPC_Telsia", SP_NPC_Telsia},
	{"NPC_Biessman", SP_NPC_Biessman},
	{"NPC_Chang", SP_NPC_Chang},
	{"NPC_Chell", SP_NPC_Chell},
	{"NPC_Jurot", SP_NPC_Jurot},
	{"NPC_Parasite", SP_NPC_Parasite},
	{"NPC_borg", SP_NPC_borg},
	{"NPC_Stasis", SP_NPC_Stasis},
	{"NPC_klingon", SP_NPC_klingon},
	{"NPC_Malon", SP_NPC_Malon},
	{"NPC_Hirogen", SP_NPC_Hirogen},
	{"NPC_Hirogen_Alpha", SP_NPC_Hirogen_Alpha},
	{"NPC_Imperial", SP_NPC_Imperial},
	{"NPC_Imperial_Blue", SP_NPC_Imperial},
	{"NPC_Imperial_Gold", SP_NPC_Imperial},
	{"NPC_Imperial_Raider", SP_NPC_Imperial},
	{"NPC_Headbot", SP_NPC_Headbot},
	{"NPC_scoutbot", SP_NPC_scoutbot},
	{"NPC_HKbot", SP_NPC_HKbot},	//FIXME: Obsolete
	{"NPC_HunterSeeker", SP_NPC_HunterSeeker},
	{"NPC_warriorbot", SP_NPC_warriorbot},
	{"NPC_Harvester", SP_NPC_Harvester},
	{"NPC_Reaver", SP_NPC_Reaver},
	{"NPC_ReaverGuard", SP_NPC_ReaverGuard},
	{"NPC_Avatar", SP_NPC_Avatar},
	{"NPC_Species8472", SP_NPC_Species8472},
	{"NPC_Vohrsoth", SP_NPC_Vohrsoth},

	{"NPC_Desperado", SP_NPC_Desperado},
	{"NPC_Paladin", SP_NPC_Paladin},
	{"NPC_BioHulk", SP_NPC_BioHulk},
	{"NPC_ChaoticaGuard", SP_NPC_ChaoticaGuard},
	{"NPC_Chaotica", SP_NPC_Chaotica},
	{"NPC_SatansRobot", SP_NPC_SatansRobot},
	{"NPC_CaptainProton", SP_NPC_CaptainProton},
	{"NPC_Buster", SP_NPC_Buster},
	{"NPC_Goodheart", SP_NPC_Goodheart},

	{"ambient_etherian_mine", SP_ambient_etherian_mine},
	{"ambient_etherian_fixits", SP_ambient_etherian_fixits},
	{"ambient_etherian_fliers", SP_ambient_etherian_fliers},

	{"waypoint", SP_waypoint},
	{"waypoint_small", SP_waypoint_small},
	{"waypoint_squadpath", SP_waypoint_squadpath},
	{"waypoint_navgoal", SP_waypoint_navgoal},
	{"waypoint_navgoal_8", SP_waypoint_navgoal_8},
	{"waypoint_navgoal_4", SP_waypoint_navgoal_4},
	{"waypoint_navgoal_2", SP_waypoint_navgoal_2},
	{"waypoint_navgoal_1", SP_waypoint_navgoal_1},
	
	{"fx_spark", SP_fx_spark},
	{"fx_steam", SP_fx_steam},
	{"fx_bolt", SP_fx_bolt},
	{"fx_drip", SP_fx_drip},
	{"fx_plasma", SP_fx_plasma},
	{"fx_elecfire", SP_fx_electricfire},
	{"fx_surface_explosion", SP_fx_surface_explosion},
	{"fx_electrical_explosion", SP_fx_electrical_explosion},
	{"fx_smoke", SP_fx_smoke},
	{"fx_stasis_teleporter", SP_fx_teleporter },	// Really evil name hacking
	{"fx_energy_stream", SP_fx_stream },
	{"fx_psycho_jism", SP_fx_psycho_jism},
	{"fx_cooking_steam", SP_fx_cooking_steam},
	{"fx_laser", SP_fx_laser},
	{"fx_transporter_stream", SP_fx_transporter_stream},
	{"fx_explosion_trail", SP_fx_explosion_trail},
	{"fx_blow_chunks", SP_fx_blow_chunks},
	{"fx_borg_energy_beam", SP_fx_borg_energy_beam},
	{"fx_shimmery_thing", SP_fx_shimmery_thing},
	{"fx_borg_bolt", SP_fx_borg_bolt},
	{"fx_crew_beam_in", SP_fx_crew_beam_in},
	{"fx_forge_bolt", SP_fx_forge_bolt},
	{"fx_teleporter", SP_fx_stasis_teleporter},		// Really evil name hacking
	{"fx_stasis_mushrooms", SP_fx_stasis_mushrooms},
	{"fx_dn_beam_glow", SP_fx_dn_beam_glow},
	{"fx_garden_fountain", SP_fx_garden_fountain},

	{"object_cargo_barrel1", SP_object_cargo_barrel1},
	{"point_combat", SP_point_combat},

	{NULL, NULL}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn( gentity_t *ent ) {
	spawn_t	*s;
	gitem_t	*item;

	if ( !ent->classname ) {
		gi.Printf (S_COLOR_RED"G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check item spawn functions
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
		if ( !strcmp(item->classname, ent->classname) ) {
			// found it
			G_SpawnItem( ent, item );
			return qtrue;
		}
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !strcmp(s->name, ent->classname) ) {
			// found it
			s->spawn(ent);
			return qtrue;
		}
	}
	gi.Printf (S_COLOR_RED"%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;

	if(!string || !string[0])
	{
		//gi.Printf(S_COLOR_RED"Error: G_NewString called with NULL string!\n");
		return (char *)string;
	}

	l = strlen(string) + 1;

	newb = (char *) G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}
	
	return newb;
}




/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void Q3_SetParm (int entID, int parmNum, const char *parmValue);
void G_ParseField( const char *key, const char *value, gentity_t *ent ) {
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;
	vec4_t	vec4;

	for ( f=fields ; f->name ; f++ ) {
		if ( !Q_stricmp(f->name, key) ) {
			// found it
			b = (byte *)ent;

			switch( f->type ) {
			case F_LSTRING:
				*(char **)(b+f->ofs) = G_NewString (value);
				break;
			case F_VECTOR:
			{
				int _iFieldsRead = sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				assert(_iFieldsRead==3);
				if (_iFieldsRead!=3)
				{
					gi.Printf (S_COLOR_YELLOW"G_ParseField: VEC3 sscanf() failed to read 3 floats ('angle' key bug?)\n");
				}
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			}
			case F_VECTOR4:
			{
				int _iFieldsRead =  sscanf (value, "%f %f %f %f", &vec4[0], &vec4[1], &vec4[2], &vec4[3]);
				assert(_iFieldsRead==4);
				if (_iFieldsRead!=4)
				{
					gi.Printf (S_COLOR_YELLOW"G_ParseField: VEC4 sscanf() failed to read 4 floats\n");
				}
				((float *)(b+f->ofs))[0] = vec4[0];
				((float *)(b+f->ofs))[1] = vec4[1];
				((float *)(b+f->ofs))[2] = vec4[2];
				((float *)(b+f->ofs))[3] = vec4[3];
				break;
			}
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_PARM1:
			case F_PARM2:
			case F_PARM3:
			case F_PARM4:
			case F_PARM5:
			case F_PARM6:
			case F_PARM7:
			case F_PARM8:
			case F_PARM9:
			case F_PARM10:
			case F_PARM11:
			case F_PARM12:
			case F_PARM13:
			case F_PARM14:
			case F_PARM15:
			case F_PARM16:
				Q3_SetParm( ent->s.number, (f->type - F_PARM1), (char *) value );
				break;
			case F_FLAG:
				{//try to find the proper flag for that key:
					int flag = GetIDForString ( flagTable, key );

					if ( flag > 0 )
					{
						G_SpawnFlag( key, flag, (int *)(b+f->ofs) );
					}
					else
					{
						gi.Printf (S_COLOR_YELLOW"WARNING: G_ParseField: can't find flag for key %s\n", key);
					}
				}
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}

}

static qboolean SpawnForCurrentDifficultySetting( gentity_t *ent )
{
extern cvar_t	*com_buildScript;
	if (com_buildScript->integer) {	//always spawn when building a pak file
		return qtrue;
	}
	if ( ent->spawnflags & ( 1 << (8 + g_spskill->integer )) )	{// easy -256	medium -512		hard -1024
		return qfalse;
	} else {
		return qtrue;
	}
}

/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/

void G_SpawnGEntityFromSpawnVars( void ) {
	int			i;
	gentity_t	*ent;

	// get the next free entity
	ent = G_Spawn();

	for ( i = 0 ; i < numSpawnVars ; i++ ) {
		G_ParseField( spawnVars[i][0], spawnVars[i][1], ent );
	}

	G_SpawnInt( "notsingle", "0", &i );
	if ( i || !SpawnForCurrentDifficultySetting( ent ) ) {
		G_FreeEntity( ent );
		return;
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->currentOrigin );

	if ( VALIDSTRING( ent->infoString ) )
	{
		int	inf;

		// The infoString, if there is one, should just be the enum, so get the index
		inf = IS_GetInfostring( ent->infoString );

		if ( inf )
		{
			// Now we'll get the real infostring
			ent->infoString = bg_infoItemList[inf].infoString;
			VectorCopy( bg_infoItemList[inf].color, ent->startRGBA );
		}
		else
		{
			// Hmmm, bogus infostring
			ent->infoString = NULL;
		}
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
		return;
	}

	//Tag on the ICARUS scripting information only to valid recipients
	if ( ICARUS_ValidEnt( ent ) )
	{
		ICARUS_InitEnt( ent );

		if ( ent->classname && ent->classname[0] )
		{
			if ( strncmp( "NPC_", ent->classname, 4 ) != 0 )
			{//Not an NPC_spawner
				if ( ent->behaviorSet[BSET_SPAWN] )
				{
					G_ActivateBehavior( ent, BSET_SPAWN );
				}
			}
		}
	}

	G_InitBoltOnData( ent );
}



/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_VARS" );
	}

	dest = spawnVarChars + numSpawnVarChars;
	memcpy( dest, string, l+1 );

	numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( char **data ) {
	char		keyname[MAX_STRING_CHARS];
	char		*com_token;

	numSpawnVars = 0;
	numSpawnVarChars = 0;

	// parse the opening brace	
	com_token = COM_Parse( data );
	if ( !*data ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		G_Error( "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		com_token = COM_Parse( data );
		if ( com_token[0] == '}' ) {
			break;
		}
		if ( !data ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		Q_strncpyz( keyname, com_token, sizeof(keyname) );
		
		// parse value	
		com_token = COM_Parse( data );
		if ( com_token[0] == '}' ) {
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( !data ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}
		if ( numSpawnVars == MAX_SPAWN_VARS ) {
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		spawnVars[ numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		spawnVars[ numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		numSpawnVars++;
	}

	return qtrue;
}



/*QUAKED worldspawn (0 0 0) ?
Every map should have exactly one worldspawn.
"music"    path to WAV or MP3 files (e.g. "music\intro.mp3 music\loopfile.mp3")
"gravity"  800 is default gravity
"message"  Text to print during connection process
"soundSet" Ambient sound set to play when map is started
"spawnscript" runs this script
*/
void SP_worldspawn( void ) {
	char	*s;
	int		i;

	for ( i = 0 ; i < numSpawnVars ; i++ ) 
	{
		if ( Q_stricmp( "spawnscript", spawnVars[i][0] ) == 0 )
		{//ONly let them set spawnscript, we don't want them setting an angle or something on the world.
			G_ParseField( spawnVars[i][0], spawnVars[i][1], &g_entities[ENTITYNUM_WORLD] );
		}
	}

	G_SpawnString( "classname", "", &s );
	if ( Q_stricmp( s, "worldspawn" ) ) {
		G_Error( "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	// make some data visible to connecting client
	G_SpawnString( "music", "", &s );
	gi.SetConfigstring( CS_MUSIC, s );

	G_SpawnString( "infostring", "", &s );

	if (*s)
	{
		int	inf;
		// The infoString, if there is one, should just be the enum, so get the index
		inf = IS_GetInfostring( s );

		if ( inf )
		{
			// map specific message
			// Now we'll get the real infostring
			s = va("%i",inf);
			gi.SetConfigstring( CS_MESSAGE, s );				
		}
	}
	else
	{
		G_SpawnString( "message", "", &s );
		gi.SetConfigstring( CS_MESSAGE, s );				// map specific message
	}

	G_SpawnString( "gravity", "800", &s );
	extern SavedGameJustLoaded_e g_eSavedGameJustLoaded;
	if (g_eSavedGameJustLoaded != eFULL)
	{
		gi.cvar_set( "g_gravity", s );
	}

	G_SpawnString( "soundSet", "default", &s );
	gi.SetConfigstring( CS_AMBIENT_SET, s );

	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";
}

/*
-------------------------
G_ParsePrecaches
-------------------------
*/

void G_ParsePrecaches( void )
{
	gentity_t	*ent = NULL;

	//Clear any old lists
	as_preCacheMap.clear();

	for ( int i = 0; i < globals.num_entities; i++ )
	{
		ent = &g_entities[i];

		if VALIDSTRING( ent->soundSet )
		{
			as_preCacheMap[ (char *) ent->soundSet ] = 1;
		}
	}
}

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
extern int num_waypoints;
extern void	RG_RouteGen(void);
extern void NAV_GenerateSquadPaths (void);
extern qboolean NPCsPrecached;
void G_SpawnEntitiesFromString( const char *entityString ) {
	char		*entities;

	entities = (char *)entityString;	// I had problems getting the compiler to live with const

	// allow calls to G_Spawn*()
	spawning = qtrue;
	NPCsPrecached = qfalse;
	numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars( &entities ) ) {
		G_Error( "SpawnEntities: no entities" );
	}
	
	SP_worldspawn();

	// parse ents
	while( G_ParseSpawnVars( &entities ) ) 
	{
		G_SpawnGEntityFromSpawnVars();
	}	

	//Search the entities for precache information
	G_ParsePrecaches();


	if( g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN] && g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN][0] )
	{//World has a spawn script, but we don't want the world in ICARUS and running scripts,
		//so make a scriptrunner and start it going.
		gentity_t *script_runner = G_Spawn();
		if ( script_runner )
		{
			script_runner->behaviorSet[BSET_USE] = g_entities[ENTITYNUM_WORLD].behaviorSet[BSET_SPAWN];
			script_runner->count = 1;
			script_runner->e_ThinkFunc = thinkF_scriptrunner_run;
			script_runner->nextthink = level.time + 100;

			if ( ICARUS_ValidEnt( script_runner ) )
			{
				ICARUS_InitEnt( script_runner );
			}
		}
	}

	//gi.Printf(S_COLOR_YELLOW"Total waypoints: %d\n", num_waypoints);
	//Automatically run routegen
	//RG_RouteGen();
	NAV_GenerateSquadPaths();

	spawning = qfalse;			// any future calls to G_Spawn*() will be errors
}

