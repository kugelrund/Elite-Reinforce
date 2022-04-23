// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "cg_media.h"	//just for cgs....

void CG_TargetCommand_f( void );
void CG_RangeIncrease_f( void );
void CG_RangeDecrease_f( void );
extern qboolean	player_locked;
extern void CMD_CGCam_Disable( void );
/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	cgi_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	cgi_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("%s (%i %i %i) : %i\n", cgs.mapname, (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}

void CG_WriteCam_f (void)
{
	char	text[1024];
	char	*targetname;
	static	int	numCams;

	numCams++;
	
	targetname = (char	*)CG_Argv(1);

	if( !targetname || !targetname[0] )
	{
		targetname = "nameme!";
	}

	CG_Printf( "Camera #%d ('%s') written to: ", numCams, targetname );
	sprintf( text, "//entity %d\n{\n\"classname\"	\"ref_tag\"\n\"targetname\"	\"%s\"\n\"origin\" \"%i %i %i\"\n\"angles\" \"%i %i %i\"\n\"fov\" \"%i\"\n}\n", numCams, targetname, (int)cg.refdef.vieworg[0], (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], (int)cg.refdefViewAngles[0], (int)cg.refdefViewAngles[1], (int)cg.refdefViewAngles[2], cg_fov.integer );
	gi.WriteCam( text );
}

void Lock_Disable ( void )
{
	player_locked = qfalse;
}



#if 1
static void CG_InfoDown_f( void ) {
	cg.showInformation = qtrue;
}

static void CG_InfoUp_f( void ) 
{
	cg.showInformation = qfalse;
}
#else
/*static void CG_Info_f( void ) {
	cg.showInformation = !cg.showInformation;
}*/
#endif


/*
===================
CG_SaveHolodeck_f
===================
*/
void CG_SaveHolodeck_f (void) {
	const char* info = CG_ConfigString( CS_SERVERINFO );
	const char* orig_mapname = Info_ValueForKey( info, "mapname" );

	if (orig_mapname[0] != '_') {
		CG_Printf( "Map '%s' is not a holodeck map.\n", orig_mapname);
		return;
	}

	// The game prohibits saving on maps with '_' as first character, so that
	// a player cannot save on holodeck maps for example. To trick it into
	// saving anyways, we replace that '_' in the mapname. Luckily, there seems
	// to be a delay in setting the mapname inside of the config string, so
	// that the original name with the underscore is written to the save file,
	// while the check for the underscore (which looks at the cvar directly)
	// already sees the modified name. That is exactly what we need; the check
	// for being allowed to save is tricked, but the correct map is read when
	// loading that save.
	// However, after some delay, the underscore in the config string will be
	// changed. A holodeck save created afterwards would then have the modified
	// mapname and could not be loaded. That's why we queue the command
	// "restore_holodeck_mapname" after saving, which resets "mapname" to the
	// original name with underscore again.

	char modified_mapname[MAX_QPATH];
	Q_strncpyz(modified_mapname, orig_mapname, sizeof(modified_mapname));
	modified_mapname[0] = '-';  // replace the '_' with anything else
	cgi_Cvar_Set("mapname", modified_mapname);

	cgi_SendConsoleCommand(va("save %s; restore_holodeck_mapname", CG_Argv(1)));
}

/*
===================
CG_RestoreHolodeckMapname_f
===================
*/
void CG_RestoreHolodeckMapname_f (void) {
	// Undo the modification to the mapname cvar. Because of the delay in
	// setting the mapname in the config string, we can still obtain the
	// original name from that and simply set the cvar again, but now back to
	// that original.
	const char* info = CG_ConfigString( CS_SERVERINFO );
	const char* orig_mapname = Info_ValueForKey( info, "mapname" );
	cgi_Cvar_Set("mapname", orig_mapname);
}


typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "writecam", CG_WriteCam_f },
	{ "+info", CG_InfoDown_f },
	{ "-info", CG_InfoUp_f },
//	{ "info",CG_Info_f},
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "+range", CG_RangeIncrease_f },
	{ "-range", CG_RangeDecrease_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "messagemode2", CG_Weapon_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "cam_disable", CMD_CGCam_Disable },	//gets out of camera mode for debuggin
	{ "cam_enable", CGCam_Enable },	//gets into camera mode for precise camera placement
	{ "lock_disable", Lock_Disable },	//player can move now
	{ "+analysis", CG_AnalysisDown_f },
	{ "-analysis", CG_AnalysisUp_f },
	{ "saveholodeck", CG_SaveHolodeck_f },
	{ "restore_holodeck_mapname", CG_RestoreHolodeckMapname_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		cgi_AddCommand( commands[i].cmd );
	}
}
