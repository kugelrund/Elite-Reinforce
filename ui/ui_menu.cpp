#include "ui_local.h"

void UI_VirtualSaveMenu(int from);

qboolean	holoMatch;


// Data for Quit Menu
static struct 
{
	menuframework_s menu;

	menutext_s		voyager_label;
	menubitmap_s	voyager_pic;
	menutext_s		thrusters_label;
	menubitmap_s	thrusters_pic;
	menutext_s		nacelles_label;
	menubitmap_s	nacelles_pic;
	menutext_s		bussard_label;
	menubitmap_s	bussard_pic;
	menutext_s		midhull_label;
	menubitmap_s	midhull_pic;
	menutext_s		ventral_label;
	menubitmap_s	ventral_pic;
	menutext_s		torpedo_label;
	menubitmap_s	torpedo_pic;
	menutext_s		phaser_label;
	menubitmap_s	phaser_pic;
	menutext_s		bridge_label;
	menubitmap_s	bridge_pic;
	menutext_s		raven_label;

	qhandle_t	corner_ur;

	qhandle_t	swoop1;
	qhandle_t	swoop2;

	sfxHandle_t	pingsound;
} s_quitmenu;

static struct 
{
	menuframework_s menu;
	qhandle_t	screen[6];
	int			screenI;
} s_demoend_menu;

static struct 
{
	menuframework_s menu;
	qhandle_t	bracket;
	qhandle_t	bird;
	sfxHandle_t	borgsound;
	int				raventimer;
} s_raven;

// Data for LCARS out Menu
static struct 
{
	qhandle_t	federation;
} s_LCARSoutmenu;

// Data for LCARS in Menu
static struct 
{
	qhandle_t	federation;
} s_LCARSinmenu;


// Precache stuff for Main Menu
static struct 
{

} mainmenu_cache;

// Precache stuff for Ingame Main Menu
static struct 
{
	qhandle_t suit;
} s_ingamemenu_cache;

// Data for Ingame Menu
static struct 
{
	menutext_s		powerconv_label;
	menubitmap_s	powerconv_pic;
	menutext_s		commbadge_label;
	menubitmap_s	commbadge_pic;
	menutext_s		logistics_label;
	menubitmap_s	logistics_pic;
	menutext_s		energypack_label;
	menubitmap_s	energypack_pic;
	menutext_s		wavegenerator_label;
	menubitmap_s	wavegenerator_pic;
	menutext_s		scanners_label;
	menubitmap_s	scanners_pic;
	menutext_s		pouches_label;
	menubitmap_s	pouches_pic;
	menutext_s		buffer_label;
	menubitmap_s	buffer_pic;
} s_ingamemenu;

// ===========================================
// LCARS In Data
// ===========================================
void UI_LCARSIn_Menu(void);
menuframework_s s_lcarsin_menu;
static sfxHandle_t	welcomesnd;

// ===========================================
// LCARS Out Data
// ===========================================
void UI_LCARSOut_Menu(void);
menuframework_s s_lcarsout_menu;
static sfxHandle_t	logoutsnd;


int			ingameFlag;	// true when in game menu is in use

qhandle_t			quadrants;
qhandle_t			buttonbar1;
qhandle_t			directoryUpperCorner;
qhandle_t			directoryLowerCorner;
static sfxHandle_t	backgroundsnd;

menubitmap_s	s_quitgame;
menubitmap_s	s_returntogame;
menubitmap_s	s_ingame_setup;
menubitmap_s	s_ingame_save;
menubitmap_s	s_ingame_load;
menubitmap_s	s_screenshot;
menubitmap_s	s_leavegame;
menubitmap_s	s_quitprog;
menubitmap_s	s_tourmode;
menubitmap_s	s_mods;


static menubitmap_s	s_federation_label;
static menubitmap_s	s_cardassian_label;
static menubitmap_s	s_ferengi_label;
static menubitmap_s	s_romulan_label;
static menubitmap_s	s_klingon_label;
static menubitmap_s	s_voyager_label;
static menubitmap_s	s_borg_label;
static menubitmap_s	s_wormhole_label;
static menubitmap_s	s_dominion_label;
static menubitmap_s	s_core_label;
static menubitmap_s	s_alphaquad_label;
static menubitmap_s	s_betaquad_label;
static menubitmap_s	s_deltaquad_label;
static menubitmap_s	s_gammaquad_label;
qboolean	loadModelInitialized;
static float federationTimer;

void M_Main_Event (void* ptr, int notification);
void QuitMenuInterrupt(int labelId);
void UI_CrewMenu(void);
static void Raven_MenuDraw(void);
static void Quit_MenuDraw( void );
static void UI_RavenMenu( void );

typedef enum 
{
	MMG_ARROW1,
	MMG_ARROW2,
	MMG_ARROW3,
	MMG_ARROW4,
	MMG_GRAPH_BAR1,
	MMG_GRAPH_BAR2,
	MMG_GRAPH_BAR3,
	MMG_GRAPH_BAR4,

	MMG_SECTIONS_BEGIN,

	MMG_FEDERATION,
	MMG_CARDASSIA,
	MMG_FERENGINAR,
	MMG_ROMULANEMPIRE,
	MMG_KLINGONEMPIRE,
	MMG_DOMINION,
	MMG_VOYAGER,
	MMG_BORG,
	MMG_WORMHOLE,
	MMG_GALACTICCORE,
	MMG_ALPHAQUAD,
	MMG_BETAQUAD,
	MMG_DELTAQUAD,
	MMG_GAMMAQUAD,
	MMG_SECTIONS_END,

	MMG_SYMBOLS_BEGIN,
	MMG_FEDERATION_SYM,
	MMG_CARDASSIA_SYM,
	MMG_FERENGI_SYM,
	MMG_ROMULAN_SYM,
	MMG_KLINGON_SYM,
	MMG_DOMINION_SYM,
	MMG_BORG_SYM,
	MMG_SYMBOLS_END,

	MMG_SECTION_TEXT_BEGIN,
	MMG_FEDERATION_TXT,
	MMG_CARDASSIA_TXT,
	MMG_FERENGI_TXT,
	MMG_ROMULAN_TXT,
	MMG_KLINGON_TXT,
	MMG_DOMINION_TXT,	
	MMG_VOYAGER_TXT,
	MMG_BORG_TXT,
	MMG_WORMHOLE_TXT,
	MMG_GALACTICCORE_TXT,
	MMG_ALPHAQUAD_TXT,
	MMG_BETAQUAD_TXT,
	MMG_DELTAQUAD_TXT,
	MMG_GAMAQUAD_TXT,
	MMG_SECTION_TEXT_END,
	
	MMG_RH_COL1_SPECNUM1,

	MMG_RH_COL1_NUM1,
	MMG_RH_COL1_NUM2,
	MMG_RH_COL1_NUM3,
	MMG_RH_COL1_NUM4,
	MMG_RH_COL1_NUM5,
	MMG_RH_COL1_NUM6,

	MMG_RH_COL2_SPECNUM1,

	MMG_RH_COL2_NUM1,
	MMG_RH_COL2_NUM2,
	MMG_RH_COL2_NUM3,
	MMG_RH_COL2_NUM4,
	MMG_RH_COL2_NUM5,
	MMG_RH_COL2_NUM6,

	MMG_RH_COL3_SPECNUM1,

	MMG_RH_COL3_NUM1,
	MMG_RH_COL3_NUM2,
	MMG_RH_COL3_NUM3,

	MMG_RH_COL4_SPECNUM1,

	MMG_RH_COL4_NUM1,
	MMG_RH_COL4_NUM2,
	MMG_RH_COL4_NUM3,

/*	MMG_TOP_NUMBERS,
	MMG_SPEC_TOP_ROW,
	MMG_TOP_COL1_NUM1,
	MMG_TOP_COL1_NUM2,
	MMG_TOP_COL1_NUM3,
	MMG_TOP_COL1_NUM4,
	MMG_TOP_COL1_NUM5,

	MMG_TOP_COL2_NUM1,
	MMG_TOP_COL2_NUM2,
	MMG_TOP_COL2_NUM3,
	MMG_TOP_COL2_NUM4,
	MMG_TOP_COL2_NUM5,

	MMG_TOP_COL3_NUM1,
	MMG_TOP_COL3_NUM2,
	MMG_TOP_COL3_NUM3,
	MMG_TOP_COL3_NUM4,
	MMG_TOP_COL3_NUM5,

	MMG_TOP_COL4_NUM1,
	MMG_TOP_COL4_NUM2,
	MMG_TOP_COL4_NUM3,
	MMG_TOP_COL4_NUM4,
	MMG_TOP_COL4_NUM5,

	MMG_TOP_COL5_NUM1,
	MMG_TOP_COL5_NUM2,
	MMG_TOP_COL5_NUM3,
	MMG_TOP_COL5_NUM4,
	MMG_TOP_COL5_NUM5,
*/
	MMG_MAX
} mainmenu_graphics_t;

menugraphics_s mainmenu_graphics[MMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text						graphic,	min		max	target	inc		style	color
	MG_GRAPHIC,	0.0,	116,	258,	8,		8,		"menu/common/left_arrow.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_LTPURPLE2,	NULL,	// MMG_ARROW1
	MG_GRAPHIC,	0.0,	116,	338,	8,		8,		"menu/common/left_arrow.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_LTPURPLE2,	NULL,	// MMG_ARROW2
	MG_GRAPHIC,	0.0,	481,	224,	8,		8,		"menu/common/right_arrow.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_LTPURPLE2,	NULL,	// MMG_ARROW3
	MG_GRAPHIC,	0.0,	481,	370,	8,		8,		"menu/common/right_arrow.tga",0,0,	NULL,		0,		0,	0,		1,		0,		CT_LTPURPLE2,	NULL,	// MMG_ARROW4		

	MG_GRAPHIC,	0.0,	108,	218,	32,		64,		"menu/common/mon_bar.tga",	0,0,	NULL,		0,		0,	0,		0,		0,							CT_MDGREY,	NULL,	// MMG_GRAPH_BAR1
	MG_GRAPHIC,	0.0,	108,	318,	32,		64,		"menu/common/mon_bar.tga",	0,0,	NULL,		0,		0,	0,		0,		0,							CT_MDGREY,	NULL,	// MMG_GRAPH_BAR2
	MG_GRAPHIC,	0.0,	489,	218,	32,		64,		"menu/common/mon_bar.tga",	0,0,	NULL,		0,		0,	0,		0,		0,							CT_MDGREY,	NULL,	// MMG_GRAPH_BAR3
	MG_GRAPHIC,	0.0,	489,	318,	32,		64,		"menu/common/mon_bar.tga",	0,0,	NULL,		0,		0,	0,		0,		0,							CT_MDGREY,	NULL,	// MMG_GRAPH_BAR4


//	type		timer	x		y		width	height	file/text						graphic,	symbol					desc text			target	inc		style	color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,			0,0,				NULL,		0,						0,					0,		0,		0,							CT_NONE,	NULL,							// MMG_SECTIONS_BEGIN
	MG_VAR,		0.0,	260,	377,	0,		0,		NULL,			0,0,				NULL,		MMG_FEDERATION_SYM,		MMG_FEDERATION_TXT,	0,		0,		UI_RIGHT|UI_TINYFONT,		CT_YELLOW,	(void *) &s_federation_label,	// MMG_FEDERATION
	MG_VAR,		0.0,	260,	359,	0,		0,		NULL,			0,0,				NULL,		MMG_CARDASSIA_SYM,		MMG_CARDASSIA_TXT,	0,		0,		UI_RIGHT|UI_TINYFONT,		CT_YELLOW,	(void *) &s_cardassian_label,	// MMG_CARDASSIA
	MG_VAR,		0.0,	260,	347,	0,		0,		NULL,			0,0,				NULL,		MMG_FERENGI_SYM,		MMG_FERENGI_TXT,	0,		0,		UI_RIGHT|UI_TINYFONT,		CT_YELLOW,	(void *) &s_ferengi_label,		// MMG_FERENGINAR
	MG_VAR,		0.0,	344,	354,	0,		0,		NULL,			0,0,				NULL,		MMG_ROMULAN_SYM,		MMG_ROMULAN_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_romulan_label,		// MMG_ROMULANEMPIRE
	MG_VAR,		0.0,	344,	365,	0,		0,		NULL,			0,0,				NULL,		MMG_KLINGON_SYM,		MMG_KLINGON_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_klingon_label,		// MMG_KLINGONEMPIRE
	MG_VAR,		0.0,	224,	212,	0,		0,		NULL,			0,0,				NULL,		MMG_DOMINION_SYM,		MMG_DOMINION_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_dominion_label,		// MMG_DOMINION
	MG_VAR,		0.0,	372,	198,	0,		0,		NULL,			0,0,				NULL,		MMG_FEDERATION_SYM,		MMG_VOYAGER_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_voyager_label,		// MMG_VOYAGER
	MG_VAR,		0.0,	372,	198,	0,		0,		NULL,			0,0,				NULL,		MMG_BORG_SYM,			MMG_BORG_TXT,		0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_borg_label,			// MMG_BORG
	MG_VAR,		0.0,	219,	228,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_WORMHOLE_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_wormhole_label,		// MMG_WORMHOLE
	MG_VAR,		0.0,	273,	278,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_GALACTICCORE_TXT,0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_core_label,			// MMG_GALACTICCORE
	MG_VAR,		0.0,	273,	278,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_ALPHAQUAD_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_alphaquad_label,	// MMG_ALPHAQUAD
	MG_VAR,		0.0,	273,	278,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_BETAQUAD_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_betaquad_label,		// MMG_BETAQUAD
	MG_VAR,		0.0,	273,	278,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_DELTAQUAD_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_deltaquad_label,	// MMG_DELTAQUAD
	MG_VAR,		0.0,	273,	278,	0,		0,		NULL,			0,0,				NULL,		0,						MMG_GAMAQUAD_TXT,	0,		0,		UI_TINYFONT,				CT_YELLOW,	(void *) &s_gammaquad_label,	// MMG_GAMMAQUAD
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,			0,0,				NULL,		0,						0,					0,		0,		0,							CT_NONE,	NULL,	// MMG_SECTIONS_END

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,		0,		0,							CT_NONE,	NULL,	// MMG_SYMBOLS_BEGIN
	MG_GRAPHIC,	0.0,	125,	396,	64,		32,		"menu/special/federation.tga",	0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_FEDERATION_SYM
	MG_GRAPHIC,	0.0,	125,	379,	32,		64,		"menu/special/cardassia.tga",	0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_CARDASSIA_SYM
	MG_GRAPHIC,	0.0,	125,	400,	64,		32,		"menu/special/ferengi.tga",		0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_FERENGI_SYM
	MG_GRAPHIC,	0.0,	125,	396,	64,		32,		"menu/special/romulan.tga",		0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_ROMULAN_SYM
	MG_GRAPHIC,	0.0,	125,	387,	32,		64,		"menu/special/klingon.tga",		0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_KLINGON_SYM
	MG_GRAPHIC,	0.0,	125,	393,	32,		64,		"menu/special/dominion.tga",	0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_DOMINION_SYM
	MG_GRAPHIC,	0.0,	125,	368,	32,		64,		"menu/special/borg.tga",		0,0,NULL,		0,		0,	0,		0,		0,							CT_WHITE,	NULL,	// MMG_BORG_SYM
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,							0,0,NULL,		0,		0,	0,		0,		0,							CT_NONE,	NULL,	// MMG_SYMBOLS_END

//	type		timer	x		y		width	height	file/text									graphic,	min		max	target	inc		style	color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,								0,0,		NULL,		0,		0,	0,		0,		0,							CT_NONE,	NULL,	// MMG_SECTION_TEXT_BEGIN
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,MNT_FEDERATION_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_FEDERATION_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,MNT_CARDASSIA_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_CARDASSIA_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,	MNT_FERENGI_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_FERENGI_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,	MNT_ROMULAN_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_ROMULAN_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,	MNT_KLINGON_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_KLINGON_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,	MNT_DOMINION_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_DOMINION_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,	MNT_VOYAGER_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_VOYAGER_TXT
	MG_STRING,	0.0,	168,	419,	0,		0,		NULL,		MNT_BORG_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_BORG_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,	MNT_WORMHOLE_TXT,0,		NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_WORMHOLE_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,	MNT_GALACTICCORE_TXT,0,	NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_GALACTICCORE_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,	MNT_ALPHAQUAD_TXT,0,	NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_ALPHAQUAD_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,		MNT_BETAQUAD_TXT,0,	NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_BETAQUAD_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,	MNT_DELTAQUAD_TXT,0,	NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_DELTAQUAD_TXT
	MG_STRING,	0.0,	125,	419,	0,		0,		NULL,	MNT_GAMMAQUAD_TXT,0,	NULL,		0,		0,	0,		0,		UI_TINYFONT,				CT_YELLOW,	NULL,	// MMG_GAMMAQUAD_TXT
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,									0,0,	NULL,		0,		0,	0,		0,		0,							CT_NONE,	NULL,	// MMG_SECTION_TEXT_END

//	type		timer	x		y		width	height	file/text						graphic,	min		max	target	inc		style	color
	MG_GRAPHIC,	0.0,	  0,	  0,	4,		4,		"menu/common/circle.tga",	0,0,	NULL,		0,		9,	0,		0,		0,							CT_LTGOLD1,	NULL,	// MMG_RH_COL1_SPECNUM1

	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM1
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM2
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM3
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM4
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM5
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL1_NUM6

	MG_GRAPHIC,	0.0,	  0,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		9,	0,		0,		0,							CT_LTGOLD1,	NULL,	// MMG_RH_COL2_SPECNUM1

	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM1
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM2
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM3
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM4
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM5
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL2_NUM6

	MG_GRAPHIC,	0.0,	  0,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		9,	0,		0,		0,							CT_LTGOLD1,	NULL,	// MMG_RH_COL3_SPECNUM1

	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL3_NUM1
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL3_NUM2
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL3_NUM3

	MG_GRAPHIC,	0.0,	  0,	  0,	4,		4,		"menu/common/circle.tga",0,0,		NULL,		0,		9,	0,		0,		0,							CT_LTGOLD1,	NULL,	// MMG_RH_COL4_SPECNUM1

	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL4_NUM1
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL4_NUM2
	MG_NUMBER,	0.0,	  0,	  0,	16,		10,		NULL,					0,0,		NULL,		0,		9,	0,		12,		UI_TINYFONT,				CT_LTGOLD1,	NULL,	// MMG_RH_COL4_NUM3


//	type		timer	x		y		width	height	file/text					graphic,	min		max		target	inc		style						color			ptr
/*	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,					0,0,	NULL,		0,		0,		0,		0,		0,							0,				NULL,	// MMG_TOP_NUMBERS
	MG_GRAPHIC,	0.0,	460,	0,		4,		4,		"menu/common/circle.tga",0,0,	NULL,		0,		9,		0,		0,		0,							CT_LTPURPLE2,	NULL,	// MMG_SPEC_TOP_ROW
	MG_NUMBER,	0.0,	468,	62,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL1_NUM1
	MG_NUMBER,	0.0,	468,	76,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL1_NUM2
	MG_NUMBER,	0.0,	468,	90,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL1_NUM3
	MG_NUMBER,	0.0,	468,	104,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL1_NUM4
	MG_NUMBER,	0.0,	468,	118,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL1_NUM5

	MG_NUMBER,	0.0,	493,	62,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL2_NUM1
	MG_NUMBER,	0.0,	493,	76,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL2_NUM2
	MG_NUMBER,	0.0,	493,	90,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL2_NUM3
	MG_NUMBER,	0.0,	493,	104,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL2_NUM4
	MG_NUMBER,	0.0,	493,	118,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL2_NUM5

	MG_NUMBER,	0.0,	518,	62,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL3_NUM1
	MG_NUMBER,	0.0,	518,	76,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL3_NUM2
	MG_NUMBER,	0.0,	518,	90,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL3_NUM3
	MG_NUMBER,	0.0,	518,	104,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL3_NUM4
	MG_NUMBER,	0.0,	518,	118,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL3_NUM5

	MG_NUMBER,	0.0,	543,	62,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL4_NUM1
	MG_NUMBER,	0.0,	543,	76,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL4_NUM2
	MG_NUMBER,	0.0,	543,	90,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL4_NUM3
	MG_NUMBER,	0.0,	543,	104,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL4_NUM4
	MG_NUMBER,	0.0,	543,	118,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL4_NUM5

	MG_NUMBER,	0.0,	568,	62,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL5_NUM1
	MG_NUMBER,	0.0,	568,	76,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL5_NUM2
	MG_NUMBER,	0.0,	568,	90,		16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL5_NUM3
	MG_NUMBER,	0.0,	568,	104,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL5_NUM4
	MG_NUMBER,	0.0,	568,	118,	16,		10,		NULL,				0,0,		NULL,		0,		3,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// MMG_TOP_COL5_NUM5
	*/
};

#define SYSTEM_MAXDESC 5
char systemDesc[SYSTEM_MAXDESC][512];

#define SUIT_MAXDESC 8
char suitDesc[SUIT_MAXDESC][512];

typedef enum 
{
	QMG_NUMBERS,
	QMG_COL1_NUM1,
	QMG_COL1_NUM2,
	QMG_COL1_NUM3,
	QMG_COL1_NUM4,
	QMG_COL1_NUM5,

	QMG_COL2_NUM1,
	QMG_COL2_NUM2,
	QMG_COL2_NUM3,
	QMG_COL2_NUM4,
	QMG_COL2_NUM5,

	QMG_COL3_NUM1,
	QMG_COL3_NUM2,
	QMG_COL3_NUM3,
	QMG_COL3_NUM4,
	QMG_COL3_NUM5,

	QMG_COL4_NUM1,
	QMG_COL4_NUM2,
	QMG_COL4_NUM3,
	QMG_COL4_NUM4,
	QMG_COL4_NUM5,

	QMG_COL5_NUM1,
	QMG_COL5_NUM2,
	QMG_COL5_NUM3,
	QMG_COL5_NUM4,
	QMG_COL5_NUM5,

	QMG_COL6_NUM1,
	QMG_COL6_NUM2,
	QMG_COL6_NUM3,
	QMG_COL6_NUM4,
	QMG_COL6_NUM5,

	QMG_ACTIVE_SYSTEM,

	QMG_DESC_START,
	QMG_BRIDGE_DESC1,
	QMG_PHASER_STRIP1_DESC1,
	QMG_TORPEDOS_DESC1,
	QMG_VENTRAL_DESC1,
	QMG_MIDHULL_DESC1,
	QMG_BUSSARD_DESC1,
	QMG_NACELLES_DESC1,
	QMG_THRUSTERS_DESC1,
	QMG_VOYAGER_DESC1,
	QMG_DESC_END,

	QMG_LABEL_START,
	QMG_BRIDGE_LABEL,
	QMG_PHASER_LABEL,
	QMG_TORPEDO_LABEL,
	QMG_VENTRAL_LABEL,
	QMG_MIDHULL_LABEL,
	QMG_BUSSARD_LABEL,
	QMG_NACELLES_LABEL,
	QMG_THRUSTERS_LABEL,
	QMG_VOYAGER_LABEL,
	QMG_LABEL_END,

	QMG_SWOOP_START,
	QMG_BRIDGE_SWOOP,
	QMG_PHASER_STRIP1_SWOOP,
	QMG_TORPEDO_SWOOP,
	QMG_VENTRAL_SWOOP,
	QMG_MIDHULL_SWOOP,
	QMG_BUSSARD_SWOOP,
	QMG_NACELLES_SWOOP,
	QMG_THRUSTERS_SWOOP,
	QMG_SWOOP_END,

	QMG_BOTTOM_BLIP,
	QMG_BOTTOM_BLIP2,
	QMG_MAX
} quitmenu_graphics_t;

menugraphics_s quitmenu_graphics[QMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text		graphic,	min		max		target	inc		style						color

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,	0,0,		NULL,		0,		0,		0,		0,		0,							0,				NULL,	// QMG_NUMBERS
	MG_NUMBER,	0.0,	368,	54,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL1_NUM1
	MG_NUMBER,	0.0,	368,	68,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL1_NUM2
	MG_NUMBER,	0.0,	368,	82,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL1_NUM3
	MG_NUMBER,	0.0,	368,	96,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL1_NUM4
	MG_NUMBER,	0.0,	368,	110,	16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL1_NUM5

	MG_NUMBER,	0.0,	423,	54,		16,		10,		NULL,	0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL2_NUM1
	MG_NUMBER,	0.0,	423,	68,		16,		10,		NULL,	0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL2_NUM2
	MG_NUMBER,	0.0,	423,	82,		16,		10,		NULL,	0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL2_NUM3
	MG_NUMBER,	0.0,	423,	96,		16,		10,		NULL,	0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL2_NUM4
	MG_NUMBER,	0.0,	423,	110,	16,		10,		NULL,	0,0,		NULL,		0,		6,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL2_NUM5

	MG_NUMBER,	0.0,	463,	54,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL3_NUM1
	MG_NUMBER,	0.0,	463,	68,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL3_NUM2
	MG_NUMBER,	0.0,	463,	82,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL3_NUM3
	MG_NUMBER,	0.0,	463,	96,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL3_NUM4
	MG_NUMBER,	0.0,	463,	110,	16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL3_NUM5

	MG_NUMBER,	0.0,	526,	54,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL4_NUM1
	MG_NUMBER,	0.0,	526,	68,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL4_NUM2
	MG_NUMBER,	0.0,	526,	82,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL4_NUM3
	MG_NUMBER,	0.0,	526,	96,		16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL4_NUM4
	MG_NUMBER,	0.0,	526,	110,	16,		10,		NULL,	0,0,		NULL,		0,		9,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL4_NUM5

	MG_NUMBER,	0.0,	581,	54,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL5_NUM1
	MG_NUMBER,	0.0,	581,	68,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL5_NUM2
	MG_NUMBER,	0.0,	581,	82,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL5_NUM3
	MG_NUMBER,	0.0,	581,	96,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL5_NUM4
	MG_NUMBER,	0.0,	581,	110,	16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL5_NUM5

	MG_NUMBER,	0.0,	601,	54,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL6_NUM1
	MG_NUMBER,	0.0,	601,	68,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL6_NUM2
	MG_NUMBER,	0.0,	601,	82,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL6_NUM3
	MG_NUMBER,	0.0,	601,	96,		16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL6_NUM4
	MG_NUMBER,	0.0,	601,	110,	16,		10,		NULL,	0,0,		NULL,		0,		2,		0,		0,		UI_TINYFONT,				CT_DKPURPLE2,	NULL,	// QMG_COL6_NUM5


//	type		timer	x		y		width	height	file/text						graphic,	min		max	target							inc		style						color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,				0,0,			NULL,		0,		0,	0,								0,		0,							0,				NULL,	// QMG_ACTIVE_SYSTEM

//	type		timer	x		y		width	height	file/text												graphic,	min		max	target							inc		style						color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,				0,0,				NULL,		0,		0,	0,								0,		0,							0,				NULL,	// QMG_DESC_START
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,				MNT_BRIDGE_DESC1,0,				NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_BRIDGE_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,	MNT_PHASER_STRIP1_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_PHASER_STRIP1_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_TORPEDOS_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_TORPEDOS_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_VENTRAL_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_VENTRAL_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_MIDHULL_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_MIDHULL_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_BUSSARD_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_BUSSARD_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_NACELLES_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_NACELLES_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_THRUSTERS_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_THRUSTERS_DESC1
	MG_STRING,	0.0,	385,	378,	0,		0,		NULL,		MNT_VOYAGER_DESC1,0,			NULL,		0,		0,	0,								0,		UI_TINYFONT,				CT_LTGOLD1,		NULL,	// QMG_VOYAGER_DESC1

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,									0,0,				NULL,		0,		0,	0,								0,		0,							0,				NULL,	// QMG_DESC_END

//	type		timer	x		y		width	height	file/text		graphic,	min	(desc)						max		target	(swoop)					inc		style						color
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		0,								0,		0,								0,		0,							0,				NULL,								// QMG_LABEL_START
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_BRIDGE_DESC1,				0,		QMG_BRIDGE_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.bridge_pic,	// QMG_BRIDGE_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_PHASER_STRIP1_DESC1,		0,		QMG_PHASER_STRIP1_SWOOP,		0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.phaser_pic,	// QMG_PHASER_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_TORPEDOS_DESC1,				0,		QMG_TORPEDO_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.torpedo_pic,	// QMG_TORPEDOS_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_VENTRAL_DESC1,				0,		QMG_VENTRAL_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.ventral_pic,	// QMG_VENTRAL_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_MIDHULL_DESC1,				0,		QMG_MIDHULL_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.midhull_pic,	// QMG_MIDHULL_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_BUSSARD_DESC1,				0,		QMG_BUSSARD_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.bussard_pic,	// QMG_BUSSARD_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_NACELLES_DESC1,				0,		QMG_NACELLES_SWOOP,				0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.nacelles_pic,	// QMG_NACELLES_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_THRUSTERS_DESC1,			0,		QMG_THRUSTERS_SWOOP,			0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.thrusters_pic,	// QMG_THRUSTERS_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		QMG_VOYAGER_DESC1,				0,		0,								0,		0,							CT_DKBROWN1,	(void *) &s_quitmenu.voyager_pic,	// QMG_VOYAGER_LABEL
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,		0,0,	NULL,		0,								0,		0,								0,		0,							0,				NULL,								// QMG_LABEL_END

	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,								0,		0,							0,				NULL,	// QMG_SWOOP_START
	MG_GRAPHIC,	0.0,	156,	198,	93,		1,		"menu/common/square.tga",	0,0,	NULL,		0,		0,	QMG_BRIDGE_DESC1,				1,		0,							CT_WHITE,		NULL,	// QMG_BRIDGE_SWOOP
	MG_GRAPHIC,	0.0,	155,	229,	16,		64,		"menu/voyager/swoop1.tga",	0,0,	NULL,		0,		0,	QMG_PHASER_STRIP1_DESC1,		2,		0,							CT_WHITE,		NULL,	// QMG_PHASER_STRIP1_SWOOP
	MG_GRAPHIC,	0.0,	252,	272,	32,		32,		"menu/voyager/swoop2.tga",	0,0,	NULL,		0,		0,	QMG_TORPEDOS_DESC1,				1,		0,							CT_WHITE,		NULL,	// QMG_TORPEDOS_SWOOP
	MG_GRAPHIC,	0.0,	318,	310,	32,		32,		"menu/voyager/swoop3.tga",	0,0,	NULL,		0,		0,	QMG_VENTRAL_DESC1,				1,		0,							CT_WHITE,		NULL,	// QMG_VENTRAL_SWOOP
	MG_GRAPHIC,	0.0,	366,	247,	16,		128,	"menu/voyager/swoop4.tga",	0,0,	NULL,		0,		0,	QMG_MIDHULL_DESC1,				1,		0,							CT_WHITE,		NULL,	// QMG_MIDHULL_SWOOP
	MG_GRAPHIC,	0.0,	457,	281,	64,		64,		"menu/voyager/swoop5.tga",	0,0,	NULL,		0,		0,	QMG_BUSSARD_DESC1,				3,		0,							CT_WHITE,		NULL,	// QMG_BUSSARD_SWOOP
	MG_GRAPHIC,	0.0,	563,	181,	64,		128,	"menu/voyager/swoop6.tga",	0,0,	NULL,		0,		0,	QMG_NACELLES_DESC1,				2,		0,							CT_WHITE,		NULL,	// QMG_NACELLES_SWOOP
	MG_GRAPHIC,	0.0,	288,	183,	64,		128,	"menu/voyager/swoop7.tga",	0,0,	NULL,		0,		0,	QMG_THRUSTERS_DESC1,			1,		0,							CT_WHITE,		NULL,	// QMG_THRUSTERS_SWOOP
	MG_VAR,		0.0,	0,		0,		0,		0,		NULL,						0,0,	NULL,		0,		0,	0,								0,		0,							0,				NULL,	// QMG_SWOOP_END

//	type		timer	x		y		width	height	file/text							graphic,	min	(desc)						max		target	(swoop)					inc		style						color
	MG_GRAPHIC,	0.0,	  0,	353,	8,		8,		"menu/common/square.tga",	0,0,		NULL,		77,		550,0,								12,		UI_TINYFONT,				CT_LTPURPLE1,	NULL,	// QMG_BOTTOM_BLIP
	MG_GRAPHIC,	0.0,	  0,	167,	8,		8,		"menu/common/square.tga",	0,0,		NULL,		77,		550,0,								12,		UI_TINYFONT,				CT_LTPURPLE1,	NULL,	// QMG_BOTTOM_BLIP2
};



typedef enum 
{
	IGMG_ACTIVE_SYSTEM,

	IGMG_POWERCONVERTER_LINE,
	IGMG_COMMBADGE_LINE,
	IGMG_LOGISTICS_LINE,
	IGMG_ENERGY_PACK_LINE,
	IGMG_WAVEGENERATOR_LINE,
	IGMG_SCANNERS_LINE,
	IGMG_POUCHES_LINE,
	IGMG_BUFFER_LINE,

	IGMG_LABEL_START,
	IGMG_POWERCONVERTER_LABEL,
	IGMG_COMMBADGE_LABEL,
	IGMG_LOGISTICS_LABEL,
	IGMG_ENERGY_PACK_LABEL,
	IGMG_WAVEGENERATOR_LABEL,
	IGMG_SCANNERS_LABEL,
	IGMG_POUCHES_LABEL,
	IGMG_BUFFER_LABEL,
	IGMG_LABEL_END,

	IGMG_POWERCONVERTER_DESC,
	IGMG_COMMBADGE_DESC,
	IGMG_LOGISTICS_DESC,
	IGMG_ENERGY_PACK_DESC,
	IGMG_WAVEGENERATOR_DESC,
	IGMG_SCANNERS_DESC,
	IGMG_POUCHES_DESC,
	IGMG_BUFFER_DESC,

	IGMG_MAX
} ingamemenu_graphics_t;

menugraphics_s ingamemenu_graphics[QMG_MAX] = 
{
//	type		timer		x		y		width	height	file/text										graphic,	min						max		target		inc			style					color			ptr
	MG_VAR,		0.0,		0,		0,		0,		0,		NULL,								0,0,			NULL,		0,						0,		0,			0,			0,						0,				NULL,	// IGMG_ACTIVE_SYSTEM

	MG_GRAPHIC,	0.0,		150,	232,	64,		32,		"menu/suit/power_conv_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_POWERCONVERTER_LINE
	MG_GRAPHIC,	0.0,		226,	192,	32,		32,		"menu/suit/combadge_break.tga",		0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_COMMBADGE_LINE
	MG_GRAPHIC,	0.0,		312,	285,	32,		32,		"menu/suit/direc_log_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_LOGISTICS_LINE
	MG_GRAPHIC,	0.0,		402,	248,	64,		32,		"menu/suit/ener_pack_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_ENERGY_PACK_LINE
	MG_GRAPHIC,	0.0,		188,	162,	64,		64,		"menu/suit/multi_ph_break.tga",		0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_WAVEGENERATOR_LINE
	MG_GRAPHIC,	0.0,		127,	285,	32,		32,		"menu/suit/direc_log_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_SCANNERS_LINE
	MG_GRAPHIC,	0.0,		428,	300,	64,		32,		"menu/suit/ener_pack_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_POUCHES_LINE
	MG_GRAPHIC,	0.0,		421,	268,	64,		32,		"menu/suit/trans_buff_break.tga",	0,0,			NULL,		0,						0,		0,			0,			NULL,					CT_WHITE,		NULL,	// IGMG_BUFFER_LINE

//	type		timer		x		y		width	height	file/text										graphic,	min							max		target						inc		style					color			ptr
	MG_VAR,		0.0,		0,		0,		0,		0,		NULL,								0,0,			NULL,		0,							0,		0,							0,		0,						0,				NULL,										// IGMG_LABEL_START
	MG_VAR,		0.0,		148,	228,	0,		0,		NULL,								0,0,			NULL,		IGMG_POWERCONVERTER_DESC,	0,		IGMG_POWERCONVERTER_LINE,	1,		UI_RIGHT|UI_TINYFONT,	CT_DKBROWN1,	(void *) &s_ingamemenu.powerconv_pic,		// IGMG_POWERCONVERTER_LABEL
	MG_VAR,		0.0,		252,	185,	0,		0,		NULL,								0,0,			NULL,		IGMG_COMMBADGE_DESC,		0,		IGMG_COMMBADGE_LINE,		1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.commbadge_pic,		// IGMG_COMMBADGE_LABEL
	MG_VAR,		0.0,		312,	320,	0,		0,		NULL,								0,0,			NULL,		IGMG_LOGISTICS_DESC,		0,		IGMG_LOGISTICS_LINE,		1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.logistics_pic,		// IGMG_LOGISTICS_LABEL
	MG_VAR,		0.0,		466,	242,	0,		0,		NULL,								0,0,			NULL,		IGMG_ENERGY_PACK_DESC,		0,		IGMG_ENERGY_PACK_LINE,		1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.energypack_pic,		// IGMG_ENERGY_PACK_LABEL
	MG_VAR,		0.0,		254,	158,	0,		0,		NULL,								0,0,			NULL,		IGMG_WAVEGENERATOR_DESC,	0,		IGMG_WAVEGENERATOR_LINE,	1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.wavegenerator_pic,	// IGMG_WAVEGENERATOR_LABEL
	MG_VAR,		0.0,		127,	320,	0,		0,		NULL,								0,0,			NULL,		IGMG_SCANNERS_DESC,			0,		IGMG_SCANNERS_LINE,			1,		UI_RIGHT|UI_TINYFONT,	CT_DKBROWN1,	(void *) &s_ingamemenu.scanners_pic,		// IGMG_SCANNERS_LABEL
	MG_VAR,		0.0,		490,	295,	0,		0,		NULL,								0,0,			NULL,		IGMG_POUCHES_DESC,			0,		IGMG_POUCHES_LINE,			1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.pouches_pic,			// IGMG_POUCHES_LABEL
	MG_VAR,		0.0,		472,	281,	0,		0,		NULL,								0,0,			NULL,		IGMG_BUFFER_DESC,			0,		IGMG_BUFFER_LINE,			1,		UI_TINYFONT,			CT_DKBROWN1,	(void *) &s_ingamemenu.buffer_pic,			// IGMG_BUFFER_LABEL
	MG_VAR,		0.0,		0,		0,		0,		0,		NULL,								0,0,			NULL,		0,							0,		0,							0,		0,						0,				NULL,							// IGMG_LABEL_END

//	type		timer		x		y		width	height	file/text										graphic,	min		max		target	inc		style			color			ptr
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,MNT_POWERCONVERTER_DESC1,0,		NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// IGMG_POWERCONVERTER_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,	MNT_COMMBADGE_DESC1,0,		NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// IGMG_COMMBADGE_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,	MNT_LOGISTICS_DESC1,0,		NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// IGMG_LOGISTICS_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,	MNT_ENERGY_PACK_DESC1,0,	NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// 	IGMG_ENERGY_PACK_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,	MNT_WAVEGENERATOR_DESC1,0,	NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// 	IGMG_WAVEGENERATOR_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,		MNT_SCANNERS_DESC1,0,	NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// 	IGMG_SCANNERS_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,		MNT_POUCHES_DESC1,0,	NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// 	IGMG_POUCHES_DESC,
	MG_STRING,	0.0,		455,	330,	0,		0,		NULL,			MNT_BUFFER_DESC1,0,	NULL,		0,		0,		0,		0,		UI_TINYFONT,	CT_LTGOLD1,		NULL,	// 	IGMG_BUFFER_DESC,
};


// menu action identifiers
//#define	ID_LANGAME				100
//#define ID_INTERNETGAME			101
//#define ID_STARTSERVER			102
#define ID_TOUR						103
#define ID_CUSTOMIZECONTROLS		104
#define ID_INGAMECUSTOMIZECONTROLS	105

//#define ID_PLAYDEMO				106
//#define ID_SINGLEPLAYER			107
#define ID_MODS					107
#define ID_QUIT					108
#define ID_NO					109
#define ID_YES					110
#define ID_IMLOADGAME			111
#define ID_RETURN				112
#define ID_LEAVEGAME			113
//#define ID_SERVERINFO			113
#define ID_LEAVEARENA			117
#define ID_TEAM					118
#define ID_NEWGAME				119
#define ID_LOADGAME				120
#define ID_MAINMENU				121
#define ID_CREDITS				122
#define ID_EXPLORE				123
#define ID_RETURNTOGAME			124
#define ID_INGAMESAVE			125
#define ID_SCREENSHOT			126

#define	ID_FEDERATION_LABEL		201
#define	ID_CARDASSIAN_LABEL		202
#define	ID_FERENGI_LABEL		203
#define	ID_ROMULAN_LABEL		204
#define	ID_KLINGON_LABEL		205
#define	ID_VOYAGER_LABEL		206
#define	ID_DOMINION_LABEL		207
#define	ID_CORE_LABEL			208
#define	ID_ALHAPQUAD_LABEL		209
#define	ID_BETAQUAD_LABEL		210
#define	ID_DELTAQUAD_LABEL		211
#define	ID_GAMMAQUAD_LABEL		212
#define	ID_BORG_LABEL			213
#define	ID_WORMHOLE_LABEL		214

// Quit Menu
#define	ID_PHASER_LABEL			301
#define	ID_TORPEDO_LABEL		302
#define	ID_VENTRAL_LABEL		303
#define	ID_MIDHULL_LABEL		304
#define	ID_BUSSARD_LABEL		305
#define	ID_NACELLES_LABEL		306
#define	ID_THRUSTERS_LABEL		307
#define	ID_Q_VOYAGER_LABEL		308
#define	ID_BRIDGE_LABEL			309
#define	ID_Q_RAVEN_LABEL		310

// Ingame Main Menu
#define	ID_POWERCONVERTER_LABEL	321
#define	ID_COMMBADGE_LABEL		322
#define	ID_LOGISTICS_LABEL		323
#define	ID_ENERGYPACK_LABEL		324
#define	ID_WAVEGENERATOR_LABEL	325
#define	ID_SCANNERS_LABEL		326
#define	ID_POUCHES_LABEL		327
#define	ID_BUFFER_LABEL			328


static void UI_SystemConfigurationMenu( void );
static void UI_QuitMenu( void );
void InGameMenu_ChangeAreaFocus(int newSystem);

void UI_StartDemoLoop( void ) {
	ui.Cmd_ExecuteText( EXEC_APPEND, "d1\n" );
}

/*
=======================================================================

INGAME MENU

=======================================================================
*/
static menuframework_s	s_ingame_menu;
static menuaction_s		s_team_action;
static menuaction_s		s_customize_player_action;
static menuaction_s		s_customize_controls_action;
static menuaction_s		s_system_configuration_action;
static menuaction_s		s_screenshot_action;
static menuaction_s		s_leave_arena_action;
static menuaction_s		s_quit_game_action;


static float mm_buttons[8][2] = 
{
{129,62},
{129,86},
{129,109},
{305,62},
{305,86},
{305,109},
{481,62},
{481,86}
};

static float ingame_buttons[6][2] = 
{
{129,62},
{129,86},
{129,109},
{305,62},
{305,86},
{305,109}
};

/*
=================
InGameMenuInterrupt
=================
*/
void InGameMenuInterrupt(int labelId)
{
	int newArea;

	switch (labelId)
	{
		case ID_POWERCONVERTER_LABEL:
			newArea = IGMG_POWERCONVERTER_LABEL;
			break;
		case ID_COMMBADGE_LABEL:
			newArea = IGMG_COMMBADGE_LABEL;
			break;
		case ID_LOGISTICS_LABEL:
			newArea = IGMG_LOGISTICS_LABEL;
			break;
		case ID_ENERGYPACK_LABEL:
			newArea = IGMG_ENERGY_PACK_LABEL;
			break;
		case ID_WAVEGENERATOR_LABEL:
			newArea = IGMG_WAVEGENERATOR_LABEL;
			break;
		case ID_SCANNERS_LABEL:
			newArea = IGMG_SCANNERS_LABEL;
			break;
		case ID_POUCHES_LABEL:
			newArea = IGMG_POUCHES_LABEL;
			break;
		case ID_BUFFER_LABEL:
			newArea = IGMG_BUFFER_LABEL;
			break;
	}

	InGameMenu_ChangeAreaFocus(newArea);

	// ten seconds from now, start the auto animation again
	ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].timer = uis.realtime + 10000;
}


/*
=================
InGame_MenuKey
=================
*/
static sfxHandle_t InGame_MenuKey( int key )
{
	return Menu_DefaultKey( &s_ingame_menu, key );
}

/*
=================
InGame_Event
=================
*/
void InGame_Event (void* ptr, int notification)
{
	char	commandString[128];

	if (notification != QM_ACTIVATED)
		return;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_POWERCONVERTER_LABEL:
		case ID_COMMBADGE_LABEL:
		case ID_LOGISTICS_LABEL:
		case ID_ENERGYPACK_LABEL:
		case ID_WAVEGENERATOR_LABEL:
		case ID_SCANNERS_LABEL:
		case ID_POUCHES_LABEL:
		case ID_BUFFER_LABEL:
			InGameMenuInterrupt(((menucommon_s*)ptr)->id);
			break;

		case ID_SCREENSHOT:
			UI_ForceMenuOff();
			ui.Cmd_ExecuteText( EXEC_APPEND, "wait; wait; wait; wait; screenshot\n" );
			break;

		// Customize Controls
		case ID_CUSTOMIZECONTROLS:
			//UI_ControlsMenu();
			break;

		case ID_INGAMESAVE:	// Need screen

			if ((ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)&& (holoMatch	== qfalse))
			{
				Com_sprintf( commandString,  sizeof(commandString), "save virtual\n");
				ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			}
			else
			{
				UI_SaveGameMenu(qtrue);
			}
			break;

		case ID_IMLOADGAME:
			if (ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)
			{
				Com_sprintf( commandString,  sizeof(commandString), "load virtual\n");
				ui.Cmd_ExecuteText( EXEC_APPEND, commandString);
			}
			else
			{
				UI_LoadGameMenu(qtrue);
			}
			break;

		// Quit
		case ID_QUIT:
			UI_QuitMenu();	
			break;

		case ID_LEAVEGAME:
			UI_LeaveGameMenu();
			break;

		case ID_RETURNTOGAME:
			UI_PopMenu();
			return;
	}
}

/*
=================
UI_InGameMenuButtons
=================
*/
void UI_InGameMenuButtons(int count)
{
	int i;

	ui.R_SetColor( colorTable[CT_DKPURPLE1]);

	for (i=0;i<count;++i)
	{
		UI_DrawHandlePic(ingame_buttons[i][0] - 14,ingame_buttons[i][1], MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);
	}
}


/*
=================
IngameMenu_Label
=================
*/
menutext_s *IngameMenu_Label(int labelId)
{
	menutext_s *label;

	switch(labelId)
	{
		case IGMG_POWERCONVERTER_LABEL :
			label = &s_ingamemenu.powerconv_label;
			break;

		case IGMG_COMMBADGE_LABEL :
			label = &s_ingamemenu.commbadge_label;
			break;

		case IGMG_LOGISTICS_LABEL :
			label = &s_ingamemenu.logistics_label;
			break;

		case IGMG_ENERGY_PACK_LABEL :
			label = &s_ingamemenu.energypack_label;
			break;

		case IGMG_WAVEGENERATOR_LABEL :
			label = &s_ingamemenu.wavegenerator_label;
			break;

		case IGMG_SCANNERS_LABEL :
			label = &s_ingamemenu.scanners_label;
			break;

		case IGMG_POUCHES_LABEL :
			label = &s_ingamemenu.pouches_label;
			break;

		case IGMG_BUFFER_LABEL :
			label = &s_ingamemenu.buffer_label;
			break;

		default:
			label = &s_ingamemenu.buffer_label;
			break;
	}

	return(label);
}

/*
=================
M_Ingame_SplitDesc
=================
*/
void M_Ingame_SplitDesc(int descI)
{
	int	lineWidth,currentWidth,charCnt,currentLineI;
	char *s,*holds;
	char holdChar[2];

	// Clean out any old data
	memset(suitDesc,0,sizeof(suitDesc));

	// Break into individual lines
	s = menu_normal_text[ingamemenu_graphics[descI].normaltextEnum];
	holds = s;


	lineWidth = 159;	// How long (in pixels) a line can be
	currentWidth = 0;
	holdChar[1] = '\0';
	charCnt= 0;
	currentLineI = 0;

	while( *s ) 
	{
		++charCnt;
		holdChar[0] = *s;
		currentWidth += UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
		currentWidth +=1; // The space between characters

		if ( currentWidth >= lineWidth )
		{//Reached max length of this line
			//step back until we find a space

			while((currentWidth) && (*s != ' '))
			{
				holdChar[0] = *s;
				currentWidth -= UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
				--s;
				--charCnt;
			}

			Q_strncpyz( suitDesc[currentLineI], holds, charCnt);
			suitDesc[currentLineI][charCnt] = NULL;

			++currentLineI;
			currentWidth = 0;
			charCnt = 0;

			holds = s;
			++holds;

			if (currentLineI > SUIT_MAXDESC)
			{
				currentLineI = (SUIT_MAXDESC -1);
				break;
			}
		}	
		++s;
	}

	++charCnt;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( suitDesc[currentLineI], holds, charCnt);
	suitDesc[currentLineI][charCnt] = NULL;

}

/*
=================
InGameMenu_ChangeAreaFocus
=================
*/
void InGameMenu_ChangeAreaFocus(int newSystem)
{
	int swoopI;
	int descI;
	int oldSystem;
	menubitmap_s *bitmap;
	menutext_s *oldLabel,*newLabel;


	// Turn off current system info
	oldSystem = ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].target;

	// Turn old label brown
	oldLabel = IngameMenu_Label(oldSystem);
	oldLabel->color = CT_DKBROWN1;

	// Turn off swoop
	swoopI = ingamemenu_graphics[oldSystem].target;
	if (swoopI)
	{
		ingamemenu_graphics[swoopI].type = MG_OFF;
	}

	// Turn pic off
	if (ingamemenu_graphics[oldSystem].pointer)
	{
		bitmap = (menubitmap_s *) ingamemenu_graphics[oldSystem].pointer;
		bitmap->generic.flags |= QMF_HIDDEN;	
	}


	// Turning on the new system graphics
	ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].target = newSystem;

	// Turn on new label
	newLabel = IngameMenu_Label(newSystem);
	newLabel->color = CT_YELLOW;

	// Turn on system description
	descI = ingamemenu_graphics[newSystem].min;
	if (descI)
	{
		M_Ingame_SplitDesc(descI);
	}

	// Turn on pic
	if (ingamemenu_graphics[newSystem].pointer)
	{
		bitmap = (menubitmap_s *) ingamemenu_graphics[newSystem].pointer;
		bitmap->generic.flags &= ~QMF_HIDDEN;	
	}

	// Turn onswoop
	swoopI = ingamemenu_graphics[newSystem].target;
	if (swoopI)
	{
		ingamemenu_graphics[swoopI].type = MG_GRAPHIC;
	}
}


/*
=================
InGameMenu_Blinkies
=================
*/
void InGameMenu_Blinkies(void)
{
	int activeSystem;

	// Time to change systems???
	if ( ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].timer < uis.realtime )
	{
		activeSystem = ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].target;

		// Change again in five seconds
		ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].timer = uis.realtime + 5000;

		// Advance to next system
		++activeSystem;
		if (activeSystem >= IGMG_LABEL_END)	// Past max strings
		{
			activeSystem = IGMG_LABEL_START +1;	// Reset
		}

		InGameMenu_ChangeAreaFocus(activeSystem);

		ui.S_StartLocalSound( uis.menu_choice1_snd, CHAN_LOCAL_SOUND );	// Ping!

	}
}

/*
=================
M_InGame_Graphics
=================
*/
void M_InGame_Graphics (void)
{
	int i,y;

	UI_MenuFrame(&s_ingame_menu);

	UI_InGameMenuButtons(6);	// Button ends

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 14, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
	UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH + 2, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right

	ui.R_SetColor( colorTable[CT_DKBLUE1]);
	UI_DrawHandlePic(140,170, 512, 256, s_ingamemenu_cache.suit);	// Hazard suit, Front & Back

	UI_DrawProportionalString(  85,  162, menu_normal_text[MNT_HAZARDSUIT],UI_SMALLFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  108, 406, menu_normal_text[MNT_FRONT],UI_SMALLFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString(  296, 406, menu_normal_text[MNT_BACK],UI_SMALLFONT, colorTable[CT_DKGOLD1]);

	// Description Box
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(445, 318,   6, 103, uis.whiteShader);	// Left side
	UI_DrawHandlePic(445, 318, 168,   8, uis.whiteShader);	// Top
	UI_DrawHandlePic(445, 421, 168,   8, uis.whiteShader);	// Bottom

	ui.R_SetColor( colorTable[CT_DKPURPLE3]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);	// Long left column square on bottom 3rd

	UI_DrawProportionalString(  74,  66, "80-345",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "67-568",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "451-05",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "452",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "57258",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString( 592, 142, "1001001",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	InGameMenu_Blinkies();

	UI_PrintMenuGraphics(ingamemenu_graphics,IGMG_MAX);

	y = 329;
	for (i=0;i<SUIT_MAXDESC;++i)
	{
		UI_DrawProportionalString(454, y, suitDesc[i],UI_TINYFONT,colorTable[CT_LTGOLD1]);
		y += 12;
	}

}


/*
=================
InGame_MenuDraw
=================
*/
static void InGame_MenuDraw (void)
{
	M_InGame_Graphics();

	Menu_Draw( &s_ingame_menu );

}

/*
===============
UI_InGameMenu_Cache
===============
*/
void UI_InGameMenu_Cache( void ) 
{
	int i;


	// Precache all menu graphics in array
	for (i=0;i<IGMG_MAX;++i)
	{
		if (ingamemenu_graphics[i].type == MG_GRAPHIC)
		{
			ingamemenu_graphics[i].graphic = ui.R_RegisterShaderNoMip(ingamemenu_graphics[i].file);
		}
	}

	s_ingamemenu_cache.suit = ui.R_RegisterShaderNoMip("menu/suit/breakout_suit.tga");

	ui.R_RegisterShaderNoMip("menu/suit/power_conv.tga");
	ui.R_RegisterShaderNoMip("menu/suit/combadge.tga");
	ui.R_RegisterShaderNoMip("menu/suit/direct_log_circ.tga");
	ui.R_RegisterShaderNoMip("menu/suit/energy_pack.tga");
	ui.R_RegisterShaderNoMip("menu/suit/multi_wavegen.tga");
	ui.R_RegisterShaderNoMip("menu/suit/pass_acscan.tga");
	ui.R_RegisterShaderNoMip("menu/suit/pouches.tga");
	ui.R_RegisterShaderNoMip("menu/suit/trans_buff.tga");
}


/*
=================
InGame_MenuButtonsInit
=================
*/
void InGame_MenuButtonsInit(menuframework_s *menu)
{

	s_returntogame.generic.type				= MTYPE_BITMAP;      
	s_returntogame.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_returntogame.generic.x				= ingame_buttons[0][0];
	s_returntogame.generic.y				= ingame_buttons[0][1];
	s_returntogame.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_returntogame.generic.id				= ID_RETURNTOGAME;
	s_returntogame.generic.callback			= InGame_Event; 
	s_returntogame.width					= MENU_BUTTON_MED_WIDTH;
	s_returntogame.height					= MENU_BUTTON_MED_HEIGHT;
	s_returntogame.color					= CT_DKPURPLE1;
	s_returntogame.color2					= CT_LTPURPLE1;
	s_returntogame.textX					= MENU_BUTTON_TEXT_X;
	s_returntogame.textY					= MENU_BUTTON_TEXT_Y;
	s_returntogame.textEnum					= MBT_RETURNTOGAME;
	s_returntogame.textcolor				= CT_BLACK;
	s_returntogame.textcolor2				= CT_WHITE;

	s_ingame_save.generic.type				= MTYPE_BITMAP;      
	s_ingame_save.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_ingame_save.generic.x					= ingame_buttons[1][0];
	s_ingame_save.generic.y					= ingame_buttons[1][1];
	s_ingame_save.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_ingame_save.generic.id				= ID_INGAMESAVE;
	s_ingame_save.generic.callback			= InGame_Event; 
	s_ingame_save.width						= MENU_BUTTON_MED_WIDTH;
	s_ingame_save.height					= MENU_BUTTON_MED_HEIGHT;
	s_ingame_save.color						= CT_DKPURPLE1;
	s_ingame_save.color2					= CT_LTPURPLE1;
	s_ingame_save.textX						= MENU_BUTTON_TEXT_X;
	s_ingame_save.textY						= MENU_BUTTON_TEXT_Y;
	s_ingame_save.textEnum					= MBT_SAVEGAME;
	s_ingame_save.textcolor					= CT_BLACK;
	s_ingame_save.textcolor2				= CT_WHITE;

	if (!ui.SG_GameAllowedToSaveHere(qfalse))//full check of system, not just if inCamera
	{//shouldn't be able to get here anymore becuase the menu is inhibited if inCamera
		s_ingame_save.generic.flags			|= QMF_GRAYED;
	}

	s_ingame_load.generic.type				= MTYPE_BITMAP;      


	if (ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)
	{
		if (ui.SG_ValidateForLoadSaveScreen("virtual"))
		{
			s_ingame_load.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
		}
		else
		{
			s_ingame_load.generic.flags				= QMF_GRAYED;
		}
	}
	else
	{
		s_ingame_load.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	}

	s_ingame_load.generic.x					= ingame_buttons[2][0];
	s_ingame_load.generic.y					= ingame_buttons[2][1];
	s_ingame_load.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_ingame_load.generic.id					= ID_IMLOADGAME;
	s_ingame_load.generic.callback			= InGame_Event; 
	s_ingame_load.width						= MENU_BUTTON_MED_WIDTH;
	s_ingame_load.height						= MENU_BUTTON_MED_HEIGHT;
	s_ingame_load.color						= CT_DKPURPLE1;
	s_ingame_load.color2					= CT_LTPURPLE1;
	s_ingame_load.textX						= MENU_BUTTON_TEXT_X;
	s_ingame_load.textY						= MENU_BUTTON_TEXT_Y;
	s_ingame_load.textEnum					= MBT_IMLOADGAME;
	s_ingame_load.textcolor					= CT_BLACK;
	s_ingame_load.textcolor2				= CT_WHITE;

	s_ingame_setup.generic.type				= MTYPE_BITMAP;      
	s_ingame_setup.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_ingame_setup.generic.x				= ingame_buttons[3][0];
	s_ingame_setup.generic.y				= ingame_buttons[3][1];
	s_ingame_setup.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_ingame_setup.generic.id				= ID_INGAMECUSTOMIZECONTROLS;
	s_ingame_setup.generic.callback			= M_Main_Event; 
	s_ingame_setup.width					= MENU_BUTTON_MED_WIDTH;
	s_ingame_setup.height					= MENU_BUTTON_MED_HEIGHT;
	s_ingame_setup.color					= CT_DKPURPLE1;
	s_ingame_setup.color2					= CT_LTPURPLE1;
	s_ingame_setup.textX					= MENU_BUTTON_TEXT_X;
	s_ingame_setup.textY					= MENU_BUTTON_TEXT_Y;
	s_ingame_setup.textEnum					= MBT_CONFIGURE;
	s_ingame_setup.textcolor				= CT_BLACK;
	s_ingame_setup.textcolor2				= CT_WHITE;

	s_leavegame.generic.type				= MTYPE_BITMAP;      
	s_leavegame.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_leavegame.generic.x					= ingame_buttons[4][0];
	s_leavegame.generic.y					= ingame_buttons[4][1];
	s_leavegame.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_leavegame.generic.id					= ID_LEAVEGAME;
	s_leavegame.generic.callback			= InGame_Event; 
	s_leavegame.width						= MENU_BUTTON_MED_WIDTH;
	s_leavegame.height						= MENU_BUTTON_MED_HEIGHT;
	s_leavegame.color						= CT_DKPURPLE1;
	s_leavegame.color2						= CT_LTPURPLE1;
	s_leavegame.textX						= MENU_BUTTON_TEXT_X;
	s_leavegame.textY						= MENU_BUTTON_TEXT_Y;
	s_leavegame.textEnum					= MBT_QUITGAME;
	s_leavegame.textcolor					= CT_BLACK;
	s_leavegame.textcolor2					= CT_WHITE;

	s_quitprog.generic.type					= MTYPE_BITMAP;      
	s_quitprog.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_quitprog.generic.x					= ingame_buttons[5][0];
	s_quitprog.generic.y					= ingame_buttons[5][1];
	s_quitprog.generic.name					= GRAPHIC_BUTTONRIGHT;
	s_quitprog.generic.id					= ID_QUIT;
	s_quitprog.generic.callback				= InGame_Event; 
	s_quitprog.width						= MENU_BUTTON_MED_WIDTH;
	s_quitprog.height						= MENU_BUTTON_MED_HEIGHT;
	s_quitprog.color						= CT_DKPURPLE1;
	s_quitprog.color2						= CT_LTPURPLE1;
	s_quitprog.textX						= MENU_BUTTON_TEXT_X;
	s_quitprog.textY						= MENU_BUTTON_TEXT_Y;

	if (ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)
	{
		s_quitprog.textEnum						= MBT_EXITTOUR;
	}
	else
	{	
		s_quitprog.textEnum						= MBT_EXITPROG;
	}

	s_quitprog.textcolor					= CT_BLACK;
	s_quitprog.textcolor2					= CT_WHITE;

	s_screenshot.generic.type				= MTYPE_BITMAP;      
	s_screenshot.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_screenshot.generic.x					= 481;
	s_screenshot.generic.y					= 77;
	s_screenshot.generic.name				= "menu/common/square.tga";
	s_screenshot.generic.id					= ID_SCREENSHOT;
	s_screenshot.generic.callback			= InGame_Event; 
	s_screenshot.width						= MENU_BUTTON_MED_WIDTH;
	s_screenshot.height						= 36;
	s_screenshot.color						= CT_DKPURPLE1;
	s_screenshot.color2						= CT_LTPURPLE1;
	s_screenshot.textX						= MENU_BUTTON_TEXT_X;
	s_screenshot.textY						= MENU_BUTTON_TEXT_Y;
	s_screenshot.textEnum					= MBT_SCREENSHOT;
	s_screenshot.textcolor					= CT_BLACK;
	s_screenshot.textcolor2					= CT_WHITE;


	Menu_AddItem( menu, ( void * ) &s_returntogame );
	Menu_AddItem( menu, ( void * ) &s_ingame_save );
	Menu_AddItem( menu, ( void * ) &s_ingame_load );
	Menu_AddItem( menu, ( void * ) &s_ingame_setup );
	Menu_AddItem( menu, ( void * ) &s_leavegame );
	Menu_AddItem( menu, ( void * ) &s_quitprog );
	Menu_AddItem( menu, ( void * ) &s_screenshot );


}



/*
=================
InGame_MenuInit
=================
*/
void InGame_MenuInit( void )
{

	int i;
	int	normalColor,highlightColor;
	int picColor;

	UI_InGameMenu_Cache();

	s_ingame_menu.nitems					= 0;
	s_ingame_menu.wrapAround				= qtrue;
	s_ingame_menu.opening					= NULL;
	s_ingame_menu.closing					= NULL;
	s_ingame_menu.draw						= InGame_MenuDraw;
	s_ingame_menu.key						= InGame_MenuKey;
	s_ingame_menu.fullscreen				= qtrue;
	s_ingame_menu.descX						= MENU_DESC_X;
	s_ingame_menu.descY						= MENU_DESC_Y;
	s_ingame_menu.listX						= 230;
	s_ingame_menu.listY						= 188;
	s_ingame_menu.titleX					= MENU_TITLE_X;
	s_ingame_menu.titleY					= MENU_TITLE_Y;
	s_ingame_menu.titleI					= MNT_INGAMEMENU_TITLE;
	s_ingame_menu.footNoteEnum				= MNT_HAZARDSUIT_SPECS;

	InGame_MenuButtonsInit(&s_ingame_menu);

	normalColor = CT_DKBROWN1;
	highlightColor= CT_WHITE;
	picColor		= CT_LTGOLD1;


	s_ingamemenu.powerconv_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.powerconv_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_ingamemenu.powerconv_label.generic.x					= 148;
	s_ingamemenu.powerconv_label.generic.y					= 228;
	s_ingamemenu.powerconv_label.generic.id					= ID_POWERCONVERTER_LABEL;
	s_ingamemenu.powerconv_label.generic.callback			= InGame_Event; 
	s_ingamemenu.powerconv_label.buttontextEnum				= MBT_POWERCONV;
	s_ingamemenu.powerconv_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_ingamemenu.powerconv_label.color						= normalColor;
	s_ingamemenu.powerconv_label.color2						= highlightColor;

	s_ingamemenu.powerconv_pic.generic.type					= MTYPE_BITMAP;      
	s_ingamemenu.powerconv_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.powerconv_pic.generic.x					= 201;
	s_ingamemenu.powerconv_pic.generic.y					= 263;
	s_ingamemenu.powerconv_pic.generic.name					= "menu/suit/power_conv.tga";
	s_ingamemenu.powerconv_pic.width						= 32;
	s_ingamemenu.powerconv_pic.height						= 32;
	s_ingamemenu.powerconv_pic.color						= picColor;
	
	s_ingamemenu.commbadge_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.commbadge_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.commbadge_label.generic.x					= 252;
	s_ingamemenu.commbadge_label.generic.y					= 185;
	s_ingamemenu.commbadge_label.generic.id					= ID_COMMBADGE_LABEL;
	s_ingamemenu.commbadge_label.generic.callback			= InGame_Event; 
	s_ingamemenu.commbadge_label.buttontextEnum				= MBT_COMMBADGE;
	s_ingamemenu.commbadge_label.style						= UI_TINYFONT;	
	s_ingamemenu.commbadge_label.color						= normalColor;
	s_ingamemenu.commbadge_label.color2						= highlightColor;

	s_ingamemenu.commbadge_pic.generic.type					= MTYPE_BITMAP;      
	s_ingamemenu.commbadge_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.commbadge_pic.generic.x					= 219;
	s_ingamemenu.commbadge_pic.generic.y					= 220;
	s_ingamemenu.commbadge_pic.generic.name					= "menu/suit/combadge.tga";
	s_ingamemenu.commbadge_pic.width						= 16;
	s_ingamemenu.commbadge_pic.height						= 16;
	s_ingamemenu.commbadge_pic.color						= picColor;

	s_ingamemenu.logistics_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.logistics_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.logistics_label.generic.x					= 312;
	s_ingamemenu.logistics_label.generic.y					= 320;
	s_ingamemenu.logistics_label.generic.id					= ID_LOGISTICS_LABEL;
	s_ingamemenu.logistics_label.generic.callback			= InGame_Event; 
	s_ingamemenu.logistics_label.buttontextEnum				= MBT_LOGISTICS;
	s_ingamemenu.logistics_label.style						= UI_TINYFONT;	
	s_ingamemenu.logistics_label.color						= normalColor;
	s_ingamemenu.logistics_label.color2						= highlightColor;

	s_ingamemenu.logistics_pic.generic.type					= MTYPE_BITMAP;      
	s_ingamemenu.logistics_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.logistics_pic.generic.x					= 338;
	s_ingamemenu.logistics_pic.generic.y					= 278;
	s_ingamemenu.logistics_pic.generic.name					= "menu/suit/direct_log_circ.tga";
	s_ingamemenu.logistics_pic.width						= 32;
	s_ingamemenu.logistics_pic.height						= 16;
	s_ingamemenu.logistics_pic.color						= picColor;

	s_ingamemenu.energypack_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.energypack_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.energypack_label.generic.x					= 466;
	s_ingamemenu.energypack_label.generic.y					= 242;
	s_ingamemenu.energypack_label.generic.id				= ID_ENERGYPACK_LABEL;
	s_ingamemenu.energypack_label.generic.callback			= InGame_Event; 
	s_ingamemenu.energypack_label.buttontextEnum			= MBT_ENERGY_PACK;
	s_ingamemenu.energypack_label.style						= UI_TINYFONT;	
	s_ingamemenu.energypack_label.color						= normalColor;
	s_ingamemenu.energypack_label.color2					= highlightColor;

	s_ingamemenu.energypack_pic.generic.type				= MTYPE_BITMAP;      
	s_ingamemenu.energypack_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.energypack_pic.generic.x					= 387;
	s_ingamemenu.energypack_pic.generic.y					= 266;
	s_ingamemenu.energypack_pic.generic.name				= "menu/suit/energy_pack.tga";
	s_ingamemenu.energypack_pic.width						= 32;
	s_ingamemenu.energypack_pic.height						= 16;
	s_ingamemenu.energypack_pic.color						= picColor;

	s_ingamemenu.wavegenerator_label.generic.type			= MTYPE_TEXT;      
	s_ingamemenu.wavegenerator_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.wavegenerator_label.generic.x				= 254;
	s_ingamemenu.wavegenerator_label.generic.y				= 158;
	s_ingamemenu.wavegenerator_label.generic.id				= ID_WAVEGENERATOR_LABEL;
	s_ingamemenu.wavegenerator_label.generic.callback		= InGame_Event; 
	s_ingamemenu.wavegenerator_label.buttontextEnum			= MBT_WAVEGENERATOR;
	s_ingamemenu.wavegenerator_label.style					= UI_TINYFONT;	
	s_ingamemenu.wavegenerator_label.color					= normalColor;
	s_ingamemenu.wavegenerator_label.color2					= highlightColor;

	s_ingamemenu.wavegenerator_pic.generic.type				= MTYPE_BITMAP;      
	s_ingamemenu.wavegenerator_pic.generic.flags			= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.wavegenerator_pic.generic.x				= 183;
	s_ingamemenu.wavegenerator_pic.generic.y				= 206;
	s_ingamemenu.wavegenerator_pic.generic.name				= "menu/suit/multi_wavegen.tga";
	s_ingamemenu.wavegenerator_pic.width					= 64;
	s_ingamemenu.wavegenerator_pic.height					= 16;
	s_ingamemenu.wavegenerator_pic.color					= picColor;

	s_ingamemenu.scanners_label.generic.type			= MTYPE_TEXT;      
	s_ingamemenu.scanners_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.scanners_label.generic.x				= 127;
	s_ingamemenu.scanners_label.generic.y				= 320;
	s_ingamemenu.scanners_label.generic.id				= ID_SCANNERS_LABEL;
	s_ingamemenu.scanners_label.generic.callback		= InGame_Event; 
	s_ingamemenu.scanners_label.buttontextEnum			= MBT_SCANNERS;
	s_ingamemenu.scanners_label.style					= UI_TINYFONT;	
	s_ingamemenu.scanners_label.color					= normalColor;
	s_ingamemenu.scanners_label.color2					= highlightColor;

	s_ingamemenu.scanners_pic.generic.type				= MTYPE_BITMAP;      
	s_ingamemenu.scanners_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.scanners_pic.generic.x					= 155;
	s_ingamemenu.scanners_pic.generic.y					= 278;
	s_ingamemenu.scanners_pic.generic.name				= "menu/suit/pass_acscan.tga";
	s_ingamemenu.scanners_pic.width						= 32;
	s_ingamemenu.scanners_pic.height					= 32;
	s_ingamemenu.scanners_pic.color						= picColor;

	s_ingamemenu.pouches_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.pouches_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.pouches_label.generic.x				= 490;
	s_ingamemenu.pouches_label.generic.y				= 295;
	s_ingamemenu.pouches_label.generic.id				= ID_POUCHES_LABEL;
	s_ingamemenu.pouches_label.generic.callback			= InGame_Event; 
	s_ingamemenu.pouches_label.buttontextEnum			= MBT_POUCHES;
	s_ingamemenu.pouches_label.style					= UI_TINYFONT;	
	s_ingamemenu.pouches_label.color					= normalColor;
	s_ingamemenu.pouches_label.color2					= highlightColor;

	s_ingamemenu.pouches_pic.generic.type				= MTYPE_BITMAP;      
	s_ingamemenu.pouches_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.pouches_pic.generic.x					= 419;
	s_ingamemenu.pouches_pic.generic.y					= 295;
	s_ingamemenu.pouches_pic.generic.name				= "menu/suit/pouches.tga";
	s_ingamemenu.pouches_pic.width						= 16;
	s_ingamemenu.pouches_pic.height						= 32;
	s_ingamemenu.pouches_pic.color						= picColor;


	s_ingamemenu.buffer_label.generic.type				= MTYPE_TEXT;      
	s_ingamemenu.buffer_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_ingamemenu.buffer_label.generic.x					= 472;
	s_ingamemenu.buffer_label.generic.y					= 281;
	s_ingamemenu.buffer_label.generic.id				= ID_BUFFER_LABEL;
	s_ingamemenu.buffer_label.generic.callback			= InGame_Event; 
	s_ingamemenu.buffer_label.buttontextEnum			= MBT_BUFFER;
	s_ingamemenu.buffer_label.style						= UI_TINYFONT;	
	s_ingamemenu.buffer_label.color						= normalColor;
	s_ingamemenu.buffer_label.color2					= highlightColor;

	s_ingamemenu.buffer_pic.generic.type				= MTYPE_BITMAP;      
	s_ingamemenu.buffer_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_ingamemenu.buffer_pic.generic.x					= 402;
	s_ingamemenu.buffer_pic.generic.y					= 264;
	s_ingamemenu.buffer_pic.generic.name				= "menu/suit/trans_buff.tga";
	s_ingamemenu.buffer_pic.width						= 16;
	s_ingamemenu.buffer_pic.height						= 16;
	s_ingamemenu.buffer_pic.color						= picColor;

	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.powerconv_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.powerconv_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.commbadge_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.commbadge_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.logistics_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.logistics_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.energypack_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.energypack_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.wavegenerator_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.wavegenerator_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.scanners_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.scanners_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.pouches_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.pouches_pic);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.buffer_label);
	Menu_AddItem( &s_ingame_menu, &s_ingamemenu.buffer_pic);

	// Turn off all the blinkie stuff
	for (i=0;i<IGMG_MAX;++i)
	{
		if ((ingamemenu_graphics[i].type == MG_GRAPHIC) || (ingamemenu_graphics[i].type == MG_STRING))
		{
			ingamemenu_graphics[i].type = MG_OFF;
		}
	}

	// Point to first system
	ingamemenu_graphics[IGMG_ACTIVE_SYSTEM].target = IGMG_LABEL_START + 1;

}

/*
=================
UI_InGameMenu
=================
*/
void UI_InGameMenu(const char*holoFlag)
{
	//this also kills music, bad.
//	ui.Cmd_ExecuteText( EXEC_APPEND, "stopsound\n" );	//	Kill all sounds currently running

	// See if in a holomatch map
	holoMatch = qfalse;
	if (holoFlag)
	{
		if (!_strcmpi(holoFlag,"HOLO"))
		{
			holoMatch = qtrue;
		}
	}

	ui.PrecacheScreenshot();

	uis.menusp = 0;

	ingameFlag = qtrue;	// true when in game menu is in use

	Mouse_Show();

	InGame_MenuInit();

	UI_PushMenu( &s_ingame_menu );	

	Menu_AdjustCursor( &s_ingame_menu, 1 );	
}

/*
=======================================================================

MAIN MENU

=======================================================================
*/
qboolean		loadModelFlag;
float			loadModelTime;
menuframework_s s_main_menu;

#define ID_TEXTLANGUAGE		102
#define ID_VOICELANGUAGE	103
#define ID_KEYBOARDLANGUAGE	104

static const char *s_keyboardlanguage_Names[] =
{
	"AMERICAN",
	"DEUTSCH",
	"FRANCAIS",
	"ESPANOL",
	"ITALIANO",
	0
};

static struct 
{
	menuframework_s		menu;

	menulist_s			textlanguage;
	menulist_s			voicelanguage;
	menulist_s			keyboardlanguage;

	menuslider_s		gamma;
	qhandle_t			test;

	menuaction_s		apply;
} s_initialsetup;

menubitmap_s	s_newgame;
menubitmap_s	s_loadgame;
menubitmap_s s_holomatch;

menubitmap_s	s_setup;
menubitmap_s	s_explorevoyager;
menubitmap_s	s_credits;


/*
=================
InitialSetup_Event
=================
*/
static void InitialSetup_Event( void* ptr, int notification )
{
	menuframework_s*	m;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	m = ((menucommon_s*)ptr)->parent;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_TEXTLANGUAGE:
			ui.Cvar_Set( "g_language", menu_normal_text[s_textlanguage_Names[s_initialsetup.textlanguage.curvalue]] );
			UI_LoadButtonText();
			UI_LoadMenuText();
			break;
		case ID_VOICELANGUAGE:
			ui.Cvar_Set( "s_language", menu_normal_text[s_textlanguage_Names[s_initialsetup.voicelanguage.curvalue]] );
			break;
		case ID_KEYBOARDLANGUAGE:
			ui.Cvar_Set( "k_language", s_keyboardlanguage_Names[s_initialsetup.keyboardlanguage.curvalue] );
			break;
	}
}

/*
=================
InitialSetupApplyChanges - 
=================
*/
static void InitialSetupApplyChanges( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
	{
		return;
	}

	if (!uis.glconfig.deviceSupportsGamma)
	{
		ui.Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
	}

	ui.Cvar_Set("ui_initialsetup", "1");	// so this won't come up again
	UI_MainMenu();
}

/*
=================
M_InitialSetupMenu_Key
=================
*/
sfxHandle_t M_InitialSetupMenu_Key (int key)
{
	if (key == K_ESCAPE)
	{
		return(0);
	}

	return ( Menu_DefaultKey( &s_initialsetup.menu, key ) );
}

/*
=================
M_InitialSetupMenu_Graphics
=================
*/
void M_InitialSetupMenu_Graphics (void)
{
	int y;

	UI_MenuFrame2(&s_initialsetup.menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203, 47, 186, uis.whiteShader);	// Middle left line

	y = 97;

	if (uis.glconfig.deviceSupportsGamma)
	{
		ui.R_SetColor( colorTable[CT_DKGREY]);
		UI_DrawHandlePic(  178, y, 68, 68, uis.whiteShader);	// Map background

		ui.R_SetColor( colorTable[CT_WHITE]);
		UI_DrawHandlePic(  180, y+2, 64, 64, s_initialsetup.test);	// Starfleet graphic

		UI_DrawProportionalString( 256,  y + 5, menu_normal_text[MNT_GAMMA_LINE1],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 256,  y + 25, menu_normal_text[MNT_GAMMA_LINE2],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 256,  y + 45, menu_normal_text[MNT_GAMMA_LINE3],UI_SMALLFONT,colorTable[CT_LTGOLD1]);
	}
	else
	{
		UI_DrawProportionalString( 178,  y + 5, menu_normal_text[MNT_GAMMA2_LINE1],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
		UI_DrawProportionalString( 178,  y + 25,menu_normal_text[MNT_GAMMA2_LINE2],UI_SMALLFONT, colorTable[CT_LTGOLD1]);
	}
}

/*
=================
M_InitialSetupMenu_Draw
=================
*/
void M_InitialSetupMenu_Draw (void)
{	
	// Draw graphics particular to Main Menu
	M_InitialSetupMenu_Graphics();

	Menu_Draw( &s_initialsetup.menu );
}

/*
===============
InitialSetup_SetValues
===============
*/
void InitialSetup_SetValues(void)
{
	char buffer[32];
	int *language;

	ui.Cvar_VariableStringBuffer( "g_language", buffer, 32 );
	language = s_textlanguage_Names;
	
	s_initialsetup.textlanguage.curvalue=0;
	if (buffer[0]) 
	{
		while (*language)
		{
			if (Q_stricmp(menu_normal_text[*language],buffer)==0)
			{
				break;
			}
			language++;
			s_initialsetup.textlanguage.curvalue++;
		}

		if (!*language)
		{
			s_initialsetup.textlanguage.curvalue = 0;
		}
	}

	ui.Cvar_VariableStringBuffer( "s_language", buffer, 32 );
	language = s_voicelanguage_Names;

	s_initialsetup.voicelanguage.curvalue=0;
	if (buffer[0]) 
	{
		while (*language)
		{
			if (Q_stricmp(menu_normal_text[*language],buffer)==0)
			{
				break;
			}
			language++;
			s_initialsetup.voicelanguage.curvalue++;
		}

		if (!*language)
		{
			s_initialsetup.voicelanguage.curvalue = 0;
		}
	}

	ui.Cvar_VariableStringBuffer( "k_language", buffer, 32 );
	s_initialsetup.keyboardlanguage.curvalue=0;
	if (buffer[0]) 
	{
		while (s_keyboardlanguage_Names[s_initialsetup.keyboardlanguage.curvalue])
		{
			if (Q_stricmp(s_keyboardlanguage_Names[s_initialsetup.keyboardlanguage.curvalue],buffer)==0)
			{
				break;
			}
			s_initialsetup.keyboardlanguage.curvalue++;
		}

		if (!s_keyboardlanguage_Names[s_initialsetup.keyboardlanguage.curvalue])
		{
			s_initialsetup.keyboardlanguage.curvalue = 0;
		}
	}
	
	s_initialsetup.gamma.curvalue = ui.Cvar_VariableValue( "r_gamma" ) *  10.0f;
}

/*
===============
InitialSetupMenu_Cache
===============
*/
void InitialSetupMenu_Cache( void ) 
{

	s_initialsetup.test = ui.R_RegisterShaderNoMip("menu/special/gamma_test.tga");


}

/*
===============
InitialSetupMenu_Init
===============
*/
void InitialSetupMenu_Init( void ) 
{
	int x = 179;
	int y = 260;//279

	InitialSetupMenu_Cache();

	s_initialsetup.menu.nitems					= 0;
	s_initialsetup.menu.wrapAround				= qtrue;
	s_initialsetup.menu.draw					= M_InitialSetupMenu_Draw;
	s_initialsetup.menu.key						= M_InitialSetupMenu_Key;
	s_initialsetup.menu.fullscreen				= qtrue;
	s_initialsetup.menu.descX					= MENU_DESC_X;
	s_initialsetup.menu.descY					= MENU_DESC_Y;
	s_initialsetup.menu.titleX					= MENU_TITLE_X;
	s_initialsetup.menu.titleY					= MENU_TITLE_Y;
	s_initialsetup.menu.titleI					= MNT_INITIALSETUP_TITLE;
	s_initialsetup.menu.footNoteEnum			= MNT_INITIALSETUP;


	s_initialsetup.textlanguage.generic.type		= MTYPE_SPINCONTROL;      
	s_initialsetup.textlanguage.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_initialsetup.textlanguage.generic.x			= x;
	s_initialsetup.textlanguage.generic.y			= y;
	s_initialsetup.textlanguage.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_initialsetup.textlanguage.generic.callback	= InitialSetup_Event; 
	s_initialsetup.textlanguage.generic.id			= ID_TEXTLANGUAGE; 
	s_initialsetup.textlanguage.color				= CT_DKPURPLE1;
	s_initialsetup.textlanguage.color2				= CT_LTPURPLE1;
	s_initialsetup.textlanguage.textX				= MENU_BUTTON_TEXT_X;
	s_initialsetup.textlanguage.textY				= MENU_BUTTON_TEXT_Y;
	s_initialsetup.textlanguage.width				= 80;
	s_initialsetup.textlanguage.textEnum			= MBT_TEXTLANGUAGE;
	s_initialsetup.textlanguage.textcolor			= CT_BLACK;
	s_initialsetup.textlanguage.textcolor2			= CT_WHITE;	
	s_initialsetup.textlanguage.listnames			= s_textlanguage_Names;
	y+=32;

	s_initialsetup.voicelanguage.generic.type		= MTYPE_SPINCONTROL;      
	s_initialsetup.voicelanguage.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_initialsetup.voicelanguage.generic.x			= x;
	s_initialsetup.voicelanguage.generic.y			= y;
	s_initialsetup.voicelanguage.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_initialsetup.voicelanguage.generic.callback	= InitialSetup_Event; 
	s_initialsetup.voicelanguage.generic.id			= ID_VOICELANGUAGE; 
	s_initialsetup.voicelanguage.color				= CT_DKPURPLE1;
	s_initialsetup.voicelanguage.color2				= CT_LTPURPLE1;
	s_initialsetup.voicelanguage.textX				= MENU_BUTTON_TEXT_X;
	s_initialsetup.voicelanguage.textY				= MENU_BUTTON_TEXT_Y;
	s_initialsetup.voicelanguage.width				= 80;
	s_initialsetup.voicelanguage.textEnum			= MBT_VOICELANGUAGE;
	s_initialsetup.voicelanguage.textcolor			= CT_BLACK;
	s_initialsetup.voicelanguage.textcolor2			= CT_WHITE;	
	s_initialsetup.voicelanguage.listnames			= s_voicelanguage_Names;
	y+=32;

	s_initialsetup.keyboardlanguage.generic.type		= MTYPE_SPINCONTROL;      
	s_initialsetup.keyboardlanguage.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_initialsetup.keyboardlanguage.generic.x			= x;
	s_initialsetup.keyboardlanguage.generic.y			= y;
	s_initialsetup.keyboardlanguage.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_initialsetup.keyboardlanguage.generic.callback	= InitialSetup_Event; 
	s_initialsetup.keyboardlanguage.generic.id			= ID_KEYBOARDLANGUAGE; 
	s_initialsetup.keyboardlanguage.color				= CT_DKPURPLE1;
	s_initialsetup.keyboardlanguage.color2				= CT_LTPURPLE1;
	s_initialsetup.keyboardlanguage.textX				= MENU_BUTTON_TEXT_X;
	s_initialsetup.keyboardlanguage.textY				= MENU_BUTTON_TEXT_Y;
	s_initialsetup.keyboardlanguage.width				= 80;
	s_initialsetup.keyboardlanguage.textEnum			= MBT_KEYBOARDLANGUAGE;
	s_initialsetup.keyboardlanguage.textcolor			= CT_BLACK;
	s_initialsetup.keyboardlanguage.textcolor2			= CT_WHITE;	
	s_initialsetup.keyboardlanguage.itemnames			= s_keyboardlanguage_Names;

	x = 180;
	y = 182;
	s_initialsetup.gamma.generic.type			= MTYPE_SLIDER;
	s_initialsetup.gamma.generic.x				= x + 162;
	s_initialsetup.gamma.generic.y				= y;
	s_initialsetup.gamma.generic.flags			= QMF_SMALLFONT;
	s_initialsetup.gamma.generic.callback		= GammaCallback;
	s_initialsetup.gamma.minvalue				= 5;
	s_initialsetup.gamma.maxvalue				= 30;
	s_initialsetup.gamma.color					= CT_DKPURPLE1;
	s_initialsetup.gamma.color2					= CT_LTPURPLE1;
	s_initialsetup.gamma.generic.name			= "menu/common/monbar_2.tga";
	s_initialsetup.gamma.width					= 256;
	s_initialsetup.gamma.height					= 32;
	s_initialsetup.gamma.focusWidth				= 145;
	s_initialsetup.gamma.focusHeight			= 18;
	s_initialsetup.gamma.picName				= "menu/common/square.tga";
	s_initialsetup.gamma.picX					= x;
	s_initialsetup.gamma.picY					= y;
	s_initialsetup.gamma.picWidth				= MENU_BUTTON_MED_WIDTH + 21;
	s_initialsetup.gamma.picHeight				= MENU_BUTTON_MED_HEIGHT;
	s_initialsetup.gamma.textX					= MENU_BUTTON_TEXT_X;
	s_initialsetup.gamma.textY					= MENU_BUTTON_TEXT_Y;
	s_initialsetup.gamma.textEnum				= MBT_BRIGHTNESS;
	s_initialsetup.gamma.textcolor				= CT_BLACK;
	s_initialsetup.gamma.textcolor2				= CT_WHITE;
	s_initialsetup.gamma.thumbName				= GRAPHIC_BUTTONSLIDER;
	s_initialsetup.gamma.thumbHeight			= 32;
	s_initialsetup.gamma.thumbWidth				= 16;
	s_initialsetup.gamma.thumbGraphicWidth		= 9;
	s_initialsetup.gamma.thumbColor				= CT_DKBLUE1;
	s_initialsetup.gamma.thumbColor2			= CT_LTBLUE1;


	s_initialsetup.apply.generic.type				= MTYPE_ACTION;
	s_initialsetup.apply.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS|QMF_BLINK;
	s_initialsetup.apply.generic.x					= 501;
	s_initialsetup.apply.generic.y					= 357;
	s_initialsetup.apply.generic.callback			= InitialSetupApplyChanges;
	s_initialsetup.apply.textEnum					= MBT_ACCEPT;
	s_initialsetup.apply.textcolor					= CT_BLACK;
	s_initialsetup.apply.textcolor2					= CT_WHITE;
	s_initialsetup.apply.textcolor3					= CT_LTGREY;
	s_initialsetup.apply.color						= CT_DKPURPLE1;
	s_initialsetup.apply.color2						= CT_LTPURPLE1;
	s_initialsetup.apply.color3						= CT_DKGREY;
	s_initialsetup.apply.textX						= 5;
	s_initialsetup.apply.textY						= 47;
	s_initialsetup.apply.width						= 110;
	s_initialsetup.apply.height						= 65;


	Menu_AddItem( &s_initialsetup.menu, &s_initialsetup.gamma );
	Menu_AddItem( &s_initialsetup.menu, &s_initialsetup.textlanguage );
	Menu_AddItem( &s_initialsetup.menu, &s_initialsetup.voicelanguage );
	Menu_AddItem( &s_initialsetup.menu, &s_initialsetup.keyboardlanguage );
	Menu_AddItem( &s_initialsetup.menu, &s_initialsetup.apply );

	InitialSetup_SetValues();
}

void UI_InitialSetupMenu(void)
{
	InitialSetupMenu_Init(); 

	UI_PushMenu ( &s_initialsetup.menu );

	ui.Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
}


/*
=================
MainMenu_ChangeAreaFocus
=================
*/
void MainMenu_ChangeAreaFocus(int newArea)
{
	int sectionI,symbolI,descI;
	menubitmap_s *bitmap;

	// Turn off old area of focus
	sectionI = mainmenu_graphics[MMG_SECTIONS_BEGIN].target;
	symbolI = mainmenu_graphics[sectionI].min;
	descI = mainmenu_graphics[sectionI].max;

	// Turn off symbol
	if (symbolI)
	{
		mainmenu_graphics[symbolI].type = MG_OFF;
	}

	// Turn off description
	if (descI)
	{
		mainmenu_graphics[descI].type = MG_OFF;
	}

	// Change label to dark color
	if (mainmenu_graphics[sectionI].pointer)
	{
		bitmap = (menubitmap_s *) mainmenu_graphics[sectionI].pointer;
		bitmap->textcolor = CT_DKGOLD1;
	}

	mainmenu_graphics[mainmenu_graphics[MMG_SECTIONS_BEGIN].target].color = CT_DKGOLD1;

	// Advance to new area
	mainmenu_graphics[MMG_SECTIONS_BEGIN].target = newArea;

	// Turn on new area
	sectionI = mainmenu_graphics[MMG_SECTIONS_BEGIN].target;
	symbolI = mainmenu_graphics[sectionI].min;
	descI = mainmenu_graphics[sectionI].max;

	if (symbolI)
	{
		mainmenu_graphics[symbolI].type = MG_GRAPHIC;
	}

	if (descI)
	{
		mainmenu_graphics[descI].type = MG_STRING;
	}

	if (mainmenu_graphics[sectionI].pointer)
	{
		bitmap = (menubitmap_s *) mainmenu_graphics[sectionI].pointer;
		bitmap->textcolor = CT_YELLOW;
	}
}

/*
=================
MainMenu_Blinkies
=================
*/
void MainMenu_Blinkies(void)
{
	int i;
	float timer;
	int newArea;

	// Calculate ARROW positions
	for (i=MMG_ARROW1;i<=MMG_ARROW4 ;++i)
	{
		// Time to move??
		if ( mainmenu_graphics[i].timer < uis.realtime )
		{
			mainmenu_graphics[i].y += mainmenu_graphics[i].inc;

			if (mainmenu_graphics[i].inc > 0)	// Moving higher
			{
				if (mainmenu_graphics[i].y > mainmenu_graphics[i].target)
				{
					mainmenu_graphics[i].y = mainmenu_graphics[i].target;
				}
			}
			else if (mainmenu_graphics[i].inc < 0)	// Moving lower
			{
				if (mainmenu_graphics[i].y < mainmenu_graphics[i].target)
				{
					mainmenu_graphics[i].y = mainmenu_graphics[i].target;
				}
			}

			mainmenu_graphics[i].timer = uis.realtime + 50;
		}
	}


	// Advance to new area to brighten up
	if (mainmenu_graphics[MMG_SECTIONS_BEGIN].timer < uis.realtime)
	{
		ui.S_StartLocalSound( uis.menu_choice1_snd, CHAN_LOCAL_SOUND );

		// Advance to new area of focus
		newArea = mainmenu_graphics[MMG_SECTIONS_BEGIN].target + 1;

		// Within range???
		if (newArea >= MMG_SECTIONS_END)
		{
			newArea = MMG_SECTIONS_BEGIN + 1;
		}	

		MainMenu_ChangeAreaFocus(newArea);

		// Five seconds from now, do it again
		mainmenu_graphics[MMG_SECTIONS_BEGIN].timer = uis.realtime + 5000;

		// Change arrows y positions and increment speed
		for (i=MMG_ARROW1;i<=MMG_ARROW4 ;++i)
		{
			mainmenu_graphics[i].target =  (random() * (mainmenu_graphics[i].max - mainmenu_graphics[i].min))
				+	mainmenu_graphics[i].min;

			if (mainmenu_graphics[i].y > mainmenu_graphics[i].target)
			{
				mainmenu_graphics[i].inc = -1;
			}
			else
			{
				mainmenu_graphics[i].inc = 1;
			}
		}

		// Make right hand numbers change
		mainmenu_graphics[MMG_RH_COL1_NUM1].timer = uis.realtime;

		for (i=0;i<6;++i)
		{
			mainmenu_graphics[i + MMG_RH_COL1_NUM1].target = UI_RandomNumbers(9);
			mainmenu_graphics[i + MMG_RH_COL1_NUM1].color = CT_DKGOLD1;
			mainmenu_graphics[i + MMG_RH_COL2_NUM1].target = UI_RandomNumbers(9);
			mainmenu_graphics[i + MMG_RH_COL2_NUM1].color = CT_DKGOLD1;
		}

		i = (random() * (MMG_RH_COL1_NUM6 - MMG_RH_COL1_NUM1)) + MMG_RH_COL1_NUM1;
		mainmenu_graphics[i].color = CT_LTGOLD1;

		// Setup  special dot
		mainmenu_graphics[MMG_RH_COL1_SPECNUM1].x = mainmenu_graphics[i].x - 4;
		mainmenu_graphics[MMG_RH_COL1_SPECNUM1].y = mainmenu_graphics[i].y + 6;

		i = (random() * (MMG_RH_COL2_NUM6 - MMG_RH_COL2_NUM1)) + MMG_RH_COL2_NUM1;
		mainmenu_graphics[i].color = CT_LTGOLD1;

		// Setup  special dot
		mainmenu_graphics[MMG_RH_COL2_SPECNUM1].x = mainmenu_graphics[i].x - 4;
		mainmenu_graphics[MMG_RH_COL2_SPECNUM1].y = mainmenu_graphics[i].y + 6;


		for (i=0;i<3;++i)
		{
			mainmenu_graphics[i + MMG_RH_COL3_NUM1].target =  UI_RandomNumbers(9);
			mainmenu_graphics[i + MMG_RH_COL3_NUM1].color = CT_DKGOLD1;
			mainmenu_graphics[i + MMG_RH_COL4_NUM1].target =  UI_RandomNumbers(9);
			mainmenu_graphics[i + MMG_RH_COL4_NUM1].color = CT_DKGOLD1;
		}

		i = (random() * (MMG_RH_COL3_NUM3 - MMG_RH_COL3_NUM1)) + MMG_RH_COL3_NUM1;
		mainmenu_graphics[i].color = CT_LTGOLD1;

		// Setup  special dot
		mainmenu_graphics[MMG_RH_COL3_SPECNUM1].x = mainmenu_graphics[i].x - 4;
		mainmenu_graphics[MMG_RH_COL3_SPECNUM1].y = mainmenu_graphics[i].y + 6;

		i = (random() * (MMG_RH_COL4_NUM3 - MMG_RH_COL4_NUM1)) + MMG_RH_COL4_NUM1;
		mainmenu_graphics[i].color = CT_LTGOLD1;

		// Setup  special dot
		mainmenu_graphics[MMG_RH_COL4_SPECNUM1].x = mainmenu_graphics[i].x - 4;
		mainmenu_graphics[MMG_RH_COL4_SPECNUM1].y = mainmenu_graphics[i].y + 6;

	}


	// Reset numbers in right hand column
	for (i=0;i<6;++i)
	{
		mainmenu_graphics[i + MMG_RH_COL1_NUM1].type = MG_NONE;
		mainmenu_graphics[i + MMG_RH_COL2_NUM1].type = MG_NONE;
	}

	for (i=0;i<3;++i)
	{
		mainmenu_graphics[i + MMG_RH_COL3_NUM1].type = MG_NONE;
		mainmenu_graphics[i + MMG_RH_COL4_NUM1].type = MG_NONE;
	}


	// Turn off dots
	mainmenu_graphics[MMG_RH_COL1_SPECNUM1].type = MG_NONE;
	mainmenu_graphics[MMG_RH_COL2_SPECNUM1].type = MG_NONE;
	mainmenu_graphics[MMG_RH_COL3_SPECNUM1].type = MG_NONE;
	mainmenu_graphics[MMG_RH_COL4_SPECNUM1].type = MG_NONE;


	timer = (uis.realtime - mainmenu_graphics[MMG_RH_COL1_NUM1].timer);
	timer /= 50; 
	if (timer > 6)
	{
		timer = 6;
	}
	else 
	{
//		ui.S_StartLocalSound( uis.menu_datadisp1_snd, CHAN_LOCAL_SOUND );
	}

	for (i=0;i<timer;++i)
	{
		mainmenu_graphics[i + MMG_RH_COL1_NUM1].type = MG_NUMBER;
		if (mainmenu_graphics[i + MMG_RH_COL1_NUM1].color == CT_LTGOLD1)
		{
			mainmenu_graphics[MMG_RH_COL1_SPECNUM1].type = MG_GRAPHIC;
		}

		mainmenu_graphics[i + MMG_RH_COL2_NUM1].type = MG_NUMBER;
		if (mainmenu_graphics[i + MMG_RH_COL2_NUM1].color == CT_LTGOLD1)
		{
			mainmenu_graphics[MMG_RH_COL2_SPECNUM1].type = MG_GRAPHIC;
		}
	}

	for (i=0;i<timer;++i)
	{
		if (i > 2)
			break;

		mainmenu_graphics[i + MMG_RH_COL3_NUM1].type = MG_NUMBER;
		if (mainmenu_graphics[i + MMG_RH_COL3_NUM1].color == CT_LTGOLD1)
		{
			mainmenu_graphics[MMG_RH_COL3_SPECNUM1].type = MG_GRAPHIC;
		}

		mainmenu_graphics[i + MMG_RH_COL4_NUM1].type = MG_NUMBER;
		if (mainmenu_graphics[i + MMG_RH_COL4_NUM1].color == CT_LTGOLD1)
		{
			mainmenu_graphics[MMG_RH_COL4_SPECNUM1].type = MG_GRAPHIC;
		}
	}


	// Generate new numbers for top right
/*	if ((mainmenu_graphics[MMG_TOP_NUMBERS].timer < uis.realtime) && (mainmenu_graphics[MMG_TOP_NUMBERS].target==5))
	{

		for (i=0;i<5;++i)
		{
			mainmenu_graphics[i + MMG_TOP_COL1_NUM1].target = UI_RandomNumbers(3);
			mainmenu_graphics[i + MMG_TOP_COL2_NUM1].target = UI_RandomNumbers(3);
			mainmenu_graphics[i + MMG_TOP_COL3_NUM1].target = UI_RandomNumbers(3);
			mainmenu_graphics[i + MMG_TOP_COL4_NUM1].target = UI_RandomNumbers(3);
			mainmenu_graphics[i + MMG_TOP_COL5_NUM1].target = UI_RandomNumbers(3);
		}

		mainmenu_graphics[MMG_TOP_NUMBERS].target=1;

		// Turn off all but the first row
		for (i=0;i<4;++i)
		{
			mainmenu_graphics[i + MMG_TOP_COL1_NUM2].type = MG_OFF;
			mainmenu_graphics[i + MMG_TOP_COL2_NUM2].type = MG_OFF;
			mainmenu_graphics[i + MMG_TOP_COL3_NUM2].type = MG_OFF;
			mainmenu_graphics[i + MMG_TOP_COL4_NUM2].type = MG_OFF;
			mainmenu_graphics[i + MMG_TOP_COL5_NUM2].type = MG_OFF;
		}

		// Unhigh light old row
		i = mainmenu_graphics[MMG_SPEC_TOP_ROW].target;
		mainmenu_graphics[i + MMG_TOP_COL1_NUM1].color = CT_DKPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL2_NUM1].color = CT_DKPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL3_NUM1].color = CT_DKPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL4_NUM1].color = CT_DKPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL5_NUM1].color = CT_DKPURPLE2;

		// Choose a special row to highlight
		i = mainmenu_graphics[MMG_SPEC_TOP_ROW].target = random() * 5;

		if (i>4)
		{
			i = mainmenu_graphics[MMG_SPEC_TOP_ROW].target = 4;
		}
		// High light that row
		mainmenu_graphics[i + MMG_TOP_COL1_NUM1].color = CT_LTPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL2_NUM1].color = CT_LTPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL3_NUM1].color = CT_LTPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL4_NUM1].color = CT_LTPURPLE2;
		mainmenu_graphics[i + MMG_TOP_COL5_NUM1].color = CT_LTPURPLE2;
		mainmenu_graphics[MMG_SPEC_TOP_ROW].y = mainmenu_graphics[i + MMG_TOP_COL1_NUM1].y + 6;

	}
	// Activate the next row of numbers.  When at the last row wait and then generate new numbers
	else if ((mainmenu_graphics[MMG_TOP_NUMBERS].timer < uis.realtime) && (mainmenu_graphics[MMG_TOP_NUMBERS].target<5))
	{
		mainmenu_graphics[MMG_TOP_NUMBERS].timer = uis.realtime + 50;

		i = mainmenu_graphics[MMG_TOP_NUMBERS].target;
		mainmenu_graphics[i + MMG_TOP_COL1_NUM1].type = MG_NUMBER;
		mainmenu_graphics[i + MMG_TOP_COL2_NUM1].type = MG_NUMBER;
		mainmenu_graphics[i + MMG_TOP_COL3_NUM1].type = MG_NUMBER;
		mainmenu_graphics[i + MMG_TOP_COL4_NUM1].type = MG_NUMBER;
		mainmenu_graphics[i + MMG_TOP_COL5_NUM1].type = MG_NUMBER;

		++mainmenu_graphics[MMG_TOP_NUMBERS].target;

		if (mainmenu_graphics[MMG_TOP_NUMBERS].target == 5)
		{
			mainmenu_graphics[MMG_TOP_NUMBERS].timer = uis.realtime + 6000;
		}
	}
	*/
}

/*
=================
M_MainMenu_GraphicsTurnOff
=================
*/
void M_MainMenu_GraphicsTurnOff (void)
{
	int i;

	// The things I gotta do to make screens look pretty . . . 
	mainmenu_graphics[MMG_ARROW1].type = MG_OFF;
	mainmenu_graphics[MMG_ARROW2].type = MG_OFF;
	mainmenu_graphics[MMG_ARROW3].type = MG_OFF;
	mainmenu_graphics[MMG_ARROW4].type = MG_OFF;

	mainmenu_graphics[MMG_GRAPH_BAR1].type = MG_OFF;
	mainmenu_graphics[MMG_GRAPH_BAR2].type = MG_OFF;
	mainmenu_graphics[MMG_GRAPH_BAR3].type = MG_OFF;
	mainmenu_graphics[MMG_GRAPH_BAR4].type = MG_OFF;

	for (i=MMG_SYMBOLS_BEGIN + 1;i<MMG_SYMBOLS_END;i++)
	{
		mainmenu_graphics[i].type = MG_OFF;
	}

	for (i=MMG_SECTION_TEXT_BEGIN + 1;i<MMG_SECTION_TEXT_END;i++)
	{
		mainmenu_graphics[i].type = MG_OFF;
	}
	s_alphaquad_label.generic.flags = QMF_HIDDEN;
	s_betaquad_label.generic.flags = QMF_HIDDEN;
	s_deltaquad_label.generic.flags = QMF_HIDDEN;
	s_gammaquad_label.generic.flags = QMF_HIDDEN;
	s_federation_label.generic.flags = QMF_HIDDEN;
	s_cardassian_label.generic.flags = QMF_HIDDEN;
	s_ferengi_label.generic.flags = QMF_HIDDEN;
	s_romulan_label.generic.flags = QMF_HIDDEN;
	s_klingon_label.generic.flags = QMF_HIDDEN;
	s_dominion_label.generic.flags = QMF_HIDDEN;
	s_voyager_label.generic.flags = QMF_HIDDEN;
	s_borg_label.generic.flags = QMF_HIDDEN;
	s_wormhole_label.generic.flags = QMF_HIDDEN;
	s_core_label.generic.flags = QMF_HIDDEN;

	if (loadModelInitialized)	// Model has already been loaded
	{
		loadModelTime = 0;
	}
}

/*
=================
M_MainMenu_Graphics
=================
*/
void M_MainMenu_Graphics (void)
{

	// Draw the basic screen layout
	UI_MenuFrame(&s_main_menu);

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic( 482, 136,  MENU_BUTTON_MED_WIDTH - 22, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);
	UI_DrawHandlePic( 460 + MENU_BUTTON_MED_WIDTH - 4, 136,  -19,  MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right

	ui.R_SetColor( colorTable[CT_LTBROWN1]);
	UI_DrawHandlePic(30,203,  47, 186, uis.whiteShader);	// Long left column square on bottom 3rd


	// Numbers on frame
	UI_DrawProportionalString(  74,  66, "7617",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "4396",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  188, "81453",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  206, "93433",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "431108",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	UI_DrawProportionalString( 584, 142, "2112",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);


	if (!loadModelFlag)
	{
		ui.R_SetColor( colorTable[CT_WHITE]);
		UI_DrawHandlePic(  168,  168,  256,  256, quadrants);

		//Left Bracket around galaxy picture
		ui.R_SetColor( colorTable[CT_DKPURPLE2]);
		UI_DrawHandlePic(105,169, 16, 16, uis.graphicBracket1CornerLU);
		UI_DrawHandlePic(105,185,  8, 87, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKBROWN1]);
		UI_DrawHandlePic(105,275,  8, 10, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_LTORANGE]);
		UI_DrawHandlePic(107,288,  6, 21, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKBROWN1]);
		UI_DrawHandlePic(105,312,  8, 10, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKPURPLE2]);
		UI_DrawHandlePic(105,325,  8, 87, uis.whiteShader);
		UI_DrawHandlePic(105,412, 16, -16, uis.graphicBracket1CornerLU);	//LD


		//Right Bracket around galaxy picture
		ui.R_SetColor( colorTable[CT_DKPURPLE2]);
		UI_DrawHandlePic(485,169, -16, 16, uis.graphicBracket1CornerLU);	//RU
		UI_DrawHandlePic(493,185,  8, 87, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKBROWN1]);
		UI_DrawHandlePic(493,275,  8, 10, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_LTORANGE]);
		UI_DrawHandlePic(493,288,  6, 21, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKBROWN1]);
		UI_DrawHandlePic(493,312,  8, 10, uis.whiteShader);

		ui.R_SetColor( colorTable[CT_DKPURPLE2]);
		UI_DrawHandlePic(493,325,  8, 87, uis.whiteShader);
		UI_DrawHandlePic(485,412, -16, -16, uis.graphicBracket1CornerLU);	//RD
	}
	else
	{

		// Current save directory box
		ui.R_SetColor( colorTable[CT_VDKPURPLE1]);
		UI_DrawHandlePic( 84, 188, -32,	32, directoryUpperCorner);	// UL 
		UI_DrawHandlePic( 84, 392, -32,	32, directoryLowerCorner);	// LL
		UI_DrawHandlePic(458, 188,  32,	32, directoryUpperCorner);	// UR
		UI_DrawHandlePic(458, 392,  32,	32, directoryLowerCorner);	// LR

		UI_DrawHandlePic(114, 188, 346,  18, uis.whiteShader);	// Top of box

		UI_DrawHandlePic( 462, 206,  16,  66, uis.whiteShader);	// Top Right side
		UI_DrawHandlePic( 462, 275,  16,  50, uis.whiteShader);	// Middle Right side
		UI_DrawHandlePic( 462, 328,  16,  66, uis.whiteShader);	// Bottom Right side

		UI_DrawHandlePic(  96, 206,  16,  66, uis.whiteShader);	// Top Left side
		UI_DrawHandlePic(  96, 275,  16,  50, uis.whiteShader);	// Middle Left side
		UI_DrawHandlePic(  96, 328,  16,  66, uis.whiteShader);	// Bottom Left side

		UI_DrawHandlePic(114, 396, 346,   8, uis.whiteShader);	// Bottom  box

		UI_DrawProportionalString( 288,  288, menu_normal_text[MNT_ACCESSING],UI_SMALLFONT|UI_CENTER, colorTable[CT_LTGOLD1]);
	}

	MainMenu_Blinkies();

	UI_PrintMenuGraphics(mainmenu_graphics,MMG_MAX);

	if ((loadModelFlag) && (loadModelTime < uis.realtime))
	{
		UI_SetupWeaponsMenu();
		loadModelInitialized = qtrue;
	}
}

/*
=================
UI_Setup_MenuButtons
=================
*/
void UI_Setup_MenuButtons(void)
{
	int i,max;

	ui.R_SetColor( colorTable[CT_DKPURPLE1]);

	if (ui.Cvar_VariableValue("developer"))	// Show fonts button
	{
		max=6;
	}
	else
	{
		max=5;
	}

	for (i=0;i<max;++i)
	{
		UI_DrawHandlePic(setup_menubuttons[i][0] - 14,setup_menubuttons[i][1], MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);
	}

}


/*
=================
UI_MainMenuButtons
=================
*/
void UI_MainMenuButtons(int count)
{
	int i;

	ui.R_SetColor( colorTable[CT_DKPURPLE1]);

	for (i=0;i<count;++i)
	{
		UI_DrawHandlePic(mm_buttons[i][0] - 14,mm_buttons[i][1], MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);
	}
}


/*
=================
MainMenuInterrupt
=================
*/
void MainMenuInterrupt(int labelId)
{
	int newArea;

	switch(labelId)
	{
	case ID_FEDERATION_LABEL :
		newArea = MMG_FEDERATION;
		break;
	case ID_CARDASSIAN_LABEL :
		newArea = MMG_CARDASSIA;
		break;
	case ID_FERENGI_LABEL :
		newArea = MMG_FERENGINAR;
		break;
	case ID_ROMULAN_LABEL :
		newArea = MMG_ROMULANEMPIRE;
		break;
	case ID_KLINGON_LABEL :
		newArea = MMG_KLINGONEMPIRE;
		break;
	case ID_VOYAGER_LABEL :
		newArea = MMG_VOYAGER;
		break;
	case ID_BORG_LABEL :
		newArea = MMG_BORG;
		break;
	case ID_WORMHOLE_LABEL :
		newArea = MMG_WORMHOLE;
		break;
	case ID_DOMINION_LABEL :
		newArea = MMG_DOMINION;
		break;
	case ID_CORE_LABEL :
		newArea = MMG_GALACTICCORE;
		break;
	case ID_ALHAPQUAD_LABEL :
		newArea = MMG_ALPHAQUAD;
		break;
	case ID_BETAQUAD_LABEL :
		newArea = MMG_BETAQUAD;
		break;
	case ID_DELTAQUAD_LABEL :
		newArea = MMG_DELTAQUAD;
		break;
	case ID_GAMMAQUAD_LABEL :
		newArea = MMG_GAMMAQUAD;
		break;
	}

	MainMenu_ChangeAreaFocus(newArea);

	// ten seconds from now, start the auto animation again
	mainmenu_graphics[MMG_SECTIONS_BEGIN].timer = uis.realtime + 10000;
}

/*
=================
M_Main_Opening
=================
*/
void M_Main_Opening (void)
{
	int i,iMax, addX;
	float holdTime;
	menubitmap_s *bitMap;
	float	x;
	float	y;
	static int iMaxHold;
	int	buttonCnt;

	M_MainMenu_Graphics();

	buttonCnt = 8;

	//  Step 1 : Button ends appear 
	if (!s_main_menu.subSeqStatus[0])
	{
		ui.R_SetColor( colorTable[CT_DKPURPLE1]);

		// Figure out time since this section started
		holdTime = uis.realtime - s_main_menu.openingStart;
		iMax = (int) (holdTime / 100);	// A tenth of a second per button
		if (iMax > buttonCnt)
		{
			iMax = buttonCnt;
		}
		else if (iMax <= 0)
		{
			iMax = 1;
		}

		if (iMaxHold != iMax)
		{
			iMaxHold = iMax;
			ui.S_StartLocalSound( uis.menu_datadisp2_snd, CHAN_LOCAL_SOUND  );
		}

		UI_MainMenuButtons(iMax);

		// Print buttons
		for (i=0;i<iMax;++i)
		{
			UI_DrawHandlePic(mm_buttons[i][0]-8, mm_buttons[i][1], -MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right
		}

		if (iMax == buttonCnt)
		{
			iMaxHold = 0;
			s_main_menu.openingStart = uis.realtime;
			s_main_menu.subSeqStatus[0] = 1;
		}

		ui.R_SetColor( NULL );

	}
	// Step 2 : make buttons grow
	else if (!s_main_menu.subSeqStatus[1])
	{
		ui.R_SetColor( colorTable[CT_DKPURPLE1]);

		// Figure out time since this section started
		holdTime = uis.realtime - s_main_menu.openingStart;
		addX = (holdTime / 500) * (MENU_BUTTON_MED_WIDTH - MENU_BUTTON_MED_HEIGHT);
		if (addX > (MENU_BUTTON_MED_WIDTH - MENU_BUTTON_MED_HEIGHT))
		{
			addX = (MENU_BUTTON_MED_WIDTH - MENU_BUTTON_MED_HEIGHT);
		}
		else if (addX < 0)
		{
			addX = 0;
		}

		UI_MainMenuButtons(buttonCnt);

		// Print buttons
		for (i=0;i<buttonCnt;++i)
		{
			if (i==6)	// Tour mode button is different
			{
				ui.R_SetColor( colorTable[s_tourmode.color]);
			}
			else
			{
				ui.R_SetColor( colorTable[CT_DKPURPLE1]);
			}

			UI_DrawHandlePic(mm_buttons[i][0] + addX - 8,mm_buttons[i][1], -MENU_BUTTON_MED_HEIGHT, MENU_BUTTON_MED_HEIGHT, uis.graphicButtonLeftEnd);	//right

			UI_DrawHandlePic(mm_buttons[i][0], mm_buttons[i][1], addX, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);

		}

		if (addX == (MENU_BUTTON_MED_WIDTH - MENU_BUTTON_MED_HEIGHT))
		{
			iMaxHold = 0;
			s_main_menu.openingStart = uis.realtime;
			s_main_menu.subSeqStatus[1] = 1;
		}

		ui.R_SetColor( NULL );

	}
	// Step 3 : Print out text on each button
	else if (!s_main_menu.subSeqStatus[2])
	{
		ui.R_SetColor( colorTable[CT_DKPURPLE1]);

		UI_MainMenuButtons(buttonCnt);

		// Print buttons
		for (i=0;i<buttonCnt;++i)
		{
			if (i==6)	// Tour mode button is different
			{
				ui.R_SetColor( colorTable[s_tourmode.color]);
			}
			else
			{
				ui.R_SetColor( colorTable[CT_DKPURPLE1]);
			}

			UI_DrawHandlePic(mm_buttons[i][0] ,mm_buttons[i][1], MENU_BUTTON_MED_WIDTH, MENU_BUTTON_MED_HEIGHT, buttonbar1);
		}

		// Figure out time since this section started
		holdTime = uis.realtime - s_main_menu.openingStart;
		iMax = (int) (holdTime / 100);	// A tenth of a second per button
		if (iMax > buttonCnt)
		{
			iMax = buttonCnt;
		}
		else if (iMax < 0)
		{
			iMax = 1;
		}

		if (iMaxHold != iMax)
		{
			iMaxHold = iMax;
			ui.S_StartLocalSound( uis.menu_datadisp2_snd, CHAN_LOCAL_SOUND );
		}

		for (i=0;i<iMax;++i)
		{
			bitMap = (menubitmap_s *) s_main_menu.items[i];

			x = bitMap->generic.x;
			y = bitMap->generic.y;

			UI_DrawProportionalString( x + bitMap->textX, y + bitMap->textY , 
				menu_button_text[bitMap->textEnum][0], UI_LEFT|UI_SMALLFONT, colorTable[bitMap->textcolor]);

		}

		ui.R_SetColor( NULL );

		if (iMax == (buttonCnt - 1))	// End of menu opening
		{
			iMaxHold = 0;

			s_main_menu.subSeqStatus[0] = 0;
			s_main_menu.subSeqStatus[1] = 0;
			s_main_menu.subSeqStatus[2] = 0;

			s_main_menu.openingStart = 0;	
		}
	}

}
/*
=================
M_Main_Closing
=================
*/
void M_Main_Closing (void)
{
	int i,iMax;
	float holdTime;
	menubitmap_s *bitMap;
	float	x;
	float	y;
	static int iMaxHold;

	M_MainMenu_Graphics();

	// Reverse printing of buttons making unchosen go away
	if (!s_main_menu.subSeqStatus[0])
	{
		// Figure out time since this section started
		holdTime = uis.realtime - s_main_menu.closingStart;
		iMax = (int) (holdTime / 100);	// A tenth of a second per button
		if (iMax > 8)
		{
			iMax = 8;
		}
		else if (iMax < 0)
		{
			iMax = 1;
		}

		iMax = 8 - iMax;

		if (iMaxHold != iMax)
		{
			iMaxHold = iMax;
			ui.S_StartLocalSound( uis.menu_datadisp2_snd, CHAN_LOCAL_SOUND  );
		}

		// Print buttons and text
		for (i=0;i<iMax;++i)
		{
			ui.R_SetColor( colorTable[CT_DKPURPLE1]);

			UI_DrawHandlePic(mm_buttons[i][0] - 25, mm_buttons[i][1],  19, 20, uis.graphicButtonLeftEnd);

			UI_DrawHandlePic(mm_buttons[i][0] + 132 - 8, mm_buttons[i][1], -19, 20, uis.graphicButtonLeftEnd);	//right

			UI_DrawHandlePic(mm_buttons[i][0], mm_buttons[i][1], 132, 20, uis.whiteShader);

			bitMap = (menubitmap_s *) s_main_menu.items[i];

			x = bitMap->generic.x;
			y = bitMap->generic.y;

			UI_DrawString( x + bitMap->textX, y + bitMap->textY , 
				menu_button_text[i][0], UI_LEFT|UI_SMALLFONT, colorTable[bitMap->textcolor]);
		}

		ui.R_SetColor( NULL );

		if (iMax == 0)	// End of menu closing
		{
			iMaxHold = CT_DKORANGE;

			s_main_menu.subSeqStatus[0] = 1;
			s_main_menu.closingStart = uis.realtime;
			s_main_menu.cnt = 0;
		}
	}
	// Blink button
	else if (!s_main_menu.subSeqStatus[1])
	{

		// Figure out time since this section started
		holdTime = uis.realtime - s_main_menu.closingStart;

		if (s_main_menu.closingStart < uis.realtime)
		{
			++s_main_menu.cnt;
			s_main_menu.closingStart = uis.realtime + 200;
			if (iMaxHold == CT_DKORANGE)
				iMaxHold = CT_LTORANGE;
			else
				iMaxHold = CT_DKORANGE;
		}

		ui.R_SetColor( colorTable[iMaxHold]);

		UI_DrawHandlePic(mm_buttons[0][0] - 25,mm_buttons[0][1],  19, 20, uis.graphicButtonLeftEnd);

		UI_DrawHandlePic(mm_buttons[0][0] + 132 - 8, mm_buttons[0][1], -19, 20, uis.graphicButtonLeftEnd);	//right

		UI_DrawHandlePic(mm_buttons[0][0],mm_buttons[0][1], 132, 20, uis.whiteShader);

		bitMap = (menubitmap_s *) s_main_menu.items[0];

		x = bitMap->generic.x;
		y = bitMap->generic.y;

		UI_DrawString( x + bitMap->textX, y + bitMap->textY , 
			menu_button_text[0][0], UI_LEFT|UI_SMALLFONT, colorTable[bitMap->textcolor]);

		if (s_main_menu.cnt >= 6)
		{
			s_main_menu.subSeqStatus[0] = 0;
			s_main_menu.closingStart = 0;	
		}
	}
}


/*
=================
M_Main_Draw
=================
*/
void M_Main_Draw (void)
{
	
	// Draw graphics particular to Main Menu
	M_MainMenu_Graphics();

	// Draw buttons in upper third of screen
	ui.R_SetColor( colorTable[CT_DKPURPLE1]);
	UI_MainMenuButtons(8);
	ui.R_SetColor( NULL );

	// legals
	//ui.R_SetColor( NULL );
	//UI_DrawPic( (SCREEN_WIDTH-512)/2, SCREEN_HEIGHT-128, 512, 128, "menuinfo2");

	Menu_Draw( &s_main_menu );
}

/*
=================
M_Main_Key
=================
*/
sfxHandle_t M_Main_Key (int key)
{
	if (key == K_ESCAPE)
	{
		UI_QuitMenu();
		return menu_out_sound;
	}

	return ( Menu_DefaultKey( &s_main_menu, key ) );
}

/*
=================
M_Main_Event
=================
*/
void M_Main_Event (void* ptr, int notification)
{
	if (notification != QM_ACTIVATED)
		return;

	s_main_menu.closingStart = uis.realtime;	

	switch (((menucommon_s*)ptr)->id)
	{
		// Customize Controls
		case ID_CUSTOMIZECONTROLS:
			loadModelFlag	= qtrue;	
			loadModelTime = uis.realtime + 350;
			M_MainMenu_GraphicsTurnOff();
//			UI_SetupWeaponsMenu();
			break;
		case ID_INGAMECUSTOMIZECONTROLS:
			UI_SetupWeaponsMenu();
			break;

		case ID_LOADGAME:
			UI_LoadGameMenu(qtrue);
			break;

		// Single Player Arena
		case ID_NEWGAME:
			UI_NewGameMenu();
			break;

		case ID_TOUR:
			UI_TourGameMenu();
			break;

		// Explore
		case ID_EXPLORE:
			UI_CrewMenu();
			break;

		// Credits
		case ID_CREDITS:
			UI_CreditsMenu();
			break;

		// Quit
		case ID_QUIT:
			UI_QuitMenu();
			break;

		case ID_MODS:
			UI_ModsMenu();	
			break;

		case ID_FEDERATION_LABEL:
		case ID_CARDASSIAN_LABEL:
		case ID_FERENGI_LABEL:
		case ID_ROMULAN_LABEL:
		case ID_KLINGON_LABEL:
		case ID_VOYAGER_LABEL:
		case ID_DOMINION_LABEL:
		case ID_BORG_LABEL:
		case ID_WORMHOLE_LABEL:
		case ID_CORE_LABEL:
		case ID_ALHAPQUAD_LABEL:
		case ID_BETAQUAD_LABEL:
		case ID_DELTAQUAD_LABEL:
		case ID_GAMMAQUAD_LABEL:
			MainMenuInterrupt(((menucommon_s*)ptr)->id);
			break;

		case ID_Q_VOYAGER_LABEL:
		case ID_PHASER_LABEL:
		case ID_TORPEDO_LABEL:
		case ID_VENTRAL_LABEL:
		case ID_MIDHULL_LABEL:
		case ID_BUSSARD_LABEL:
		case ID_NACELLES_LABEL:
		case ID_THRUSTERS_LABEL:
		case ID_BRIDGE_LABEL:
			QuitMenuInterrupt(((menucommon_s*)ptr)->id);
			break;

		case ID_Q_RAVEN_LABEL:
			UI_RavenMenu();
			break;
	}
}

/*
===============
MainMenu_Cache
===============
*/
void MainMenu_Cache( void ) 
{

	quadrants	= ui.R_RegisterShaderNoMip("menu/special/quadrants.tga");
	buttonbar1  = ui.R_RegisterShaderNoMip(GRAPHIC_BUTTONRIGHT);

	directoryUpperCorner = ui.R_RegisterShaderNoMip("menu/common/sm_top.tga");
	directoryLowerCorner = ui.R_RegisterShaderNoMip("menu/common/sm_bottom.tga");


	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(mainmenu_graphics,MMG_MAX);

	backgroundsnd = ui.S_RegisterSound( "sound/interface/mainbackground.wav" );

}

/*
===============
MainMenu_Init
===============
*/
void MainMenu_Init( void ) 
{
	int i,inc,x,y;
	char buffer[32];
	int *language;

	MainMenu_Cache();

	if (!s_main_menu.openingStart)	// This only happens at the very beginning
	{
		loadModelInitialized = qfalse;
	}

	ui.Cvar_SetValue( "cg_virtualVoyager", 0 );

	s_main_menu.nitems					= 0;
	s_main_menu.wrapAround				= qtrue;
	s_main_menu.opening					= M_Main_Opening;
//	s_main_menu.closing					= M_Main_Closing;
	s_main_menu.draw					= M_Main_Draw;
	s_main_menu.key						= M_Main_Key;
	s_main_menu.fullscreen				= qtrue;
	s_main_menu.descX					= MENU_DESC_X;
	s_main_menu.descY					= MENU_DESC_Y;
	s_main_menu.titleX					= MENU_TITLE_X;
	s_main_menu.titleY					= MENU_TITLE_Y;
	s_main_menu.titleI					= MNT_MAINMENU_TITLE;
	s_main_menu.footNoteEnum			= MNT_STELLAR_CARTOGRAPHY;


/*	s_holomatch.generic.type				= MTYPE_BITMAP;      
	s_holomatch.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_holomatch.generic.x					= 30;
	s_holomatch.generic.y					= 289;
	s_holomatch.generic.name				= "menu/common/square.tga";
//	s_holomatch.generic.id					= ID_NEWGAME;
	s_holomatch.generic.callback			= M_Main_Event; 
	s_holomatch.width						= 47;
	s_holomatch.height						= 100;
	s_holomatch.color						= CT_DKPURPLE1;
	s_holomatch.color2						= CT_LTPURPLE1;
	s_holomatch.textX						= MENU_BUTTON_TEXT_X;
	s_holomatch.textY						= MENU_BUTTON_TEXT_Y;
	s_holomatch.textEnum					= MBT_HOLOMATCH;
	s_holomatch.textcolor					= CT_BLACK;
	s_holomatch.textcolor2					= CT_WHITE;*/


// All X,Y positions are given as if on a 640 x 480 screen
	s_newgame.generic.type				= MTYPE_BITMAP;      
	s_newgame.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_newgame.generic.x					= mm_buttons[0][0];
	s_newgame.generic.y					= mm_buttons[0][1];
	s_newgame.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_newgame.generic.id				= ID_NEWGAME;
	s_newgame.generic.callback			= M_Main_Event; 
	s_newgame.width						= MENU_BUTTON_MED_WIDTH;
	s_newgame.height					= MENU_BUTTON_MED_HEIGHT;
	s_newgame.color						= CT_DKPURPLE1;
	s_newgame.color2					= CT_LTPURPLE1;
	s_newgame.textX						= MENU_BUTTON_TEXT_X;
	s_newgame.textY						= MENU_BUTTON_TEXT_Y;
	s_newgame.textEnum					= MBT_NEWGAME;
	s_newgame.textcolor					= CT_BLACK;
	s_newgame.textcolor2				= CT_WHITE;


	s_loadgame.generic.type				= MTYPE_BITMAP;      
	s_loadgame.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_loadgame.generic.x				= mm_buttons[1][0];
	s_loadgame.generic.y				= mm_buttons[1][1];
	s_loadgame.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_loadgame.generic.id				= ID_LOADGAME;
	s_loadgame.generic.callback			= M_Main_Event; 
	s_loadgame.width					= MENU_BUTTON_MED_WIDTH;
	s_loadgame.height					= MENU_BUTTON_MED_HEIGHT;
	s_loadgame.color					= CT_DKPURPLE1;
	s_loadgame.color2					= CT_LTPURPLE1;
	s_loadgame.textX					= MENU_BUTTON_TEXT_X;
	s_loadgame.textY					= MENU_BUTTON_TEXT_Y;
	s_loadgame.textEnum					= MBT_LOADGAME;
	s_loadgame.textcolor				= CT_BLACK;
	s_loadgame.textcolor2				= CT_WHITE;	

	s_setup.generic.type				= MTYPE_BITMAP;      
	s_setup.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_setup.generic.x					= mm_buttons[2][0];
	s_setup.generic.y					= mm_buttons[2][1];
	s_setup.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_setup.generic.id					= ID_CUSTOMIZECONTROLS;
	s_setup.generic.callback			= M_Main_Event; 
	s_setup.width						= MENU_BUTTON_MED_WIDTH;
	s_setup.height						= MENU_BUTTON_MED_HEIGHT;
	s_setup.color						= CT_DKPURPLE1;
	s_setup.color2						= CT_LTPURPLE1;
	s_setup.textX						= MENU_BUTTON_TEXT_X;
	s_setup.textY						= MENU_BUTTON_TEXT_Y;
	s_setup.textEnum					= MBT_CONFIGURE;
	s_setup.textcolor					= CT_BLACK;
	s_setup.textcolor2					= CT_WHITE;

	s_explorevoyager.generic.type		= MTYPE_BITMAP;      
	s_explorevoyager.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_explorevoyager.generic.x			= mm_buttons[3][0];
	s_explorevoyager.generic.y			= mm_buttons[3][1];
	s_explorevoyager.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_explorevoyager.generic.id			= ID_EXPLORE;
	s_explorevoyager.generic.callback	= M_Main_Event; 
	s_explorevoyager.width				= MENU_BUTTON_MED_WIDTH;
	s_explorevoyager.height				= MENU_BUTTON_MED_HEIGHT;
	s_explorevoyager.color				= CT_DKPURPLE1;
	s_explorevoyager.color2				= CT_LTPURPLE1;
	s_explorevoyager.textX				= MENU_BUTTON_TEXT_X;
	s_explorevoyager.textY				= MENU_BUTTON_TEXT_Y;
	s_explorevoyager.textEnum			= MBT_VOYAGERCREW;
	s_explorevoyager.textcolor			= CT_BLACK;
	s_explorevoyager.textcolor2			= CT_WHITE;

	s_credits.generic.type				= MTYPE_BITMAP;      
	s_credits.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_credits.generic.x					= mm_buttons[4][0];
	s_credits.generic.y					= mm_buttons[4][1];
	s_credits.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_credits.generic.id				= ID_CREDITS;
	s_credits.generic.callback			= M_Main_Event; 
	s_credits.width						= MENU_BUTTON_MED_WIDTH;
	s_credits.height					= MENU_BUTTON_MED_HEIGHT;
	s_credits.color						= CT_DKPURPLE1;
	s_credits.color2					= CT_LTPURPLE1;
	s_credits.textX						= MENU_BUTTON_TEXT_X;
	s_credits.textY						= MENU_BUTTON_TEXT_Y;
	s_credits.textEnum					= MBT_CREDITS;
	s_credits.textcolor					= CT_BLACK;
	s_credits.textcolor2				= CT_WHITE;

	s_quitgame.generic.type				= MTYPE_BITMAP;      
	s_quitgame.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_quitgame.generic.x				= mm_buttons[5][0];
	s_quitgame.generic.y				= mm_buttons[5][1];
	s_quitgame.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_quitgame.generic.id				= ID_QUIT;
	s_quitgame.generic.callback			= M_Main_Event; 
	s_quitgame.width					= MENU_BUTTON_MED_WIDTH;
	s_quitgame.height					= MENU_BUTTON_MED_HEIGHT;
	s_quitgame.color					= CT_DKPURPLE1;
	s_quitgame.color2					= CT_LTPURPLE1;
	s_quitgame.textX					= MENU_BUTTON_TEXT_X;
	s_quitgame.textY					= MENU_BUTTON_TEXT_Y;
	s_quitgame.textEnum					= MBT_EXITPROG;
	s_quitgame.textcolor				= CT_BLACK;
	s_quitgame.textcolor2				= CT_WHITE;

	s_tourmode.generic.type				= MTYPE_BITMAP;      
	s_tourmode.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_tourmode.generic.x				= mm_buttons[6][0];
	s_tourmode.generic.y				= mm_buttons[6][1];
	s_tourmode.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_tourmode.generic.id				= ID_TOUR;
	s_tourmode.generic.callback			= M_Main_Event; 
	s_tourmode.width					= MENU_BUTTON_MED_WIDTH;
	s_tourmode.height					= MENU_BUTTON_MED_HEIGHT;
	s_tourmode.color					= CT_DKORANGE;
	s_tourmode.color2					= CT_LTORANGE;
	s_tourmode.textX					= MENU_BUTTON_TEXT_X;
	s_tourmode.textY					= MENU_BUTTON_TEXT_Y;
	s_tourmode.textEnum					= MBT_VIRTUALVOYAGER;
	s_tourmode.textcolor				= CT_BLACK;
	s_tourmode.textcolor2				= CT_WHITE;

	s_mods.generic.type				= MTYPE_BITMAP;      
	s_mods.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_mods.generic.x				= mm_buttons[7][0];
	s_mods.generic.y				= mm_buttons[7][1];
	s_mods.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_mods.generic.id				= ID_MODS;
	s_mods.generic.callback			= M_Main_Event; 
	s_mods.width					= MENU_BUTTON_MED_WIDTH;
	s_mods.height					= MENU_BUTTON_MED_HEIGHT;
	s_mods.color					= CT_DKPURPLE1;
	s_mods.color2					= CT_LTPURPLE1;
	s_mods.textX					= MENU_BUTTON_TEXT_X;
	s_mods.textY					= MENU_BUTTON_TEXT_Y;
	s_mods.textEnum					= MBT_MODS;
	s_mods.textcolor				= CT_BLACK;
	s_mods.textcolor2				= CT_WHITE;

	// Label buttons
	s_federation_label.generic.type				= MTYPE_BITMAP;      
	s_federation_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_federation_label.generic.x				= 218;
	s_federation_label.generic.y				= 376;
	s_federation_label.generic.name				= "menu/common/square2.tga";
	s_federation_label.generic.id				= ID_FEDERATION_LABEL;
	s_federation_label.generic.callback			= M_Main_Event; 
	s_federation_label.width					= 49;
	s_federation_label.height					= 11;
	s_federation_label.color					= CT_DKPURPLE1;
	s_federation_label.color2					= CT_LTPURPLE1;
	s_federation_label.textX					= 42;
	s_federation_label.textY					= 1;
	s_federation_label.textEnum					= MBT_FEDERATION_LABEL;
	s_federation_label.textcolor				= CT_DKGOLD1;
	s_federation_label.textcolor2				= CT_WHITE;
	s_federation_label.textStyle				= UI_RIGHT | UI_TINYFONT;

	s_cardassian_label.generic.type				= MTYPE_BITMAP;      
	s_cardassian_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_cardassian_label.generic.x				= 218;
	s_cardassian_label.generic.y				= 358;
	s_cardassian_label.generic.name				= "menu/common/square2.tga";
	s_cardassian_label.generic.id				= ID_CARDASSIAN_LABEL;
	s_cardassian_label.generic.callback			= M_Main_Event; 
	s_cardassian_label.width					= 49;
	s_cardassian_label.height					= 11;
	s_cardassian_label.color					= CT_DKPURPLE1;
	s_cardassian_label.color2					= CT_LTPURPLE1;
	s_cardassian_label.textX					= 42;
	s_cardassian_label.textY					= 1;
	s_cardassian_label.textEnum					= MBT_CARDASSIA_LABEL;
	s_cardassian_label.textcolor				= CT_DKGOLD1;
	s_cardassian_label.textcolor2				= CT_WHITE;
	s_cardassian_label.textStyle				= UI_RIGHT | UI_TINYFONT;

	s_ferengi_label.generic.type				= MTYPE_BITMAP;      
	s_ferengi_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_ferengi_label.generic.x					= 192;
	s_ferengi_label.generic.y					= 346;
	s_ferengi_label.generic.name				= "menu/common/square2.tga";
	s_ferengi_label.generic.id					= ID_FERENGI_LABEL;
	s_ferengi_label.generic.callback			= M_Main_Event; 
	s_ferengi_label.width						= 75;
	s_ferengi_label.height						= 11;
	s_ferengi_label.color						= CT_DKPURPLE1;
	s_ferengi_label.color2						= CT_LTPURPLE1;
	s_ferengi_label.textX						= 68;
	s_ferengi_label.textY						= 1;
	s_ferengi_label.textEnum					= MBT_FERENGI_LABEL;
	s_ferengi_label.textcolor					= CT_DKGOLD1;
	s_ferengi_label.textcolor2					= CT_WHITE;
	s_ferengi_label.textStyle					= UI_RIGHT | UI_TINYFONT;

	s_romulan_label.generic.type				= MTYPE_BITMAP;      
	s_romulan_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_romulan_label.generic.x					= 342;
	s_romulan_label.generic.y					= 353;
	s_romulan_label.generic.name				= "menu/common/square2.tga";
	s_romulan_label.generic.id					= ID_ROMULAN_LABEL;
	s_romulan_label.generic.callback			= M_Main_Event; 
	s_romulan_label.width						= 67;
	s_romulan_label.height						= 11;
	s_romulan_label.color						= CT_DKPURPLE1;
	s_romulan_label.color2						= CT_LTPURPLE1;
	s_romulan_label.textX						= 2;
	s_romulan_label.textY						= 1;
	s_romulan_label.textEnum					= MBT_ROMULAN_LABEL;
	s_romulan_label.textcolor					= CT_DKGOLD1;
	s_romulan_label.textcolor2					= CT_WHITE;
	s_romulan_label.textStyle					= UI_TINYFONT;

	s_klingon_label.generic.type				= MTYPE_BITMAP;      
	s_klingon_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_klingon_label.generic.x					= 342;
	s_klingon_label.generic.y					= 364;
	s_klingon_label.generic.name				= "menu/common/square2.tga";
	s_klingon_label.generic.id					= ID_KLINGON_LABEL;
	s_klingon_label.generic.callback			= M_Main_Event; 
	s_klingon_label.width						= 67;
	s_klingon_label.height						= 11;
	s_klingon_label.color						= CT_DKPURPLE1;
	s_klingon_label.color2						= CT_LTPURPLE1;
	s_klingon_label.textX						= 2;
	s_klingon_label.textY						= 1;
	s_klingon_label.textEnum					= MBT_KLINGON_LABEL;
	s_klingon_label.textcolor					= CT_DKGOLD1;
	s_klingon_label.textcolor2					= CT_WHITE;
	s_klingon_label.textStyle					= UI_TINYFONT;

	s_dominion_label.generic.type				= MTYPE_BITMAP;      
	s_dominion_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_dominion_label.generic.x					= 222;
	s_dominion_label.generic.y					= 211;
	s_dominion_label.generic.name				= "menu/common/square2.tga";
	s_dominion_label.generic.id					= ID_DOMINION_LABEL;
	s_dominion_label.generic.callback			= M_Main_Event; 
	s_dominion_label.width						= 75;
	s_dominion_label.height						= 11;
	s_dominion_label.color						= CT_DKPURPLE1;
	s_dominion_label.color2						= CT_LTPURPLE1;
	s_dominion_label.textX						= 2;
	s_dominion_label.textY						= 1;
	s_dominion_label.textEnum					= MBT_DOMINION_LABEL;
	s_dominion_label.textcolor					= CT_DKGOLD1;
	s_dominion_label.textcolor2					= CT_WHITE;
	s_dominion_label.textStyle					= UI_TINYFONT;

	s_voyager_label.generic.type				= MTYPE_BITMAP;      
	s_voyager_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_voyager_label.generic.x					= 370;
	s_voyager_label.generic.y					= 197;
	s_voyager_label.generic.name				= "menu/common/square2.tga";
	s_voyager_label.generic.id					= ID_VOYAGER_LABEL;
	s_voyager_label.generic.callback			= M_Main_Event; 
	s_voyager_label.width						= 100;
	s_voyager_label.height						= 23;
	s_voyager_label.color						= CT_DKPURPLE1;
	s_voyager_label.color2						= CT_LTPURPLE1;
	s_voyager_label.textX						= 2;
	s_voyager_label.textY						= 1;
	s_voyager_label.textEnum					= MBT_VOYAGER_LABEL1;
	s_voyager_label.textEnum2					= MBT_VOYAGER_LABEL2;
	s_voyager_label.textcolor					= CT_DKGOLD1;
	s_voyager_label.textcolor2					= CT_WHITE;
	s_voyager_label.textStyle					= UI_TINYFONT;

	s_borg_label.generic.type				= MTYPE_BITMAP;      
	s_borg_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_borg_label.generic.x					= 330;
	s_borg_label.generic.y					= 221;
	s_borg_label.generic.name				= "menu/common/square2.tga";
	s_borg_label.generic.id					= ID_BORG_LABEL;
	s_borg_label.generic.callback			= M_Main_Event; 
	s_borg_label.width						= 100;
	s_borg_label.height						= 23;
	s_borg_label.color						= CT_DKPURPLE1;
	s_borg_label.color2						= CT_LTPURPLE1;
	s_borg_label.textX						= 2;
	s_borg_label.textY						= 1;
	s_borg_label.textEnum					= MBT_BORG_LABEL1;
	s_borg_label.textEnum2					= MBT_BORG_LABEL2;
	s_borg_label.textcolor					= CT_DKGOLD1;
	s_borg_label.textcolor2					= CT_WHITE;
	s_borg_label.textStyle					= UI_TINYFONT;

	s_wormhole_label.generic.type				= MTYPE_BITMAP;      
	s_wormhole_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_wormhole_label.generic.x					= 219;
	s_wormhole_label.generic.y					= 228;
	s_wormhole_label.generic.name				= "menu/common/square2.tga";
	s_wormhole_label.generic.id					= ID_WORMHOLE_LABEL;
	s_wormhole_label.generic.callback			= M_Main_Event; 
	s_wormhole_label.width						= 100;
	s_wormhole_label.height						= 23;
	s_wormhole_label.color						= CT_DKPURPLE1;
	s_wormhole_label.color2						= CT_LTPURPLE1;
	s_wormhole_label.textX						= 2;
	s_wormhole_label.textY						= 1;
	s_wormhole_label.textEnum					= MBT_WORMHOLE_LABEL1;
	s_wormhole_label.textEnum2					= MBT_WORMHOLE_LABEL2;
	s_wormhole_label.textcolor					= CT_DKGOLD1;
	s_wormhole_label.textcolor2					= CT_WHITE;
	s_wormhole_label.textStyle					= UI_TINYFONT;

	s_core_label.generic.type				= MTYPE_BITMAP;      
	s_core_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_core_label.generic.x					= 271;
	s_core_label.generic.y					= 277;
	s_core_label.generic.name				= "menu/common/square2.tga";
	s_core_label.generic.id					= ID_CORE_LABEL;
	s_core_label.generic.callback			= M_Main_Event; 
	s_core_label.width						= 60;
	s_core_label.height						= 11;
	s_core_label.color						= CT_DKPURPLE1;
	s_core_label.color2						= CT_LTPURPLE1;
	s_core_label.textX						= 2;
	s_core_label.textY						= 1;
	s_core_label.textEnum					= MBT_GALACTICCORE_LABEL;
	s_core_label.textcolor					= CT_DKGOLD1;
	s_core_label.textcolor2					= CT_WHITE;
	s_core_label.textStyle					= UI_TINYFONT;

	ui.Cvar_VariableStringBuffer( "g_language", buffer, 32 );
	language = s_textlanguage_Names;
	if (buffer[0]) {
		while (*language)
		{
			if (Q_stricmp(menu_normal_text[*language],buffer)==0)
			{
				break;
			}
			language++;
		}

		if (!*language)	// No match, make it English
		{
			language = s_textlanguage_Names;
		}
	}

	s_alphaquad_label.generic.type			= MTYPE_BITMAP;      
	s_alphaquad_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_alphaquad_label.generic.x				= 166;
	s_alphaquad_label.generic.y				= 291;
	s_alphaquad_label.generic.name			= "menu/common/square2.tga";
	s_alphaquad_label.generic.id			= ID_ALHAPQUAD_LABEL;
	s_alphaquad_label.generic.callback		= M_Main_Event; 
	s_alphaquad_label.width					= 43;
	s_alphaquad_label.height				= 21;
	s_alphaquad_label.color					= CT_DKPURPLE1;
	s_alphaquad_label.color2				= CT_LTPURPLE1;
	s_alphaquad_label.textX					= 2;
	s_alphaquad_label.textY					= 1;

	if (*language != MNT_FRENCH)
	{
		s_alphaquad_label.textEnum				= MBT_ALPHA;
		s_alphaquad_label.textEnum2				= MBT_QUADRANT;
	}
	else
	{
		s_alphaquad_label.textEnum				= MBT_QUADRANT;
		s_alphaquad_label.textEnum2				= MBT_ALPHA;
	}

	s_alphaquad_label.textcolor				= CT_DKGOLD1;
	s_alphaquad_label.textcolor2			= CT_WHITE;
	s_alphaquad_label.textStyle				= UI_TINYFONT;

	s_betaquad_label.generic.type			= MTYPE_BITMAP;      
	s_betaquad_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_betaquad_label.generic.x				= 385;
	s_betaquad_label.generic.y				= 291;
	s_betaquad_label.generic.name			= "menu/common/square2.tga";
	s_betaquad_label.generic.id				= ID_BETAQUAD_LABEL;
	s_betaquad_label.generic.callback		= M_Main_Event; 
	s_betaquad_label.width					= 40;
	s_betaquad_label.height					= 21;
	s_betaquad_label.color					= CT_DKPURPLE1;
	s_betaquad_label.color2					= CT_LTPURPLE1;
	s_betaquad_label.textX					= 38;
	s_betaquad_label.textY					= 1;

	if (*language != MNT_FRENCH)
	{
		s_betaquad_label.textEnum				= MBT_BETA;
		s_betaquad_label.textEnum2				= MBT_QUADRANT;
	}
	else
	{
		s_betaquad_label.textEnum				= MBT_QUADRANT;
		s_betaquad_label.textEnum2				= MBT_BETA;
	}

	s_betaquad_label.textcolor				= CT_DKGOLD1;
	s_betaquad_label.textcolor2				= CT_WHITE;
	s_betaquad_label.textStyle				= UI_RIGHT | UI_TINYFONT;

	s_deltaquad_label.generic.type			= MTYPE_BITMAP;      
	s_deltaquad_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_deltaquad_label.generic.x				= 385;
	s_deltaquad_label.generic.y				= 265;
	s_deltaquad_label.generic.name			= "menu/common/square2.tga";
	s_deltaquad_label.generic.id			= ID_DELTAQUAD_LABEL;
	s_deltaquad_label.generic.callback		= M_Main_Event; 
	s_deltaquad_label.width					= 40;
	s_deltaquad_label.height				= 21;
	s_deltaquad_label.color					= CT_DKPURPLE1;
	s_deltaquad_label.color2				= CT_LTPURPLE1;
	s_deltaquad_label.textX					= 38;
	s_deltaquad_label.textY					= 1;
	if (*language != MNT_FRENCH)
	{
		s_deltaquad_label.textEnum				= MBT_DELTA;
		s_deltaquad_label.textEnum2				= MBT_QUADRANT;
	}
	else
	{
		s_deltaquad_label.textEnum				= MBT_QUADRANT;
		s_deltaquad_label.textEnum2				= MBT_DELTA;
	}

	s_deltaquad_label.textcolor				= CT_DKGOLD1;
	s_deltaquad_label.textcolor2			= CT_WHITE;
	s_deltaquad_label.textStyle				= UI_RIGHT | UI_TINYFONT;

	s_gammaquad_label.generic.type			= MTYPE_BITMAP;      
	s_gammaquad_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS;
	s_gammaquad_label.generic.x				= 166;
	s_gammaquad_label.generic.y				= 265;
	s_gammaquad_label.generic.name			= "menu/common/square2.tga";
	s_gammaquad_label.generic.id			= ID_GAMMAQUAD_LABEL;
	s_gammaquad_label.generic.callback		= M_Main_Event; 
	s_gammaquad_label.width					= 43;
	s_gammaquad_label.height				= 21;
	s_gammaquad_label.color					= CT_DKPURPLE1;
	s_gammaquad_label.color2				= CT_LTPURPLE1;
	s_gammaquad_label.textX					= 2;
	s_gammaquad_label.textY					= 1;
	if (*language != MNT_FRENCH)
	{
		s_gammaquad_label.textEnum				= MBT_GAMMA;
		s_gammaquad_label.textEnum2				= MBT_QUADRANT;
	}
	else
	{
		s_gammaquad_label.textEnum				= MBT_QUADRANT;
		s_gammaquad_label.textEnum2				= MBT_GAMMA;
	}
	s_gammaquad_label.textcolor				= CT_DKGOLD1;
	s_gammaquad_label.textcolor2			= CT_WHITE;
	s_gammaquad_label.textStyle				= UI_TINYFONT;

	Menu_AddItem( &s_main_menu,	&s_newgame );
	Menu_AddItem( &s_main_menu,	&s_loadgame );

//	Menu_AddItem( &s_main_menu,	&s_holomatch );

	Menu_AddItem( &s_main_menu,	&s_setup );
	Menu_AddItem( &s_main_menu,	&s_explorevoyager );
	Menu_AddItem( &s_main_menu,	&s_credits );
	Menu_AddItem( &s_main_menu,	&s_quitgame );
	Menu_AddItem( &s_main_menu,	&s_tourmode );
	Menu_AddItem( &s_main_menu,	&s_mods );
	Menu_AddItem( &s_main_menu,	&s_alphaquad_label );
	Menu_AddItem( &s_main_menu,	&s_betaquad_label );
	Menu_AddItem( &s_main_menu,	&s_deltaquad_label );
	Menu_AddItem( &s_main_menu,	&s_gammaquad_label );
	Menu_AddItem( &s_main_menu,	&s_federation_label );
	Menu_AddItem( &s_main_menu,	&s_cardassian_label );
	Menu_AddItem( &s_main_menu,	&s_ferengi_label );
	Menu_AddItem( &s_main_menu,	&s_romulan_label );
	Menu_AddItem( &s_main_menu,	&s_klingon_label );
	Menu_AddItem( &s_main_menu,	&s_dominion_label );
	Menu_AddItem( &s_main_menu,	&s_voyager_label );
	Menu_AddItem( &s_main_menu,	&s_borg_label );
	Menu_AddItem( &s_main_menu,	&s_wormhole_label );
	Menu_AddItem( &s_main_menu,	&s_core_label );

	s_main_menu.initialized = qtrue;


	mainmenu_graphics[MMG_ARROW1].timer =  uis.realtime + 500;
	mainmenu_graphics[MMG_ARROW1].min = mainmenu_graphics[MMG_GRAPH_BAR1].y;
	mainmenu_graphics[MMG_ARROW1].max = mainmenu_graphics[MMG_GRAPH_BAR1].y + mainmenu_graphics[MMG_GRAPH_BAR1].height - 8;

	mainmenu_graphics[MMG_ARROW2].timer =  uis.realtime + 500;
	mainmenu_graphics[MMG_ARROW2].min = mainmenu_graphics[MMG_GRAPH_BAR2].y;
	mainmenu_graphics[MMG_ARROW2].max = mainmenu_graphics[MMG_GRAPH_BAR2].y + mainmenu_graphics[MMG_GRAPH_BAR2].height - 8;

	mainmenu_graphics[MMG_ARROW3].timer =  uis.realtime + 500;
	mainmenu_graphics[MMG_ARROW3].min = mainmenu_graphics[MMG_GRAPH_BAR3].y;
	mainmenu_graphics[MMG_ARROW3].max = mainmenu_graphics[MMG_GRAPH_BAR3].y + mainmenu_graphics[MMG_GRAPH_BAR3].height - 8;

	mainmenu_graphics[MMG_ARROW4].timer =  uis.realtime + 500;
	mainmenu_graphics[MMG_ARROW4].min = mainmenu_graphics[MMG_GRAPH_BAR4].y;
	mainmenu_graphics[MMG_ARROW4].max = mainmenu_graphics[MMG_GRAPH_BAR4].y + mainmenu_graphics[MMG_GRAPH_BAR4].height - 8;


	// Set up arrow graphics data 
	for (i=MMG_ARROW1;i<=MMG_ARROW4 ;++i)
	{
		mainmenu_graphics[i].target =  (random() * (mainmenu_graphics[i].max - mainmenu_graphics[i].min))
			+	mainmenu_graphics[i].min;

		if (mainmenu_graphics[i].y > mainmenu_graphics[i].target)
		{
			mainmenu_graphics[i].inc *= -1;
		}
	}

	// Set up Top RH column of numbers 
	y = 168;
	x = 550;
	inc = 12;
	for (i=MMG_RH_COL1_NUM1;i<=MMG_RH_COL1_NUM6;++i)
	{
		mainmenu_graphics[i].target = UI_RandomNumbers(9);

		mainmenu_graphics[i].x = x;
		mainmenu_graphics[i].y = y;
		y += inc;
	}


	// Set up 2nd to top RH column of numbers 
	y = 251;
	x = 550;
	for (i=MMG_RH_COL2_NUM1;i<=MMG_RH_COL2_NUM6;++i)
	{
		mainmenu_graphics[i].target = UI_RandomNumbers(9);
		mainmenu_graphics[i].x = x;
		mainmenu_graphics[i].y = y;
		y += inc;
	}

	// Make right hand numbers change
	mainmenu_graphics[MMG_RH_COL1_NUM2].timer = uis.realtime;


	// Set up 3rd RH block of numbers
	y = 346;
	x = 550;
	for (i=MMG_RH_COL3_NUM1;i<=MMG_RH_COL3_NUM3;++i)
	{
		mainmenu_graphics[i].target = UI_RandomNumbers(9);
		mainmenu_graphics[i].x = x;
		mainmenu_graphics[i].y = y;
		y += inc;
	}

	// Set up 3rd RH block
	y = 393;
	x = 550;
	for (i=MMG_RH_COL4_NUM1;i<=MMG_RH_COL4_NUM3;++i)
	{
		mainmenu_graphics[i].target = UI_RandomNumbers(9);
		mainmenu_graphics[i].x = x;
		mainmenu_graphics[i].y = y;
		y += inc;
	}

	// Make right hand numbers change
	mainmenu_graphics[MMG_RH_COL1_NUM1].timer = uis.realtime;

	// Set area of focus
	mainmenu_graphics[MMG_SECTIONS_BEGIN].target = MMG_SECTIONS_BEGIN + 1;

	// Turn off all empire symbols
	for (i=MMG_SYMBOLS_BEGIN + 1;i<MMG_SYMBOLS_END;++i)
	{
		mainmenu_graphics[i].type = MG_OFF;
	}

	// Darken all empire labels
	for (i=MMG_SECTIONS_BEGIN + 1;i<MMG_SECTIONS_END;++i)
	{
		mainmenu_graphics[i].color = CT_DKGOLD1;		
	}

	// Turn off all empire descriptions
	for (i=MMG_SECTION_TEXT_BEGIN + 1;i<MMG_SECTION_TEXT_END;++i)
	{
		mainmenu_graphics[i].type = MG_OFF;		
	}
	// Force numbers to change
//	mainmenu_graphics[MMG_TOP_NUMBERS].timer = 0;	// To get numbers right away
//	mainmenu_graphics[MMG_TOP_NUMBERS].target=5;
//	mainmenu_graphics[MMG_SPEC_TOP_ROW].target = 0;


	mainmenu_graphics[MMG_ARROW1].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_ARROW2].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_ARROW3].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_ARROW4].type = MG_GRAPHIC;

	mainmenu_graphics[MMG_GRAPH_BAR1].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_GRAPH_BAR2].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_GRAPH_BAR3].type = MG_GRAPHIC;
	mainmenu_graphics[MMG_GRAPH_BAR4].type = MG_GRAPHIC;

}


/*
===============
UI_MainMenu

The main menu only comes up when not in a game,
so make sure that the attract loop server is down
and that local cinematics are killed
===============
*/
void UI_MainMenu(void)
{

	ui.Cvar_Set("sv_killserver", "1");	// let the demo server know it should shut down

//#ifdef NDEBUG
	if (!s_lcarsin_menu.initialized)	// Haven't played LCARS In menu yet
	{
		UI_LCARSIn_Menu();
		return;
	}
//#endif
	holoMatch		= qfalse;

	loadModelFlag	= qfalse;	
	uis.noversion	= qfalse;
	Mouse_Show();

	if (!ui.Cvar_VariableValue( "ui_initialsetup" ))
	{
		UI_InitialSetupMenu();
		return;
	}

	s_main_menu.openingStart = uis.realtime;
	if (!s_main_menu.openingStart)	// This only happens at the very beginning
	{
		s_main_menu.openingStart = 1;
	}

	MainMenu_Init(); 
//	ui.S_StartLocalLoopingSound( backgroundsnd);

	UI_PushMenu ( &s_main_menu );

	ui.Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
}

/*
=======================================================================

LCARS IN MENU

=======================================================================
*/

/*
=================
M_LCARSIn_Key
=================
*/
sfxHandle_t M_LCARSIn_Key (int key)
{
	// Advance to Main Menu
	if ((key == K_SPACE) || (key == K_ENTER) || (key == K_ESCAPE) || (key == K_MOUSE1))
	{
		UI_PopMenu();	// Get rid of whatever is ontop
		UI_MainMenu();
	}
	return ( menu_out_sound );
}

/*
===============
LCARSInMenu_Draw
===============
*/
void LCARSInMenu_Draw(void)
{
	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 89, 25, 512, 512, s_LCARSinmenu.federation); 
	UI_DrawProportionalString(320, 398, menu_normal_text[MNT_ACCESSINGLCARS], UI_BIGFONT | UI_CENTER, colorTable[CT_WHITE]);
	UI_DrawProportionalString(320, 450, menu_normal_text[MNT_PARAMOUNT_LEGAL], UI_TINYFONT | UI_CENTER, colorTable[CT_MDGREY]);
	UI_DrawProportionalString(320, 465, menu_normal_text[MNT_ID_LEGAL], UI_TINYFONT | UI_CENTER, colorTable[CT_MDGREY]);

	if (federationTimer < uis.realtime)
	{
		UI_PopMenu();	// Get rid of whatever is ontop
		UI_MainMenu();
	}
}


/*
===============
LCARSInMenu_Cache
===============
*/
void LCARSInMenu_Cache (void)
{
	s_LCARSinmenu.federation = ui.R_RegisterShaderNoMip("menu/suit/federation.tga"); 
	welcomesnd = ui.S_RegisterSound( "sound/voice/computer/menu/welcome.wav" );
}

/*
===============
LCARSInMenu_Init
===============
*/
void LCARSInMenu_Init(void) 
{
	LCARSInMenu_Cache();

	s_lcarsin_menu.nitems					= 0;
	s_lcarsin_menu.wrapAround				= qtrue;
	s_lcarsin_menu.draw						= LCARSInMenu_Draw;
//	s_lcarsin_menu.opening					= M_Main_Opening;
//	s_lcarsin_menu.closing					= M_Main_Closing;
	s_lcarsin_menu.key						= M_LCARSIn_Key;
	s_lcarsin_menu.fullscreen				= qtrue;
	s_lcarsin_menu.descX					= 0;
	s_lcarsin_menu.descY					= 0;
	s_lcarsin_menu.titleX					= 0;
	s_lcarsin_menu.titleY					= 0;
	s_lcarsin_menu.titleI					= MNT_NONE;

	s_lcarsin_menu.initialized = qtrue;

	ui.S_StartLocalSound( welcomesnd,CHAN_MENU1);

	uis.noversion	= qtrue;

	federationTimer = uis.realtime + 4000;
}

/*
===============
UI_LCARSIn_Menu
===============
*/
void UI_LCARSIn_Menu(void)
{

	Mouse_Hide();

	LCARSInMenu_Init(); 

	UI_PushMenu ( &s_lcarsin_menu );

	ui.Key_SetCatcher( KEYCATCH_UI );
}

/*
=======================================================================

LCARS OUT MENU

=======================================================================
*/

/*
=================
M_LCARSOut_Key
=================
*/
sfxHandle_t M_LCARSOut_Key (int key)
{
	// Get out now!!
	if ((key == K_SPACE) || (key == K_ENTER) || (key == K_ESCAPE) || (key == K_MOUSE1))
	{
		federationTimer = 0;	// Get out of game now!!!
		return(0);
	}

	return ( Menu_DefaultKey( &s_lcarsout_menu, key ) );
}

/*
===============
LCARSOutMenu_Draw
===============
*/
void LCARSOutMenu_Draw(void)
{
	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 89, 25, 512, 512, s_LCARSoutmenu.federation); 
	UI_DrawProportionalString(320, 398, menu_normal_text[MNT_CLOSINGLCARS], UI_BIGFONT | UI_CENTER, colorTable[CT_WHITE]);
	UI_DrawProportionalString(320, 450, menu_normal_text[MNT_PARAMOUNT_LEGAL], UI_TINYFONT | UI_CENTER, colorTable[CT_MDGREY]);
	UI_DrawProportionalString(320, 465, menu_normal_text[MNT_ID_LEGAL], UI_TINYFONT | UI_CENTER, colorTable[CT_MDGREY]);

	// Leave game 
	if (federationTimer < uis.realtime)
	{
		ui.Cmd_ExecuteText( EXEC_NOW, "quit\n" );
	}
}

/*
===============
LCARSOutMenu_Cache
===============
*/
void LCARSOutMenu_Cache (void)
{
	s_LCARSoutmenu.federation = ui.R_RegisterShaderNoMip("menu/suit/federation.tga"); 
	logoutsnd = ui.S_RegisterSound( "sound/voice/computer/menu/logout.wav" );
}

/*
===============
LCARSOutMenu_Init
===============
*/
void LCARSOutMenu_Init(void) 
{
	LCARSOutMenu_Cache();

	s_lcarsout_menu.nitems					= 0;
	s_lcarsout_menu.wrapAround				= qtrue;
	s_lcarsout_menu.draw					= LCARSOutMenu_Draw;
//	s_lcarsout_menu.opening					= M_Main_Opening;
//	s_lcarsout_menu.closing					= M_Main_Closing;
	s_lcarsout_menu.key						= M_LCARSOut_Key;
	s_lcarsout_menu.fullscreen				= qtrue;
	s_lcarsout_menu.descX					= 0;
	s_lcarsout_menu.descY					= 0;
	s_lcarsout_menu.titleX					= 0;
	s_lcarsout_menu.titleY					= 0;
	s_lcarsout_menu.titleI					= MNT_NONE;

	s_lcarsout_menu.initialized = qtrue;
	uis.noversion	= qtrue;

	federationTimer = uis.realtime + 3000;
	ui.S_StartLocalSound( logoutsnd,CHAN_MENU1);
}

/*
===============
UI_LCARSOut_Menu
===============
*/
void UI_LCARSOut_Menu(void)
{

	LCARSOutMenu_Init(); 

	UI_PushMenu ( &s_lcarsout_menu );

	ui.Key_SetCatcher( KEYCATCH_UI );
}

int	DemotextI;
char Demotext[9][256];
int	DemotextY[9];

/*
=================
M_DemoEnd_SplitText
=================
*/
void M_DemoEnd_SplitText(char *string)
{
	int	lineWidth,currentWidth,charCnt;
	char *s,*holds;
	char holdChar[2];

	// Break into individual lines
	s = string;
	holds = s;

	lineWidth = 600;	// How long (in pixels) a line can be
	currentWidth = 0;
	holdChar[1] = '\0';
	charCnt= 0;

	while( *s ) 
	{
		++charCnt;
		holdChar[0] = *s;
		currentWidth += UI_ProportionalStringWidth(holdChar,UI_SMALLFONT);
		currentWidth +=PROP_GAP_WIDTH; // The space between characters

		if ( currentWidth >= lineWidth )
		{//Reached max length of this line
			//step back until we find a space

			while((currentWidth) && (*s != ' '))
			{
				holdChar[0] = *s;
				currentWidth -= UI_ProportionalStringWidth(holdChar,UI_SMALLFONT);
				--s;
				--charCnt;
			}

			Q_strncpyz( Demotext[DemotextI], holds, charCnt);
			Demotext[DemotextI][charCnt] = NULL;

			DemotextI++;
			DemotextY[DemotextI] = DemotextY[DemotextI - 1] + 20;
			currentWidth = 0;

			holds = s;
			holds++;

		}	
		++s;
	}

	++charCnt;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( Demotext[DemotextI], holds, charCnt);
	Demotext[DemotextI][charCnt] = NULL;
	DemotextI++;
	DemotextY[DemotextI] = DemotextY[DemotextI - 1] + 24;

}

/*
=================
M_DemoEnd_Key
=================
*/
sfxHandle_t M_DemoEnd_Key (int key)
{
	// Advance to Main Menu
	if (key == K_ESCAPE)
	{
		federationTimer = 0;	// Get out of game now!!!
		s_demoend_menu.screenI = 6;
	}
	else if ((key == K_SPACE) || (key == K_ENTER) || (key == K_MOUSE1))
	{
		federationTimer = 0;	// Advance to next screen
	}

	return ( menu_out_sound);
}

/*
===============
DemoEndMenu_Draw
===============
*/
void DemoEndMenu_Draw(void)
{
	int x,inc,i;

	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 0, 0, 640, 480, s_demoend_menu.screen[s_demoend_menu.screenI]); 

	if ((s_demoend_menu.screenI >= 0) && (s_demoend_menu.screenI <= 2))
	{
		x = 10;
		inc = 20;
		for (i=0;i<8;i++)
		{
			if (!DemotextY[i])
			{
				break;
			}

			if (Demotext[i][0] == '-')  // New lines begin with '-'
			{
				UI_DrawProportionalString(x, DemotextY[i], Demotext[i], UI_SMALLFONT | UI_DROPSHADOW, colorTable[CT_YELLOW]);
			}
			else	// If it's not a new line, indent a little.
			{
				UI_DrawProportionalString(x + 16, DemotextY[i], Demotext[i], UI_SMALLFONT | UI_DROPSHADOW, colorTable[CT_YELLOW]);
			}
		}
	}
	else if (s_demoend_menu.screenI == 3)
	{
		UI_DrawProportionalString(320, 245, menu_normal_text[MNT_DEMOSCR4_LINE1], UI_BIGFONT | UI_CENTER , colorTable[CT_YELLOW]);
		UI_DrawProportionalString(320, 282, menu_normal_text[MNT_DEMOSCR4_LINE2], UI_BIGFONT | UI_CENTER, colorTable[CT_YELLOW]);
		UI_DrawProportionalString( 10, 456, menu_normal_text[MNT_DEMOSCR4_LINE3], UI_TINYFONT , colorTable[CT_WHITE]);
	}
	else if (s_demoend_menu.screenI == 4)
	{
		UI_DrawProportionalString(320, 8, menu_normal_text[MNT_DEMOSCR5_LINE1], UI_SMALLFONT | UI_CENTER , colorTable[CT_YELLOW]);
		UI_DrawProportionalString(320, 310, menu_normal_text[MNT_DEMOSCR5_LINE2], UI_SMALLFONT | UI_CENTER , colorTable[CT_YELLOW]);
		UI_DrawProportionalString(320, 456, menu_normal_text[MNT_DEMOSCR5_LINE3], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
		UI_DrawProportionalString(320, 466, menu_normal_text[MNT_DEMOSCR5_LINE4], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
	}
	else if (s_demoend_menu.screenI == 5)
	{
		UI_DrawProportionalString(320, 18, menu_normal_text[MNT_DEMOSCR6_LINE1], UI_BIGFONT | UI_CENTER , colorTable[CT_YELLOW]);
		UI_DrawProportionalString(320, 48, menu_normal_text[MNT_DEMOSCR6_LINE2], UI_BIGFONT | UI_CENTER , colorTable[CT_YELLOW]);

		UI_DrawProportionalString(320, 422, menu_normal_text[MNT_DEMOSCR6_LINE3], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
		UI_DrawProportionalString(320, 432, menu_normal_text[MNT_DEMOSCR6_LINE4], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
		UI_DrawProportionalString(320, 442, menu_normal_text[MNT_DEMOSCR6_LINE5], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
		UI_DrawProportionalString(320, 452, menu_normal_text[MNT_DEMOSCR6_LINE6], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
		UI_DrawProportionalString(320, 462, menu_normal_text[MNT_DEMOSCR6_LINE7], UI_TINYFONT | UI_CENTER , colorTable[CT_MDGREY]);
	}


	// Leave game 
	if (federationTimer < uis.realtime)
	{
		s_demoend_menu.screenI++;
		federationTimer = uis.realtime + 20000;

		// Clean out any old data
		memset(Demotext,0,sizeof(Demotext));
		DemotextI = 0;
		memset(DemotextY,0,sizeof(DemotextY));
		DemotextY[0] = 300;
		if (s_demoend_menu.screenI == 1)
		{
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR2_LINE1]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR2_LINE2]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR2_LINE3]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR2_LINE4]);
		}
		else if (s_demoend_menu.screenI == 2)
		{
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE1]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE2]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE3]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE4]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE5]);
			M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR3_LINE6]);
		}
		else if (s_demoend_menu.screenI > 5)
		{
			UI_LCARSOut_Menu();
		}
	}
}
/*
===============
UI_DemoEndMenu_Cache
===============
*/
void UI_DemoEndMenu_Cache (void)
{
	s_demoend_menu.screen[0] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen1.tga"); 
	s_demoend_menu.screen[1] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen2.tga"); 
	s_demoend_menu.screen[2] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen3.tga"); 
	s_demoend_menu.screen[3] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen4.tga"); 
	s_demoend_menu.screen[4] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen5.tga"); 
	s_demoend_menu.screen[5] = ui.R_RegisterShaderNoMip("menu/demo_screens/screen6.tga"); 
	s_demoend_menu.screenI = 0;

	// Clean out any old data
	memset(Demotext,0,sizeof(Demotext));
	memset(DemotextY,0,sizeof(DemotextY));
	DemotextI = 0;

	DemotextY[0] = 300;

	M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR1_LINE1]);
	M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR1_LINE2]);
	M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR1_LINE3]);
	M_DemoEnd_SplitText(menu_normal_text[MNT_DEMOSCR1_LINE4]);
}

/*
===============
UI_DemoEndMenu_Init
===============
*/
void UI_DemoEndMenu_Init(void) 
{
	UI_DemoEndMenu_Cache();

	s_demoend_menu.menu.nitems					= 0;
	s_demoend_menu.menu.wrapAround				= qtrue;
	s_demoend_menu.menu.draw						= DemoEndMenu_Draw;
	s_demoend_menu.menu.key						= M_DemoEnd_Key;
	s_demoend_menu.menu.fullscreen				= qtrue;
	s_demoend_menu.menu.descX					= 0;
	s_demoend_menu.menu.descY					= 0;
	s_demoend_menu.menu.titleX					= 0;
	s_demoend_menu.menu.titleY					= 0;
	s_demoend_menu.menu.titleI					= MNT_NONE;

	uis.noversion	= qtrue;

	federationTimer = uis.realtime + 20000;
}

/*
===============
UI_DemoEnd_Menu
===============
*/
void UI_DemoEnd_Menu(void)
{

	UI_DemoEndMenu_Init(); 

	UI_PushMenu ( &s_demoend_menu.menu );

	ui.Key_SetCatcher( KEYCATCH_UI );
}

/*
=======================================================================

QUIT MENU

=======================================================================
*/
menubitmap_s	s_quit_mainmenu;
menubitmap_s	s_quit_no;
menubitmap_s	s_quit_yes;

/*
=================
M_Quit_SplitDesc
=================
*/
void M_Quit_SplitDesc(int descI)
{
	int	lineWidth,currentWidth,charCnt,currentLineI;
	char *s,*holds;
	char holdChar[2];

	// Clean out any old data
	memset(systemDesc,0,sizeof(systemDesc));

	// Break into individual lines
	s = menu_normal_text[quitmenu_graphics[descI].normaltextEnum];
	holds = s;


	lineWidth = 228;	// How long (in pixels) a line can be
	currentWidth = 0;
	holdChar[1] = '\0';
	charCnt= 0;
	currentLineI = 0;

	while( *s ) 
	{
		++charCnt;
		holdChar[0] = *s;
		currentWidth += UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
		currentWidth +=1; // The space between characters

		if ( currentWidth >= lineWidth )
		{//Reached max length of this line
			//step back until we find a space

			while((currentWidth) && (*s != ' '))
			{
				holdChar[0] = *s;
				currentWidth -= UI_ProportionalStringWidth(holdChar,UI_TINYFONT);
				--s;
				--charCnt;
			}

			Q_strncpyz( systemDesc[currentLineI], holds, charCnt);
			systemDesc[currentLineI][charCnt] = NULL;

			++currentLineI;
			currentWidth = 0;
			charCnt = 0;

			holds = s;
			++holds;

			if (currentLineI > SYSTEM_MAXDESC)
			{
				currentLineI = (SYSTEM_MAXDESC -1);
				break;
			}
		}	
		++s;
	}

	++charCnt;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( systemDesc[currentLineI], holds, charCnt);
	systemDesc[currentLineI][charCnt] = NULL;

}

/*
=================
M_Quit_Event
=================
*/
void M_Quit_Event (void* ptr, int notification)
{
	float fs_restrict;

	if (notification != QM_ACTIVATED)
		return;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MAINMENU:
			UI_PopMenu ();
			break;

		case ID_NO:
			UI_PopMenu ();
			break;

		case ID_YES:
			fs_restrict = ui.Cvar_VariableValue( "fs_restrict" );

			if (fs_restrict != 1) // Not in demo mode
			{
				if ((ui.Cvar_VariableValue( "cg_virtualVoyager" )==1)&& (holoMatch	== qfalse))
				{
					UI_VirtualSaveMenu(1);
				}
				else
				{
					UI_LCARSOut_Menu();
				}
			}
			else
			{
				UI_DemoEnd_Menu();
			}
			break;
	}
}


/*
=================
QuitMenu_Label
=================
*/
menutext_s *QuitMenu_Label(int labelId)
{
	menutext_s *label;

	switch(labelId)
	{
		case QMG_PHASER_LABEL :
			label = &s_quitmenu.phaser_label;
			break;

		case QMG_TORPEDO_LABEL :
			label = &s_quitmenu.torpedo_label;
			break;

		case QMG_VENTRAL_LABEL :
			label = &s_quitmenu.ventral_label;
			break;

		case QMG_MIDHULL_LABEL :
			label = &s_quitmenu.midhull_label;
			break;

		case QMG_BUSSARD_LABEL :
			label = &s_quitmenu.bussard_label;
			break;

		case QMG_NACELLES_LABEL :
			label = &s_quitmenu.nacelles_label;
			break;

		case QMG_THRUSTERS_LABEL :
			label = &s_quitmenu.thrusters_label;
			break;

		case QMG_BRIDGE_LABEL :
			label = &s_quitmenu.bridge_label;
			break;

		case QMG_VOYAGER_LABEL :
			label = &s_quitmenu.voyager_label;
			break;

		default:
			label = &s_quitmenu.bridge_label;
			break;
	}

	return(label);
}

/*
=================
QuitMenu_ChangeAreaFocus
=================
*/
void QuitMenu_ChangeAreaFocus(int newSystem)
{
	int swoopI;
	int descI;
	int oldSystem;
	menubitmap_s *bitmap;
	menutext_s *oldLabel,*newLabel;


	// Turn off current system info
	oldSystem = quitmenu_graphics[QMG_ACTIVE_SYSTEM].target;


	// Turn old label brown
	oldLabel = QuitMenu_Label(oldSystem);
	oldLabel->color = CT_DKBROWN1;

	// Turn off swoop
	swoopI = quitmenu_graphics[oldSystem].target;
	if (swoopI)
	{
		quitmenu_graphics[swoopI].type = MG_OFF;
	}

	// Turn pic off
	if (quitmenu_graphics[oldSystem].pointer)
	{
		bitmap = (menubitmap_s *) quitmenu_graphics[oldSystem].pointer;

		if (oldSystem == QMG_VOYAGER_LABEL)	//Voyager graphic can't be hidden
		{
			bitmap->color = CT_LTBLUE1;	
		}
		else
		{
			bitmap->generic.flags |= QMF_HIDDEN;	
		}
	}

	// Turning on the new system graphics
	quitmenu_graphics[QMG_ACTIVE_SYSTEM].target = newSystem;

	// Turn on new label
	newLabel = QuitMenu_Label(newSystem);
	newLabel->color = CT_YELLOW;

	// Turn on system description
	descI = quitmenu_graphics[newSystem].min;
	if (descI)
	{
		M_Quit_SplitDesc(descI);
	}

	// Turn on pic
	if (quitmenu_graphics[newSystem].pointer)
	{
		bitmap = (menubitmap_s *) quitmenu_graphics[newSystem].pointer;
		bitmap->generic.flags &= ~QMF_HIDDEN;	

		if (newSystem == QMG_VOYAGER_LABEL)	//Voyager graphic can't be hidden
		{
			bitmap->color = CT_LTGOLD1;	
		}
	}

	// Turn onswoop
	swoopI = quitmenu_graphics[newSystem].target;
	if (swoopI)
	{
		quitmenu_graphics[swoopI].type = MG_GRAPHIC;
	}

}


/*
=================
QuitMenuInterrupt
=================
*/
void QuitMenuInterrupt(int labelId)
{
	int newSystem;

	switch(labelId)
	{
		case ID_PHASER_LABEL :
			newSystem = QMG_PHASER_LABEL;
			break;

		case ID_TORPEDO_LABEL :
			newSystem = QMG_TORPEDO_LABEL;
			break;

		case ID_VENTRAL_LABEL :
			newSystem = QMG_VENTRAL_LABEL;
			break;

		case ID_MIDHULL_LABEL :
			newSystem = QMG_MIDHULL_LABEL;
			break;

		case ID_BUSSARD_LABEL :
			newSystem = QMG_BUSSARD_LABEL;
			break;

		case ID_NACELLES_LABEL :
			newSystem = QMG_NACELLES_LABEL;
			break;

		case ID_THRUSTERS_LABEL :
			newSystem = QMG_THRUSTERS_LABEL;
			break;

		case ID_BRIDGE_LABEL :
			newSystem = QMG_BRIDGE_LABEL;
			break;

		case ID_Q_VOYAGER_LABEL :
			newSystem = QMG_VOYAGER_LABEL;
			break;

		default:
			newSystem = QMG_VOYAGER_LABEL;
			break;
	}

	QuitMenu_ChangeAreaFocus(newSystem);

	// ten seconds from now, start the auto animation again
	quitmenu_graphics[QMG_ACTIVE_SYSTEM].timer = uis.realtime + 10000;
}

/*
===============
Quit_MenuKey
===============
*/
static sfxHandle_t Quit_MenuKey( int key )
{
	switch ( key )
	{
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
		case K_KP_RIGHTARROW:
		case K_RIGHTARROW:
			key = K_TAB;
			break;

		case 'n':
		case 'N':
			M_Quit_Event( &s_quit_no, QM_ACTIVATED );
			break;

		case 'y':
		case 'Y':
			M_Quit_Event( &s_quit_yes, QM_ACTIVATED );
			break;
	}

	return ( Menu_DefaultKey( &s_quitmenu.menu, key ) );
}

/*
===============
Quit_MenuBlinkies
===============
*/
static void Quit_MenuBlinkies( void )
{
	int activeSystem,i;

	// Move bottom blip
	if ( quitmenu_graphics[QMG_BOTTOM_BLIP].timer < uis.realtime )
	{
		quitmenu_graphics[QMG_BOTTOM_BLIP].x += 7;
		quitmenu_graphics[QMG_BOTTOM_BLIP2].x = quitmenu_graphics[QMG_BOTTOM_BLIP].x;

		if (quitmenu_graphics[QMG_BOTTOM_BLIP].x > quitmenu_graphics[QMG_BOTTOM_BLIP].max)
		{
			ui.S_StartLocalSound(s_quitmenu.pingsound, CHAN_LOCAL);

			quitmenu_graphics[QMG_BOTTOM_BLIP].x = quitmenu_graphics[QMG_BOTTOM_BLIP].min;
			quitmenu_graphics[QMG_BOTTOM_BLIP2].x = quitmenu_graphics[QMG_BOTTOM_BLIP].min;
		}

		// Middle ping?
		if ((quitmenu_graphics[QMG_BOTTOM_BLIP].x > 304) && 
			(quitmenu_graphics[QMG_BOTTOM_BLIP].x < 314))
		{
			ui.S_StartLocalSound(s_quitmenu.pingsound, CHAN_LOCAL);
		}

		quitmenu_graphics[QMG_BOTTOM_BLIP].timer = uis.realtime + 75;
	}

	// Time to change systems???
	if ( quitmenu_graphics[QMG_ACTIVE_SYSTEM].timer < uis.realtime )
	{
		activeSystem = quitmenu_graphics[QMG_ACTIVE_SYSTEM].target;

		// Change again in five seconds
		quitmenu_graphics[QMG_ACTIVE_SYSTEM].timer = uis.realtime + 5000;

		// Advance to next system
		++activeSystem;
		if (activeSystem >= QMG_LABEL_END)	// Past max strings
		{
			activeSystem = QMG_LABEL_START +1;	// Reset
		}

		QuitMenu_ChangeAreaFocus(activeSystem);

		ui.S_StartLocalSound( uis.menu_choice1_snd, CHAN_MENU1 );	// Ping!

	}

	// Generate new numbers
	if ((quitmenu_graphics[QMG_NUMBERS].timer < uis.realtime) && (quitmenu_graphics[QMG_NUMBERS].target==5))
	{

		for (i=0;i<5;++i)
		{
			quitmenu_graphics[i + QMG_COL1_NUM1].target = (random() * (900000000)) + 99999999;
			quitmenu_graphics[i + QMG_COL3_NUM1].target = (random() * (900000000)) + 99999999;
			quitmenu_graphics[i + QMG_COL4_NUM1].target = (random() * (900000000)) + 99999999;
		}

		for (i=0;i<5;++i)
		{
			quitmenu_graphics[i + QMG_COL2_NUM1].target = (random() * (900000)) + 99999;
		}

		for (i=0;i<5;++i)
		{
			quitmenu_graphics[i + QMG_COL5_NUM1].target = (random() * (90)) + 9;
			quitmenu_graphics[i + QMG_COL6_NUM1].target = (random() * (90)) + 9;
		}

		quitmenu_graphics[QMG_NUMBERS].target=1;

		// Turn off all but the first row
		for (i=0;i<4;++i)
		{
			quitmenu_graphics[i + QMG_COL1_NUM2].type = MG_OFF;
			quitmenu_graphics[i + QMG_COL2_NUM2].type = MG_OFF;
			quitmenu_graphics[i + QMG_COL3_NUM2].type = MG_OFF;
			quitmenu_graphics[i + QMG_COL4_NUM2].type = MG_OFF;
			quitmenu_graphics[i + QMG_COL5_NUM2].type = MG_OFF;
			quitmenu_graphics[i + QMG_COL6_NUM2].type = MG_OFF;
		}
	}
	// Activate the next row of numbers.  When at the last row wait and then generate new numbers
	else if ((quitmenu_graphics[QMG_NUMBERS].timer < uis.realtime) && (quitmenu_graphics[QMG_NUMBERS].target<5))
	{
		ui.S_StartLocalSound( uis.menu_datadisp1_snd, CHAN_MENU2 );	// 

		quitmenu_graphics[QMG_NUMBERS].timer = uis.realtime + 50;

		i = quitmenu_graphics[QMG_NUMBERS].target;
		quitmenu_graphics[i + QMG_COL1_NUM1].type = MG_NUMBER;
		quitmenu_graphics[i + QMG_COL2_NUM1].type = MG_NUMBER;
		quitmenu_graphics[i + QMG_COL3_NUM1].type = MG_NUMBER;
		quitmenu_graphics[i + QMG_COL4_NUM1].type = MG_NUMBER;
		quitmenu_graphics[i + QMG_COL5_NUM1].type = MG_NUMBER;
		quitmenu_graphics[i + QMG_COL6_NUM1].type = MG_NUMBER;

		++quitmenu_graphics[QMG_NUMBERS].target;

		if (quitmenu_graphics[QMG_NUMBERS].target == 5)
		{
			quitmenu_graphics[QMG_NUMBERS].timer = uis.realtime + 6000;
		}
	}
}


/*
===============
Quit_MenuDraw
===============
*/
static void Quit_MenuDraw( void )
{
	int i,y;

	UI_MenuFrame(&s_quitmenu.menu);

	UI_DrawProportionalString(  74,  66, "7617",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  84, "4396",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,  395, "431108",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30,364,  47, 25, uis.whiteShader);	// Left hand column
	UI_DrawHandlePic(80,364, 247, 7, uis.whiteShader);	// Top line above CONFIDENTIAL
	UI_DrawHandlePic(334,386, 30, 50, uis.whiteShader);	// LH Middle column
	UI_DrawHandlePic(327,364,  64, 64, s_quitmenu.swoop1);	// 

	UI_DrawHandlePic(334,386, 30, 50, uis.whiteShader);	// LH Middle column

	UI_DrawHandlePic(367,379, 15, 57, uis.whiteShader);	// RH Middle column
	UI_DrawHandlePic(387,364, 225, 7, uis.whiteShader);	// Top line above systems description
	UI_DrawHandlePic(367,364,  32, 32, s_quitmenu.swoop2);	// 

	ui.R_SetColor( colorTable[CT_BLACK]);
	UI_DrawHandlePic(30, 164, 47, 3, uis.whiteShader);	// Upper left hand blip columns
	UI_DrawHandlePic(30, 175, 47, 25, uis.whiteShader);	// Upper left hand blip columns

	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30, 167, 47, 8, uis.whiteShader);	// Blip columns
	UI_DrawHandlePic(565, 167, 47, 8, uis.whiteShader);	// Blip columns


	ui.R_SetColor( colorTable[CT_DKPURPLE2]);
	UI_DrawHandlePic(30, 353, 47, 8, uis.whiteShader);	// Blip columns
	UI_DrawHandlePic(565,353, 47, 8, uis.whiteShader);	// Blip columns

	UI_DrawProportionalString(97, 380, menu_normal_text[MNT_STARTREK], UI_SMALLFONT, colorTable[CT_LTBROWN1] );
	UI_DrawProportionalString(  97, 428, menu_normal_text[MNT_FORMOREINFO], UI_TINYFONT, colorTable[CT_LTBROWN1] );

	// Leave Voyager box
	ui.R_SetColor( colorTable[CT_DKBROWN1]);
	UI_DrawHandlePic(301,  57,  64,  32, s_quitmenu.corner_ur);		// Corner
	UI_DrawHandlePic( 81,  57, 224,  18, uis.whiteShader);	// Top
	UI_DrawHandlePic(305,  74,  60,  62, uis.whiteShader);	// Right side
	UI_DrawProportionalString(157,58,menu_normal_text[MNT_QUIT_ELITE_FORCE], UI_BLINK | UI_SMALLFONT, colorTable[CT_LTGOLD1] );

	// Litte squares off to the side of the LEAVE VOYAGER buttons
	ui.R_SetColor( colorTable[CT_DKPURPLE1]);
	UI_DrawHandlePic(140,81, 8, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);	// LEAVE VOYAGER?
	UI_DrawHandlePic(140,81 + MENU_BUTTON_MED_HEIGHT + 6, 8, MENU_BUTTON_MED_HEIGHT, uis.whiteShader);	// LEAVE VOYAGER?

	ui.R_SetColor( colorTable[CT_NONE]);

	UI_DrawProportionalString(  361,  126, "4096",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	UI_DrawProportionalString(  74,   366, "1411",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

	Menu_Draw( &s_quitmenu.menu );

	Quit_MenuBlinkies();

	UI_PrintMenuGraphics(quitmenu_graphics,QMG_MAX);

	y = 374;
	for (i=0;i<SYSTEM_MAXDESC;++i)
	{
		UI_DrawProportionalString(384, y, systemDesc[i],UI_TINYFONT,colorTable[CT_LTGOLD1]);
		y += 12;
	}

	uis.noversion				= 0;	// Because the Raven screen shuts off version
}


/*
===============
QuitMenu_Cache
===============
*/
void QuitMenu_Cache( void ) 
{
	s_quitmenu.corner_ur	= ui.R_RegisterShaderNoMip("menu/common/corner_ur_18_60.tga");
	s_quitmenu.swoop1		= ui.R_RegisterShaderNoMip("menu/common/swoop1.tga");
	s_quitmenu.swoop2		= ui.R_RegisterShaderNoMip("menu/common/swoop2.tga");
	s_quitmenu.pingsound	= ui.S_RegisterSound( "sound/interface/sensorping.wav" );

	// Precache all menu graphics in array
	UI_PrecacheMenuGraphics(quitmenu_graphics,QMG_MAX);

	ui.R_RegisterShaderNoMip("menu/voyager/phaser_strip.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/photon_launch.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/bottom_strip.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/mid_hull.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/warpnac.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/bussard.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/rcs.tga");
	ui.R_RegisterShaderNoMip("menu/voyager/bridge.tga");
	ui.R_RegisterShaderNoMip("menu/special/voy_1.tga");
}

/*
=================
UI_QuitMenu
=================
*/
static void Quit_MenuInit(void)
{
	int y,x,i;
	int	normalColor,picColor;
	int highlightColor;


	QuitMenu_Cache(); 

	s_quitmenu.menu.nitems					= 0;
	s_quitmenu.menu.fullscreen				= qtrue;
	s_quitmenu.menu.draw					= Quit_MenuDraw;
	s_quitmenu.menu.key						= Quit_MenuKey;
	s_quitmenu.menu.wrapAround				= qtrue;
	s_quitmenu.menu.descX					= MENU_DESC_X;
	s_quitmenu.menu.descY					= MENU_DESC_Y;
	s_quitmenu.menu.titleX					= MENU_TITLE_X;
	s_quitmenu.menu.titleY					= MENU_TITLE_Y;
	s_quitmenu.menu.titleI					= MNT_QUITMENU_TITLE;
	s_quitmenu.menu.footNoteEnum			= MNT_SHIP_SYSTEMS;

	s_quit_mainmenu.generic.type		= MTYPE_BITMAP;      
	s_quit_mainmenu.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	s_quit_mainmenu.generic.x			= 482;
	s_quit_mainmenu.generic.y			= 136;
	s_quit_mainmenu.generic.name		= GRAPHIC_BUTTONRIGHT;
	s_quit_mainmenu.generic.id			= ID_MAINMENU;
	s_quit_mainmenu.generic.callback	= M_Quit_Event;
	s_quit_mainmenu.width				= MENU_BUTTON_MED_WIDTH;
	s_quit_mainmenu.height				= MENU_BUTTON_MED_HEIGHT;
	s_quit_mainmenu.color				= CT_DKPURPLE1;
	s_quit_mainmenu.color2				= CT_LTPURPLE1;
	s_quit_mainmenu.textX				= MENU_BUTTON_TEXT_X;
	s_quit_mainmenu.textY				= MENU_BUTTON_TEXT_Y;
	s_quit_mainmenu.textEnum			= MBT_MAINMENU;
	s_quit_mainmenu.textcolor			= CT_BLACK;
	s_quit_mainmenu.textcolor2			= CT_WHITE;

	if (!ingameFlag)
	{
		s_quit_mainmenu.textEnum			= MBT_MAINMENU;
	}
	else	// In game menu
	{
		s_quit_mainmenu.textEnum			= MBT_INGAMEMENU;
	}
	
	y = 81;
	x = 152;
	s_quit_yes.generic.type				= MTYPE_BITMAP;      
	s_quit_yes.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS; 
	s_quit_yes.generic.x				= x;                 
	s_quit_yes.generic.y				= y;
	s_quit_yes.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_quit_yes.generic.id				= ID_YES;
	s_quit_yes.generic.callback			= M_Quit_Event;
	s_quit_yes.width					= MENU_BUTTON_MED_WIDTH;
	s_quit_yes.height					= MENU_BUTTON_MED_HEIGHT;
	s_quit_yes.color					= CT_DKPURPLE1;
	s_quit_yes.color2					= CT_LTPURPLE1;
	s_quit_yes.textX					= MENU_BUTTON_TEXT_X;
	s_quit_yes.textY					= MENU_BUTTON_TEXT_Y;
	s_quit_yes.textEnum					= MBT_QUITGAME_YES;
	s_quit_yes.textcolor				= CT_BLACK;
	s_quit_yes.textcolor2				= CT_WHITE;

	s_quit_no.generic.type				= MTYPE_BITMAP;      
	s_quit_no.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS;
	s_quit_no.generic.x					= x;
	s_quit_no.generic.y					= y + MENU_BUTTON_MED_HEIGHT + 6;
	s_quit_no.generic.name				= GRAPHIC_BUTTONRIGHT;
	s_quit_no.generic.id				= ID_NO;
	s_quit_no.generic.callback			= M_Quit_Event; 
	s_quit_no.width						= MENU_BUTTON_MED_WIDTH;
	s_quit_no.height					= MENU_BUTTON_MED_HEIGHT;
	s_quit_no.color						= CT_DKPURPLE1;
	s_quit_no.color2					= CT_LTPURPLE1;
	s_quit_no.textX						= MENU_BUTTON_TEXT_X;
	s_quit_no.textY						= MENU_BUTTON_TEXT_Y;
	s_quit_no.textEnum					= MBT_QUITGAME_NO;
	s_quit_no.textcolor					= CT_BLACK;
	s_quit_no.textcolor2				= CT_WHITE;


	normalColor		= CT_DKBROWN1;
	highlightColor	= CT_WHITE;
	picColor		= CT_LTGOLD1;

	s_quitmenu.phaser_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.phaser_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.phaser_label.generic.x					= 152;
	s_quitmenu.phaser_label.generic.y					= 290;
	s_quitmenu.phaser_label.generic.id					= ID_PHASER_LABEL;
	s_quitmenu.phaser_label.generic.callback			= M_Main_Event; 
	s_quitmenu.phaser_label.buttontextEnum				= MBT_V_PHASER_LABEL;
	s_quitmenu.phaser_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.phaser_label.color						= normalColor;
	s_quitmenu.phaser_label.color2						= highlightColor;

	s_quitmenu.phaser_pic.generic.type					= MTYPE_BITMAP;      
	s_quitmenu.phaser_pic.generic.flags					= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.phaser_pic.generic.x						= 99;
	s_quitmenu.phaser_pic.generic.y						= 225;
	s_quitmenu.phaser_pic.generic.name					= "menu/voyager/phaser_strip.tga";
	s_quitmenu.phaser_pic.width							= 256;
	s_quitmenu.phaser_pic.height						= 16;
	s_quitmenu.phaser_pic.color							= picColor;

	s_quitmenu.torpedo_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.torpedo_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.torpedo_label.generic.x					= 250;
	s_quitmenu.torpedo_label.generic.y					= 278;
	s_quitmenu.torpedo_label.generic.id					= ID_TORPEDO_LABEL;
	s_quitmenu.torpedo_label.generic.callback			= M_Main_Event; 
	s_quitmenu.torpedo_label.buttontextEnum				= MBT_V_TORPEDOS_LABEL;
	s_quitmenu.torpedo_label.buttontextEnum2			= MBT_V_TORPEDOS_LABEL2;
	s_quitmenu.torpedo_label.buttontextEnum3			= MBT_V_TORPEDOS_LABEL3;
	s_quitmenu.torpedo_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.torpedo_label.color						= normalColor;
	s_quitmenu.torpedo_label.color2						= highlightColor;

	s_quitmenu.torpedo_pic.generic.type					= MTYPE_BITMAP;      
	s_quitmenu.torpedo_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.torpedo_pic.generic.x					= 273;
	s_quitmenu.torpedo_pic.generic.y					= 265;
	s_quitmenu.torpedo_pic.generic.name					= "menu/voyager/photon_launch.tga";
	s_quitmenu.torpedo_pic.width						= 16;
	s_quitmenu.torpedo_pic.height						= 16;
	s_quitmenu.torpedo_pic.color						= picColor;

	s_quitmenu.ventral_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.ventral_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.ventral_label.generic.x					= 316;
	s_quitmenu.ventral_label.generic.y					= 322;
	s_quitmenu.ventral_label.generic.id					= ID_VENTRAL_LABEL;
	s_quitmenu.ventral_label.generic.callback			= M_Main_Event; 
	s_quitmenu.ventral_label.buttontextEnum				= MBT_V_VENTRAL_LABEL;
	s_quitmenu.ventral_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.ventral_label.color						= normalColor;
	s_quitmenu.ventral_label.color2						= highlightColor;

	s_quitmenu.ventral_pic.generic.type					= MTYPE_BITMAP;      
	s_quitmenu.ventral_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.ventral_pic.generic.x					= 330;
	s_quitmenu.ventral_pic.generic.y					= 296;
	s_quitmenu.ventral_pic.generic.name					= "menu/voyager/bottom_strip.tga";
	s_quitmenu.ventral_pic.width						= 16;
	s_quitmenu.ventral_pic.height						= 16;
	s_quitmenu.ventral_pic.color						= picColor;

	s_quitmenu.midhull_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.midhull_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_quitmenu.midhull_label.generic.x					= 381;
	s_quitmenu.midhull_label.generic.y					= 322;
	s_quitmenu.midhull_label.generic.id					= ID_MIDHULL_LABEL;
	s_quitmenu.midhull_label.generic.callback			= M_Main_Event; 
	s_quitmenu.midhull_label.buttontextEnum				= MBT_V_MIDHULL_LABEL;
	s_quitmenu.midhull_label.buttontextEnum2			= MBT_V_MIDHULL_LABEL2;
	s_quitmenu.midhull_label.style						= UI_TINYFONT;	
	s_quitmenu.midhull_label.color						= normalColor;
	s_quitmenu.midhull_label.color2						= highlightColor;

	s_quitmenu.midhull_pic.generic.type					= MTYPE_BITMAP;      
	s_quitmenu.midhull_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.midhull_pic.generic.x					= 357;
	s_quitmenu.midhull_pic.generic.y					= 244;
	s_quitmenu.midhull_pic.generic.name					= "menu/voyager/mid_hull.tga";
	s_quitmenu.midhull_pic.width						= 32;
	s_quitmenu.midhull_pic.height						= 8;
	s_quitmenu.midhull_pic.color						= picColor;

	s_quitmenu.nacelles_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.nacelles_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.nacelles_label.generic.x					= 560;
	s_quitmenu.nacelles_label.generic.y					= 180;
	s_quitmenu.nacelles_label.generic.id				= ID_NACELLES_LABEL;
	s_quitmenu.nacelles_label.generic.callback			= M_Main_Event; 
	s_quitmenu.nacelles_label.buttontextEnum			= MBT_V_NACELLES_LABEL;
	s_quitmenu.nacelles_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.nacelles_label.color						= normalColor;
	s_quitmenu.nacelles_label.color2					= highlightColor;

	s_quitmenu.nacelles_pic.generic.type				= MTYPE_BITMAP;      
	s_quitmenu.nacelles_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.nacelles_pic.generic.x					= 470;
	s_quitmenu.nacelles_pic.generic.y					= 265;
	s_quitmenu.nacelles_pic.generic.name				= "menu/voyager/warpnac.tga";
	s_quitmenu.nacelles_pic.width						= 256;
	s_quitmenu.nacelles_pic.height						= 32;
	s_quitmenu.nacelles_pic.color						= picColor;

	s_quitmenu.bussard_label.generic.type				= MTYPE_TEXT;      
	s_quitmenu.bussard_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_quitmenu.bussard_label.generic.x					= 489;
	s_quitmenu.bussard_label.generic.y					= 322;
	s_quitmenu.bussard_label.generic.id					= ID_BUSSARD_LABEL;
	s_quitmenu.bussard_label.generic.callback			= M_Main_Event; 
	s_quitmenu.bussard_label.buttontextEnum				= MBT_V_BUSSARD_LABEL;
	s_quitmenu.bussard_label.style						= UI_TINYFONT;	
	s_quitmenu.bussard_label.color						= normalColor;
	s_quitmenu.bussard_label.color2						= highlightColor;

	s_quitmenu.bussard_pic.generic.type					= MTYPE_BITMAP;      
	s_quitmenu.bussard_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.bussard_pic.generic.x					= 439;
	s_quitmenu.bussard_pic.generic.y					= 269;
	s_quitmenu.bussard_pic.generic.name					= "menu/voyager/bussard.tga";
	s_quitmenu.bussard_pic.width						= 32;
	s_quitmenu.bussard_pic.height						= 32;
	s_quitmenu.bussard_pic.color						= picColor;

	s_quitmenu.thrusters_label.generic.type				= MTYPE_TEXT;
	s_quitmenu.thrusters_label.generic.flags			= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.thrusters_label.generic.x				= 283;
	s_quitmenu.thrusters_label.generic.y				= 180;
	s_quitmenu.thrusters_label.generic.id				= ID_THRUSTERS_LABEL;
	s_quitmenu.thrusters_label.generic.callback			= M_Main_Event; 
	s_quitmenu.thrusters_label.buttontextEnum			= MBT_V_THRUSTERS_LABEL;
	s_quitmenu.thrusters_label.style					= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.thrusters_label.color					= normalColor;
	s_quitmenu.thrusters_label.color2					= highlightColor;

	s_quitmenu.thrusters_pic.generic.type				= MTYPE_BITMAP;
	s_quitmenu.thrusters_pic.generic.flags				= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.thrusters_pic.generic.x					= 314;
	s_quitmenu.thrusters_pic.generic.y					= 243;
	s_quitmenu.thrusters_pic.generic.name				= "menu/voyager/rcs.tga";
	s_quitmenu.thrusters_pic.width						= 32;
	s_quitmenu.thrusters_pic.height						= 16;
	s_quitmenu.thrusters_pic.color						= picColor;

	s_quitmenu.bridge_label.generic.type				= MTYPE_TEXT;
	s_quitmenu.bridge_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_RIGHT_JUSTIFY | QMF_MOUSEONLY;
	s_quitmenu.bridge_label.generic.x					= 152;
	s_quitmenu.bridge_label.generic.y					= 195;
	s_quitmenu.bridge_label.generic.id					= ID_BRIDGE_LABEL;
	s_quitmenu.bridge_label.generic.callback			= M_Main_Event; 
	s_quitmenu.bridge_label.buttontextEnum				= MBT_V_BRIDGE_LABEL;
	s_quitmenu.bridge_label.style						= UI_TINYFONT | UI_RIGHT;	
	s_quitmenu.bridge_label.color						= normalColor;
	s_quitmenu.bridge_label.color2						= highlightColor;

	s_quitmenu.bridge_pic.generic.type					= MTYPE_BITMAP;
	s_quitmenu.bridge_pic.generic.flags					= QMF_HIDDEN | QMF_INACTIVE;
	s_quitmenu.bridge_pic.generic.x						= 250;
	s_quitmenu.bridge_pic.generic.y						= 198;
	s_quitmenu.bridge_pic.generic.name					= "menu/voyager/bridge.tga";
	s_quitmenu.bridge_pic.width							= 32;
	s_quitmenu.bridge_pic.height						= 32;
	s_quitmenu.bridge_pic.color							= picColor;

	s_quitmenu.voyager_label.generic.type				= MTYPE_TEXT;
	s_quitmenu.voyager_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_quitmenu.voyager_label.generic.x					= 30;
	s_quitmenu.voyager_label.generic.y					= 325;
	s_quitmenu.voyager_label.generic.id					= ID_Q_VOYAGER_LABEL;
	s_quitmenu.voyager_label.generic.callback			= M_Main_Event; 
	s_quitmenu.voyager_label.buttontextEnum				= MBT_V_VOYAGER_LABEL;
	s_quitmenu.voyager_label.style						= UI_BIGFONT;	
	s_quitmenu.voyager_label.color						= normalColor;
	s_quitmenu.voyager_label.color2						= highlightColor;

	s_quitmenu.voyager_pic.generic.type					= MTYPE_BITMAP;
	s_quitmenu.voyager_pic.generic.flags				= QMF_INACTIVE;
	s_quitmenu.voyager_pic.generic.x					= 30;
	s_quitmenu.voyager_pic.generic.y					= 180;
	s_quitmenu.voyager_pic.generic.name					= "menu/special/voy_1.tga";
	s_quitmenu.voyager_pic.width						= 1024;
	s_quitmenu.voyager_pic.height						= 256;
	s_quitmenu.voyager_pic.color						= CT_LTBLUE1;

	s_quitmenu.raven_label.generic.type					= MTYPE_TEXT;
	s_quitmenu.raven_label.generic.flags				= QMF_HIGHLIGHT_IF_FOCUS | QMF_MOUSEONLY;
	s_quitmenu.raven_label.generic.x					= 97;
	s_quitmenu.raven_label.generic.y					= 398;
	s_quitmenu.raven_label.generic.id					= ID_Q_RAVEN_LABEL;
	s_quitmenu.raven_label.generic.callback				= M_Main_Event; 
	s_quitmenu.raven_label.buttontextEnum				= MBT_BYRAVENSOFTWARE;
	s_quitmenu.raven_label.style						= UI_SMALLFONT;	
	s_quitmenu.raven_label.color						= CT_LTBROWN1;
	s_quitmenu.raven_label.color2						= highlightColor;
	
	Menu_AddItem( &s_quitmenu.menu,	&s_quit_yes );             
	Menu_AddItem( &s_quitmenu.menu,	&s_quit_no );
	Menu_AddItem( &s_quitmenu.menu,	&s_quit_mainmenu );

	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.voyager_pic );     // This has to be first
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.voyager_label );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.bridge_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.bridge_label ); 
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.thrusters_pic );    
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.thrusters_label );  
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.nacelles_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.nacelles_label );   
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.bussard_pic );
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.bussard_label ); 
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.midhull_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.midhull_label );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.ventral_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.ventral_label ); 
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.torpedo_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.torpedo_label ); 
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.phaser_pic );     
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.phaser_label ); 
	Menu_AddItem( &s_quitmenu.menu,	&s_quitmenu.raven_label );

	s_quitmenu.menu.initialized = qtrue;

	quitmenu_graphics[QMG_BOTTOM_BLIP].x = quitmenu_graphics[QMG_BOTTOM_BLIP].min;
	quitmenu_graphics[QMG_BOTTOM_BLIP2].x = quitmenu_graphics[QMG_BOTTOM_BLIP].x;
	quitmenu_graphics[QMG_BOTTOM_BLIP].timer = uis.realtime + 50;

    // Turn off swoops
	for (i=QMG_SWOOP_START+1;i<QMG_SWOOP_END;++i)
	{
		quitmenu_graphics[i].type = CT_LTGREY;
	}

    // Turn off descriptions
	for (i=QMG_DESC_START+1;i<QMG_DESC_END;++i)
	{
		quitmenu_graphics[i].type = MG_OFF;
	}

	// Set labels to brown
	for (i=(QMG_LABEL_START+1);i<QMG_LABEL_END;++i)	
	{
		quitmenu_graphics[i].color = CT_DKBROWN1;
	}

	// Turn on active system info
	quitmenu_graphics[QMG_ACTIVE_SYSTEM].timer = uis.realtime + 100;	// When to change to next system
	quitmenu_graphics[QMG_ACTIVE_SYSTEM].target = QMG_LABEL_END - 1;	// Give it an old system to turn off

	QuitMenu_ChangeAreaFocus(QMG_LABEL_START + 1);

	// Force numbers to change
	quitmenu_graphics[QMG_NUMBERS].timer = 0;	// To get numbers right away
	quitmenu_graphics[QMG_NUMBERS].target=5;
}

/*
=================
UI_QuitMenu
=================
*/
void UI_QuitMenu( void )
{

	Quit_MenuInit();

	UI_PushMenu( &s_quitmenu.menu );
}





/*
===============
Raven_MenuDraw
===============
*/
static void Raven_MenuDraw( void )
{

	ui.R_SetColor( colorTable[CT_WHITE]);
	UI_DrawHandlePic( 120, 40, 400, 400, s_raven.bird);

	if (s_raven.raventimer < uis.realtime)
	{
			UI_PopMenu();
			uis.noversion= 0;
			return;
	}

	UI_DrawHandlePic( 500,  40,  64, 256,s_raven.bracket);	// Top right
	UI_DrawHandlePic( 500, 200,  64,-256,s_raven.bracket);	// Bottom right

	UI_DrawHandlePic(  76,  40, -64, 256,s_raven.bracket);	// Top left
	UI_DrawHandlePic(  76, 200, -64,-256,s_raven.bracket);	// Bottom left


	ui.R_SetColor( colorTable[CT_LTPURPLE2]);
	UI_DrawHandlePic( 522, 243, 15, 10, uis.whiteShader);		// Right bracket middle
	UI_DrawHandlePic( 103, 243, 15, 10, uis.whiteShader);		// Left bracket middle

	ui.R_SetColor( colorTable[CT_LTPURPLE1]);
	UI_DrawHandlePic( 545, 127, 11,  4, uis.whiteShader);		// 
	UI_DrawHandlePic( 545, 164, 11,  4, uis.whiteShader);		// 
	UI_DrawHandlePic( 545, 218, 11,  4, uis.whiteShader);		// 

	UI_DrawHandlePic( 545, 274, 11,  4, uis.whiteShader);		// 
	UI_DrawHandlePic( 545, 328, 11,  4, uis.whiteShader);		// 
	UI_DrawHandlePic( 545, 370, 11,  4, uis.whiteShader);		//

	UI_DrawProportionalString( 559, 121, "10",UI_SMALLFONT, colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString( 559, 158, "20",UI_SMALLFONT, colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString( 559, 212, "30",UI_SMALLFONT, colorTable[CT_LTRED1]);

	UI_DrawProportionalString( 559, 268, "40",UI_SMALLFONT, colorTable[CT_LTRED1]);
	UI_DrawProportionalString( 559, 322, "50",UI_SMALLFONT, colorTable[CT_LTBLUE1]);
	UI_DrawProportionalString( 559, 364, "60",UI_SMALLFONT, colorTable[CT_LTBLUE1]);

	UI_DrawProportionalString( 70,  70, "111238",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70,  84, "689100",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70,  98, "761",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 112, "00153",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 126, "259099",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 140, "18",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 158, "451",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 172, "19812",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 186, "90155776",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 200, "00101324",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 214, "99132517",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);

	UI_DrawProportionalString( 70, 274, "381232",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 288, "167",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 302, "007167",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 316, "54200",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 330, "243500",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 344, "0005655",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 358, "3452",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 372, "89899",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 386, "9",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 400, "4481",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);
	UI_DrawProportionalString( 70, 414, "51254",UI_RIGHT|UI_TINYFONT, colorTable[CT_DKGOLD1]);

	UI_DrawProportionalString( 320, 447, menu_normal_text[MNT_RESISTANCEISFUTILE],UI_CENTER|UI_BIGFONT, colorTable[CT_MDGREY]);	

}

/*
=================
Raven_MenuKey
=================
*/
static sfxHandle_t Raven_MenuKey( int key )
{
	return Menu_DefaultKey( &s_raven.menu, key );
}

/*
===============
UI_RavenMenu_Cache
===============
*/
void UI_RavenMenu_Cache( void ) 
{
	s_raven.borgsound	= ui.S_RegisterSound( "sound/enemies/borg/futile.mp3" );
	s_raven.bracket		= ui.R_RegisterShaderNoMip("menu/common/bird_bracket");
	s_raven.bird		= ui.R_RegisterShaderNoMip("menu/common/raven");
}

/*
=================
UI_RavenMenuInit
=================
*/
static void UI_RavenMenuInit(void)
{

	UI_RavenMenu_Cache(); 

	s_raven.menu.nitems					= 0;
	s_raven.menu.fullscreen				= qtrue;
	s_raven.menu.draw					= Raven_MenuDraw;
	s_raven.menu.key					= Raven_MenuKey;
	s_raven.menu.wrapAround				= qtrue;
}

/*
=================
UI_RavenMenu
=================
*/
static void UI_RavenMenu( void )
{

	UI_RavenMenuInit();

	UI_PushMenu( &s_raven.menu );

	ui.S_StartLocalSound( s_raven.borgsound,CHAN_MENU1);

	s_raven.raventimer			= uis.realtime + 10000;
	uis.noversion				= 1;
}


