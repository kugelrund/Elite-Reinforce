
#include "cg_local.h"
#include "fx_public.h"
#include "..\client\vmachine.h"
#include "cg_text.h"
#include "..\game\characters.h"
#include "..\game\speakers.h"
#include "..\speedrun\PlayerOverbouncePrediction.hpp"

//NOTENOTE: Be sure to change the mirrored code in g_shared.h
typedef	map< string, unsigned char, less<string>, allocator< unsigned char >  >	namePrecache_m;
extern namePrecache_m	as_preCacheMap;
extern void CG_RegisterNPCCustomSounds( clientInfo_t *ci );
extern qboolean G_AddSexToMunroString ( char *string, qboolean qDoBoth );
extern void CG_RegisterNPCEffects( team_t team );
extern qboolean G_ParseAnimFileSet( const char *filename, int *animFileIndex, qboolean cacheSounds );

void CG_Init( int serverCommandSequence );
qboolean CG_ConsoleCommand( void );
void CG_Shutdown( void );
int CG_GetCameraPos( vec3_t camerapos );
void CG_LoadIngameText(void);

#define NUM_CHUNKS		6
#define BORG_CHUNKS		3	
#define STASIS_CHUNKS	4


/*
================
vmMain

This is the only way control passes into the cgame module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7 ) {
	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, (stereoFrame_t) arg1 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_CAMERA_POS:
		return CG_GetCameraPos( (float*)arg0);
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_paused;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t 	cg_developer;
vmCvar_t 	cg_timescale;
vmCvar_t	cg_skippingcin;
vmCvar_t	cg_language;

vmCvar_t	cg_pano;
vmCvar_t	cg_panoNumShots;
vmCvar_t	cg_freezeFX;
vmCvar_t	cg_debugFX;
vmCvar_t	fx_memoryInfo;
vmCvar_t	cg_virtualVoyager;
vmCvar_t	cg_missionInfoFlashTime;

// Additions for Elite Reinforce
vmCvar_t	cg_drawOverbounceInfo;
vmCvar_t	cg_drawStrafeHelper;
vmCvar_t	cg_strafeHelperCenter;
vmCvar_t	cg_strafeHelperCenterMarker;
vmCvar_t	cg_strafeHelperHeight;
vmCvar_t	cg_strafeHelperScale;
vmCvar_t	cg_strafeHelperY;


typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

cvarTable_t		cvarTable[] = {
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "80", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },

	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "1", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },

	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },

	{ &cg_gun_frame, "gun_frame", "0", CVAR_CHEAT },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },

	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },

	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },

	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", 0 },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", 0 },
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },

	{ &cg_pano, "pano", "0", 0 },
	{ &cg_panoNumShots, "panoNumShots", "10", 0 },

	{ &cg_freezeFX, "fx_freeze", "0", 0 },
	{ &cg_debugFX, "fx_debug", "0", 0 },
	{ &fx_memoryInfo, "fx_memoryInfo", "0", 0 },
	// the following variables are created in other parts of the system,
	// but we also reference them here

	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_developer, "developer", "", 0 }, 
	{ &cg_timescale, "timescale", "1", 0 }, 	
	{ &cg_skippingcin, "skippingCinematic", "0", CVAR_ROM},
	{ &cg_language,	"g_language", "", CVAR_ARCHIVE | CVAR_NORESTART},
	{ &cg_virtualVoyager, "cg_virtualvoyager", "0", CVAR_NORESTART },
	{ &cg_missionInfoFlashTime, "cg_missionInfoFlashTime", "15000", CVAR_ARCHIVE  },
	
	// Additions for Elite Reinforce
	{ &cg_drawOverbounceInfo, "cg_drawOverbounceInfo", "0", CVAR_ARCHIVE },
	{ &cg_drawStrafeHelper, "cg_drawStrafeHelper", "0", CVAR_ARCHIVE },
	{ &cg_strafeHelperCenter, "cg_strafeHelperCenter", "1", CVAR_ARCHIVE },
	{ &cg_strafeHelperCenterMarker, "cg_strafeHelperCenterMarker", "1", CVAR_ARCHIVE },
	{ &cg_strafeHelperHeight, "cg_strafeHelperHeight", "20", CVAR_ARCHIVE },
	{ &cg_strafeHelperScale, "cg_strafeHelperScale", "4", CVAR_ARCHIVE },
	{ &cg_strafeHelperY, "cg_strafeHelperY", "50", CVAR_ARCHIVE },
};

int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

characterName_t CharacterNames[CHARACTER_NUM_CHARS] =
{
	//NPC_type		sound									soundIndex
	//HazTeam Alpha
	"foster",		"sound/voice/munro/misc/sir.wav",		0, //CHARACTER_FOSTER,
	"telsia",		"sound/voice/munro/misc/telsia.wav",	0, //CHARACTER_TELSIA,
	"biessman",		"sound/voice/munro/misc/rick.wav",		0, //CHARACTER_BIESSMAN,
	"chang",		"sound/voice/munro/misc/austin.wav",	0, //CHARACTER_CHANG,
	"chell",		"sound/voice/munro/misc/chell.wav",		0, //CHARACTER_CHELL,
	"jurot",		"sound/voice/munro/misc/jurot.wav",		0, //CHARACTER_JUROT,
	//HazTeam Beta
	"laird",		"sound/voice/munro/misc/liz.wav",		0, //CHARACTER_LAIRD,
	"kenn",			"sound/voice/munro/misc/kenn.wav",		0, //CHARACTER_KENN,
	"oviedo",		"sound/voice/munro/misc/oviedo.wav",	0, //CHARACTER_OVIEDO,
	"odell",		"sound/voice/munro/misc/tom.wav",		0, //CHARACTER_ODELL,
	"nelson",		"sound/voice/munro/misc/jeff.wav",		0, //CHARACTER_NELSON,
	"jaworski",		"sound/voice/munro/misc/michael.wav",	0, //CHARACTER_JAWORSKI,
	"csatlos",		"sound/voice/munro/misc/mitch.wav",		0, //CHARACTER_CSATLOS,
	//Senior Crew
	"janeway",		"sound/voice/munro/misc/captain.wav",	0, //CHARACTER_JANEWAY,
	"chakotay",		"sound/voice/munro/misc/commander.wav",	0, //CHARACTER_CHAKOTAY,
	"tuvok",		"sound/voice/munro/misc/tuvok.wav",		0, //CHARACTER_TUVOK,
	"tuvokhaz",		"sound/voice/munro/misc/tuvok.wav",		0, //CHARACTER_TUVOKHAZ,
	"torres",		"sound/voice/munro/misc/torres.wav",	0, //CHARACTER_TORRES,
	"paris",		"sound/voice/munro/misc/tom.wav",		0, //CHARACTER_PARIS,
	"kim",			"sound/voice/munro/misc/harry.wav",		0, //CHARACTER_KIM,
	"doctor",		"sound/voice/munro/misc/doctor.wav",	0, //CHARACTER_DOCTOR,
	"seven",		"sound/voice/munro/misc/seven.wav",		0, //CHARACTER_SEVEN,
	"sevenhazard",	"sound/voice/munro/misc/seven.wav",		0, //CHARACTER_SEVENHAZ,
	"neelix",		"sound/voice/munro/misc/neelix.wav",	0, //CHARACTER_NEELIX,
	//Other Crew
	"pelletier",	"sound/voice/munro/misc/brian.wav",		0, //CHARACTER_PELLETIER,
	//Generic Crew
	"crewman",		"sound/voice/munro/misc/crewman.wav",	0, //CHARACTER_CREWMAN,
	//"ensign",		"sound/voice/munro/misc/ensign.wav",	0, //CHARACTER_ENSIGN,
	"lt",			"sound/voice/munro/misc/lt.wav",		0, //CHARACTER_LT
	"commander",	"sound/voice/munro/misc/commander.wav",	0, //CHARACTER_COMM
	"captain",		"sound/voice/munro/misc/captain.wav",	0, //CHARACTER_CAPT
	"generic1",		"sound/voice/munro/misc/hey1.wav",		0, //CHARACTER_GENERIC1
	"generic2",		"sound/voice/munro/misc/hey2.wav",		0, //CHARACTER_GENERIC2
	"generic3",		"sound/voice/munro/misc/excuseme1.wav",	0, //CHARACTER_GENERIC3
	"generic4",		"sound/voice/munro/misc/excuseme2.wav",	0 //CHARACTER_GENERIC4
};
/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgi_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}
	if ( cg_thirdPerson.integer == 2 )//to appease the testers who deliberately avoid the reset at the end of borgslayer
	{//if in borgslayer mode, restore to normal
		cgi_Cvar_Set("cg_thirdPerson","0");
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		cgi_Cvar_Update( cv->vmCvar );
	}
}


int CG_CrosshairPlayer( void ) 
{
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) 
	{
		return -1;
	}
	return cg.crosshairClientNum;
}


int CG_GetCameraPos( vec3_t camerapos ) {
	if ( in_camera) {
		VectorCopy(client_camera.origin, camerapos);
		return 1;
	}
	return 0;
}

void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (targetNum <= 0) {
		return;
	}

	cgi_Argv( 1, test, 4 );	//FIXME: this is now an exec_now command - in case we start using it... JFM
	cgi_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



void CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	cgi_Printf( text );
}

void CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	cgi_Error( text );
}


/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	cgi_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if (item->pickup_sound)
	{
		cgi_S_RegisterSound( item->pickup_sound );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			cgi_S_RegisterSound( data );
		}
	}
}


/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );
	cgi_UpdateScreen();
}

/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;

	CG_LoadingString( "ambient sound sets" );

	//Load the ambient sets

	//FIXME: Don't ask... I had to get around a really nasty MS error in the templates with this...
	namePrecache_m::iterator	pi;
	STL_ITERATE( pi, as_preCacheMap )
	{
		cgi_AS_AddPrecacheEntry( ((*pi).first).c_str() );
	}

	cgi_AS_ParseSets();

	CG_LoadingString( "general sounds" );

	cgs.media.selectSound = cgi_S_RegisterSound( "sound/weapons/change.wav" );
	cgs.media.useNothingSound = cgi_S_RegisterSound( "sound/items/use_nothing.wav" );

	cgs.media.noAmmoSound = cgi_S_RegisterSound( "sound/weapons/noammo.wav" );
	cgs.media.talkSound = 	cgi_S_RegisterSound( "sound/interface/communicator.wav" );
	cgs.media.landSound =	cgi_S_RegisterSound( "sound/player/land1.wav");
	cgs.media.tedTextSound = cgi_S_RegisterSound( "sound/interface/tedtext.wav" );

	cgs.media.interfaceSnd1 = cgi_S_RegisterSound( "sound/interface/button4.wav" );
	cgs.media.interfaceSnd2 = cgi_S_RegisterSound( "sound/interface/button2.wav" );
	cgs.media.interfaceSnd3 = cgi_S_RegisterSound( "sound/interface/button1.wav" );

	cgs.media.watrInSound = cgi_S_RegisterSound ("sound/player/watr_in.wav");
	cgs.media.watrOutSound = cgi_S_RegisterSound ("sound/player/watr_out.wav");
	cgs.media.watrUnSound = cgi_S_RegisterSound ("sound/player/watr_un.wav");

	cgs.media.respawnSound = cgi_S_RegisterSound( "sound/items/respawn1.wav" );
	
	//cgs.media.teleInSound = cgi_S_RegisterSound("sound/world/telein.wav");
	//cgs.media.teleOutSound = cgi_S_RegisterSound("sound/world/teleout.wav");
	cgs.media.transporterSound = cgi_S_RegisterSound( "sound/world/transporter.wav");

	// Zoom
	cgs.media.zoomStart = cgi_S_RegisterSound( "sound/interface/zoomstart.wav" );
	cgs.media.zoomLoop = cgi_S_RegisterSound( "sound/interface/zoomloop.wav" );
	cgs.media.zoomEnd = cgi_S_RegisterSound( "sound/interface/zoomend.wav" );

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = cgi_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = cgi_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = cgi_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/enemies/borg/borgstep%ia.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BORG][i] = cgi_S_RegisterSound (name);
	}

	cg.loadLCARSStage = 1;
	CG_LoadingString( "item sounds" );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' ) {
			CG_RegisterItemSounds( i );
		}
	}

	cg.loadLCARSStage = 2;
	CG_LoadingString( "preregistered sounds" );

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		if (i&31) {
			CG_LoadingString( soundName );
		}
		cgs.sound_precache[i] = cgi_S_RegisterSound( soundName );
	}
}

/*
=============================================================================

CLIENT INFO

=============================================================================
*/

qhandle_t CG_RegisterHeadSkin( const char *headModelName, const char *headSkinName, qboolean *extensions )
{
	char		hfilename[MAX_QPATH];
	qhandle_t	headSkin;

	Com_sprintf( hfilename, sizeof( hfilename ), "models/players/%s/head_%s.skin", headModelName, headSkinName );
	headSkin = cgi_R_RegisterSkin( hfilename );
	if ( headSkin < 0 ) 
	{	//have extensions
		*extensions = qtrue;
		headSkin = -headSkin;
	} 
	else 
	{
		*extensions = qfalse;	//just to be sure.
	}

	if ( !headSkin )
	{
		Com_Printf( "Failed to load skin file: %s : %s\n", headModelName, headSkinName );
	}
	return headSkin;
}

/*
==========================
CG_RegisterClientSkin
==========================
*/
qboolean	CG_RegisterClientSkin( clientInfo_t *ci,
								  const char *headModelName, const char *headSkinName,
								  const char *torsoModelName, const char *torsoSkinName,
								  const char *legsModelName, const char *legsSkinName) 
{
	char		hfilename[MAX_QPATH];
	char		tfilename[MAX_QPATH];
	char		lfilename[MAX_QPATH];

	Com_sprintf( lfilename, sizeof( lfilename ), "models/players/%s/lower_%s.skin", legsModelName, legsSkinName );
	ci->legsSkin = cgi_R_RegisterSkin( lfilename );

	if ( !ci->legsSkin )
	{
		Com_Printf( "Failed to load skin file: %s : %s\n", legsModelName, legsSkinName );
		//return qfalse;
	}

	if(torsoModelName && torsoSkinName && torsoModelName[0] && torsoSkinName[0])
	{
		Com_sprintf( tfilename, sizeof( tfilename ), "models/players/%s/upper_%s.skin", torsoModelName, torsoSkinName );
		ci->torsoSkin = cgi_R_RegisterSkin( tfilename );

		if ( !ci->torsoSkin )
		{
			Com_Printf( "Failed to load skin file: %s : %s\n", torsoModelName, torsoSkinName );
			return qfalse;
		}
	}

	if(headModelName && headSkinName && headModelName[0] && headSkinName[0])
	{
		Com_sprintf( hfilename, sizeof( hfilename ), "models/players/%s/head_%s.skin", headModelName, headSkinName );
		ci->headSkin = cgi_R_RegisterSkin( hfilename );
		if (ci->headSkin <0) {	//have extensions
			ci->extensions = qtrue;
			ci->headSkin = -ci->headSkin;
		} else {
			ci->extensions = qfalse;	//just to be sure.
		}

		if ( !ci->headSkin )
		{
			Com_Printf( "Failed to load skin file: %s : %s\n", headModelName, headSkinName );
			return qfalse;
		}
	}

	return qtrue;
}

/*
==========================
CG_RegisterClientModelname
==========================
*/
qboolean CG_RegisterClientModelname( clientInfo_t *ci,
									const char *headModelName, const char *headSkinName,
									const char *torsoModelName, const char *torsoSkinName,
									const char *legsModelName, const char *legsSkinName ) 
{
	char		filename[MAX_QPATH];

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.mdr", legsModelName );
	ci->legsModel = cgi_R_RegisterModel( filename );
	if ( !ci->legsModel ) 
	{//he's not skeletal, try the old way
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", legsModelName );
		ci->legsModel = cgi_R_RegisterModel( filename );
		if ( !ci->legsModel )
		{
			Com_Printf( S_COLOR_RED"Failed to load model file %s\n", filename );
			return qfalse;
		}
	}

	if(torsoModelName && torsoModelName[0])
	{//You are trying to set one
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.mdr", torsoModelName );
		ci->torsoModel = cgi_R_RegisterModel( filename );
		if ( !ci->torsoModel ) 
		{//he's not skeletal, try the old way
			Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", torsoModelName );
			ci->torsoModel = cgi_R_RegisterModel( filename );
			if ( !ci->torsoModel ) 
			{
				Com_Printf( S_COLOR_RED"Failed to load model file %s\n", filename );
				return qfalse;
			}
		}
	}
	else
	{
		ci->torsoModel = 0;
	}

	if(headModelName && headModelName[0])
	{//You are trying to set one
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", headModelName );
		ci->headModel = cgi_R_RegisterModel( filename );
		if ( !ci->headModel ) 
		{
			Com_Printf( S_COLOR_RED"Failed to load model file %s\n", filename );
			return qfalse;
		}
	}
	else
	{
		ci->headModel = 0;
	}


	// if any skins failed to load, return failure
	if ( !CG_RegisterClientSkin( ci, headModelName, headSkinName, torsoModelName, torsoSkinName, legsModelName, legsSkinName ) ) 
	{
		//Com_Printf( "Failed to load skin file: %s : %s/%s : %s/%s : %s\n", headModelName, headSkinName, torsoModelName, torsoSkinName, legsModelName, legsSkinName );
		return qfalse;
	}

	//FIXME: for now, uses the legs model dir for anim cfg, but should we set this in some sort of NPCs.cfg?
	// load the animation file set
	if ( !G_ParseAnimFileSet( legsModelName, &ci->animFileIndex, qtrue ) ) 
	{
		Com_Printf( S_COLOR_RED"Failed to load animation file set models/players/%s\n", legsModelName );
		return qfalse;
	}

	return qtrue;
}


void CG_RegisterClientRenderInfo(clientInfo_t *ci, renderInfo_t *ri)
{
	char			*slash;
	char			headModelName[MAX_QPATH];
	char			torsoModelName[MAX_QPATH];
	char			legsModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
	char			torsoSkinName[MAX_QPATH];
	char			legsSkinName[MAX_QPATH];

	if(!ri->legsModelName || !ri->legsModelName[0])
	{//Must have at LEAST a legs model
		return;
	}

	Q_strncpyz( legsModelName, ri->legsModelName, sizeof( legsModelName ) );
	//Legs skin
	slash = strchr( legsModelName, '/' );
	if ( !slash ) 
	{
		// modelName didn not include a skin name
		Q_strncpyz( legsSkinName, "default", sizeof( legsSkinName ) );
	} 
	else 
	{
		Q_strncpyz( legsSkinName, slash + 1, sizeof( legsSkinName ) );
		// truncate modelName
		*slash = 0;
	}

	if(ri->torsoModelName && ri->torsoModelName[0])
	{
		Q_strncpyz( torsoModelName, ri->torsoModelName, sizeof( torsoModelName ) );
		//Torso skin
		slash = strchr( torsoModelName, '/' );
		if ( !slash ) 
		{
			// modelName didn't include a skin name
			Q_strncpyz( torsoSkinName, "default", sizeof( torsoSkinName ) );
		} 
		else 
		{
			Q_strncpyz( torsoSkinName, slash + 1, sizeof( torsoSkinName ) );
			// truncate modelName
			*slash = 0;
		}
	}
	else
	{
		torsoModelName[0] = 0;
	}

	//Head
	if(ri->headModelName && ri->headModelName[0])
	{
		Q_strncpyz( headModelName, ri->headModelName, sizeof( headModelName ) );
		//Head skin
		slash = strchr( headModelName, '/' );
		if ( !slash ) 
		{
			// modelName didn not include a skin name
			Q_strncpyz( headSkinName, "default", sizeof( headSkinName ) );
		} 
		else 
		{
			Q_strncpyz( headSkinName, slash + 1, sizeof( headSkinName ) );
			// truncate modelName
			*slash = 0;
		}
	}
	else
	{
		headModelName[0] = 0;
	}

	if ( !CG_RegisterClientModelname( ci, headModelName, headSkinName, torsoModelName, torsoSkinName, legsModelName, legsSkinName) ) 
	{
		if ( !CG_RegisterClientModelname( ci, DEFAULT_HEADMODEL, "default", DEFAULT_TORSOMODEL, "default", DEFAULT_LEGSMODEL, "default" ) ) 
		{
			CG_Error( "DEFAULT_MODELS failed to register");
		}
	}
}
/*
void CG_RegisterClientModels (int entityNum)

Only call if clientInfo->infoValid is not true

For players and NPCs to register their models
*/
void CG_RegisterClientModels (int entityNum)
{
	gentity_t		*ent;

	if(entityNum < 0 || entityNum > ENTITYNUM_WORLD)
	{
		return;
	}

	ent = &g_entities[entityNum];

	if(!ent->client)
	{
		return;
	}

	CG_RegisterClientRenderInfo(&ent->client->clientInfo, &ent->client->renderInfo);

	ent->client->clientInfo.infoValid = qtrue;

	if(entityNum < MAX_CLIENTS)
	{
		memcpy(&cgs.clientinfo[entityNum], &ent->client->clientInfo, sizeof(clientInfo_t));
	}
}

//===================================================================================
extern	cvar_t	*com_buildScript;
sfxHandle_t CG_RegisterSexedSound(char* string)
{
	if (com_buildScript->integer) {	
		cgi_S_RegisterSound(string);	//grab the original file before changing for sex
	}

	G_AddSexToMunroString( string, qfalse );
	return cgi_S_RegisterSound( string );
}

//===================================================================================

void CG_PrecachePlayerGreetingSound( char *NPC_type )
{
	for ( int character = 0; character < CHARACTER_NUM_CHARS; character++ )
	{
		if ( Q_stricmp( NPC_type, CharacterNames[character].name ) == 0 )
		{
			//if ( !CharacterNames[character].soundIndex )
			//remmed out because soundIndexes weren't being updated on vid_restarts
			//FIXME?  Should we zero this array on vid_restarts?  Called also from NPC spawns
			char	temp[MAX_QPATH];
			
			Q_strncpyz(temp, CharacterNames[character].sound, sizeof(temp), qtrue );
			CharacterNames[character].soundIndex = CG_RegisterSexedSound( temp );
		}
	}
}

extern void NPC_Precache ( gentity_t *spawner );
qboolean NPCsPrecached = qfalse;
/*
=================
CG_PrepRefresh

Call before entering a new level, or after changing renderers
This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero",
		"gfx/2d/numbers/one",
		"gfx/2d/numbers/two",
		"gfx/2d/numbers/three",
		"gfx/2d/numbers/four",
		"gfx/2d/numbers/five",
		"gfx/2d/numbers/six",
		"gfx/2d/numbers/seven",
		"gfx/2d/numbers/eight",
		"gfx/2d/numbers/nine",
		"gfx/2d/numbers/minus",
	};

	static char		*sb_t_nums[11] = {
		"gfx/2d/numbers/t_zero",
		"gfx/2d/numbers/t_one",
		"gfx/2d/numbers/t_two",
		"gfx/2d/numbers/t_three",
		"gfx/2d/numbers/t_four",
		"gfx/2d/numbers/t_five",
		"gfx/2d/numbers/t_six",
		"gfx/2d/numbers/t_seven",
		"gfx/2d/numbers/t_eight",
		"gfx/2d/numbers/t_nine",
		"gfx/2d/numbers/t_minus",
	};

	FX_Free();

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	cgi_R_ClearScene();

	cg.loadLCARSStage = 3;
	CG_LoadingString( cgs.mapname );

	cgi_R_LoadWorldMap( cgs.mapname );

	cg.loadLCARSStage = 4;
	CG_LoadingString( "game media shaders" );

	for ( i=0; i < 11; i++ )
	{
		cgs.media.numberShaders[i] = cgi_R_RegisterShaderNoMip( sb_nums[i] );
		cgs.media.smallnumberShaders[i] = cgi_R_RegisterShaderNoMip( sb_t_nums[i] );
	}

	cgs.media.sparkShader				= cgi_R_RegisterShader( "gfx/misc/spark" );
	cgs.media.spark2Shader				= cgi_R_RegisterShader( "gfx/misc/spark2" );
	cgs.media.spark3Shader				= cgi_R_RegisterShader( "gfx/misc/spark_or" );	//used in PsychoJismThinkGo from cg_env
	cgs.media.steamShader				= cgi_R_RegisterShader( "gfx/misc/steam" );
	cgs.media.smokeShader				= cgi_R_RegisterShader( "gfx/misc/smoke" );
	cgs.media.spooShader				= cgi_R_RegisterShader( "gfx/misc/spoo" );

	cgs.media.explosionModel				= cgi_R_RegisterModel ( "models/weaphits/explosion.md3" );
	cgs.media.electricalExplosionSlowShader	= cgi_R_RegisterShader( "electricalExplosionSlow" );//elec, imod, and prifle
	cgs.media.surfaceExplosionShader		= cgi_R_RegisterShader( "surfaceExplosion" );

	cgs.media.boltShader			= cgi_R_RegisterShader( "gfx/effects/electric" );	//FX_StasisAttackThink
	cgs.media.pjBoltShader			= cgi_R_RegisterShader( "gfx/effects/electric_or" );

	//Used by fx_bolts and borg taser, so leave it here for now until can figure out
	//way to have fx_bolts precache their shaders
	for ( i = 0; i < 4; i++ ) {
		cgs.media.borgLightningShaders[i] = cgi_R_RegisterShader( va( "gfx/misc/blightning%i", i+1 ) );
	}
	
	cgs.media.solidWhiteShader			= cgi_R_RegisterShader( "white2" );
	
	cgs.media.whiteRingShader			= cgi_R_RegisterShader( "gfx/misc/whitering" );
	cgs.media.plasmaShader				= cgi_R_RegisterShader( "gfx/misc/plasma" );
	cgs.media.bolt2Shader				= cgi_R_RegisterShader( "gfx/effects/electrica" );
	cgs.media.yellowBoltShader			= cgi_R_RegisterShader( "gfx/effects/yellow_electric" );
	cgs.media.waterDropShader			= cgi_R_RegisterShader( "gfx/misc/drop" );
	cgs.media.ltblueParticleShader		= cgi_R_RegisterShader( "gfx/misc/ltblueparticle" );
	cgs.media.blueParticleShader		= cgi_R_RegisterShader( "gfx/misc/blueparticle" );
	cgs.media.purpleParticleShader		= cgi_R_RegisterShader( "gfx/misc/purpleparticle" );
	cgs.media.yellowParticleShader		= cgi_R_RegisterShader( "gfx/misc/yellowparticle" );
	cgs.media.orangeParticleShader		= cgi_R_RegisterShader( "gfx/misc/orangeparticle" );
	cgs.media.dkorangeParticleShader	= cgi_R_RegisterShader( "gfx/misc/dkorangeparticle" );
	cgs.media.whiteLaserShader			= cgi_R_RegisterShader( "gfx/effects/whitelaser" );
	cgs.media.oilDropShader				= cgi_R_RegisterShader( "gfx/misc/oildrop" );
	
	cgs.media.sunnyFlareShader			= cgi_R_RegisterShader( "gfx/misc/sunny_flare" );
	cgs.media.bigBoomShader				= cgi_R_RegisterShader( "gfx/misc/bigboom" );
	cgs.media.blueHitShader				= cgi_R_RegisterShader( "gfx/misc/blue_hit" );
	cgs.media.ghostRingShader			= cgi_R_RegisterShader( "gfx/misc/ghost_ring" );
	cgs.media.splosionShader			= cgi_R_RegisterShader( "gfx/misc/splosion" );
	cgs.media.yellowTrailShader			= cgi_R_RegisterShader( "gfx/misc/yellowtrail" );
	cgs.media.stasisBoltShader			= cgi_R_RegisterShader( "gfx/effects/electric_stasis" );
	cgs.media.fountainShader			= cgi_R_RegisterShader( "garden/fountain" );
	cgs.media.rippleShader				= cgi_R_RegisterShader( "garden/ripple" );

	//on players
	cgs.media.psychicShader				= cgi_R_RegisterShader( "gfx/misc/psychic" );
	cgs.media.transportShader			= cgi_R_RegisterShader( "powerups/beamEffect" );//always precache this since the player uses it
	cgs.media.disruptorShader			= cgi_R_RegisterShader( "powerups/disruptor");
	cgs.media.quantumDisruptorShader	= cgi_R_RegisterShader( "powerups/quantum_disruptor");
	cgs.media.phaserDisruptorShader		= cgi_R_RegisterShader( "powerups/phaser_disruptor");
	cgs.media.shieldShader				= cgi_R_RegisterShader( "gfx/effects/hirogenshield" );
	cgs.media.playerTeleportShader		= cgi_R_RegisterShader( "playerTeleport" );
	cgs.media.borgFullBodyShieldShader	= cgi_R_RegisterShader( "gfx/effects/borgfullbodyshield" );
	cgs.media.fixitEffectShader			= cgi_R_RegisterShader( "fixitEffect" );
	
	cgs.media.trans1Shader			= cgi_R_RegisterShader( "gfx/misc/trans1" );
	cgs.media.trans2Shader			= cgi_R_RegisterShader( "gfx/misc/trans2" );

	//interface
	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = cgi_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a'+i) );
	}
	cgs.media.backTileShader		= cgi_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader			= cgi_R_RegisterShader( "gfx/interface/noammo");

	// Zoom interface
	cgs.media.zoomMaskShader		= cgi_R_RegisterShader( "gfx/misc/zoom_mask2" );
	cgs.media.zoomBarShader			= cgi_R_RegisterShader( "gfx/2d/zoom_ctrl" );
	cgs.media.zoomBar2Shader		= cgi_R_RegisterShader( "gfx/2d/zoom_ctrl2" );
	cgs.media.zoomArrowShader		= cgi_R_RegisterShaderNoMip( "gfx/2d/arrow" );
	cgs.media.zoomInsertShader		= cgi_R_RegisterShaderNoMip( "gfx/misc/zoom_insert" );
	cgs.media.zoomInsert2Shader		= cgi_R_RegisterShaderNoMip( "gfx/misc/zoom_insert2" );

	cg.loadLCARSStage = 5;
	CG_LoadingString( "game media models" );
	
	// Chunk models
	//FIXME: jfm:? bother to conditionally load these if an ent has this material type?
	for (i = 0; i < NUM_CHUNKS; i++)
		cgs.media.chunkModels[0][i] = cgi_R_RegisterModel( va( "models/chunks/generic/chunks_%i.md3", i+1 ) );

	cgs.media.chunkSound			= cgi_S_RegisterSound("sound/weapons/explosions/glasslcar.wav");
	cgs.media.surfaceExpSound[0]	= cgi_S_RegisterSound("sound/weapons/explosions/explode8.wav");
	cgs.media.surfaceExpSound[1]	= cgi_S_RegisterSound("sound/weapons/explosions/explode9.wav");
	cgs.media.surfaceExpSound[2]	= cgi_S_RegisterSound("sound/weapons/explosions/explode14.wav");
	cgs.media.electricExpSound[0]	= cgi_S_RegisterSound("sound/weapons/explosions/explode10.wav");
	cgs.media.electricExpSound[1]	= cgi_S_RegisterSound("sound/weapons/explosions/explode11.wav");
	cgs.media.electricExpSound[2]	= cgi_S_RegisterSound("sound/weapons/explosions/explode15.wav");
	cgs.media.bigSurfExpSound		= cgi_S_RegisterSound("sound/weapons/explosions/explode12.wav");

	for (i = 0; i < NUM_CHUNKS; i++)
		cgs.media.glassChunkModels[0][i] = cgi_R_RegisterModel( va( "models/chunks/glass/glchunks_%i.md3", i+1 ) );
	cgs.media.glassChunkSound = cgi_S_RegisterSound("sound/weapons/explosions/glassbreak1.wav");

	for (i = 0; i < BORG_CHUNKS; i++)
		cgs.media.borgChunkModels[0][i] = cgi_R_RegisterModel( va( "models/chunks/borg/borg_%i.md3", i+1 ) );
	cgs.media.borgChunkSound = cgi_S_RegisterSound("sound/weapons/explosions/metalexplode.wav");

	for (i = 0; i < STASIS_CHUNKS; i++)
		cgs.media.stasisChunkModels[0][i] = cgi_R_RegisterModel( va( "models/chunks/stasis/stasis_%i.md3", i+1 ) );
	cgs.media.stasisChunkSound = cgi_S_RegisterSound("sound/weapons/explosions/mine1.wav");

	//Models & Shaders
	cgs.media.laserShader		= cgi_R_RegisterShader ( "textures/borg/rbeam" );
	cgs.media.borgEyeFlareShader	= cgi_R_RegisterShader( "gfx/misc/borgeyeflare" );
	cgs.media.assimTubesShader	= cgi_R_RegisterShader("models/players/borgbolts/b_asstubes");

	//Vohrsoth shaders
	cgs.media.vohrConeShader	= cgi_R_RegisterShader( /*"gfx/effects/vor_cone"*/"textures/borg/rbeam" );

	cg.loadLCARSStage = 6;

	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' ) 
		{
			if (bg_itemlist[i].pickup_name)
			{
				CG_LoadingString( bg_itemlist[i].pickup_name );
				CG_RegisterItemVisuals( i );
			}
		}
	}

	// wall marks
	cgs.media.compressionMarkShader			= cgi_R_RegisterShader( "gfx/damage/burnmark1" );
	cgs.media.IMODMarkShader				= cgi_R_RegisterShader( "gfx/damage/burnmark2" );
	cgs.media.phaserMarkShader				= cgi_R_RegisterShader( "gfx/damage/burnmark3" );
	cgs.media.scavMarkShader				= cgi_R_RegisterShader( "gfx/damage/burnmark4" );
	cgs.media.bulletmarksShader				= cgi_R_RegisterShader( "textures/decals/bulletmark4" );
	cgs.media.rivetMarkShader				= cgi_R_RegisterShader( "gfx/damage/rivetmark" );

	cgs.media.shadowMarkShader	= cgi_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader	= cgi_R_RegisterShader( "wake" );

	CG_LoadingString("map brushes");
	// register the inline models
	cgs.numInlineModels = cgi_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = cgi_R_RegisterModel( name );
		cgi_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	cg.loadLCARSStage = 7;
	CG_LoadingString("map models");
	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.model_draw[i] = cgi_R_RegisterModel( modelName );
	}

	for (i=0 ; i<MAX_CLIENTS ; i++) 
	{
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) 
		{
			continue;
		}

		//feedback( va("client %i", i ) );
		CG_NewClientinfo( i );
	}

	for (i=0 ; i < ENTITYNUM_WORLD ; i++)
	{
		if(&g_entities[i])
		{
			if(g_entities[i].client)
			{
				//if(!g_entities[i].client->clientInfo.infoValid)
				//We presume this
				{
					CG_LoadingString( va("client %s", g_entities[i].client->clientInfo.name ) );
					CG_RegisterClientModels(i);
					if ( i != 0 )
					{//Client weapons already precached
						CG_RegisterWeapon( g_entities[i].client->ps.weapon );
						CG_RegisterNPCCustomSounds( &g_entities[i].client->clientInfo );
						CG_RegisterNPCEffects( g_entities[i].client->playerTeam );
						CG_PrecachePlayerGreetingSound( g_entities[i].NPC_type );
						if ( g_entities[i].NPC_type && Q_stricmp( "satan", g_entities[i].NPC_type ) == 0 )
						{//very special case
							for ( int x = 0; x < 4; x++ ) {
								cgs.media.BWLightningShaders[x] = cgi_R_RegisterShader( va( "gfx/misc/bwlightning%i", x+1 ) );
							}
						}
					}
				}
			}
			else if ( g_entities[i].svFlags & SVF_NPC_PRECACHE && g_entities[i].NPC_type && g_entities[i].NPC_type[0] )
			{//Precache the NPC_type
				//FIXME: make sure we didn't precache this NPC_type already
				CG_LoadingString( va("NPC %s", g_entities[i].NPC_type ) );
				NPC_Precache( &g_entities[i] );
				CG_PrecachePlayerGreetingSound( g_entities[i].NPC_type );
			}
		}
	}

	//always precache these, I guess...
	CG_PrecachePlayerGreetingSound( "crewman" );
	CG_PrecachePlayerGreetingSound( "ensign" );
	CG_PrecachePlayerGreetingSound( "lt" );
	CG_PrecachePlayerGreetingSound( "generic1" );
	CG_PrecachePlayerGreetingSound( "generic2" );
	CG_PrecachePlayerGreetingSound( "generic3" );
	CG_PrecachePlayerGreetingSound( "generic4" );

	cg.loadLCARSStage = 8;

	// Registering interface graphics
	for (i=0;i<IG_MAX;++i)
	{
		if (interface_graphics[i].file)
		{
			interface_graphics[i].graphic = cgi_R_RegisterShaderNoMip(interface_graphics[i].file);
		}

		// Turn everything off at first
		if ((interface_graphics[i].type == SG_GRAPHIC) || (interface_graphics[i].type == SG_NUMBER))
		{
			interface_graphics[i].type = SG_OFF;
		}
	}

	interface_graphics[IG_GROW].type = SG_OFF;

	//register speaker table skins
	for ( i = 0; i < SP_MAX ; i++ )
	{
		if ( speakerTable[i].headModel != 0 )
		{//had a model registered for it, so register the skins
			speakerTable[i].headSkin = CG_RegisterHeadSkin( speakerTable[i].headModelFile, speakerTable[i].skinName, &speakerTable[i].extensions );
		}
	}

	cg.loadLCARSStage = 9;

	// For mission objectives
	cgs.media.objcorner1= cgi_R_RegisterShaderNoMip("menu/objectives/swoop1.tga");
	cgs.media.objcorner2= cgi_R_RegisterShaderNoMip("menu/objectives/swoop2.tga");
	cgs.media.objcorner3= cgi_R_RegisterShaderNoMip("menu/common/corner_ll_22_47.tga");
	cgs.media.pending= cgi_R_RegisterShaderNoMip("menu/objectives/circle_out.tga");
	cgs.media.notpending= cgi_R_RegisterShaderNoMip("menu/objectives/circle.tga");

	//lcars interface
	cgs.media.weaponcap1 = cgi_R_RegisterShaderNoMip("gfx/interface/cap4");
	cgs.media.weaponcap2 = cgi_R_RegisterShaderNoMip("gfx/interface/cap5");
	cgs.media.ammoslider = cgi_R_RegisterShaderNoMip("gfx/interface/ammobar");
	cgs.media.weaponbox	 = cgi_R_RegisterShaderNoMip( "gfx/interface/weapon_box");

	cgs.media.talkingtop = cgi_R_RegisterShaderNoMip("gfx/interface/captop");
	cgs.media.talkingbot = cgi_R_RegisterShaderNoMip("gfx/interface/capbot");
	cgs.media.talkingcap = cgi_R_RegisterShaderNoMip("gfx/interface/captopside");

	cgs.media.bracketlu = cgi_R_RegisterShader("gfx/interface/bracketlefttop");
	cgs.media.bracketld = cgi_R_RegisterShader("gfx/interface/bracketleftbot");
	cgs.media.bracketru = cgi_R_RegisterShader("gfx/interface/bracketrighttop");
	cgs.media.bracketrd = cgi_R_RegisterShader("gfx/interface/bracketrightbot");

	NPCsPrecached = qtrue;

	if (com_buildScript->integer) {
		cgi_R_RegisterShader( "gfx/misc/nav_cpoint" );
		cgi_R_RegisterShader( "gfx/misc/nav_line" );
		cgi_R_RegisterShader( "gfx/misc/nav_arrow" );
		cgi_R_RegisterShader( "gfx/misc/nav_node" );
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

void CG_LinkCentsToGents(void)
{
	int	i;

	for(i = 0; i < MAX_GENTITIES; i++)
	{
		cg_entities[i].gent = &g_entities[i];
	}
}

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	cgi_S_StartBackgroundTrack( parm1, parm2 );
}

/*
======================
CG_GameStateReceived

Displays the info screen while loading media
======================
*/
int iCGResetCount=0;
qboolean qbVidRestartOccured = qfalse;
void CG_GameStateReceived( void ) {
	// clear everything
	
	extern void CG_ClearAnimSndCache( void );
	CG_ClearAnimSndCache();	// else sound handles wrong after vid_restart

	qbVidRestartOccured = qtrue;
	iCGResetCount++;
	if (iCGResetCount == 1)	// this will only equal 1 first time, after each vid_restart it just gets higher.
	{						//	This non-clear is so the user can vid_restart during scrolling text without losing it.
		qbVidRestartOccured = qfalse;		
	}

	if (!qbVidRestartOccured)
	{
		memset( &cg, 0, sizeof( cg ) );
	}
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );
	
	CG_LinkCentsToGents();

	cg.weaponSelect = WP_PHASER;

	// get the rendering configuration from the client system
	cgi_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;


/*	cgs.charScale = cgs.glconfig.vidHeight * (1.0/480.0);
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		// wide screen
		cgs.bias = 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * (640.0/480.0) ) );
	}
	else {
		// no wide screen
		cgs.bias = 0;
	}
*/
	// get the gamestate from the client system
	cgi_GetGameState( &cgs.gameState );

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	cgi_CM_LoadMap( cgs.mapname );

	CG_RegisterSounds();

	CG_RegisterGraphics();

	//jfm: moved down to preinit
//	CG_InitLocalEntities();
//	CG_InitMarkPolys();

	CG_StartMusic();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// To get the interface timing started
	cg.interfaceStartupTime = 0;
	cg.interfaceStartupDone = 0;

	CGCam_Init();
	FX_Init();

}


/*
=================
CG_PreInit

Called when DLL loads (after subsystem restart, but before gamestate is received)
=================
*/
void CG_PreInit() {
	memset( &cg, 0, sizeof( cg ) );
	memset( &cgs, 0, sizeof( cgs ) );
	iCGResetCount = 0;

	CG_RegisterCvars();

//moved from CG_GameStateReceived because it's loaded sooner now
	CG_InitLocalEntities();

	CG_InitMarkPolys();
}

/*
=================
CG_Init

Called after every level change or subsystem restart
=================
*/
void CG_Init( int serverCommandSequence ) {
	cgs.serverCommandSequence = serverCommandSequence;

//	cgs.media.charsetShader = cgi_R_RegisterShaderNoMip( "gfx/2d/bigchars" );
	cgs.media.charsetShader = cgi_R_RegisterShaderNoMip("gfx/2d/charsgrid_med");
	cgs.media.charsetProp = cgi_R_RegisterShaderNoMip("gfx/2d/chars_medium");
	cgs.media.charsetPropTiny = cgi_R_RegisterShaderNoMip("gfx/2d/chars_tiny");
	cgs.media.charsetPropBig = cgi_R_RegisterShaderNoMip("gfx/2d/chars_big");

	cgs.media.whiteShader   = cgi_R_RegisterShader( "white" );
	cgs.media.status_corner_18_22  = cgi_R_RegisterShaderNoMip( "menu/common/corner_ul_18_22");
	cgs.media.status_corner_16_18  = cgi_R_RegisterShaderNoMip( "menu/common/corner_ll_16_18");
	cgs.media.status_corner_8_16_b  = cgi_R_RegisterShaderNoMip( "menu/common/corner_lr_8_16_b");
	cgs.media.status_corner_8_22  = cgi_R_RegisterShaderNoMip( "menu/common/corner_ur_8_22");

	CG_LoadIngameText();

	CG_LoadFonts();

	// Loading graphics
	cgs.media.loading1		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece1.tga" );
	cgs.media.loading2		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece2.tga" );
	cgs.media.loading3		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece3.tga" );
	cgs.media.loading4		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece4.tga" );
	cgs.media.loading5		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece5.tga" );
	cgs.media.loading6		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece6.tga" );
	cgs.media.loading7		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece7.tga" );
	cgs.media.loading8		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece8.tga" );
	cgs.media.loading9		= cgi_R_RegisterShaderNoMip( "menu/loading/smpiece9.tga" );
	cgs.media.loadingcircle = cgi_R_RegisterShaderNoMip( "menu/loading/arrowpiece.tga" );
	cgs.media.loadingquarter= cgi_R_RegisterShaderNoMip( "menu/loading/quarter.tga" );
	cgs.media.loadingcorner	= cgi_R_RegisterShaderNoMip( "menu/common/corner_lr_8_16.tga" );
	cgs.media.loadingtrim	= cgi_R_RegisterShaderNoMip( "menu/loading/trimupper.tga" );

	cg.loadLCARSStage		= 0;
	cg.loadLCARScnt			= 0;

	CG_GameStateReceived();

	CG_InitConsoleCommands();

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	cgi_AddCommand ("kill");
	cgi_AddCommand ("give");
	cgi_AddCommand ("god");
	cgi_AddCommand ("notarget");
	cgi_AddCommand ("noclip");
	cgi_AddCommand ("undying");
	cgi_AddCommand ("setviewpos");
	cgi_AddCommand ("setobjective");
	cgi_AddCommand ("viewobjective");

	// Not until it's done will it be seen
	interface_graphics[IG_HEALTH_START].timer =	cg.time + 3000;
	interface_graphics[IG_ARMOR_START].timer = 	cg.time + 3100;
	interface_graphics[IG_AMMO_START].timer = 	cg.time + 3200;

	cg.missionInfoFlashTime = 0;
	cg.missionStatusShow = qfalse;

}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) 
{
	FX_Free();
	playerOverbouncePredictor.reset();
}

//// DEBUG STUFF
/*
-------------------------
CG_DrawNode
-------------------------
*/
void CG_DrawNode( vec3_t origin, int type )
{
	vec3_t	color;
	float	scale = 16.0f;

	switch ( type )
	{
	case NODE_NORMAL:
		VectorSet( color, 255, 0, 0 );
		break;

	case NODE_START:
		VectorSet( color, 0, 0, 255 );
		scale += 16.0f;
		break;

	case NODE_GOAL:
		VectorSet( color, 0, 255, 0 );
		scale += 16.0f;
		break;

	case NODE_NAVGOAL:
		VectorSet( color, 255, 255, 0 );
		break;
	}
	
	FX_AddSprite( origin, NULL, NULL, scale, 0.0f, 1.0f, 1.0f, color, color, 0.0f, 0.0f, 100, cgi_R_RegisterShader( "gfx/misc/nav_node" ) );
}

/*
-------------------------
CG_DrawRadius
-------------------------
*/

void CG_DrawRadius( vec3_t origin, unsigned int radius, int type )
{
	vec3_t	color;

	switch ( type )
	{
	case NODE_NORMAL:
		VectorSet( color, 255, 0, 0 );
		break;

	case NODE_START:
		VectorSet( color, 0, 0, 255 );
		break;

	case NODE_GOAL:
		VectorSet( color, 0, 255, 0 );
		break;

	case NODE_NAVGOAL:
		VectorSet( color, 255, 255, 0 );
		break;
	}
	
	vec3_t	normal = { 0, 0, 1 };	//Up

	FX_AddQuad( origin, normal, NULL, NULL, radius, 0.0f, 1.0f, 1.0f, color, color, 0, 0, 0, 100, cgi_R_RegisterShader( "gfx/misc/nav_node" ) );
}

/*
-------------------------
CG_DrawEdge
-------------------------
*/

void CG_DrawEdge( vec3_t start, vec3_t end, int type )
{
	switch ( type )
	{
	case EDGE_PATH:
		FX_AddLine( start, end, 4.0f, 4.0f, 0.0f, 1.0f, 1.0f, 100, cgi_R_RegisterShader( "gfx/misc/nav_arrow" ), 0 );
		break;

	case EDGE_NORMAL:
		FX_AddLine( start, end, 8.0f, 4.0f, 0.0f, 0.5f, 0.5f, 100, cgi_R_RegisterShader( "gfx/misc/nav_line" ), 0 );
		break;
	
	case EDGE_BROKEN:
		{
			vec3_t	color = { 255, 0, 0 };

			FX_AddLine( start, end, 8.0f, 4.0f, 0.0f, 0.5f, 0.5f, color, color, 100, cgi_R_RegisterShader( "gfx/misc/nav_line" ), 0 );
		}
		break;

	default:
		break;
	}
}

/*
-------------------------
CG_DrawCombatPoint
-------------------------
*/

void CG_DrawCombatPoint( vec3_t origin, int type )
{
	vec3_t	color	= { 0, 255, 255 };

	switch( type )
	{
	case 0:	//FIXME: To shut up the compiler warning (more will be added here later of course)
	default:
		FX_AddSprite( origin, NULL, NULL, 8.0f, 0.0f, 1.0f, 1.0f, color, color, 0.0f, 0.0f, 100, cgi_R_RegisterShader( "gfx/misc/nav_cpoint" ) );
		break;
	}
}

/*
-------------------------
CG_DrawAlert
-------------------------
*/

void CG_DrawAlert( vec3_t origin, float rating )
{
	vec3_t	drawPos;

	VectorCopy( origin, drawPos );
	drawPos[2] += 48;

	vec3_t	startRGB;

	//Fades from green at 0, to red at 1
	startRGB[0] = rating;
	startRGB[1] = 1 - rating;
	startRGB[2] = 0;

	FX_AddSprite( drawPos, NULL, NULL, 16, 0.0f, 1.0f, 1.0f, startRGB, startRGB, 0, 0, 100, cgs.media.whiteShader );
}

#define MAXINGAMETEXT 4000
char ingameText[MAXINGAMETEXT];



/*
=================
CG_ParseIngameText
=================
*/
void CG_ParseIngameText(void)
{
	char	*token;
	char *buffer;
	int i;
	int len;

	COM_BeginParseSession();

	buffer = ingameText;
	i = 1;	// Zero is null string
	while ( buffer ) 
	{
		token = COM_ParseExt( &buffer, qtrue );

		len = strlen(token);
		if (len)
		{
			ingame_text[i] = (buffer - (len + 1));	// The +1 is to get rid of the " at the beginning of the sting.
			*(buffer - 1) = '\0';		//	Place an string end where is belongs.

			++i;
		}

		if (i> IGT_MAX)
		{
			Com_Printf( S_COLOR_RED "CG_ParseIngameText : too many values! Needed %d but got %d.\n",IGT_MAX,i);
			return;
		}
	}


	if (i != IGT_MAX)
	{
		Com_Printf( S_COLOR_RED "CG_ParseIngameText : not enough values! Needed %d but only got %d.\n",IGT_MAX,i);
		for(;i<IGT_MAX;i++) {
			ingame_text[i] = "?";
		}
	}

}


/*
CG_LanguageFilename - create a filename with an extension based on the value in g_language
*/
void CG_LanguageFilename(char *baseName,char *baseExtension,char *finalName)
{
	char	language[32];
	fileHandle_t	file;

	Q_strncpyz(language,cg_language.string,sizeof(language));
	
	// If it's English then no extension
	if (language[0]=='\0' || Q_stricmp("ENGLISH",language)==0)
	{
		Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
	}
	else
	{
		Com_sprintf(finalName,MAX_QPATH,"%s_%s.%s",baseName,language,baseExtension);

		//Attempt to load the file
		cgi_FS_FOpenFile( finalName, &file, FS_READ );

		if ( file == 0 )	//	This extension doesn't exist, go English.
		{
			Com_sprintf(finalName,MAX_QPATH,"%s.%s",baseName,baseExtension);
		}
		else
		{
			cgi_FS_FCloseFile( file );
		}
	}
}

/*
=================
CG_LoadIngameText
=================
*/
void CG_LoadIngameText(void)
{
	int len;
	fileHandle_t	f;
	char	filename[MAX_QPATH];

	CG_LanguageFilename("ext_data/sp_ingametext","dat",filename);

	len = cgi_FS_FOpenFile( filename, &f, FS_READ );

	if ( !f ) 
	{
		Com_Printf( S_COLOR_RED "CG_LoadIngameText : sp_ingametext.dat file not found!\n");
		return;
	}

	if (len > MAXINGAMETEXT)
	{
		Com_Printf( S_COLOR_RED "CG_LoadIngameText : sp_ingametext.dat file bigger than %d!\n",MAXINGAMETEXT);
		return;
	}

	// initialise the data area
	memset(ingameText, 0, sizeof(ingameText));	

	cgi_FS_Read( ingameText, len, f );

	cgi_FS_FCloseFile( f );


	CG_ParseIngameText();

}



