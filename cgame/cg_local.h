#ifndef	__CG_LOCAL_H__
#define	__CG_LOCAL_H__

#include "../game/q_shared.h"

// define GAME_INCLUDE so that g_public.h does not define the
// short, server-visible gclient_t and gentity_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "../game/g_shared.h"
#include "cg_camera.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define	POWERUP_BLINKS		5
#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
// Zoom vars
#define	ZOOM_TIME			150		// not currently used?
#define MAX_ZOOM_FOV		5.0f
#define ZOOM_IN_TIME		1500.0f	
#define ZOOM_OUT_TIME		100.0f
#define ZOOM_START_PERCENT	0.3f	

#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	CHARSMALL_WIDTH		16
#define	CHARSMALL_HEIGHT	32

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define MAX_PRINTTEXT		128
#define MAX_CAPTIONTEXT		 64
#define MAX_LCARSTEXT		128


#define NUM_FONT_BIG   1
#define NUM_FONT_SMALL 2

#define CS_BASIC	0
#define CS_COMBAT	1
#define CS_EXTRA	2
#define CS_SCAV		3
//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
	lerpFrame_t		legs, torso;
	int				painTime;
	int				painDirection;	// flip from 0 to 1

	// For persistent beam weapons, so they don't play their start sound more than once
	qboolean		lightningFiring;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================


// Matches values in interfaceColors
typedef enum 
{
	MC_AMMOBRIGHT_STARFLEET,
	MC_AMMODARK_STARFLEET,
	MC_RED,
	MC_GREY,
	MC_AMMOBRIGHT_ALIEN,
	MC_AMMODARK_ALIEN,
	MC_DARKRED,
	MC_AMMOBRIGHT_PHASER,
	MC_AMMODARK_PHASER,
	MC_WHITE,
	MC_YELLOW,
	MC_MAX
} interfacecolor_t;


extern float interfaceColors[MC_MAX][4]; 

//=================================================


// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				miscTime;

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
	
	//Pointer to corresponding gentity
	gentity_t		*gent;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independently from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_PUFF,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_LIGHT
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE = 0x0001,			// do not scale size over time
	LEF_TUMBLE			= 0x0002,			// tumble over time, used for ejecting shells
	LEF_FADE_RGB		= 0x0004,			// explicitly fade
	LEF_NO_RANDOM_ROTATE= 0x0008			// MakeExplosion adds random rotate which could be bad in some cases
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;
	int				ownerGentNum;
} localEntity_t;

//======================================================================


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models;
	qhandle_t		icon;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


//======================================================================


// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after
 
typedef struct {
	int			clientFrame;		// incremented each frame
	
	qboolean	levelShot;			// taking a level menu screenshot

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet
	int			processedSnapshotNum;// the number of snapshots cgame has requested
	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			timelimitWarnings;	// 5 min, 1 min, overtime

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predicted_player_state;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	qboolean	zoomLocked;
	int			zoomTime;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];
	qboolean	showInformation;

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// Scrolling text, caption text and LCARS text use this
	char		printText[MAX_PRINTTEXT][128];	
	int			printTextY;			 	

	char		captionText[MAX_CAPTIONTEXT][128];	
	int			captionTextY;

	int			scrollTextLines;	// Number of lines being printed
	int			scrollTextTime;

	int			captionNextTextTime;
	int			captionTextCurrentLine;
	int			captionTextTime;
	int			captionLetterTime;

	int			gameNextTextTime;
	int			gameTextCurrentLine;
	int			gameTextTime;
	int			gameTextSpeaker;
	int			gameTextEntNum;
	int			gameLetterTime;

	char		LCARSText[MAX_LCARSTEXT][128];	
	int			LCARSTextLines;	// Number of lines being printed
	int			LCARSTextTime;

	// For flashing health armor counter
	int			oldhealth;
	int			oldHealthTime;
	int			oldarmor;
	int			oldArmorTime;
	int			oldammo;
	int			oldAmmoTime;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	int			interfaceStartupTime;	// Timer for menu graphics
	int			interfaceStartupDone;	// True when menu is done starting up

	// crosshair client ID
	int			crosshairClientNum;		//who you're looking at
	int			crosshairClientTime;	//last time you looked at them

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	int			loadLCARSStage;
	int			loadLCARScnt;

	int			missionInfoFlashTime;
	qboolean	missionStatusShow;
	int			missionStatusDeadTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

} cg_t;

//==============================================================================

typedef struct
{
	int				type;		// STRING or GRAPHIC
	float			timer;		// When it changes
	int				x;			// X position
	int				y;			// Y positon
	int				width;		// Graphic width
	int				height;		// Graphic height
	char			*file;		// File name of graphic/ text if STRING
	qhandle_t		graphic;	// Handle of graphic if GRAPHIC
	int				min;
	int				max;
	int				color;		// Normal color
	int				style;		// Style of characters
	void			*pointer;		// To an address
} interfacegraphics_s;


typedef enum 
{
	IG_GROW,

	IG_HEALTH_START,
	IG_HEALTH_BEGINCAP,
	IG_HEALTH_BOX1,
	IG_HEALTH_SLIDERFULL,
	IG_HEALTH_SLIDEREMPTY,
	IG_HEALTH_ENDCAP,
	IG_HEALTH_COUNT,
	IG_HEALTH_END,

	IG_ARMOR_START,
	IG_ARMOR_BEGINCAP,
	IG_ARMOR_BOX1,
	IG_ARMOR_SLIDERFULL,
	IG_ARMOR_SLIDEREMPTY,
	IG_ARMOR_ENDCAP,
	IG_ARMOR_COUNT,
	IG_ARMOR_END,

	IG_AMMO_START,
	IG_AMMO_UPPER_BEGINCAP,
	IG_AMMO_UPPER_ENDCAP,
	IG_AMMO_LOWER_BEGINCAP,
	IG_AMMO_SLIDERFULL,
	IG_AMMO_SLIDEREMPTY,
	IG_AMMO_LOWER_ENDCAP,
	IG_AMMO_COUNT,
	IG_AMMO_END,

	IG_MAX
} interface_graphics_t;


extern interfacegraphics_s interface_graphics[IG_MAX];

#define SG_OFF		0
#define SG_STRING	1
#define SG_GRAPHIC	2
#define SG_NUMBER	3
#define SG_VAR		4

typedef struct
{
	int				type;		// STRING or GRAPHIC
	float			timer;		// When it changes
	int				x;			// X position
	int				y;			// Y positon
	int				width;		// Graphic width
	int				height;		// Graphic height
	char			*file;		// File name of graphic/ text if STRING
	int				ingameEnum;	// Index to ingame_text[]
	qhandle_t		graphic;	// Handle of graphic if GRAPHIC
	int				min;		// 
	int				max;
	int				target;		// Final value
	int				inc;
	int				style;
	int				color;		// Normal color
	void			*pointer;		// To an address
} screengraphics_s;


extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];


extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_developer;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_skippingcin;
extern	vmCvar_t		cg_language;

extern	vmCvar_t		cg_pano;
extern	vmCvar_t		cg_panoNumShots;
extern	vmCvar_t		cg_freezeFX;
extern	vmCvar_t		cg_debugFX;
extern	vmCvar_t		fx_memoryInfo;
extern	vmCvar_t		cg_virtualVoyager;
extern	vmCvar_t		cg_missionInfoFlashTime;

// Additions for Elite Reinforce
extern	vmCvar_t		cg_drawOverbounceInfo;
extern	vmCvar_t		cg_drawStrafeHelper;
extern	vmCvar_t		cg_strafeHelperCenter;
extern	vmCvar_t		cg_strafeHelperCenterMarker;
extern	vmCvar_t		cg_strafeHelperHeight;
extern	vmCvar_t		cg_strafeHelperScale;
extern	vmCvar_t		cg_strafeHelperY;


void CG_NewClientinfo( int clientNum );
//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);

void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView );


//
// cg_drawtools.c
//

#define CG_LEFT			0x00000000	// default
#define CG_CENTER		0x00000001
#define CG_RIGHT		0x00000002
#define CG_FORMATMASK	0x00000007
#define CG_SMALLFONT	0x00000010
#define CG_BIGFONT		0x00000020	// default
#define CG_GIANTFONT	0x00000040
#define CG_DROPSHADOW	0x00000800
#define CG_BLINK		0x00001000
#define CG_INVERSE		0x00002000
#define CG_PULSE		0x00004000
#define CG_UNDERLINE	0x00008000
#define CG_TINYFONT		0x00010000


void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_Scissor( float x, float y, float width, float height);
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawRotatePic( float x, float y, float width, float height,float angle, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string, 
				   float charWidth, float charHeight, const float *modulate );
void CG_PrintInterfaceGraphics(int min,int max);
void CG_DrawNumField (int x, int y, int width, int value,int charWidth,int charHeight,int style,qboolean zeroFill);
void CG_PrintScreenGraphics(screengraphics_s *screenGraphics,int maxI);
void CG_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
int CG_ProportionalStringWidth( const char* str,int style );
void CG_LoadFonts(void);


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );


//
// cg_draw.c
//
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_ScrollText( const char *str, int y, int charWidth );
void CG_CaptionText( const char *str, int sound, int y, int charWidth );
void CG_CaptionTextStop( void );
void CG_GameText(int y, int charWidth );
void CG_LCARSText( const char *str, int y, int charWidth );

//
// cg_text.c
//
void CG_DrawScrollText( void );
void CG_DrawCaptionText( void );
void CG_DrawGameText( void );
void CG_DrawCenterString( void ); 
void CG_DrawLCARSText(void);


//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, int powerups, gentity_t *gent );

//
// cg_predict.c
//
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );

//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent, int color );
void CG_Cylinder( vec3_t start, vec3_t end, float radius, vec3_t color );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int atTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName, orientation_t *tagOrient );



//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );
void CG_RegisterItemSounds( int itemNum );

void CG_FireWeapon( centity_t *cent, qboolean alt_fire );
//void CG_ChargeWeapon( centity_t *cent );

void CG_AddViewWeapon (playerState_t *ps);
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?
void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );


//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader, 
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, int numframes, qhandle_t shader, int msec,
								qboolean isSprite, float scale = 1.0f );// Overloaded

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								qhandle_t hModel, int numframes, qhandle_t shader, int msec,
								qboolean isSprite, float scale, int flags );// Overloaded

localEntity_t *CG_AddTempLight( vec3_t origin, float scale, vec3_t color, int msec );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_info.c
//
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawScoreboard( void );
extern void CG_AnalysisUp_f( void );
extern void CG_AnalysisDown_f( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );


//===============================================

//
// system calls
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void	cgi_Printf( const char *fmt );

// abort the game
void	cgi_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int		cgi_Milliseconds( void );

// console variable interaction
void	cgi_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void	cgi_Cvar_Update( vmCvar_t *vmCvar );
void	cgi_Cvar_Set( const char *var_name, const char *value );


// ServerCommand and ConsoleCommand parameter access
int		cgi_Argc( void );
void	cgi_Argv( int n, char *buffer, int bufferLength );
void	cgi_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int		cgi_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
int		cgi_FS_Read( void *buffer, int len, fileHandle_t f );
int		cgi_FS_Write( const void *buffer, int len, fileHandle_t f );
void	cgi_FS_FCloseFile( fileHandle_t f );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void	cgi_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void	cgi_AddCommand( const char *cmdName );

// send a string to the server over the network
void	cgi_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void	cgi_UpdateScreen( void );

// model collision
void	cgi_CM_LoadMap( const char *mapname );
int		cgi_CM_NumInlineModels( void );
clipHandle_t cgi_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t cgi_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int		cgi_CM_PointContents( const vec3_t p, clipHandle_t model );
int		cgi_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void	cgi_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask );
void	cgi_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int		cgi_CM_MarkFragments( int numPoints, const vec3_t *points, 
				const vec3_t projection,
				int maxPoints, vec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void	cgi_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );

// a local sound is always played full volume
void	cgi_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void	cgi_S_ClearLoopingSounds( void );
void	cgi_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void	cgi_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void	cgi_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], qboolean inwater );
sfxHandle_t	cgi_S_RegisterSound( const char *sample );		// returns buzz if not found
void	cgi_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
float	cgi_S_GetSampleLength( sfxHandle_t sfx);


// I've made these into ints instead of original typedefs to cut down on rebuild time
//	if I update the module they're in. No point in rebuilding all CGAME modules...
//
void	cgi_FF_StartFX( int iFX );
void	cgi_FF_EnsureFX( int iFX );
void	cgi_FF_StopFX( int iFX );
void	cgi_FF_StopAllFX( void );




void	cgi_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	cgi_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	cgi_R_RegisterSkin( const char *name );			
qhandle_t	cgi_R_RegisterShader( const char *name );			// returns default shader if not found
qhandle_t	cgi_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void	cgi_R_ClearScene( void );
void	cgi_R_AddRefEntityToScene( const refEntity_t *re );
void	cgi_R_GetLighting( const vec3_t origin, vec3_t ambientLight, vec3_t directedLight, vec3_t ligthDir );

// polys are intended for simple wall marks, not really for doing
// significant construction
void	cgi_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void	cgi_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void	cgi_R_RenderScene( const refdef_t *fd );
void	cgi_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void	cgi_R_DrawStretchPic( float x, float y, float w, float h, 
	float s1, float t1, float s2, float t2, qhandle_t hShader );
void	cgi_R_DrawScreenShot( float x, float y, float w, float h);

void	cgi_R_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs );
void	cgi_R_LerpTag( orientation_t *tag, qhandle_t mod, int startFrame, int endFrame, 
					 float frac, const char *tagName );
void	cgi_R_DrawRotatePic( float x, float y, float w, float h, 
	float s1, float t1, float s2, float t2,float a, qhandle_t hShader );
void	cgi_R_Scissor( float x, float y, float w, float h);

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		cgi_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		cgi_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		cgi_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	cgi_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	cgi_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			cgi_GetCurrentCmdNumber( void );	
qboolean	cgi_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		cgi_SetUserCmdValue( int stateValue, float sensitivityScale );

void		cgi_S_UpdateAmbientSet( const char *name, vec3_t origin );
void		cgi_AS_ParseSets( void );
void		cgi_AS_AddPrecacheEntry( const char *name );
int			cgi_S_AddLocalSet( const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time );
sfxHandle_t	cgi_AS_GetBModelSound( const char *name, int stage );

//-----------------------------
// Effects related prototypes
//-----------------------------

// Misc teleport and disintegration effects
void FX_PlayerTeleportOut( centity_t *cent, vec3_t position );
void FX_PlayerTeleportIn( centity_t *cent, vec3_t position );
void FX_Replicator( centity_t *cent, vec3_t position );

void FX_Disruptor( vec3_t org, float length );
void FX_ForgeTeleportOut( vec3_t position );
void FX_ForgeBeamIn( vec3_t org );

void CG_EnergyGibs( int owner, vec3_t origin );
void FX_QuantumColumns( vec3_t origin );

void FX_Transporter( vec3_t org );
void FX_StasisBeamIn( vec3_t org );
void FX_StasisBeamOut( vec3_t org );
void FX_8472Teleport( vec3_t pos, int parm );

// Environmental effects 
void CG_Smoke( vec3_t origin, vec3_t dir, float radius, float speed, qhandle_t shader, int flags = 0 );
void CG_Spark( vec3_t origin, vec3_t dir );
void CG_Steam( vec3_t position, vec3_t dest, vec3_t dir, float scale, qboolean jet );
void CG_Bolt( centity_t *cent );
void CG_Drip( vec3_t origin, vec4_t startRGBA, float diameter, int thinktime, qhandle_t shader );
void CG_Plasma( vec3_t position, vec3_t dest, vec4_t startRGBA, vec4_t finalRGBA );
void CG_ElectricFire( vec3_t origin, vec3_t dir );
void CG_ElectricalExplosion( vec3_t start, vec3_t end, float radius );
void CG_Teleporter( centity_t *cent );
void CG_ParticleStream( centity_t *cent );
void CG_CookingSteam( vec3_t origin, float radius );
void CG_FireLaser( vec3_t start, vec3_t end, vec3_t normal, vec4_t laserRGB, qboolean hit_ent );
void CG_AimLaser( vec3_t start, vec3_t end, vec3_t normal );
void CG_TransporterStream( centity_t *cent );
void CG_ExplosionTrail( centity_t *cent );
void CG_BorgEnergyBeam( centity_t *cent );
void CG_ShimmeryThing( vec3_t start, vec3_t end, float radius, qboolean taper );
void CG_Borg_Bolt( centity_t *cent );
void CG_ForgeBolt( centity_t *cent );
void CG_StasisTeleporter( vec3_t origin, float radius, qboolean exit_fx );
void CG_StasisMushrooms( vec3_t origin, int count );
void CG_DreadnoughtBeamGlow( vec3_t origin, vec3_t normal, float radius );
void CG_MagicSmoke( vec3_t origin );
void CG_FountainSpurt( vec3_t origin );

void CG_ExplosionEffects( vec3_t origin, int intensity, int radius );
void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );
void CG_SmokeSpawn( vec3_t origin, vec3_t origin2, vec3_t dir, vec3_t user );

void FX_StasisMineExplode( vec3_t origin );

// Player weapons effects
void FX_PhaserFire( vec3_t start, vec3_t end, vec3_t normal, qboolean spark, qboolean impact, qboolean weak );
void FX_PhaserAltFire( vec3_t start, vec3_t end, vec3_t normal, qboolean spark, qboolean impact );

void FX_CompressionShot( vec3_t start, vec3_t end );
void FX_CompressionExplosion( vec3_t origin, vec3_t normal );
void FX_CompressionHit( vec3_t origin );
void FX_CompressionAltShot( vec3_t start, vec3_t end );
void FX_CompressionAltMiss( vec3_t origin, vec3_t normal );

void FX_IMODShot( vec3_t end, vec3_t start, vec3_t dir);
void FX_IMODExplosion( vec3_t origin, vec3_t normal );
void FX_AltIMODShot( vec3_t end, vec3_t start, vec3_t dir );
void FX_AltIMODExplosion( vec3_t origin, vec3_t normal );

void FX_ScavengerProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ScavengerAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ScavengerWeaponHitWall( vec3_t origin, vec3_t normal, qboolean fired_by_NPC, qboolean greyscale );
void FX_ScavengerWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean fired_by_NPC, qboolean greyscale );
void FX_ScavengerAltExplode( vec3_t origin, vec3_t normal );

void FX_StasisWeaponHitWall( vec3_t origin, vec3_t dir, int size );
void FX_StasisWeaponHitPlayer( vec3_t origin, vec3_t dir, int size );
void FX_StasisProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_StasisChange( vec3_t start, vec3_t end );
void FX_StasisAmbientThings( centity_t *cent );
void FX_StasisShot( centity_t *cent, vec3_t end, vec3_t start );
void FX_StasisShotImpact( vec3_t end, vec3_t dir );
void FX_StasisShotMiss( vec3_t end, vec3_t dir );

void FX_GrenadeExplode( vec3_t origin, vec3_t normal );
void FX_GrenadeShrapnelExplode( vec3_t origin, vec3_t norm );
void FX_GrenadeShrapnelBits( vec3_t start, vec3_t end, vec3_t dir );
void FX_GrenadeThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_GrenadeAltThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_GrenadeHitWall( vec3_t origin, vec3_t normal );
void FX_GrenadeHitPlayer( vec3_t origin, vec3_t normal );
void FX_GrenadeTeleport( vec3_t origin );

void FX_TetrionWeaponHitWall( vec3_t origin, vec3_t normal );
void FX_TetrionShot( vec3_t start, vec3_t end, vec3_t dir );
void FX_TetrionProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_TetrionRicochet( vec3_t origin, vec3_t normal );
void FX_TetrionAltHitWall( vec3_t origin, vec3_t normal );

void FX_DreadnoughtHitWall( vec3_t origin, vec3_t normal, qboolean spark );
void FX_DreadnoughtHitPlayer( vec3_t origin, vec3_t normal );
void FX_DreadnoughtFire( vec3_t origin, vec3_t end, vec3_t normal, qboolean spark );
void FX_DreadnoughtThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_DreadnoughtShotMiss( vec3_t origin, vec3_t dir );
void FX_DNTurretHitPlayer( vec3_t origin, vec3_t normal );
void FX_DNTurretHitWall( vec3_t origin, vec3_t normal );

void FX_QuantumThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_QuantumHitWall( vec3_t origin, vec3_t normal );
void FX_QuantumHitPlayer( vec3_t origin, vec3_t normal );
void FX_QuantumAltHitWall( vec3_t origin, vec3_t normal );

void FX_HypoSpray( vec3_t origin, vec3_t dir, qboolean red );  // When not red, it'll be blue

void FX_ProtonShot( vec3_t start, vec3_t end );
void FX_ProtonExplosion( vec3_t origin, vec3_t normal );
void FX_ProtonHit( vec3_t origin );
void FX_ProtonAltShot( vec3_t start, vec3_t end );
void FX_ProtonAltMiss( vec3_t origin, vec3_t normal );

// Borg effects
void FX_BorgTaser( vec3_t start, vec3_t end );
void FX_BWTaser( vec3_t end, vec3_t start );
void FX_BorgTeleport( vec3_t origin );
void FX_BorgRecycle( vec3_t origin );
void FX_BorgDeathSparks( vec3_t origin );
void FX_BorgHit( vec3_t origin, vec3_t dir );
void FX_BorgProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BorgWeaponHitWall( vec3_t origin, vec3_t normal );
void FX_BorgWeaponHitPlayer( vec3_t origin, vec3_t normal );
void FX_BorgShield( vec3_t position, vec3_t dir );

void CG_BounceEffect( centity_t *cent, int weapon, vec3_t origin, vec3_t normal );
void CG_MissileHitPlayer( centity_t *cent, int weapon, vec3_t origin, vec3_t dir );
void CG_MissileHitWall( centity_t *cent, int weapon, vec3_t origin, vec3_t dir );

// Enemy weapon effects
void CG_StasisFlierAttack( centity_t *cent );

void FX_BotWelder( vec3_t start, vec3_t end, vec3_t normal, qboolean impact );
void FX_BotLaser( vec3_t start, vec3_t end, vec3_t normal, qboolean impact );

void FX_BotRocketHitPlayer( vec3_t origin, vec3_t dir );
void FX_BotRocketHitWall( vec3_t origin, vec3_t dir );
void FX_BotTurretBolt( vec3_t start, vec3_t end, vec3_t dir );
void FX_BotExplode( centity_t *cent );

void FX_ForgeProjHitPlayer( vec3_t origin, vec3_t dir );
void FX_ForgeProjHitWall( vec3_t origin, vec3_t dir );

void FX_ForgeProj2HitPlayer( vec3_t origin, vec3_t dir );
void FX_ForgeProj2HitWall( vec3_t origin, vec3_t dir );

void FX_ForgePsychHitPlayer( vec3_t origin, vec3_t dir );
void FX_ForgePsychHitWall( vec3_t origin, vec3_t dir );

void FX_ParasiteAcidHitPlayer( vec3_t origin, vec3_t dir );
void FX_ParasiteAcidHitWall( vec3_t origin, vec3_t dir );

void FX_StasisAttackHitPlayer( vec3_t origin, vec3_t dir );
void FX_StasisAttackHitWall( vec3_t origin, vec3_t dir );

void FX_PaladinProjHitWall( vec3_t origin, vec3_t dir );
void FX_PaladinProjHitPlayer( vec3_t origin, vec3_t dir );

void FX_RifleShot( vec3_t start, vec3_t end );
void FX_RifleHitWall( vec3_t origin, vec3_t dir );
void FX_RifleHitPlayer( vec3_t origin );

#endif	//__CG_LOCAL_H__
