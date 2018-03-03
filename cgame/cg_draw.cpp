// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "cg_media.h"
#include "cg_text.h"
#include "..\game\objectives.h"
#include "..\game\speakers.h"
#include "..\speedrun\PlayerOverbouncePrediction.hpp"
#include "..\speedrun\strafe_helper\StrafeHelper.hpp"


qboolean G_ParseInt( char **data, int *i );
qboolean G_ParseString( char **data, char **s ); 
qboolean G_ParseFloat( char **data, float *f );
void CG_ColorForGivenHealth( vec4_t hcolor, int health );
void CG_DrawMissionInformation( void );
extern float g_crosshairEntDist;
extern int g_crosshairSameEntTime;
extern int g_crosshairEntNum;
extern int g_crosshairEntTime;

speakerTable_t speakerTable [SP_MAX] = 
{
	"NONE",				"",					0,	"", 0, qfalse,	// SP_NONE
	//Senior Officers
	"JANEWAY",			"janeway",			0,	"default", 0, qfalse,	// SP_JANEWAY
	"CHAKOTAY",			"chakotay",			0,	"default", 0, qfalse,	// SP_CHAKOTAY	
	"TUVOK",			"tuvok",			0,	"default", 0, qfalse,	// SP_TUVOK
	"TORRES",			"torres",			0,	"default", 0, qfalse,	// SP_TORRES
	"PARIS",			"paris",			0,	"default", 0, qfalse,	// SP_PARIS
	"NEELIX",			"neelix",			0,	"default", 0, qfalse,	// SP_NEELIX
	"SEVEN",			"seven",			0,	"default", 0, qfalse,	// SP_SEVEN
	"DOCTOR",			"doctor",			0,	"default", 0, qfalse,	// SP_DOCTOR
	"KIM",				"kim",				0,	"default", 0, qfalse,	// SP_KIM
	//HazTeam alpha
	"FOSTER",			"foster",			0,	"default", 0, qfalse,	// SP_FOSTER
	"MUNRO",			"munro",			0,	"default", 0, qfalse,	// SP_MUNRO
	"BIESSMAN",			"biessman",			0,	"default", 0, qfalse,	// SP_BIESSMAN  
	"CHANG",			"chang",			0,	"default", 0, qfalse,	// SP_CHANG
	"CHELL",			"chell",			0,	"default", 0, qfalse,	// SP_CHELL
	"JUROT",			"telsia",			0,	"jurot", 0, qfalse,	// SP_JUROT
	"TELSIA",			"telsia",			0,	"default", 0, qfalse,	// SP_TELSIA
	//HazTeam beta
	"KENN",				"munro",			0,	"kenn", 0, qfalse,		// SP_HOEKSTRA
	"CSATLOS",			"oviedo_h",			0,	"jaworski", 0, qfalse,	// SP_CSATLOS
	"NELSON",			"chell",			0,	"long", 0, qfalse,		// SP_NELSON
	"ODELL",			"chang",			0,	"odell", 0, qfalse,	// SP_ODELL
	"OVIEDO",			"oviedo_h",			0,	"default", 0, qfalse,	// SP_OVIEDO
	"JAWORSKI",			"oviedo_h",			0,	"csatlos", 0, qfalse,	// SP_JAWORSKI
	"LAIRD",			"garren",			0,	"mackey", 0, qfalse,	// SP_LAIRD
	//Misc crew
	"LANG",				"chakotay",			0,	"nelson", 0, qfalse,	// SP_LANG
	"RENNER",			"munrocrew",		0,	"jon", 0, qfalse,	// SP_RENNER
	"PELLETIER",		"pelletier",		0,	"default", 0, qfalse,	// SP_PELLETIER
	"GREEN",			"green",			0,	"default", 0, qfalse,	// SP_GREEN
	"SALMA",			"garren",			0,	"salma", 0, qfalse,	// SP_SALMA
	
	//Baddies
	"HIROGENALPHA",		"hirogen_boss",		0,	"default", 0, qfalse,	// SP_HIROGEN_BOSS
	"DOCKREEGE",		"imperial4",		0,	"default", 0, qfalse,	// SP_KREEGE
	//Other
	"COMPUTER",			"",					0,	"", 0, qfalse,	// SP_COMPUTER
};


float interfaceColors[MC_MAX][4] = 
{ 
	{ 0.671875, 0.515625, 0., 1.0 } ,	// Ammo Count, Star Fleet Orange (brighter)
	{0.66666F,0.309803F,0.0,1.0},			// Ammo Icon, Star Fleet Orange  (darker)
	{ 1.0F, 0.2F, 0.2F, 1.0F },				// Red
	{0.5, 0.5, 0.5, 1},					// Grey
	{0.592156F, 0.592156F, 0.850980F, 1},	// Ammo Alien Bright Color
	{0.492156F, 0.492156F, 0.750980F, 1},	// Ammo Alien Dark Color
	{ 1.0, 0.4F, 0.4F, 1.0 },				// Dark Red
	{1.0, 0.811764F, 0.376470F, 1.0},		// Ammo Phaser Bright Color
	{0.90F, 0.711764F, 0.276470F, 1.0},	// Ammo Phaser Dark Color
	{1.0,1.0,1.0,1.0},					// MC_WHITE
	{1.0,1.0,0.0,1.0}					// MC_YELLOW
};

char *ingame_text[IGT_MAX]; 
/*
char *ingame_text[IGT_MAX] = 
{
NULL,								//	IGT_NONE
"GAME PAUSED",						//	IGT_PAUSED,
"MISSION INFORMATION",				//	IGT_MISSIONINFORMATION
"TACTICAL INFORMATION",				//	IGT_TACTICALINFO
"OBJECTIVES:",						//	IGT_OBJECTIVES
"NONE",								//	IGT_NONE1,
"MISSION INFORMATION UPDATED",		//	IGT_MISSIONINFO_UPDATED,
" - FAILED",						//	IGT_FAILED,
" - SUCCEEDED",						//	IGT_SUCCEEDED,
"LCARS",							//	IGT_LCARS,
"MISSION ANALYSIS",					//	IGT_MISSIONANALYSIS,

"ENEMIES ELIMINATED :",				//	IGT_ENEMIES
"TEAM CASULATIES :",				//	IGT_CASULATIES
"SHOTS FIRED :",					//	IGT_SHOTSFIRED
"SHOTS EFFECTIVE :",				//	IGT_EFFECTIVE
"ACCURACY :",						//	IGT_ACCURACY
"PUZZLES SOLVED :",					//	IGT_PUZZLESSOLVED
"MISSION DURATION :",				//	IGT_DURATION
"FINAL ANALYSIS :",					//	IGT_ANALYSIS

"MISSION SUCCESSFUL",				//	IGT_MISSIONSUCCESSFUL,
"MISSION FAILED",					//	IGT_MISSIONFAILED,
"You performed inadequately.",		//	IGT_INADEQUATE,
"Your response time could be improved.",	// IGT_RESPONSETIME
"Spend more time on the shooting range.",	// IGT_SHOOTINRANGE
"Try again.",							//	IGT_TRYAGAIN

"You performed adequately.",				//	IGT_ADEQUATE,
"Your response time is impressive.",		// IGT_RESPONSETIMEIMPRESSIVE
"You are obviously a marksman.",			// IGT_MARKSMAN
"Well done.",						//	IGT_CONGRATULATIONS
};
*/

interfacegraphics_s interface_graphics[IG_MAX] = 
{
//	type		timer	x		y		width	height	file/text						graphic,	min		max		color			style			ptr
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_GROW

	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_HEALTH_START
	SG_GRAPHIC,	0.0,	5,		429,	32,		64,		"gfx/interface/healthcap1",		NULL,		0,		0,		CT_DKBROWN1,	0,				NULL,	// IG_HEALTH_BEGINCAP
	SG_GRAPHIC,	0.0,	64,		429,	6,		25,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_DKBROWN1,	0,				NULL,	// IG_HEALTH_BOX1
	SG_GRAPHIC,	0.0,	72,		429,	0,		25,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_LTBROWN1,	0,				NULL,	// IG_HEALTH_SLIDERFULL
	SG_GRAPHIC,	0.0,	0,		429,	0,		25,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_DKBROWN1,	0,				NULL,	// IG_HEALTH_SLIDEREMPTY
	SG_GRAPHIC,	0.0,	72,		429,	16,		32,		"gfx/interface/healthcap2",		NULL,		0,		147,	CT_DKBROWN1,	0,				NULL,	// IG_HEALTH_ENDCAP
	SG_NUMBER,	0.0,	23,		425,	16,		32,		NULL,							NULL,		0,		0,		CT_YELLOW,		NUM_FONT_BIG,	NULL,	// IG_HEALTH_COUNT
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_HEALTH_END

	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_ARMOR_START
	SG_GRAPHIC,	0.0,	20,		458,	32,		16,		"gfx/interface/armorcap1",		NULL,		0,		0,		CT_DKPURPLE1,	0,				NULL,	// IG_ARMOR_BEGINCAP
	SG_GRAPHIC,	0.0,	64,		458,	6,		12,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_DKPURPLE1,	0,				NULL,	// IG_ARMOR_BOX1
	SG_GRAPHIC,	0.0,	72,		458,	0,		12,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_LTPURPLE1,	0,				NULL,	// IG_ARMOR_SLIDERFULL
	SG_GRAPHIC,	0.0,	0,		458,	0,		12,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_DKPURPLE1,	0,				NULL,	// IG_ARMOR_SLIDEREMPTY
	SG_GRAPHIC,	0.0,	72,		458,	16,		16,		"gfx/interface/armorcap2",		NULL,		0,		147,	CT_DKPURPLE1,	0,				NULL,	// IG_ARMOR_ENDCAP
	SG_NUMBER,	0.0,	44,		458,	16,		16,		NULL,							NULL,		0,		0,		CT_YELLOW,		NUM_FONT_SMALL,	NULL,	// IG_ARMOR_COUNT
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_ARMOR_END

	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_AMMO_START
	SG_GRAPHIC,	0.0,	613,	429,	32,		64,		"gfx/interface/ammouppercap1",	NULL,		0,		0,		CT_LTPURPLE2,	0,				NULL,	// IG_AMMO_UPPER_BEGINCAP
	SG_GRAPHIC,	0.0,	607,	429,	16,		32,		"gfx/interface/ammouppercap2",	NULL,		0,		572,	CT_LTPURPLE2,	0,				NULL,	// IG_AMMO_UPPER_ENDCAP
	SG_GRAPHIC,	0.0,	613,	458,	16,		16,		"gfx/interface/ammolowercap1",	NULL,		0,		0,		CT_LTPURPLE2,	0,				NULL,	// IG_AMMO_LOWER_BEGINCAP
	SG_GRAPHIC,	0.0,	578,	458,	0,		12,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_LTPURPLE1,	0,				NULL,	// IG_AMMO_SLIDERFULL
	SG_GRAPHIC,	0.0,	0,		458,	0,		12,		"gfx/interface/ammobar",		NULL,		0,		0,		CT_DKPURPLE1,	0,				NULL,	// IG_AMMO_SLIDEREMPTY
	SG_GRAPHIC,	0.0,	607,	458,	16,		16,		"gfx/interface/ammolowercap2",	NULL,		0,		572,	CT_LTPURPLE2,	0,				NULL,	// IG_AMMO_LOWER_ENDCAP
	SG_NUMBER,	0.0,	573,	425,	16,		32,		NULL,							NULL,		0,		0,		CT_YELLOW,		NUM_FONT_BIG,	NULL,	// IG_AMMO_COUNT
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,							NULL,		0,		0,		CT_NONE,		0,				NULL,	// IG_AMMO_END

};


vec3_t	vfwd;
vec3_t	vright;
vec3_t	vup;
vec3_t	vfwd_n;
vec3_t	vright_n;
vec3_t	vup_n;
int		infoStringCount;
//===============================================================



/*
================
CG_Draw3DModel

================
*/
static void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	cgi_R_ClearScene();
	cgi_R_AddRefEntityToScene( &ent );
	cgi_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
int	USE_ENT_NUM	= 2048;
void CG_DrawHead( float x, float y, float w, float h, int speaker_i, vec3_t headAngles ) 
{
	qhandle_t		hm = 0;
	qhandle_t		hs = 0;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs;
	gentity_t	*ent;
	qboolean	extensions = qfalse;
	int			talking = 0;

	//If the talking ent is actually on the level, use his info
	if ( cg.gameTextEntNum != -1 && cg.gameTextEntNum < ENTITYNUM_WORLD )
	{
		ent = &g_entities[cg.gameTextEntNum];
		if ( ent && ent->client )
		{
			hm = ent->client->clientInfo.headModel;
			if ( hm )
			{
				hs = ent->client->clientInfo.headSkin;
				extensions = ent->client->clientInfo.extensions;
				talking = gi.S_Override[ent->s.number];
			}
		}
	}

	if ( !hm )
	{	
		hm = cgs.model_draw[speakerTable[speaker_i].headModel];
		if ( !hm ) 
		{//this will happen with the computer
			return;
		}
		//PRECACHE ME!!!  But only if we're needed on this map!
		hs = speakerTable[speaker_i].headSkin;//fabs(cgi_R_RegisterSkin( va("models/players/%s/head_%s.skin", speakerTable[speaker_i].headModelFile, speakerTable[speaker_i].skinName ) ) );
		//fabs because a < 0 skin number just means it has extensions
		extensions = speakerTable[speaker_i].extensions;
		//get talking from player, we're presuming that if it isn't playing on an NPC, it's playing on the player
		talking = gi.S_Override[0];
	}

	if ( !talking )
	{//no sound playing, don't display the head any more
		cg.gameNextTextTime = cg.time;
		return;
	}

	//add talking anim
	if ( extensions && talking > 0 )
	{
		hs = hs + talking;
	}

	// offset the origin y and z to center the head
	cgi_R_ModelBounds( hm, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

	// calculate distance so the head nearly fills the box
	// assume heads are taller than wide
	len = 0.7 * ( maxs[2] - mins[2] );		
	origin[0] = len / 0.268;	// len / tan( fov/2 )

	CG_Draw3DModel( x, y, w, h, hm, hs, origin, headAngles );
}

/*
================
CG_DrawTalk

================
*/
static void CG_DrawTalk(centity_t	*cent)
{
	float		size;
	vec3_t		angles;
//	int			totalLines,y,i;
	vec4_t		color; 

	if ( cg.gameNextTextTime > cg.time) 
	{
		color[0] = colorTable[CT_BLACK][0];
		color[1] = colorTable[CT_BLACK][1];
		color[2] = colorTable[CT_BLACK][2];
		color[3] = 0.350F;

		cgi_R_SetColor(color);	// Background
		CG_DrawPic( 5, 27,  50, 64,	cgs.media.ammoslider );

		cgi_R_SetColor(colorTable[CT_LTPURPLE1]);
		CG_DrawPic( 5, 6, 128, 64,	cgs.media.talkingtop );
/*
		totalLines = cg.scrollTextLines - cg.gameTextCurrentLine;

		y = 6;
		CG_DrawPic( 55,      y, 16, 16,	cgs.media.bracketlu );
		CG_DrawPic( 616,     y, 16, 16, cgs.media.bracketru );

		for (i=1;i<totalLines;++i)
		{
			y +=16;
			CG_DrawPic( 55, y, 16, 16,	cgs.media.ammoslider );
			CG_DrawPic( 616,y, 16, 16, cgs.media.ammoslider );
		}

		y +=16;
		CG_DrawPic( 55, y, 16, 16, cgs.media.bracketld );
		CG_DrawPic( 616,y, 16, 16, cgs.media.bracketrd );
*/
		size = ICON_SIZE * 1.5;
		VectorClear( angles );
		angles[YAW] = 180;


		CG_DrawHead( -6, 25, size, size, cg.gameTextSpeaker, angles );

		cgi_R_SetColor(colorTable[CT_LTPURPLE1]);	// Bottom
		CG_DrawPic( 5, 90, 64, 16,	cgs.media.talkingbot );
		cgi_R_SetColor(NULL);
	}
}

/*
================
CG_DrawArmor

================
*/
static void CG_DrawArmor(centity_t	*cent)
{
	int			max,i;
	float		value,xLength;
	playerState_t	*ps;
	int			lengthMax;

	ps = &cg.snap->ps;

	value = ps->stats[STAT_ARMOR];


	for (i=IG_ARMOR_START + 1;i<IG_ARMOR_END;++i)
	{
		if (interface_graphics[i].type == SG_GRAPHIC)
		{
			cgi_R_SetColor(colorTable[interface_graphics[i].color]);

			CG_DrawPic( interface_graphics[i].x, 
			interface_graphics[i].y,	
			interface_graphics[i].width, 
			interface_graphics[i].height,	
			interface_graphics[i].graphic);
		}
	}
	interface_graphics[IG_ARMOR_COUNT].max = value;


	if (cg.oldarmor < value)
	{
		cg.oldArmorTime = cg.time + 100;
	}

	cg.oldarmor = value;

	if (cg.oldArmorTime < cg.time)
	{
		interface_graphics[IG_ARMOR_COUNT].color = CT_LTPURPLE1;	// Numbers
	}
	else
	{
		interface_graphics[IG_ARMOR_COUNT].color = MC_YELLOW;	// Numbers
	}


	max = ps->stats[STAT_MAX_HEALTH];
	lengthMax = 73;
	if (max > 0)
	{
		xLength = lengthMax * (value/max);
	}
	else
	{
		max = 0;
		xLength = 0;
	}

	// Armor empty section
	interface_graphics[IG_ARMOR_SLIDEREMPTY].x = 72 + xLength;
	interface_graphics[IG_ARMOR_SLIDEREMPTY].width = lengthMax - xLength;

	// Armor full section
	interface_graphics[IG_ARMOR_SLIDERFULL].width = xLength;

	CG_PrintInterfaceGraphics(IG_ARMOR_START + 1,IG_ARMOR_END);

}

/*
================
CG_DrawHealth

================
*/
static void CG_DrawHealth(centity_t	*cent)
{
	int			max;
	float		value,xLength;
	playerState_t	*ps;
	int			lengthMax;
	int			flashHealth;

	ps = &cg.snap->ps;

	value = ps->stats[STAT_HEALTH];


	// Changing colors on numbers
	if (cg.oldhealth < value)
	{
		cg.oldHealthTime = cg.time + 100;
	}
	cg.oldhealth = value;

	flashHealth = ps->stats[STAT_MAX_HEALTH]/4;

	// Is health changing?
	if (value<flashHealth)
	{
		interface_graphics[IG_HEALTH_COUNT].color = CT_RED;	// Numbers

		interface_graphics[IG_HEALTH_SLIDERFULL].color = CT_RED;
		interface_graphics[IG_HEALTH_SLIDERFULL].style |= CG_BLINK;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].color = CT_DKRED;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].style |= CG_BLINK;

		interface_graphics[IG_HEALTH_BEGINCAP].color = CT_DKRED;
		interface_graphics[IG_HEALTH_BOX1].color = CT_DKRED;
		interface_graphics[IG_HEALTH_ENDCAP].color = CT_DKRED;
	}
	else if (cg.oldHealthTime < cg.time)
	{
		interface_graphics[IG_HEALTH_COUNT].color = CT_LTBROWN1;	// Numbers

		interface_graphics[IG_HEALTH_SLIDERFULL].color = CT_LTBROWN1;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_BEGINCAP].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_BOX1].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_ENDCAP].color = CT_DKBROWN1;

		interface_graphics[IG_HEALTH_SLIDERFULL].style &= ~ CG_BLINK;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].style &= ~ CG_BLINK;

	}
	else 
	{
		interface_graphics[IG_HEALTH_COUNT].color = MC_YELLOW;	// Numbers

		interface_graphics[IG_HEALTH_SLIDERFULL].color = CT_LTBROWN1;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_BEGINCAP].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_BOX1].color = CT_DKBROWN1;
		interface_graphics[IG_HEALTH_ENDCAP].color = CT_DKBROWN1;

		interface_graphics[IG_HEALTH_SLIDERFULL].style &= ~ CG_BLINK;
		interface_graphics[IG_HEALTH_SLIDEREMPTY].style &= ~ CG_BLINK;

	}

	interface_graphics[IG_HEALTH_COUNT].max = value;

	// Calculating size of health bar
	max = ps->stats[STAT_MAX_HEALTH];
	lengthMax = 73;
	if (max > 0)
	{
		xLength = lengthMax * (value/max);
	}
	else
	{
		max = 0;
		xLength = 0;
	}

	// Health empty section
	interface_graphics[IG_HEALTH_SLIDEREMPTY].x = 72 + xLength;
	interface_graphics[IG_HEALTH_SLIDEREMPTY].width = lengthMax - xLength;

	// Health full section
	interface_graphics[IG_HEALTH_SLIDERFULL].width = xLength;

	// Print it
	CG_PrintInterfaceGraphics(IG_HEALTH_START + 1,IG_HEALTH_END);
}

/*
================
CG_DrawAmmo

================
*/
static void CG_DrawAmmo(centity_t	*cent)
{
	float		value;
	float		xLength;
	playerState_t	*ps;
	int			max,brightColor_i,darkColor_i,numColor_i;

	ps = &cg.snap->ps;

	if (!cent->currentState.weapon ) // We don't have a weapon right now
	{
		return;
	}

	value = ps->ammo[weaponData[cent->currentState.weapon].ammoIndex];
	if (value < 0)	// No ammo
	{
		return;
	}

	interface_graphics[IG_AMMO_COUNT].max = value;

	if (weaponData[ps->weapon].ammoIndex == AMMO_STARFLEET)
	{
		brightColor_i = CT_LTBLUE2;
		darkColor_i = CT_DKBLUE2;
	}
	else if (weaponData[ps->weapon].ammoIndex == AMMO_ALIEN)
	{
		brightColor_i = CT_LTPURPLE2;
		darkColor_i = CT_DKPURPLE2;
	}
	else
	{
		brightColor_i = CT_LTGOLD1;
		darkColor_i = CT_DKGOLD1;
	}

	//
	// ammo
	//
	if (cg.oldammo < value)
	{
		cg.oldAmmoTime = cg.time + 200;
	}

	cg.oldammo = value;

	if (( cg.predicted_player_state.weaponstate == WEAPON_FIRING
		&& cg.predicted_player_state.weaponTime > 100 ))
	{
		// draw as dark grey when reloading
		numColor_i = CT_LTGREY;
	} 
	else 
	{
		if ( value >= 0 ) 
		{
			if (cg.oldAmmoTime > cg.time)
			{
				numColor_i = CT_YELLOW;
			}
			else
			{
				numColor_i = brightColor_i;
			}
		} 
		else 
		{
			numColor_i = CT_RED;
		}

	}

	// Calc bar length
	max = ammoData[weaponData[cent->currentState.weapon].ammoIndex].max;
	if (max > 0)
	{
		xLength = 33 * (value/max);
	}
	else
	{
		max = 0;
		xLength = 0;
	}
	// Armor empty section
	interface_graphics[IG_AMMO_SLIDEREMPTY].x = 578 + xLength;
	interface_graphics[IG_AMMO_SLIDEREMPTY].width = 33 - xLength;

	// Armor full section
	interface_graphics[IG_AMMO_SLIDERFULL].width = xLength;

	interface_graphics[IG_AMMO_UPPER_BEGINCAP].color = darkColor_i;
	interface_graphics[IG_AMMO_UPPER_ENDCAP].color = darkColor_i;
	interface_graphics[IG_AMMO_LOWER_BEGINCAP].color = darkColor_i;
	interface_graphics[IG_AMMO_LOWER_ENDCAP].color = darkColor_i;
	interface_graphics[IG_AMMO_SLIDERFULL].color = brightColor_i;
	interface_graphics[IG_AMMO_SLIDEREMPTY].color = darkColor_i;
	interface_graphics[IG_AMMO_COUNT].color = numColor_i;

	// Print it
	CG_PrintInterfaceGraphics(IG_AMMO_START + 1,IG_AMMO_END);
}

/*
================
CG_InterfaceStartup
================
*/
static void CG_InterfaceStartup()
{

	// Turn on Health Graphics
	if ((interface_graphics[IG_HEALTH_START].timer < cg.time) && (interface_graphics[IG_HEALTH_BEGINCAP].type == SG_OFF))
	{
		cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.interfaceSnd1);

		interface_graphics[IG_HEALTH_BEGINCAP].type = SG_GRAPHIC;
		interface_graphics[IG_HEALTH_BOX1].type = SG_GRAPHIC;
		interface_graphics[IG_HEALTH_ENDCAP].type = SG_GRAPHIC;
	}

	// Turn on Armor Graphics
	if ((interface_graphics[IG_ARMOR_START].timer <	cg.time) && (interface_graphics[IG_ARMOR_BEGINCAP].type == SG_OFF))
	{
		if (interface_graphics[IG_ARMOR_BEGINCAP].type == SG_OFF)
		{
			cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.interfaceSnd1);
		}

		interface_graphics[IG_ARMOR_BEGINCAP].type = SG_GRAPHIC;
		interface_graphics[IG_ARMOR_BOX1].type = SG_GRAPHIC;
		interface_graphics[IG_ARMOR_ENDCAP].type = SG_GRAPHIC;

	}

	// Turn on Ammo Graphics
	if (interface_graphics[IG_AMMO_START].timer <	cg.time)
	{
		if (interface_graphics[IG_AMMO_UPPER_BEGINCAP].type == SG_OFF)
		{
			cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.interfaceSnd1);
			interface_graphics[IG_GROW].type = SG_VAR;
			interface_graphics[IG_GROW].timer = cg.time;
		}

		interface_graphics[IG_AMMO_UPPER_BEGINCAP].type = SG_GRAPHIC;
		interface_graphics[IG_AMMO_UPPER_ENDCAP].type = SG_GRAPHIC;
		interface_graphics[IG_AMMO_LOWER_BEGINCAP].type = SG_GRAPHIC;
		interface_graphics[IG_AMMO_LOWER_ENDCAP].type = SG_GRAPHIC;
	}

	if (interface_graphics[IG_GROW].type == SG_VAR)
	{
		interface_graphics[IG_HEALTH_ENDCAP].x += 2; 
		interface_graphics[IG_ARMOR_ENDCAP].x += 2; 
		interface_graphics[IG_AMMO_UPPER_ENDCAP].x -= 1; 
		interface_graphics[IG_AMMO_LOWER_ENDCAP].x -= 1; 

		if (interface_graphics[IG_HEALTH_ENDCAP].x >= interface_graphics[IG_HEALTH_ENDCAP].max)
		{
			interface_graphics[IG_HEALTH_ENDCAP].x = interface_graphics[IG_HEALTH_ENDCAP].max;
			interface_graphics[IG_ARMOR_ENDCAP].x = interface_graphics[IG_ARMOR_ENDCAP].max;

			interface_graphics[IG_AMMO_UPPER_ENDCAP].x = interface_graphics[IG_AMMO_UPPER_ENDCAP].max;
			interface_graphics[IG_AMMO_LOWER_ENDCAP].x = interface_graphics[IG_AMMO_LOWER_ENDCAP].max;
			interface_graphics[IG_GROW].type = SG_OFF;

			interface_graphics[IG_HEALTH_SLIDERFULL].type = SG_GRAPHIC;
			interface_graphics[IG_HEALTH_SLIDEREMPTY].type = SG_GRAPHIC;
			interface_graphics[IG_HEALTH_COUNT].type = SG_NUMBER;

			interface_graphics[IG_ARMOR_SLIDERFULL].type = SG_GRAPHIC;
			interface_graphics[IG_ARMOR_SLIDEREMPTY].type = SG_GRAPHIC;
			interface_graphics[IG_ARMOR_COUNT].type = SG_NUMBER;

			interface_graphics[IG_AMMO_SLIDERFULL].type = SG_GRAPHIC;
			interface_graphics[IG_AMMO_SLIDEREMPTY].type = SG_GRAPHIC;
			interface_graphics[IG_AMMO_COUNT].type = SG_NUMBER;

			cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.interfaceSnd2);
			cg.interfaceStartupDone = 1;	// All done
		}

		interface_graphics[IG_GROW].timer = cg.time + 10;
	}

	cg.interfaceStartupTime = cg.time;
}

/*
================
CG_DrawZoomMask

================
*/
extern float cg_zoomFov;	//from cg_view.cpp

static void CG_DrawZoomMask( void )
{
	float		amt = 1, size, val, start_x, start_y;
	int			width, height, i, t;
	vec4_t		color1;

	if ( cg.weaponSelect == WP_PROTON_GUN )
	{
		cg.zoomLocked = qfalse;
		cg.zoomed = qfalse;
		return;
	}

	// Calc where to place the zoom mask...all calcs are based off of a virtual 640x480 screen
	size = cg_viewsize.integer;

	width = 640 * size * 0.01;
	width &= ~1;

	height = 480 * size * 0.01;
	height &= ~1;

	start_x = ( 640 - width ) * 0.5;
	start_y = ( 480 - height ) * 0.5;

	if ( cg.zoomed )
	{
		// Smoothly fade in..Turn this off for now since the zoom is set to snap to 30% or so...fade looks a bit weird when it does that
		if ( cg.time - cg.zoomTime <= ZOOM_OUT_TIME ) {
			amt = ( cg.time - cg.zoomTime ) / ZOOM_OUT_TIME;
		}

		// Fade mask in
		for ( i = 0; i < 4; i++ ) { 
			color1[i] = amt;
		}

		// Set fade color--then draw fullscreen mask
		cgi_R_SetColor( color1 );
		CG_DrawPic( start_x, start_y, width, height, cgs.media.zoomMaskShader );

		start_x = 210;
		start_y = 80;

		// Main curvy zoom art
		CG_DrawPic( 320 + start_x + 35, 241, -35, -170, cgs.media.zoomBar2Shader);
		CG_DrawPic( 320 - start_x, 241, -35, -170, cgs.media.zoomBarShader);
		CG_DrawPic( 320 + start_x, 239, 35, 170, cgs.media.zoomBarShader);
		CG_DrawPic( 320 - start_x - 35, 239, 35, 170, cgs.media.zoomBar2Shader);

		// Draw the array of ticks inset into the curvy art
		CG_DrawPic( 320 + start_x + 12, 245, 10, 108, cgs.media.zoomInsertShader );
		CG_DrawPic( 320 + start_x + 12, 127, 10, 108, cgs.media.zoomInsert2Shader );
		CG_DrawPic( 320 - start_x - 12, 353, -10, -108, cgs.media.zoomInsert2Shader );
		CG_DrawPic( 320 - start_x - 12, 235, -10, -108, cgs.media.zoomInsertShader );

		// pink
		color1[0] = 0.85f * amt;
		color1[1] = 0.55f * amt;
		color1[2] = 0.75f * amt;
		color1[3] = amt;
		cgi_R_SetColor( color1 );

		// Calculate a percent and clamp it
		val = 26 - ( cg_fov.value - cg_zoomFov ) / ( cg_fov.value - MAX_ZOOM_FOV ) * 26;

		if ( val > 17.0f )
			val = 17.0f;
		else if ( val < 0.0f )
			val = 0.0f;

		i = ((int)val) * 6;

		// Draw the tick at the current zoom position
		CG_DrawPic( 320 + start_x + 10, 230 - i, 12, 5, cgs.media.ammoslider );
		CG_DrawPic( 320 + start_x + 10, 245 + i, 12, 5, cgs.media.ammoslider );
		CG_DrawPic( 320 - start_x - 22, 230 - i, 12, 5, cgs.media.ammoslider );
		CG_DrawPic( 320 - start_x - 22, 245 + i, 12, 5, cgs.media.ammoslider );

		// Convert zoom and view axis into some numbers to throw onto the screen
		CG_DrawNumField( 468, 100, 5, (120 - cg_zoomFov) * 500, 18, 10 ,NUM_FONT_BIG, qtrue );
		CG_DrawNumField( 468, 120, 5, ceil(g_crosshairEntDist * 24), 18, 10,NUM_FONT_BIG, qtrue );
		CG_DrawNumField( 468, 140, 5, cg.refdef.viewaxis[0][0] * 9999 + 20000, 18, 10,NUM_FONT_BIG, qtrue );
		CG_DrawNumField( 468, 160, 5, cg.refdef.viewaxis[0][1] * 9999 + 20000, 18, 10,NUM_FONT_BIG, qtrue );
		CG_DrawNumField( 468, 180, 5, cg.refdef.viewaxis[0][2] * 9999 + 20000, 18, 10,NUM_FONT_BIG, qtrue );
		
		// Is it time to draw the little max zoom arrows?
		if ( val < 0.2f )
		{
			amt = sin( cg.time * 0.03 ) * 0.5 + 0.5 * amt;
			for ( t = 0; t < 4; t++ )
				color1[t] = interfaceColors[4][t] * amt;

			cgi_R_SetColor( color1 );

			// Draw the arrow centered on either side of the zoom art
			CG_DrawPic( 320 + start_x, 240 - 6, 16, 12, cgs.media.zoomArrowShader );
			CG_DrawPic( 320 - start_x, 240 - 6, -16, 12, cgs.media.zoomArrowShader );
		}
	}
	else
	{
		if ( cg.time - cg.zoomTime <= ZOOM_OUT_TIME )
		{
			amt = 1.0f - ( cg.time - cg.zoomTime ) / ZOOM_OUT_TIME;

			// Fade mask away
			for ( i = 0; i < 4; i++ ) {
				color1[i] = amt;
			}

			cgi_R_SetColor( color1 );
			CG_DrawPic( start_x, start_y, width, height, cgs.media.zoomMaskShader );
		}
	}
}

/*
================
CG_DrawTeleportEffects

================
*/ 

static void CG_DrawTeleportEffects( void ) 
{
	centity_t		*cent;

	cent = &cg_entities[0];

	if ( cg.snap->ps.clientNum != 0 || cg.renderingThirdPerson || !cent->gent )
	{
		return;
	}

	if ( cent->gent->s.powerups & ( 1 << PW_INVIS ) )
	{
		float	clr = ( cent->gent->client->ps.powerups[PW_INVIS] - 2000 - cg.time ) / 2000.0f;
		vec4_t	color;
		
		if ( clr < 0 )
			clr = 0;

		if ( clr > 1 )
			clr = 1;

		color[0] = 
		color[1] = 
		color[2] = clr;
		color[3] = 1.0f;

		cgi_R_SetColor( color );
		CG_DrawPic( 0, 0, 640, 480, cgs.media.playerTeleportShader );

		if ( cent->gent->client->ps.powerups[PW_INVIS] - 2000 < cg.time )
		{
			cent->gent->s.powerups &= ~( 1 << PW_INVIS );
			cent->gent->client->ps.powerups[PW_INVIS] = 0;
		}
	}
	else if ( cent->gent->s.powerups & ( 1 << PW_QUAD ) )
	{
		float	clr = ( cent->gent->client->ps.powerups[PW_QUAD] - 2000 - cg.time ) / 2000.0f;
		vec4_t	color;
		
		if ( clr < 0 )
			clr = 0;

		if ( clr > 1 )
			clr = 1;

		color[0] = 
		color[1] = 
		color[2] = clr;
		color[3] = 1.0f;

		cgi_R_SetColor( color );
		CG_DrawPic( 0, 0, 640, 480, cgs.media.playerTeleportShader );

		if ( cent->gent->client->ps.powerups[PW_QUAD] - 2000 < cg.time )
		{
			cent->gent->s.powerups &= ~( 1 << PW_QUAD );
			cent->gent->client->ps.powerups[PW_QUAD] = 0;
		}
	}
}

/*
================
CG_DrawStats

================
*/
static void CG_DrawStats( void ) 
{
	centity_t		*cent;
	playerState_t	*ps;
	vec3_t			angles;
//	vec3_t		origin;

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear( angles );

	// Do start
	if (!cg.interfaceStartupDone)
	{
		CG_InterfaceStartup();
	}

	CG_DrawArmor(cent);
	CG_DrawHealth(cent);
	CG_DrawAmmo(cent);

	CG_DrawTalk(cent);
}


/*
===================
CG_DrawPickupItem
===================
*/
static void CG_DrawPickupItem( void ) {
	int		value;
	float	*fadeColor;

	value = cg.itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );
//			cgi_R_SetColor( fadeColor );
//			CG_DrawPic( 8, 380, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
//			CG_DrawBigString( ICON_SIZE + 16, 398, bg_itemlist[ value ].pickup_name, fadeColor[0] );
//			CG_DrawProportionalString( ICON_SIZE + 16, 398, 
//				bg_itemlist[ value ].pickup_name, CG_SMALLFONT,fadeColor );
//			cgi_R_SetColor( NULL );
		}
	}
}

/*
===================
CG_DrawHoldableItem
===================
*/
/*
static void CG_DrawHoldableItem( void ) { 
	int		value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( 640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}

}
*/


/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void) {
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;

	if ( !cg_drawCrosshair.integer ) 
	{
		return;
	}

	if ( cg.renderingThirdPerson ) 
	{
		return;
	}

	// Don't bother drawing the crosshairs when we don't have a weapon
	if ( cg.snap->ps.weapon == WP_NONE )
	{
		return;
	}

	//NOTE: Maybe have crosshair turn red over enemies and green over allies?
	// set color based on health
	if ( cg_crosshairHealth.integer ) {
		vec4_t		hcolor;

		CG_ColorForHealth( hcolor );
		cgi_R_SetColor( hcolor );
	} else {
		//set color based on what kind of ent is under crosshair
		if ( g_crosshairEntNum >= ENTITYNUM_WORLD )
		{
			cgi_R_SetColor( NULL );
		}
		else
		{
			vec4_t	ecolor;
			gentity_t *crossEnt = &g_entities[g_crosshairEntNum];

			if ( crossEnt->client )
			{
				if ( crossEnt->client->playerTeam == TEAM_STARFLEET )
				{
					//Allies are green
					ecolor[0] = 0.0;//R
					ecolor[1] = 1.0;//G
					ecolor[2] = 0.0;//B
				}
				else
				{
					//Enemies are red
					ecolor[0] = 1.0;//R
					ecolor[1] = 0.0;//G
					ecolor[2] = 0.0;//B
				}
			}
			else
			{
				VectorCopy( crossEnt->startRGBA, ecolor );

				if ( !ecolor[0] && !ecolor[1] && !ecolor[2] )
				{
					// We really don't want black, so set it to yellow
					ecolor[0] = 0.9F;//R
					ecolor[1] = 0.7F;//G
					ecolor[2] = 0.0F;//B
				}
			}
			ecolor[3] = 1.0;

			cgi_R_SetColor( ecolor );
		}
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640( &x, &y, &w, &h );

	hShader = cgs.media.crosshairShader[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ];

	cgi_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader );
}

/*
qboolean CG_WorldCoordToScreenCoord(vec3_t worldCoord, int *x, int *y)

  Take any world coord and convert it to a 2D virtual 640x480 screen coord
*/
static qboolean CG_WorldCoordToScreenCoord(vec3_t worldCoord, int *x, int *y, qboolean clamp)
{
	int	xcenter, ycenter;
	vec3_t	local, transformed;

//	xcenter = cg.refdef.width / 2;//gives screen coords adjusted for resolution
//	ycenter = cg.refdef.height / 2;//gives screen coords adjusted for resolution
	
	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn

	VectorSubtract (worldCoord, cg.refdef.vieworg, local);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);		

	// Make sure Z is not negative.
	if(transformed[2] < 0.01)
	{
		if ( clamp )
		{
			transformed[2] = 0.01f;
		}
		else
		{
			return qfalse;
		}
	}
	// Simple convert to screen coords.
	float xzi = xcenter / transformed[2] * (90.0/cg.refdef.fov_x);
	float yzi = ycenter / transformed[2] * (90.0/cg.refdef.fov_y);

	*x = (int)(xcenter + xzi * transformed[0]);
	*y = (int)(ycenter - yzi * transformed[1]);

	return qtrue;
}

/*
=================
CG_LabelCrosshairEntity
=================
*/
static void CG_LabelViewEntity( gentity_t *crossEnt, char *name, qboolean scanAll, vec4_t color, qboolean drawHealth, char *rank, char *race, char *height, char *weight, char *weapon ) 
{//ID teammates, ID enemies, ID objectives, etc.
	vec3_t		center, maxs, mins, top, bottom, topLeft, topRight, bottomLeft, bottomRight;
	vec3_t		worldEast = {1.0f, 0, 0}, worldNorth = {0, 1.0f, 0}, worldUp = {0, 0, 1.0f};
	vec4_t		hcolor;
	int			x = 0, y = 0; 
	int			topLeftx, topLefty, topRightx, topRighty, bottomLeftx, bottomLefty, bottomRightx, bottomRighty;
	int			corner, topSize, bottomSize, leftSize, rightSize, health;
	int			charIndex, rankCharIndex,  raceCharIndex, htCharIndex, wtCharIndex, weapCharIndex;
	float		lineHorzLength = 8.0f, lineVertLength = 8.0f, lineWidth = 2.0f;
	float		fUpDot, fEastDot, fNorthDot, uNorthDot, uEastDot, hwidth;//, timedScale = 1.0f;
	qboolean	doTopLeft = qfalse;
	qboolean	doTopRight = qfalse;
	qboolean	doBottomLeft = qfalse;
	qboolean	doBottomRight = qfalse;
	qboolean	doSizes = qtrue;
	float		w;
	char		showName[1024];
	char		showRank[1024];
	char		showRace[1024];
	char		showHt[1024];
	char		showWt[1024];
	char		showWeap[1024];

	infoStringCount += cg.frametime;
	rankCharIndex = raceCharIndex = htCharIndex = wtCharIndex = weapCharIndex = charIndex = floor(infoStringCount/33);
	//TODO: have box scale in from corners of screen?  Or out from center?
	/*
	if(infoStringCount < 1000)
	{
		timedScale = (float)infoStringCount/100.0f;
		timedScale = 10.0f - timedScale;
		if(timedScale < 1.0f)
		{
			timedScale = 1.0f;
		}
	}
	*/
	//IDEA:  We COULD actually rotate a wire-mesh version of the crossEnt until it
	//			matches the crossEnt's angles then flash it and pop up this info...
	//			but that would be way too much work for something like this.
	//			Alternately, could rotate a scaled-down fully-skinned version
	//			next to it, but that, too, might be overkill... (plus, model would
	//			need back faces)

	//FIXME: can be optimized...

	//Draw frame around ent's bbox
	//FIXME: make global, do once
	fUpDot = 1.0f - fabs( DotProduct( vfwd_n, worldUp ) );//1.0 if looking up or down, so use mins and maxs more
	fEastDot = fabs( DotProduct( vfwd_n, worldEast ) );//1.0 if looking east or west, so use mins[1] and maxs[1] more
	fNorthDot = fabs( DotProduct( vfwd_n, worldNorth ) );//1.0 if looking north or south, so use mins[0] and maxs[0] more
	uEastDot = fabs( DotProduct( vup_n, worldEast ) );//1.0 if looking up or down, head towards east or west, so use mins[0] and maxs[0] more
	uNorthDot = fabs( DotProduct( vup_n, worldNorth ) );//1.0 if looking up or down, head towards north or south, so use mins[1] and maxs[1] more

	if ( crossEnt->s.solid == SOLID_BMODEL )
	{//brush model, no origin, so use the center
		VectorAdd( crossEnt->absmin, crossEnt->absmax, center );
		VectorScale( center, 0.5, center );
		VectorSubtract( crossEnt->absmax, center, maxs );
		VectorSubtract( crossEnt->absmin, center, mins );
	}
	else
	{
		VectorCopy( crossEnt->currentOrigin, center );
		VectorCopy( crossEnt->maxs, maxs );
		VectorCopy( crossEnt->mins, mins );
	}

	//NOTE: this presumes that mins[0] and maxs[0] are symmetrical and mins[1] and maxs[1] as well
	topSize = (maxs[2]*fUpDot + maxs[1]*uNorthDot + maxs[0]*uEastDot);//* timedScale
	bottomSize = (mins[2]*fUpDot + mins[1]*uNorthDot + mins[0]*uEastDot);//* timedScale
	leftSize = (fUpDot*(mins[0]*fNorthDot + mins[1]*fEastDot) + mins[0]*uNorthDot + mins[1]*uEastDot);//* timedScale
	rightSize = (fUpDot*(maxs[0]*fNorthDot + maxs[1]*fEastDot) + maxs[0]*uNorthDot + maxs[1]*uEastDot);//* timedScale

	//Find corners
	//top
	VectorMA( center, topSize, vup_n, top );
	//bottom
	VectorMA( center, bottomSize, vup_n, bottom );
	//Top-left frame
	VectorMA( top, leftSize, vright_n, topLeft );
	//Top-right frame
	VectorMA( top, rightSize, vright_n, topRight );
	//bottom-left frame
	VectorMA( bottom, leftSize, vright_n, bottomLeft );
	//bottom-right frame
	VectorMA( bottom, rightSize, vright_n, bottomRight );

	if ( CG_WorldCoordToScreenCoord( topLeft, &topLeftx, &topLefty, qfalse ) )
	{
		doTopLeft = qtrue;
	}
	else
	{
		doSizes = qfalse;
	}

	if ( CG_WorldCoordToScreenCoord( topRight, &topRightx, &topRighty, qfalse ) )
	{
		doTopRight = qtrue;
	}
	else
	{
		doSizes = qfalse;
	}

	if ( CG_WorldCoordToScreenCoord( bottomLeft, &bottomLeftx, &bottomLefty, qfalse ) )
	{
		doBottomLeft = qtrue;
	}
	else
	{
		doSizes = qfalse;
	}

	if ( CG_WorldCoordToScreenCoord( bottomRight, &bottomRightx, &bottomRighty, qfalse ) )
	{
		doBottomRight = qtrue;
	}
	else
	{
		doSizes = qfalse;
	}

	//NOTE: maybe print color-coded "Primary/Secondary Objective" on top if an objective?
	for ( corner = 0; corner < 11; corner++ )
	{//FIXME: make sure line length of 8 isn't greater than width of object
		switch ( corner )
		{
		case 0://top-left
			if ( doTopLeft )
			{
				if ( doSizes )
				{
				//Line lengths
					lineVertLength = (bottomLefty-topLefty)*0.25f;
					lineHorzLength = (topRightx-topLeftx)*0.25f;
				}
				CG_FillRect( topLeftx + 2, topLefty, lineHorzLength, lineWidth, color );
				CG_FillRect( topLeftx, topLefty, lineWidth, lineVertLength, color );
			}
			break;
		case 1://top-right
			if ( doTopRight )
			{
				if ( doSizes )
				{
				//Line lengths
					lineVertLength = (bottomRighty-topRighty)*0.25f;
					lineHorzLength = (topRightx-topLeftx)*0.25f;
				}
				CG_FillRect( topRightx-lineHorzLength, topRighty, lineHorzLength, lineWidth, color );
				CG_FillRect( topRightx, topRighty, lineWidth, lineVertLength, color );
			}
			break;
		case 2://bottom-left
			if ( doBottomLeft )
			{
				if ( doSizes )
				{
				//Line lengths
					lineVertLength = (bottomLefty-topLefty)*0.25f;
					lineHorzLength = (bottomRightx-bottomLeftx)*0.25f;
				}
				CG_FillRect( bottomLeftx, bottomLefty, lineHorzLength, lineWidth, color );
				CG_FillRect( bottomLeftx, bottomLefty-lineVertLength, lineWidth, lineVertLength, color );
			}
			break;
		case 3://bottom-right
			if ( doBottomRight )
			{
				if ( doSizes )
				{
				//Line lengths
					lineVertLength = (bottomRighty-topRighty)*0.25f;
					lineHorzLength = (bottomRightx-bottomLeftx)*0.25f;
				}
				CG_FillRect( bottomRightx-lineHorzLength, bottomRighty, lineHorzLength, lineWidth, color );
				CG_FillRect( bottomRightx, bottomRighty-lineVertLength, lineWidth, lineVertLength + 2, color );
			}
			break;
		case 4://healthBar
			if ( charIndex > 0 )
			{
				/*
				//tried to keep original functionality, but it would pop from top to bottom
				//when you let go of the button and had no way to tell then (during the
				//fade-out) whether it should be on top or bottom.  So now it is always on top.
				if ( !scanAll )
				{
					if ( !CG_WorldCoordToScreenCoord( bottom, &x, &y, qfalse ) )
					{//Can't draw bottom
						return;
					}
				}
				else
				*/
				{//try to draw at top as to not obscure the tricorder
					CG_WorldCoordToScreenCoord( top, &x, &y, qtrue );
					if ( y > 0.01 )
					{
						y -= SMALLCHAR_HEIGHT;
						if ( y > 0.01 )
						{
							if ( charIndex > 0 && name )
							{
								if ( y >= SMALLCHAR_HEIGHT )
								{
									y -= SMALLCHAR_HEIGHT;
								}
								else
								{
									y = 0.01;
								}
							}
							if ( y > 0.01 )
							{
								if ( rankCharIndex > 0 && rank )
								{
									if ( y >= SMALLCHAR_HEIGHT )
									{
										y -= SMALLCHAR_HEIGHT;
									}
								}
								if ( y > 0.01 )
								{
									if ( raceCharIndex > 0 && race )
									{
										if ( y >= SMALLCHAR_HEIGHT )
										{
											y -= SMALLCHAR_HEIGHT;
										}
									}
									if ( y > 0.01 )
									{
										if ( htCharIndex > 0 && height )
										{
											if ( y >= SMALLCHAR_HEIGHT )
											{
												y -= SMALLCHAR_HEIGHT;
											}
										}
										if ( y > 0.01 )
										{
											if ( wtCharIndex > 0 && weight )
											{
												if ( y >= SMALLCHAR_HEIGHT )
												{
													y -= SMALLCHAR_HEIGHT;
												}
											}
											if ( y > 0.01 )
											{
												if ( weapCharIndex > 0 && weapon )
												{
													if ( y >= SMALLCHAR_HEIGHT )
													{
														y -= SMALLCHAR_HEIGHT;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}

				if ( !drawHealth )
				{
					continue;
				}
				
				health = ceil( (float)crossEnt->health/(float)crossEnt->max_health*100.0f );
				CG_ColorForGivenHealth( hcolor, health );
				hwidth = (float)health*0.5f;

				y += lineWidth + 2;

				CG_FillRect( x - hwidth/2, y + lineWidth, hwidth, lineWidth*2, hcolor );

				y += lineWidth*2;
			}
			break;
		case 5://infoString (name/description)
			//Bright yellow
			VectorCopy( crossEnt->startRGBA, color );
			
			if ( !color[0] && !color[1] && !color[2] )
			{
				// We really don't want black, so set it to yellow
				color[0] = 0.9F;//R
				color[1] = 0.7F;//G
				color[2] = 0.0F;//B
			}
			color[3] = 0.75;
			if ( charIndex > 0 && name )
			{
				int	len = strlen(name);
				if ( charIndex > len+1 )
				{
					charIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showName, name, charIndex );
				w = CG_DrawStrlen( name ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showName, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		case 6://rank
			if ( rankCharIndex > 0 && rank )
			{
				int	len = strlen(rank);
				if ( rankCharIndex > len+1 )
				{
					rankCharIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showRank, rank, rankCharIndex );
				w = CG_DrawStrlen( rank ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showRank, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		case 7://race
			if ( raceCharIndex > 0 && race )
			{
				int	len = strlen(race);
				if ( raceCharIndex > len+1 )
				{
					raceCharIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showRace, race, raceCharIndex );
				w = CG_DrawStrlen( race ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showRace, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		case 8://height
			if ( htCharIndex > 0 && height )
			{
				int	len = strlen(height);
				if ( htCharIndex > len+1 )
				{
					htCharIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showHt, height, htCharIndex );
				w = CG_DrawStrlen( height ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showHt, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		case 9://weight
			if ( wtCharIndex > 0 && weight )
			{
				int	len = strlen(weight);
				if ( wtCharIndex > len+1 )
				{
					wtCharIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showWt, weight, wtCharIndex );
				w = CG_DrawStrlen( weight ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showWt, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		case 10://weapon
			if ( weapCharIndex > 0 && weapon )
			{
				int	len = strlen(weapon);
				if ( weapCharIndex > len+1 )
				{
					weapCharIndex = len+1;
				}
				else
				{
					cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.tedTextSound );
				}
				Q_strncpyz( showWeap, weapon, weapCharIndex );
				w = CG_DrawStrlen( weapon ) * SMALLCHAR_WIDTH;
				CG_DrawSmallStringColor( x - w / 2, y + lineWidth, showWeap, color );
				y += SMALLCHAR_HEIGHT;
			}
			break;
		}
	}
}

/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( qboolean scanAll ) 
{
	trace_t		trace;
	gentity_t	*traceEnt;
	vec3_t		start, end;
	int			content;

	//FIXME: debounce this to about 10fps?

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 4096, cg.refdef.viewaxis[0], end );//was 8192

	//YES!  This is very very bad... but it works!  James made me do it.  Really, he did.  Blame James.
	gi.trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, MASK_OPAQUE|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE );

	/*
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, MASK_PLAYERSOLID|CONTENTS_CORPSE|CONTENTS_ITEM );
	*/
	
	//FIXME: pick up corpses
	traceEnt = &g_entities[trace.entityNum];

	// if the object is "dead", don't show it
/*	if ( cg.crosshairClientNum && g_entities[cg.crosshairClientNum].health <= 0 )
	{
		cg.crosshairClientNum = 0;
		return;
	}
*/
	g_crosshairEntNum = trace.entityNum;
	g_crosshairEntDist = 4096*trace.fraction;
	if ( !traceEnt )
	{//not looking at anything
		g_crosshairSameEntTime = 0;
		g_crosshairEntTime = 0;
	}
	else
	{//looking at a valid ent
		//store the distance
		if ( trace.entityNum != g_crosshairEntNum )
		{//new crosshair ent
			g_crosshairSameEntTime = 0;
		}
		else if ( g_crosshairEntDist < 256 )
		{//close enough to start counting how long you've been looking
			g_crosshairSameEntTime += cg.frametime;
		}
		//remember the last time you looked at the person
		g_crosshairEntTime = cg.time;
	}

	if ( !traceEnt || !traceEnt->infoString || !traceEnt->infoString[0] || (traceEnt->s.eFlags & EF_NO_TED) )
	{
		if ( traceEnt && scanAll )
		{
		}
		else
		{
			return;
		}
	}

	// if the player is in fog, don't show it
	content = cgi_CM_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) 
	{
		return;
	}

	// if the player is invisible, don't show it
	if ( cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS ) ) 
	{
		return;
	}

	// update the fade timer
	if ( cg.crosshairClientNum != trace.entityNum )
	{
		infoStringCount = 0;
	}

	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}

static char *RaceNameForEnum( int race )  {
	switch(race)
	{
	case RACE_HUMAN:
		return ingame_text[IGT_HUMAN];
		break;
	case RACE_BORG:
		return ingame_text[IGT_BORG];
		break;
	case RACE_PARASITE:
		return ingame_text[IGT_FERROVORE];
		break;
	case RACE_KLINGON:
		return ingame_text[IGT_KLINGON];
		break;
	case RACE_MALON:
		return ingame_text[IGT_MALON];
		break;
	case RACE_HIROGEN:
		return ingame_text[IGT_HIROGEN];
		break;
	case RACE_STASIS:
		return ingame_text[IGT_ETHERIAN];
		break;
	case RACE_8472:
		return ingame_text[IGT_SPECIES8472];
		break;
	case RACE_BOT:
		return ingame_text[IGT_DREADBOT];
		break;
	case RACE_HARVESTER:
		return ingame_text[IGT_HARVESTER];
		break;
	case RACE_REAVER:
		return ingame_text[IGT_REAVER];
		break;
	case RACE_AVATAR:
		return ingame_text[IGT_AVATAR];
		break;
	case RACE_VULCAN:
		return ingame_text[IGT_VULCAN];
		break;
	case RACE_BETAZOID:
		return ingame_text[IGT_BETAZOID];
		break;
	case RACE_BOLIAN:
		return ingame_text[IGT_BOLIAN];
		break;
	case RACE_TALAXIAN:
		return ingame_text[IGT_TALAXIAN];
		break;
	case RACE_BAJORAN:
		return ingame_text[IGT_BAJORAN];
		break;
	case RACE_HOLOGRAM:
		return ingame_text[IGT_PHOTONIC];
		break;
	default:
	case RACE_NONE:
		return ingame_text[IGT_UNKNOWN];
		break;
	}
}

static char *RankNameForEnum( int rank ) {
	switch ( rank )
	{
	default:
	case RANK_CIVILIAN:
		return ingame_text[IGT_CIVILIAN];
		break;
	case RANK_CREWMAN:
		return ingame_text[IGT_CREWMAN];
		break;
	case RANK_ENSIGN:
		return ingame_text[IGT_ENSIGN]; 
		break;
	case RANK_LT_JG:
		return ingame_text[IGT_LTJG]; 
		break;
	case RANK_LT:
		return ingame_text[IGT_LT]; 
		break;
	case RANK_LT_COMM:
		return ingame_text[IGT_LTCOMMANDER]; 
		break;
	case RANK_COMMANDER:
		return ingame_text[IGT_COMMANDER]; 
		break;
	case RANK_CAPTAIN:
		return ingame_text[IGT_CAPTAIN]; 
		break;
	}
}
	/*
=====================
CG_DrawCrosshairNames
=====================
*/
extern void CG_ColorForGivenHealth( vec4_t hcolor, int health );
static void CG_DrawCrosshairNames( void ) {
	float		*color;
	gentity_t	*crossEnt;
	char		*string;
	int			textI;
	qboolean	scanAll = qfalse;
	centity_t	*player = &cg_entities[0];

	if ( (!cg_drawCrosshairNames.integer&&player->gent->client->ps.weapon!=WP_TRICORDER) || cg.renderingThirdPerson ) //!cg_drawCrosshair.integer still need infoStrings if no crosshair
	{
		infoStringCount = 0;
		return;
	}

	if ( !player->gent )
	{
		return;
	}

	if ( !player->gent->client )
	{
		return;
	}

	if ( player->gent->client->ps.weapon == WP_TRICORDER )
	{
		if ( player->gent->client->buttons & BUTTON_ATTACK )
		{
			scanAll = qtrue;
		}
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity( scanAll );

	// draw the name of the player being looked at
	//FIXME: make it fade out even if still looking at it
	//FIXME: if first time, play sound
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) 
	{
		cgi_R_SetColor( NULL );
		infoStringCount = 0;
		return;
	}

	if(cg.crosshairClientNum > 0 && cg.crosshairClientNum < ENTITYNUM_WORLD)
	{
		crossEnt = &g_entities[cg.crosshairClientNum];
		//if(crossEnt && (crossEnt->client || (crossEnt->svFlags & SVF_OBJECTIVE)) && crossEnt->targetname)
		if ( crossEnt )
		{
			if ( crossEnt->client )
			{
				if ( player->gent->client->playerTeam == crossEnt->client->playerTeam )
				{
					//Allies are green
					color[0] = 0.0;//R
					color[1] = 1.0;//G fa
					color[2] = 0.0;//B
				}
				else
				{
					//Enemies are red
					color[0] = 1.0;//R
					color[1] = 0.0;//G
					color[2] = 0.0;//B
				}
			}
			else
			{
				VectorCopy( crossEnt->startRGBA, color );

				if ( !color[0] && !color[1] && !color[2] )
				{
					// We really don't want black, so set it to yellow
					color[0] = 0.9F;//R
					color[1] = 0.7F;//G
					color[2] = 0.0F;//B
				}
			}
			color[3] *= 0.5;

			if ( !scanAll && crossEnt->infoString && crossEnt->infoString[0] )
			{//Just drawing infoString

				if (crossEnt->infoString[0] == '@')
				{
					textI = CG_SearchTextPrecache(crossEnt->infoString);
					if (textI >= 0)
					{
						string = precacheText[textI].text;
					}
					else
					{
						string = crossEnt->infoString;
					}

				}
				else
				{
					string = crossEnt->infoString;
				}

				CG_LabelViewEntity( crossEnt, string, scanAll, color, qfalse, NULL, NULL, NULL, NULL, NULL );
			}
			else
			{//Drawing full info
				char	*name = NULL;
				char	*rank = NULL;
				char	*race = NULL;
				vec3_t	size;
				float	ht = 0;
				float	wt = 0;
				char	*weap = NULL;
				char	namestr[128];
				char	rankstr[128];
				char	racestr[128];
				char	htstr[128];
				char	wtstr[128];
				char	weapstr[128];
				qboolean showSize = qfalse;

				if ( ( crossEnt->infoString && crossEnt->infoString[0] ) || scanAll )
				{
					showSize = qtrue;
				}

				if ( showSize )
				{
					vec3_t	maxs, mins;

					VectorCopy( crossEnt->maxs, maxs );
					VectorCopy( crossEnt->mins, mins );
					if ( crossEnt->client && crossEnt->NPC )
					{//only use the standing height of the NPCs because people can't understand the complex dynamics of height in weight in a ceiling-installed anti-gravitic plating environment
						maxs[2] = crossEnt->client->standheight;
					}
					VectorSubtract(maxs, mins, size);
					ht = (maxs[2] - mins[2]) * 3.46875;//magic number
					wt = VectorLength(size)*1.4;//magic number
					if ( crossEnt->client && crossEnt->NPC )
					{
						if ( strstr( crossEnt->client->renderInfo.legsModelName, "female" ) ||
							strstr( crossEnt->client->renderInfo.legsModelName, "seven" ) )
						{//crewfemale, hazardfemale or seven of nine
							wt *= 0.73f;//magic number, women are lighter than men
						}
					}
				}

				if ( crossEnt->client && crossEnt->NPC )
				{
					race = RaceNameForEnum( crossEnt->client->race );
					sprintf( racestr, "%s: %s",ingame_text[IGT_RACE], race );
					
					if ( crossEnt->client->playerTeam == TEAM_STARFLEET )
					{
						if ( crossEnt->fullName && crossEnt->fullName[0] )
						{
							name = crossEnt->fullName;
						}
						else
						{
							name = ingame_text[IGT_DATANOTAVAILABLE];//crossEnt->targetname;
						}
						sprintf( namestr, "%s: %s", ingame_text[IGT_NAME],name );

						rank = RankNameForEnum( crossEnt->NPC->rank );
						sprintf( rankstr, "%s: %s",ingame_text[IGT_RANK], rank );
					}
					else if ( crossEnt->fullName && crossEnt->fullName[0] )
					{
						name = crossEnt->fullName;
						strcpy( namestr, name );
					}
					else
					{
						name = ingame_text[IGT_UNKNOWNENTITY];
						strcpy( namestr, name );
					}

					if ( crossEnt->client && showSize )
					{
						ht *= (float)(crossEnt->client->renderInfo.scaleXYZ[2])/100.0f;
						wt *= (float)(crossEnt->client->renderInfo.scaleXYZ[0]+crossEnt->client->renderInfo.scaleXYZ[1]+crossEnt->client->renderInfo.scaleXYZ[2])/300.0f;
					}

					if ( cg_weapons[ crossEnt->client->ps.weapon ].item ) 
					{
						weap = cg_weapons[ crossEnt->client->ps.weapon ].item->pickup_name;
						sprintf( weapstr, "%s: %s", ingame_text[IGT_WEAPON],weap );
					}
				}
				else 
				{
					if ( crossEnt->fullName && crossEnt->fullName[0] )
					{
						name = crossEnt->fullName;
						strcpy( namestr, name );
					}
					else if ( crossEnt->infoString && crossEnt->infoString[0] )
					{
						name = crossEnt->infoString;
						strcpy( namestr, name );
					}
					else if ( scanAll )
					{
						name = ingame_text[IGT_UNKNOWNOBJECT];
						strcpy( namestr, name );
					}
					//also, Federation uses very lightweight materials, so fudge the weight of non-living things
					wt /= 10.0f;
				}

				if ( showSize )
				{
					sprintf( htstr, "%s: %4.2f %s",ingame_text[IGT_HT], ht,ingame_text[IGT_CM] );
					sprintf( wtstr, "%s: %4.2f %s",ingame_text[IGT_WT],wt,ingame_text[IGT_KG] );
				}

				CG_LabelViewEntity( crossEnt,
						name ? namestr : NULL,
						scanAll,
						color, qtrue,
						rank ? rankstr : NULL,
						race ? racestr : NULL,
						ht ? htstr : NULL,
						wt ? wtstr : NULL,
						weap ? weapstr : NULL );
			}
		}
		else
		{
			infoStringCount = 0;
		}
	}

	cgi_R_SetColor( NULL );
}

/*
void CG_DrawTED (void)

  Finds any NPCs (lifesigns) in PVS within 1024, draws them as an arrow
  indicating their direction
*/
static	float	TEDrange = 1024.0f;
#define	MIN_TED_RANGE	128.0f
#define	MAX_TED_RANGE	1024.0f
#define TED_ZOOM_SPEED	8.0f
#define	RADAR_ACTIVATE_TIME	500
#define RADAR_DEBOUNCE_TIME	200
static void CG_DrawTED (void) {
	int			num;
	float		dist, x, y, xdiff, ydiff, xofs, yofs, w, h, angle, pangle;
	float		sin0, sin90, sin270;
	float		TEDsize = 64.0f;//Assumes graphic is 64x64
	float		TEDscale;
	float		scale;
	vec3_t		vec;
	centity_t	*cent;
	centity_t	*player = &cg_entities[0];
	static	qboolean	radarOn = qfalse;
	static	int			radarToggleDebounceTime = 0;

	if ( !player->gent )
	{
		return;
	}

	if ( !player->gent->client )
	{
		return;
	}

	if ( player->gent->client->ps.weapon != WP_TRICORDER )
	{//switched away from Tricorder
		if ( radarOn )
		{//turn it off
			radarOn = qfalse;
			radarToggleDebounceTime = cg.time + RADAR_DEBOUNCE_TIME;
			cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.triRadarSound );
		}
	}
	else if ( player->gent->client->buttons & BUTTON_ALT_ATTACK )
	{//holding alt-fire and have tricorder in-hand
		if ( radarToggleDebounceTime < cg.time )
		{//toggle tricorder on & off
			radarOn = !radarOn;
			radarToggleDebounceTime = cg.time + RADAR_DEBOUNCE_TIME;
			cgi_S_StartSound( NULL, 0, CHAN_ITEM, cgs.media.triRadarSound );
		}
	}

	if ( !radarOn && radarToggleDebounceTime < cg.time - RADAR_ACTIVATE_TIME )
	{//has been off for more than half a second, scaled down by now, stop drawing it
		return;
	}

	//Draw the display
	scale = ((float)(cg.time - radarToggleDebounceTime + RADAR_DEBOUNCE_TIME))/((float)(RADAR_ACTIVATE_TIME));//scale up over 500 ms
	if ( scale > 1.0 )
	{
		scale = 1.0;
	}
	if ( !radarOn )
	{
		scale = 1.0 - scale;
	}
	TEDsize *= scale;
	TEDscale = TEDsize/2.0f/TEDrange;

	x = 600;
	y = 380;
	w = h = TEDsize;

	cgi_R_SetColor( NULL );

	CG_DrawPic( x - (0.5 * w), y - (0.5 * h), w, h, cgs.media.TEDshader );

	//Draw us
	w = h = 8*scale;

	pangle = player->gent->client->ps.viewangles[1];
	sin0 = sin(DEG2RAD(pangle));
	sin90 = sin(DEG2RAD(pangle+90));
	sin270 = sin(DEG2RAD(pangle+270));

	CG_DrawPic( x-(0.5*w), y-(0.5*h), w, h, cgs.media.pICONshader );

	//Draw them
	w = h = 4*scale;
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) 
	{
		//These should all be in PVS
		if ( cg.snap->entities[ num ].number == 0)
		{//We draw ourselves
			continue;
		}

		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		if ( !cent->gent )
		{
			continue;
		}

		if ( !cent->gent->client )
		{
			continue;
		}

		if ( !cent->gent->NPC )
		{
			continue;
		}

		VectorSubtract( cent->lerpOrigin, player->lerpOrigin, vec );
		dist = VectorLengthSquared( vec );

		if ( dist > TEDrange*TEDrange )
		{
			continue;
		}

		//Ok, there is an NPC in our PVS and within 1024 of our position

		//Should be relative to player's facing, so as he turns, the
		//ents move in the display
		angle = AngleNormalize360( (cent->gent->client->ps.viewangles[1] - pangle - 135) * -1);

		xdiff = (cent->lerpOrigin[0] - player->lerpOrigin[0]);
		ydiff = (player->lerpOrigin[1] - cent->lerpOrigin[1]);
		
		xofs = xdiff * sin0 + ydiff * sin90;
		yofs = ydiff * sin0 + xdiff * sin270;
		xofs *= TEDscale;
		yofs *= TEDscale;

		//FIXME: the angles backwards if facing along the y axis?!
		if ( cent->gent->client->playerTeam != player->gent->client->playerTeam )
		{
			CG_DrawRotatePic( x+xofs, y+yofs, w, h, angle, cgs.media.eICONshader );
		}
		else
		{
			CG_DrawRotatePic( x+xofs, y+yofs, w, h, angle, cgs.media.aICONshader );
		}
	}
	cgi_R_SetColor( NULL );
}

//console functions to change zoom range on TED
//used from cg_consolecommands
void CG_RangeIncrease_f (void){
	if ( TEDrange < MAX_TED_RANGE - TED_ZOOM_SPEED ){
		TEDrange += TED_ZOOM_SPEED;
	} else {
		TEDrange = MAX_TED_RANGE;
	}
}

void CG_RangeDecrease_f (void){
	if ( TEDrange > MIN_TED_RANGE + TED_ZOOM_SPEED ){
		TEDrange -= TED_ZOOM_SPEED;
	} else {
		TEDrange = MIN_TED_RANGE;
	}
}
//==============================================================================


/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
	CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {
	float		x;
	vec4_t		color;
	const char	*name;

	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( name ) );

	CG_DrawStringExt( x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT );

	return qtrue;
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_ProportionalStringWidth(s,UI_BIGFONT);
	CG_DrawProportionalString(635 - w, y + 2, s, UI_BIGFONT, colorTable[CT_LTGOLD1]);

	return y + BIGCHAR_HEIGHT + 10;
}


/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = cgi_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );
		w = CG_ProportionalStringWidth(s,UI_BIGFONT);
		CG_DrawProportionalString(635 - w, y+2, s, UI_BIGFONT, colorTable[CT_LTGOLD1]);
	}

	return y + BIGCHAR_HEIGHT + 10;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char		*s;
	int			w;
	int			mins, seconds, tens;

	seconds = cg.time / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );

	w = CG_ProportionalStringWidth(s,UI_BIGFONT);
	CG_DrawProportionalString(635 - w, y + 2, s, UI_BIGFONT, colorTable[CT_LTGOLD1]);

	return y + BIGCHAR_HEIGHT + 10;
}


/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	const char	*s;
	int			w;

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.lowAmmoWarning ) {
		return;
	}

	// Don't bother drawing the ammo warning when have no weapon selected
	if ( cg.snap->ps.weapon == WP_NONE )
	{
		return;
	}

	if ( weaponData[cg.snap->ps.weapon].ammoIndex == AMMO_NONE )
	{//doesn't use ammo, so no warning
		return;
	}

	if ( cg.lowAmmoWarning == 2 ) {
		s = ingame_text[IGT_INSUFFICIENTENERGY];
	} else {
		return;
		//s = "LOW AMMO WARNING";
	}

	w = CG_ProportionalStringWidth(s,CG_SMALLFONT);
	CG_DrawProportionalString(320 - w / 2, 64, s, CG_SMALLFONT,colorTable[CT_LTGOLD1] );

}

void CG_DrawAssimilation( void )
{
	static vec4_t	color;

	if ( g_entities[0].s.eFlags & EF_ASSIMILATED )
	{//throw the assimilation shader on view
		for ( int i = 0; i < 4; i++ )
		{
			color[i] += 0.001f;
			if ( color[i] > 3.0f )
			{
				color[i] = 3.0f;
			}
		}
		if ( color[3] > 0.5f )
		{
			color[3] = 0.5f;
		}
		cgi_R_SetColor( color );
		//I HATE SHADERS!!  Why can't I just draw circuitry creeping across the view?  Crap-ass system
		cgi_R_DrawStretchPic( cg.refdef.x, cg.refdef.y, cg.refdef.width, cg.refdef.height, 0, 0, 1, 1, cgi_R_RegisterShader("gfx/effects/assimilation") );
		cgi_R_SetColor( NULL );
	}
	else
	{
		color[0] = color[1] = color[2] = color[3] = 0.0f;
	}
}

/*
======================
CG_DrawOverbounceInfo
======================
*/
static void CG_DrawOverbounceInfo( void ) {
	gentity_t const *const player_gent = cg_entities[cg.snap->ps.clientNum].gent;
	playerState_t const &player_state = player_gent->client->ps;
	if ( player_state.groundEntityNum == ENTITYNUM_NONE ) {
		return;
	}

	vec3_t start;
	vec3_t end;
	trace_t trace;
	constexpr float TRACE_LENGTH = 4096.0f;
	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, TRACE_LENGTH, cg.refdef.viewaxis[0], end );
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_PLAYERSOLID );
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		return;
	}

	double const height_difference = player_gent->currentOrigin[2] +
	                                 player_gent->mins[2] - trace.endpos[2];
	if ( height_difference <= 0.0 ) {
		return;
	}

	if ( !playerOverbouncePredictor ) {
		playerOverbouncePredictor = std::make_unique<OverbouncePrediction>();
		playerOverbouncePredictor->start();
	}
	playerOverbouncePredictor->setParameters( height_difference, player_state.gravity, JUMP_VELOCITY );

	double const go_overbounce_probability = playerOverbouncePredictor->getProbabilityForGo();
	double const go_overbounce_percentage = std::round( go_overbounce_probability * 100.0 );
	if ( go_overbounce_probability > 0.0 ) {
		CG_DrawProportionalString( 10, 235, va( "G: %.0f%%", go_overbounce_percentage ),
		                           UI_BIGFONT, colorTable[CT_LTGOLD1] );
	}
	double const jump_overbounce_probability = playerOverbouncePredictor->getProbabilityForJump();
	double const jump_overbounce_percentage = std::round( jump_overbounce_probability * 100.0 );
	if ( jump_overbounce_probability > 0.0 ) {
		CG_DrawProportionalString( 10, 265, va( "J: %.0f%%", jump_overbounce_percentage ),
		                           UI_BIGFONT, colorTable[CT_LTGOLD1] );
	}
}

/*
====================
CG_DrawStrafeHelper
====================
*/
static void CG_DrawStrafeHelper( void ) {
	StrafeHelper::setScale(cg_strafeHelperScale.value);
	StrafeHelper::draw(cg_strafeHelperCenter.integer, cg_strafeHelperCenterMarker.integer,
	                   cg_strafeHelperHeight.value, cg_strafeHelperY.value);
}

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void ) 
{
	centity_t *cent;

	cent = &cg_entities[cg.snap->ps.clientNum];

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) 
	{
		return;
	}

	if ( cg_draw2D.integer == 0 ) 
	{
		return;
	}

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) 
	{
		CG_DrawIntermission();
		return;
	}

	CGCam_DrawWideScreen();

	// Draw this before the text so that any text won't get clipped off
	if ( !cg.renderingThirdPerson && !in_camera )
	{
		CG_DrawZoomMask();
	}

	CG_DrawScrollText();

	CG_DrawCaptionText();
 
	CG_DrawGameText();

	CG_DrawLCARSText();

	if ( in_camera )
		return;
	// don't draw any status if dead
	if ( cg.snap->ps.stats[STAT_HEALTH] > 0 ) 
	{
		if ( cg_drawStrafeHelper.integer )
		{
			CG_DrawStrafeHelper();
		}

		CG_DrawTeleportEffects();
		CG_DrawStats();
		CG_DrawAmmoWarning();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
		CG_DrawTED();
		CG_DrawWeaponSelect();
		CG_DrawPickupItem();
		CG_DrawAssimilation();
	}

	float y = 0;
	if (cg_drawSnapshot.integer) {
		y=CG_DrawSnapshot(y);
	} 
	if (cg_drawFPS.integer) {
		y=CG_DrawFPS(y);
	} 
	if (cg_drawTimer.integer) {
		y=CG_DrawTimer(y);
	}

	CG_DrawFollow();
	
	if ( cg_drawOverbounceInfo.integer )
	{
		CG_DrawOverbounceInfo();
	}

	// don't draw center string if scoreboard is up
	if ( !CG_DrawScoreboard() ) {
		CG_DrawCenterString();
	}

	if (cg.showInformation)
	{
		CG_DrawMissionInformation();
	}
	else if (missionInfo_Updated)
	{	
		if (cg.predicted_player_state.pm_type != PM_DEAD)
		{
			if (!cg.missionInfoFlashTime)
			{
				cg.missionInfoFlashTime	= cg.time + cg_missionInfoFlashTime.integer;
			}

			if (cg.missionInfoFlashTime < cg.time)	// Time's up.  They didn't read it.
			{
				cg.missionInfoFlashTime = 0;
				missionInfo_Updated = qfalse;
			}

			if (cg_virtualVoyager.value == 0)
			{

				CG_DrawProportionalString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2), ingame_text[IGT_MISSIONINFO_UPDATED],
					CG_PULSE | CG_CENTER| CG_SMALLFONT, colorTable[CT_LTRED1] );

		//		if (cent->gent->client->sess.missionObjectivesShown<3)
		//		{
					CG_DrawProportionalString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2) + 20, ingame_text[IGT_MISSIONINFO_UPDATED2],
						CG_PULSE | CG_CENTER| CG_SMALLFONT, colorTable[CT_LTRED1] );
		//		}
			}
			else
			{

				CG_DrawProportionalString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2), ingame_text[IGT_SECURITYINFO_UPDATED],
					CG_PULSE | CG_CENTER| CG_SMALLFONT, colorTable[CT_LTRED1] );

				CG_DrawProportionalString((SCREEN_WIDTH/2), (SCREEN_HEIGHT/2) + 20, ingame_text[IGT_SECURITYINFO_UPDATED2],
					CG_PULSE | CG_CENTER| CG_SMALLFONT, colorTable[CT_LTRED1] );
			}

		}
	}
}


/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	//FIXME: these globals done once at start of frame for various funcs
	AngleVectors (cg.refdefViewAngles, vfwd, vright, vup);
	VectorCopy( vfwd, vfwd_n );
	VectorCopy( vright, vright_n );
	VectorCopy( vup, vup_n );
	VectorNormalize( vfwd_n );
	VectorNormalize( vright_n );
	VectorNormalize( vup_n );

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	// draw 3D view
	cgi_R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

	// draw status bar and other floating elements
	CG_Draw2D();

}

