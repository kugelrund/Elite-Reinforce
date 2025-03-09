//NPC_stats.cpp
#include "b_local.h"
#include "b_public.h"
#include "boltOns.h"
#include "anims.h"

extern qboolean NPCsPrecached;
extern vec3_t playerMins;
extern vec3_t playerMaxs;

int						numKnownAnimFileSets = 0;

char	*TeamNames[TEAM_NUM_TEAMS] = 
{
	"",
	"starfleet",
	"borg",
	"parasite",
	"scavengers",
	"klingon",
	"malon",
	"hirogen",
	"imperial",
	"stasis",
	"species8472",
	"dreadnought",
	"forge",
	"disguise",
	"player (not valid)"
};

bool NPC_ShouldShowPath( gentity_t* self )
{
	if ( Q_stricmp( "all", g_showPaths->string ) == 0 )
	{
		return true;
	}
	if ( self->client->playerTeam >= 0 && self->client->playerTeam < TEAM_NUM_TEAMS &&
		 strstr( g_showPaths->string, TeamNames[self->client->playerTeam] ) != NULL )
	{
		return true;
	}
	if ( self->targetname != NULL && strstr( g_showPaths->string, self->targetname ) != NULL )
	{
		return true;
	}
	return false;
}

/*
NPC_ReactionTime
*/
//FIXME use grandom in here
int NPC_ReactionTime ( void ) 
{
	return 200 * ( 6 - NPCInfo->stats.reactions );
}

//
// parse support routines
//

qboolean G_ParseLiteral( char **data, const char *string ) 
{
	char	*token;

	token = COM_ParseExt( data, qtrue );
	if ( token[0] == 0 ) 
	{
		gi.Printf( "unexpected EOF\n" );
		return qtrue;
	}

	if ( Q_stricmp( token, string ) ) 
	{
		gi.Printf( "required string '%s' missing\n", string );
		return qtrue;
	}

	return qfalse;
}

qboolean G_ParseString( char **data, char **s ) 
{
	*s = COM_ParseExt( data, qfalse );
	if ( s[0] == 0 ) 
	{
		gi.Printf( "unexpected EOF\n" );
		return qtrue;
	}
	return qfalse;
}


qboolean G_ParseInt( char **data, int *i ) 
{
	char	*token;

	token = COM_ParseExt( data, qfalse );
	if ( token[0] == 0 ) 
	{
		gi.Printf( "unexpected EOF\n" );
		return qtrue;
	}

	*i = atoi( token );
	return qfalse;
}

qboolean G_ParseFloat( char **data, float *f ) 
{
	char	*token;

	token = COM_ParseExt( data, qfalse );
	if ( token[0] == 0 ) 
	{
		gi.Printf( "unexpected EOF\n" );
		return qtrue;
	}

	*f = atof( token );
	return qfalse;
}
//
// NPC parameters file : scripts/NPCs.cfg
//
#define MAX_NPC_DATA_SIZE 0x10000
char	NPCParms[MAX_NPC_DATA_SIZE];

team_t TranslateTeamName( const char *name ) 
{
	for ( int n = (TEAM_FREE + 1); n < TEAM_NUM_TEAMS; n++ )
	{
		if ( Q_stricmp( TeamNames[n], name ) == 0 )
		{
			return ((team_t) n);
		}
	}
	
	return TEAM_FREE;
}

static race_t TranslateRaceName( const char *name ) 
{
	if ( !Q_stricmp( name, "human" ) ) 
	{
		return RACE_HUMAN;
	}
	
	if ( !Q_stricmp( name, "borg" ) ) 
	{
		return RACE_BORG;
	}
	
	if ( !Q_stricmp( name, "parasite" ) ) 
	{
		return RACE_PARASITE;
	}
	
	if ( !Q_stricmp( name, "klingon" ) ) 
	{
		return RACE_KLINGON;
	}
	
	if ( !Q_stricmp( name, "malon" ) ) 
	{
		return RACE_MALON;
	}
	
	if ( !Q_stricmp( name, "hirogen" ) ) 
	{
		return RACE_HIROGEN;
	}
	
	if ( !Q_stricmp( name, "stasis" ) ) 
	{
		return RACE_STASIS;
	}
	
	if ( !Q_stricmp( name, "species8472" ) ) 
	{
		return RACE_8472;
	}
	
	if ( !Q_stricmp( name, "dreadnought" ) ) 
	{
		return RACE_BOT;
	}
	
	if ( !Q_stricmp( name, "harvester" ) || !Q_stricmp( name, "biohulk" ) ) 
	{
		return RACE_HARVESTER;
	}
	
	if ( !Q_stricmp( name, "reaver" ) ) 
	{
		return RACE_REAVER;
	}

	if ( !Q_stricmp( name, "avatar" ) ) 
	{
		return RACE_AVATAR;
	}

	if ( !Q_stricmp( name, "vulcan" ) ) 
	{
		return RACE_VULCAN;
	}

	if ( !Q_stricmp( name, "betazoid" ) ) 
	{
		return RACE_BETAZOID;
	}

	if ( !Q_stricmp( name, "bolian" ) ) 
	{
		return RACE_BOLIAN;
	}

	if ( !Q_stricmp( name, "talaxian" ) ) 
	{
		return RACE_TALAXIAN;
	}

	if ( !Q_stricmp( name, "bajoran" ) ) 
	{
		return RACE_BAJORAN;
	}

	if ( !Q_stricmp( name, "hologram" ) ) 
	{
		return RACE_HOLOGRAM;
	}

	return RACE_NONE;
}

/*
static rank_t TranslateRankName( const char *name ) 

  Should be used to determine pip bolt-ons
*/
static rank_t TranslateRankName( const char *name ) 
{
	if ( !Q_stricmp( name, "civilian" ) ) 
	{
		return RANK_CIVILIAN;
	}

	if ( !Q_stricmp( name, "crewman" ) ) 
	{
		return RANK_CREWMAN;
	}
	
	if ( !Q_stricmp( name, "ensign" ) ) 
	{
		return RANK_ENSIGN;
	}
	
	if ( !Q_stricmp( name, "ltjg" ) ) 
	{
		return RANK_LT_JG;
	}
	
	if ( !Q_stricmp( name, "lt" ) ) 
	{
		return RANK_LT;
	}
	
	if ( !Q_stricmp( name, "ltcomm" ) ) 
	{
		return RANK_LT_COMM;
	}
	
	if ( !Q_stricmp( name, "commander" ) ) 
	{
		return RANK_COMMANDER;
	}
	
	if ( !Q_stricmp( name, "captain" ) ) 
	{
		return RANK_CAPTAIN;
	}

	return RANK_CIVILIAN;
}

/* static int MethodNameToNumber( const char *name ) {
	if ( !Q_stricmp( name, "EXPONENTIAL" ) ) {
		return METHOD_EXPONENTIAL;
	}
	if ( !Q_stricmp( name, "LINEAR" ) ) {
		return METHOD_LINEAR;
	}
	if ( !Q_stricmp( name, "LOGRITHMIC" ) ) {
		return METHOD_LOGRITHMIC;
	}
	if ( !Q_stricmp( name, "ALWAYS" ) ) {
		return METHOD_ALWAYS;
	}
	if ( !Q_stricmp( name, "NEVER" ) ) {
		return METHOD_NEVER;
	}
	return -1;
}

static int ItemNameToNumber( const char *name, int itemType ) {
//	int		n;

	for ( n = 0; n < bg_numItems; n++ ) {
		if ( bg_itemlist[n].type != itemType ) {
			continue;
		}
		if ( Q_stricmp( bg_itemlist[n].classname, name ) == 0 ) {
			return bg_itemlist[n].tag;
		}
	}
	return -1;
}
*/

static int MoveTypeNameToEnum( const char *name ) 
{
	if(!Q_stricmp("runjump", name))
	{
		return MT_RUNJUMP;
	}
	else if(!Q_stricmp("walk", name))
	{
		return MT_WALK;
	}
	else if(!Q_stricmp("flyswim", name))
	{
		return MT_FLYSWIM;
	}
	else if(!Q_stricmp("static", name))
	{
		return MT_STATIC;
	}

	return MT_STATIC;
}

extern void CG_RegisterClientRenderInfo(clientInfo_t *ci, renderInfo_t *ri);
extern void CG_RegisterClientModels (int entityNum);
extern void CG_RegisterNPCCustomSounds( clientInfo_t *ci );
extern void CG_RegisterNPCEffects( team_t team );
extern void CG_ParseAnimationSndFile( const char *filename, int animFileIndex );

/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc

======================
*/
qboolean G_ParseAnimationFile( const char *filename ) 
{
	char		*text_p;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	int			animNum;
	animation_t	*animations = knownAnimFileSets[numKnownAnimFileSets].animations;

	// load the file
	len = gi.FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) 
	{
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) 
	{
		gi.Printf( "File %s too long\n", filename );
		return qfalse;
	}

	gi.FS_Read( text, len, f );
	text[len] = 0;
	gi.FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quiet the compiler warning

	//FIXME: have some way of playing anims backwards... negative numFrames?

	//initialize anim array so that from 0 to MAX_ANIMATIONS, set default values of 0 1 0 100
	for(i = 0; i < MAX_ANIMATIONS; i++)
	{
		animations[i].firstFrame = 0;
		animations[i].numFrames = 0;
		animations[i].loopFrames = -1;
		animations[i].frameLerp = 100;
		animations[i].initialLerp = 100;
	}

	// read information for each frame
	while(1) 
	{
		token = COM_Parse( &text_p );

		if ( !token || !token[0]) 
		{
			break;
		}

		animNum = GetIDForString(animTable, token);
		if(animNum == -1)
		{
			Com_Printf(S_COLOR_RED"WARNING: Unknown token %s in %s\n", token, filename);
			continue;
		}

		token = COM_Parse( &text_p );
		if ( !token ) 
		{
			break;
		}
		animations[animNum].firstFrame = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) 
		{
			break;
		}
		animations[animNum].numFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) 
		{
			break;
		}
		animations[animNum].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !token ) 
		{
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) 
		{
			fps = 1;//Don't allow divide by zero error
		}
		if ( fps < 0 )
		{//backwards
			animations[animNum].frameLerp = floor(1000.0f / fps);
		}
		else
		{
			animations[animNum].frameLerp = ceil(1000.0f / fps);
		}

		animations[animNum].initialLerp = ceil(1000.0f / fabs(fps));
	}

	return qtrue;
}

qboolean G_ParseAnimFileSet( const char *filename, int *animFileIndex, qboolean cacheSounds )
{
	char		afilename[MAX_QPATH];
	char		strippedName[MAX_QPATH];
	int			i;
	char		*slash;

	Q_strncpyz( strippedName, filename, sizeof(strippedName), qtrue);			
	slash = strchr( strippedName, '/' );
	if ( slash ) 
	{
		// truncate modelName to find just the dir not the extension
		*slash = 0;
	}

	//if this anims file was loaded before, don't parse it again, just point to the correct table of info
	for ( i = 0; i < numKnownAnimFileSets; i++ )
	{
		if ( Q_stricmp(knownAnimFileSets[i].filename, strippedName ) == 0 )
		{
			*animFileIndex = i;
			if ( cacheSounds )
			{//we want to cache sounds do it NOW
				CG_ParseAnimationSndFile( strippedName, *animFileIndex );
			}
			return qtrue;
		}
	}

	if ( numKnownAnimFileSets == MAX_ANIM_FILES )
	{//TOO MANY!
		G_Error( "G_ParseAnimFileSet: MAX_ANIM_FILES" );
	}

	//Okay, time to parse in a new one
	Q_strncpyz( knownAnimFileSets[numKnownAnimFileSets].filename, strippedName, sizeof( knownAnimFileSets[numKnownAnimFileSets].filename ) );

	// Load and parse animations.cfg file
	Com_sprintf( afilename, sizeof( afilename ), "models/players/%s/animation.cfg", strippedName );
	if ( !G_ParseAnimationFile( afilename ) )
	{
		*animFileIndex = -1;
		return qfalse;
	}

	//set index and increment
	*animFileIndex = numKnownAnimFileSets++;

	if ( cacheSounds )
	{//we want to cache sounds do it NOW
		CG_ParseAnimationSndFile( strippedName, *animFileIndex );
	}

	return qtrue;
}

void NPC_PrecacheAnimationCFG( const char *NPC_type )
{
	char	*token;
	char	*value;
	char	*p;
	int		junk;

	if ( !Q_stricmp( "random", NPC_type ) )
	{//sorry, can't precache a random just yet
		return;
	}

	p = NPCParms;
	COM_BeginParseSession();

	// look for the right NPC
	while ( p ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 )
			return;

		if ( !Q_stricmp( token, NPC_type ) ) 
		{
			break;
		}

		SkipBracedSection( &p );
	}

	if ( !p ) 
	{
		return;
	}

	if ( G_ParseLiteral( &p, "{" ) ) 
	{
		return;
	}

	// parse the NPC info block
	while ( 1 ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if ( !token[0] ) 
		{
			gi.Printf( S_COLOR_RED"ERROR: unexpected EOF while parsing '%s'\n", NPC_type );
			return;
		}

		if ( !Q_stricmp( token, "}" ) ) 
		{
			break;
		}

		// legsmodel
		if ( !Q_stricmp( token, "legsmodel" ) ) 
		{
			if ( G_ParseString( &p, &value ) ) 
			{
				continue;
			}
			G_ParseAnimFileSet( value, &junk, qfalse );
			return;
		}
	}
}

/*
void NPC_Precache ( char *NPCName )

Precaches NPC skins, tgas and md3s.

*/
void NPC_Precache ( gentity_t *spawner )
{
	clientInfo_t	ci={0};
	renderInfo_t	ri={0};
	team_t			playerTeam = TEAM_FREE;
	char	*token;
	char	*value;
	char	*p;
	char	*patch;
	char	sound[MAX_QPATH];

	if ( !Q_stricmp( "random", spawner->NPC_type ) )
	{//sorry, can't precache a random just yet
		return;
	}

	p = NPCParms;
	COM_BeginParseSession();

	// look for the right NPC
	while ( p ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 )
			return;

		if ( !Q_stricmp( token, spawner->NPC_type ) ) 
		{
			break;
		}

		SkipBracedSection( &p );
	}

	if ( !p ) 
	{
		return;
	}

	if ( G_ParseLiteral( &p, "{" ) ) 
	{
		return;
	}

	// parse the NPC info block
	while ( 1 ) 
	{
		token = COM_ParseExt( &p, qtrue );
		if ( !token[0] ) 
		{
			gi.Printf( S_COLOR_RED"ERROR: unexpected EOF while parsing '%s'\n", spawner->NPC_type );
			return;
		}

		if ( !Q_stricmp( token, "}" ) ) 
		{
			break;
		}

		// headmodel
		if ( !Q_stricmp( token, "headmodel" ) ) 
		{
			if ( G_ParseString( &p, &value ) ) 
			{
				continue;
			}

			if(!Q_stricmp("none", value))
			{
			}
			else
			{
				Q_strncpyz( ri.headModelName, value, sizeof(ri.headModelName), qtrue);
			}
			continue;
		}
		
		// torsomodel
		if ( !Q_stricmp( token, "torsomodel" ) ) 
		{
			if ( G_ParseString( &p, &value ) ) 
			{
				continue;
			}

			if(!Q_stricmp("none", value))
			{
			}
			else
			{
				Q_strncpyz( ri.torsoModelName, value, sizeof(ri.torsoModelName), qtrue);
			}
			continue;
		}

		// legsmodel
		if ( !Q_stricmp( token, "legsmodel" ) ) 
		{
			if ( G_ParseString( &p, &value ) ) 
			{
				continue;
			}
			Q_strncpyz( ri.legsModelName, value, sizeof(ri.legsModelName), qtrue);			
			continue;
		}

		// playerTeam
		if ( !Q_stricmp( token, "playerTeam" ) ) 
		{
			if ( G_ParseString( &p, &value ) ) 
			{
				continue;
			}
			playerTeam = TranslateTeamName(value);
			continue;
		}

		// snd
		if ( !Q_stricmp( token, "snd" ) ) {
			if ( G_ParseString( &p, &value ) ) {
				continue;
			}
			if ( !(spawner->svFlags&SVF_NO_BASIC_SOUNDS) )
			{
				//FIXME: store this in some sound field or parse in the soundTable like the animTable...
				Q_strncpyz( sound, value, sizeof( sound ) );
				patch = strstr( sound, "/" );
				if ( patch ) 
				{
					*patch = 0;
				}
				ci.customBasicSoundDir = G_NewString( sound );
			}
			continue;
		}

		// sndcombat
		if ( !Q_stricmp( token, "sndcombat" ) ) {
			if ( G_ParseString( &p, &value ) ) {
				continue;
			}
			if ( !(spawner->svFlags&SVF_NO_COMBAT_SOUNDS) )
			{
				//FIXME: store this in some sound field or parse in the soundTable like the animTable...
				Q_strncpyz( sound, value, sizeof( sound ) );
				patch = strstr( sound, "/" );
				if ( patch ) 
				{
					*patch = 0;
				}
				ci.customCombatSoundDir = G_NewString( sound );
			}
			continue;
		}
		
		// sndextra
		if ( !Q_stricmp( token, "sndextra" ) ) {
			if ( G_ParseString( &p, &value ) ) {
				continue;
			}
			if ( !(spawner->svFlags&SVF_NO_EXTRA_SOUNDS) )
			{
				//FIXME: store this in some sound field or parse in the soundTable like the animTable...
				Q_strncpyz( sound, value, sizeof( sound ) );
				patch = strstr( sound, "/" );
				if ( patch ) 
				{
					*patch = 0;
				}
				ci.customExtraSoundDir = G_NewString( sound );
			}
			continue;
		}

		// sndscav
		if ( !Q_stricmp( token, "sndscav" ) ) {
			if ( G_ParseString( &p, &value ) ) {
				continue;
			}
			if ( !(spawner->svFlags&SVF_NO_SCAV_SOUNDS) )
			{
				//FIXME: store this in some sound field or parse in the soundTable like the animTable...
				Q_strncpyz( sound, value, sizeof( sound ) );
				patch = strstr( sound, "/" );
				if ( patch ) 
				{
					*patch = 0;
				}
				ci.customScavSoundDir = G_NewString( sound );
			}
			continue;
		}
	}

	CG_RegisterClientRenderInfo( &ci, &ri );
	CG_RegisterNPCCustomSounds( &ci );
	CG_RegisterNPCEffects( playerTeam );
	//FIXME: precache the beam-in/exit sound and effect if not silentspawn
	//FIXME: Look for a "sounds" directory and precache death, pain, alert sounds
}

void NPC_BuildRandom( gentity_t *NPC )
{
	int	sex, color, head;

	sex = Q_irand(0, 2);
	color = Q_irand(0, 2);
	switch( sex )
	{
	case 0://female
		head = Q_irand(0, 2);
		switch( head )
		{
		default:
		case 0:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "garren", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 1:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "garren/salma", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 2:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "garren/mackey", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			color = Q_irand(3, 5);//torso needs to be afam
			break;
		}
		switch( color )
		{
		default:
		case 0:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale/gold", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 1:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 2:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale/blue", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 3:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale/aframG", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 4:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale/aframR", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 5:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewfemale/aframB", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		}
		Q_strncpyz( NPC->client->renderInfo.legsModelName, "crewfemale", sizeof(NPC->client->renderInfo.legsModelName), qtrue );
		break;
	default:
	case 1://male
	case 2://male
		head = Q_irand(0, 4);
		switch( head )
		{
		default:
		case 0:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "chakotay/nelson", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 1:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "paris/chase", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 2:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "doctor/pasty", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 3:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "kim/durk", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		case 4:
			Q_strncpyz( NPC->client->renderInfo.headModelName, "paris/kray", sizeof(NPC->client->renderInfo.headModelName), qtrue );
			break;
		}
		switch( color )
		{
		default:
		case 0:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewthin/red", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 1:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewthin", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
		case 2:
			Q_strncpyz( NPC->client->renderInfo.torsoModelName, "crewthin/blue", sizeof(NPC->client->renderInfo.torsoModelName), qtrue );
			break;
			//NOTE: 3 - 5 should be red, gold & blue, afram hands
		}
		Q_strncpyz( NPC->client->renderInfo.legsModelName, "crewthin", sizeof(NPC->client->renderInfo.legsModelName), qtrue );
		break;
	}

	NPC->client->renderInfo.scaleXYZ[0] = NPC->client->renderInfo.scaleXYZ[1] = NPC->client->renderInfo.scaleXYZ[2] = Q_irand(87, 102);
	NPC->client->race = RACE_HUMAN;
	NPC->NPC->rank = RANK_CREWMAN;
	NPC->client->playerTeam = TEAM_STARFLEET;
	NPC->client->clientInfo.customBasicSoundDir = "munro";
}

qboolean NPC_ParseParms( const char *NPCName, gentity_t *NPC ) 
{
	char	*token;
	char	*value;
	char	*p;
	int		n;
	float	f;
	char	*patch;
	char	sound[MAX_QPATH];
	clientInfo_t	*ci = &NPC->client->clientInfo;
	renderInfo_t	*ri = &NPC->client->renderInfo;
	gNPCstats_t		*stats = &NPC->NPC->stats;

	if ( !NPCName || !NPCName[0]) 
	{
		NPCName = "Player";
	}

	Q_strncpyz( ci->name, NPCName, sizeof( ci->name ) );
	
	// fill in defaults
	stats->aggression	= 3;
	stats->aim			= 3;
	stats->earshot		= 1024;
	stats->evasion		= 3;
	stats->hfov			= 75;
	stats->intelligence	= 3;
	stats->move			= 3;
	stats->reactions	= 3;
	stats->vfov			= 45;
	stats->vigilance	= 0.1f;
	stats->visrange		= 1024;

	stats->moveType		= MT_RUNJUMP;
	stats->yawSpeed		= 90;
	stats->walkSpeed	= 90;
	stats->runSpeed		= 300;
	stats->acceleration	= 15;//Increase/descrease speed this much per frame (20fps)

	ri->scaleXYZ[0] = ri->scaleXYZ[1] = ri->scaleXYZ[2] = 100;
	
	//Set defaults
	//FIXME: should probably put default torso and head models, but what about enemies
	//that don't have any- like Stasis?
	Q_strncpyz( ri->headModelName,	DEFAULT_HEADMODEL,  sizeof(ri->headModelName),	qtrue);
	Q_strncpyz( ri->torsoModelName, DEFAULT_TORSOMODEL, sizeof(ri->torsoModelName),	qtrue);
	Q_strncpyz( ri->legsModelName,	DEFAULT_LEGSMODEL,  sizeof(ri->legsModelName),	qtrue);

	//FIXME: should we have one for weapon too?
	
	/*
	ri->headYawRangeLeft = 50;
	ri->headYawRangeRight = 50;
	ri->headPitchRangeUp = 40;
	ri->headPitchRangeDown = 50;
	ri->torsoYawRangeLeft = 60;
	ri->torsoYawRangeRight = 60;
	ri->torsoPitchRangeUp = 30;
	ri->torsoPitchRangeDown = 70;
	*/

	ri->headYawRangeLeft = 80;
	ri->headYawRangeRight = 80;
	ri->headPitchRangeUp = 45;
	ri->headPitchRangeDown = 45;
	ri->torsoYawRangeLeft = 90;
	ri->torsoYawRangeRight = 90;
	ri->torsoPitchRangeUp = 30;
	ri->torsoPitchRangeDown = 50;

/*
	NPC->NPC->allWeaponOrder[0]	= WP_COMPRESSION_RIFLE;
	NPC->NPC->allWeaponOrder[1]	= WP_PHASER;
	NPC->NPC->allWeaponOrder[2]	= WP_IMOD;
	NPC->NPC->allWeaponOrder[3]	= WP_SCAVENGER_RIFLE;
	NPC->NPC->allWeaponOrder[4]	= WP_TRICORDER;
	NPC->NPC->allWeaponOrder[6]	= WP_NONE;
	NPC->NPC->allWeaponOrder[6]	= WP_NONE;
	NPC->NPC->allWeaponOrder[7]	= WP_NONE;
*/

	VectorCopy(playerMins, NPC->mins);
	VectorCopy(playerMaxs, NPC->maxs);
	NPC->client->crouchheight = CROUCH_MAXS_2;
	NPC->client->standheight = DEFAULT_MAXS_2;

	//ci->customBasicSoundDir = "munro";

	if ( !Q_stricmp( "random", NPCName ) )
	{//Randomly assemble a starfleet guy
		NPC_BuildRandom( NPC );
	}
	else
	{
		p = NPCParms;
		COM_BeginParseSession();

		// look for the right NPC
		while ( p ) 
		{
			token = COM_ParseExt( &p, qtrue );
			if ( token[0] == 0 )
			{
				return qfalse;
			}

			if ( !Q_stricmp( token, NPCName ) ) 
			{
				break;
			}

			SkipBracedSection( &p );
		}
		if ( !p ) 
		{
			return qfalse;
		}

		if ( G_ParseLiteral( &p, "{" ) ) 
		{
			return qfalse;
		}
			
		// parse the NPC info block
		while ( 1 ) 
		{
			token = COM_ParseExt( &p, qtrue );
			if ( !token[0] ) 
			{
				gi.Printf( S_COLOR_RED"ERROR: unexpected EOF while parsing '%s'\n", NPCName );
				return qfalse;
			}

			if ( !Q_stricmp( token, "}" ) ) 
			{
				break;
			}
	//===MODEL PROPERTIES===========================================================
			// headmodel
			if ( !Q_stricmp( token, "headmodel" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}

				if(!Q_stricmp("none", value))
				{
					ri->headModelName[0] = NULL;
					//Zero the head clamp range so the torso & legs don't lag behind
					ri->headYawRangeLeft = 
					ri->headYawRangeRight = 
					ri->headPitchRangeUp = 
					ri->headPitchRangeDown = 0;
				}
				else
				{
					Q_strncpyz( ri->headModelName, value, sizeof(ri->headModelName), qtrue);
				}
				continue;
			}
			
			// torsomodel
			if ( !Q_stricmp( token, "torsomodel" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}

				if(!Q_stricmp("none", value))
				{
					ri->torsoModelName[0] = NULL;
					//Zero the torso clamp range so the legs don't lag behind
					ri->torsoYawRangeLeft = 
					ri->torsoYawRangeRight = 
					ri->torsoPitchRangeUp = 
					ri->torsoPitchRangeDown = 0;
				}
				else
				{
					Q_strncpyz( ri->torsoModelName, value, sizeof(ri->torsoModelName), qtrue);
				}
				continue;
			}

			// legsmodel
			if ( !Q_stricmp( token, "legsmodel" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				Q_strncpyz( ri->legsModelName, value, sizeof(ri->legsModelName), qtrue);			
				//Need to do this here to get the right index
				G_ParseAnimFileSet( value, &ci->animFileIndex, qfalse );
				continue;
			}
			
			//headYawRangeLeft
			if ( !Q_stricmp( token, "headYawRangeLeft" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->headYawRangeLeft = n;
				continue;
			}

			//headYawRangeRight
			if ( !Q_stricmp( token, "headYawRangeRight" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->headYawRangeRight = n;
				continue;
			}

			//headPitchRangeUp
			if ( !Q_stricmp( token, "headPitchRangeUp" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->headPitchRangeUp = n;
				continue;
			}
			
			//headPitchRangeDown
			if ( !Q_stricmp( token, "headPitchRangeDown" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->headPitchRangeDown = n;
				continue;
			}

			//torsoYawRangeLeft
			if ( !Q_stricmp( token, "torsoYawRangeLeft" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->torsoYawRangeLeft = n;
				continue;
			}

			//torsoYawRangeRight
			if ( !Q_stricmp( token, "torsoYawRangeRight" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->torsoYawRangeRight = n;
				continue;
			}

			//torsoPitchRangeUp
			if ( !Q_stricmp( token, "torsoPitchRangeUp" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->torsoPitchRangeUp = n;
				continue;
			}

			//torsoPitchRangeDown
			if ( !Q_stricmp( token, "torsoPitchRangeDown" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->torsoPitchRangeDown = n;
				continue;
			}

			// Uniform XYZ scale
			if ( !Q_stricmp( token, "scale" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->scaleXYZ[0] = ri->scaleXYZ[1] = ri->scaleXYZ[2] = n;
				continue;
			}

			//X scale
			if ( !Q_stricmp( token, "scaleX" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->scaleXYZ[0] = n;
				continue;
			}

			//Y scale
			if ( !Q_stricmp( token, "scaleY" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->scaleXYZ[1] = n;
				continue;
			}

			//Z scale
			if ( !Q_stricmp( token, "scaleZ" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				ri->scaleXYZ[2] = n;
				continue;
			}

			if ( !Q_stricmp( token, "boltOn" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}

				G_AddBoltOn( NPC, value );
				continue;
			}
	//===AI STATS=====================================================================
			// aggression
			if ( !Q_stricmp( token, "aggression" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->aggression = n;
				continue;
			}

			// aim
			if ( !Q_stricmp( token, "aim" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) {
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->aim = n;
				continue;
			}

			// earshot
			if ( !Q_stricmp( token, "earshot" ) ) {
				if ( G_ParseFloat( &p, &f ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( f < 0.0f ) 
				{
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->earshot = f;
				continue;
			}

			// evasion
			if ( !Q_stricmp( token, "evasion" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->evasion = n;
				continue;
			}

			// hfov
			if ( !Q_stricmp( token, "hfov" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 30 || n > 180 ) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->hfov = n;// / 2;	//FIXME: Why was this being done?!
				continue;
			}

			// intelligence
			if ( !Q_stricmp( token, "intelligence" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->intelligence = n;
				continue;
			}
			
			// move
			if ( !Q_stricmp( token, "move" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->move = n;
				continue;
			}

			// reactions
			if ( !Q_stricmp( token, "reactions" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 1 || n > 5 ) {
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->reactions = n;
				continue;
			}

			// shootDistance
			if ( !Q_stricmp( token, "shootDistance" ) ) {
				if ( G_ParseFloat( &p, &f ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( f < 0.0f ) 
				{
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->shootDistance = f;
				continue;
			}

			// vfov
			if ( !Q_stricmp( token, "vfov" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 30 || n > 180 ) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->vfov = n / 2;
				continue;
			}

			// vigilance
			if ( !Q_stricmp( token, "vigilance" ) ) {
				if ( G_ParseFloat( &p, &f ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( f < 0.0f ) 
				{
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->vigilance = f;
				continue;
			}

			// visrange
			if ( !Q_stricmp( token, "visrange" ) ) {
				if ( G_ParseFloat( &p, &f ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( f < 0.0f ) 
				{
					gi.Printf( "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->visrange = f;
				continue;
			}

			// race
			if ( !Q_stricmp( token, "race" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				NPC->client->race = TranslateRaceName(value);
				continue;
			}

			// rank
			if ( !Q_stricmp( token, "rank" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				NPC->NPC->rank = TranslateRankName(value);
				continue;
			}

			// fullName
			if ( !Q_stricmp( token, "fullName" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				NPC->fullName = G_NewString(value);
				continue;
			}

			// playerTeam
			if ( !Q_stricmp( token, "playerTeam" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				NPC->client->playerTeam = TranslateTeamName(value);
				continue;
			}

			// enemyTeam
			if ( !Q_stricmp( token, "enemyTeam" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}
				NPC->client->enemyTeam = TranslateTeamName(value);
				continue;
			}

	//===MOVEMENT STATS============================================================
			
			if ( !Q_stricmp( token, "width" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					continue;
				}

				NPC->mins[0] = NPC->mins[1] = -n;
				NPC->maxs[0] = NPC->maxs[1] = n;
				continue;
			}

			if ( !Q_stricmp( token, "height" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					continue;
				}

				NPC->mins[2] = DEFAULT_MINS_2;//Cannot change
				NPC->maxs[2] = NPC->client->standheight = n + DEFAULT_MINS_2;
				continue;
			}

			if ( !Q_stricmp( token, "crouchheight" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					continue;
				}

				NPC->client->crouchheight = n + DEFAULT_MINS_2;
				continue;
			}

			if ( !Q_stricmp( token, "movetype" ) ) 
			{
				if ( G_ParseString( &p, &value ) ) 
				{
					continue;
				}

				stats->moveType = (movetype_t)MoveTypeNameToEnum(value);
				continue;
			}
				
			// yawSpeed
			if ( !Q_stricmp( token, "yawSpeed" ) ) {
				if ( G_ParseInt( &p, &n ) ) {
					SkipRestOfLine( &p );
					continue;
				}
				if ( n <= 0) {
					gi.Printf(  "bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->yawSpeed = ((float)(n));
				continue;
			}

			// walkSpeed
			if ( !Q_stricmp( token, "walkSpeed" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->walkSpeed = n;
				continue;
			}
			
			//runSpeed
			if ( !Q_stricmp( token, "runSpeed" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->runSpeed = n;
				continue;
			}

			//acceleration
			if ( !Q_stricmp( token, "acceleration" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < 0 ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				stats->acceleration = n;
				continue;
			}
	//===MISC===============================================================================
			// default behavior
			if ( !Q_stricmp( token, "behavior" ) ) 
			{
				if ( G_ParseInt( &p, &n ) ) 
				{
					SkipRestOfLine( &p );
					continue;
				}
				if ( n < BS_DEFAULT || n >= NUM_BSTATES ) 
				{
					gi.Printf( S_COLOR_YELLOW"WARNING: bad %s in NPC '%s'\n", token, NPCName );
					continue;
				}
				NPC->NPC->defaultBehavior = (bState_t)(n);
				continue;
			}

			// snd
			if ( !Q_stricmp( token, "snd" ) ) {
				if ( G_ParseString( &p, &value ) ) {
					continue;
				}
				if ( !(NPC->svFlags&SVF_NO_BASIC_SOUNDS) )
				{
					//FIXME: store this in some sound field or parse in the soundTable like the animTable...
					Q_strncpyz( sound, value, sizeof( sound ) );
					patch = strstr( sound, "/" );
					if ( patch ) 
					{
						*patch = 0;
					}
					ci->customBasicSoundDir = G_NewString( sound );
				}
				continue;
			}

			// sndcombat
			if ( !Q_stricmp( token, "sndcombat" ) ) {
				if ( G_ParseString( &p, &value ) ) {
					continue;
				}
				if ( !(NPC->svFlags&SVF_NO_COMBAT_SOUNDS) )
				{
					//FIXME: store this in some sound field or parse in the soundTable like the animTable...
					Q_strncpyz( sound, value, sizeof( sound ) );
					patch = strstr( sound, "/" );
					if ( patch ) 
					{
						*patch = 0;
					}
					ci->customCombatSoundDir = G_NewString( sound );
				}
				continue;
			}
			
			// sndextra
			if ( !Q_stricmp( token, "sndextra" ) ) {
				if ( G_ParseString( &p, &value ) ) {
					continue;
				}
				if ( !(NPC->svFlags&SVF_NO_EXTRA_SOUNDS) )
				{
					//FIXME: store this in some sound field or parse in the soundTable like the animTable...
					Q_strncpyz( sound, value, sizeof( sound ) );
					patch = strstr( sound, "/" );
					if ( patch ) 
					{
						*patch = 0;
					}
					ci->customExtraSoundDir = G_NewString( sound );
				}
				continue;
			}

			// sndscav
			if ( !Q_stricmp( token, "sndscav" ) ) {
				if ( G_ParseString( &p, &value ) ) {
					continue;
				}
				if ( !(NPC->svFlags&SVF_NO_SCAV_SOUNDS) )
				{
					//FIXME: store this in some sound field or parse in the soundTable like the animTable...
					Q_strncpyz( sound, value, sizeof( sound ) );
					patch = strstr( sound, "/" );
					if ( patch ) 
					{
						*patch = 0;
					}
					ci->customScavSoundDir = G_NewString( sound );
				}
				continue;
			}

			gi.Printf( "WARNING: unknown keyword '%s' while parsing '%s'\n", token, NPCName );
			SkipRestOfLine( &p );
		}
	}

	ci->infoValid = qfalse;

	if(	NPCsPrecached )
	{//Spawning in after initial precache, our models are precached, we just need to set our clientInfo
		CG_RegisterClientModels( NPC->s.number );
		CG_RegisterNPCCustomSounds( ci );
		CG_RegisterNPCEffects( NPC->client->playerTeam );
	}

	return qtrue;
}

void NPC_LoadParms( void ) 
{
	int			len, totallen, npcExtFNLen, mainBlockLen, fileCnt, i;
	const char	filename[] = "ext_data/NPCs.cfg";
	char		*buffer, *holdChar, *marker;
	char		npcExtensionListBuf[2048];			//	The list of file names read in

	//First, load in the npcs.cfg
	gi.Printf( "Parsing %s\n", filename );
	len = gi.FS_ReadFile( filename, (void **) &buffer );
	if ( len == -1 ) {
		gi.Printf( "file not found\n" );
		return;
	}

	if ( len >= MAX_NPC_DATA_SIZE ) {
		G_Error( "ext_data/NPCs.cfg is too large" );
	}

	strncpy( NPCParms, buffer, sizeof( NPCParms ) - 1 );
	gi.FS_FreeFile( buffer );

	//remember where to store the next one
	totallen = mainBlockLen = len;
	marker = NPCParms+totallen;

	//now load in the extra .npc extensions
	fileCnt = gi.FS_GetFileList("ext_data", ".npc", npcExtensionListBuf, sizeof(npcExtensionListBuf) );

	holdChar = npcExtensionListBuf;
	for ( i = 0; i < fileCnt; i++, holdChar += npcExtFNLen + 1 ) 
	{
		npcExtFNLen = strlen( holdChar );

		gi.Printf( "Parsing %s\n", holdChar );

		len = gi.FS_ReadFile( va( "ext_data/%s", holdChar), (void **) &buffer );

		if ( len == -1 ) 
		{
			gi.Printf( "error reading file\n" );
		}
		else
		{
			if ( totallen + len >= MAX_NPC_DATA_SIZE ) {
				G_Error( "NPC extensions (*.npc) are too large" );
			}
			strcat( marker, buffer );
			gi.FS_FreeFile( buffer );

			totallen += len;
			marker = NPCParms+totallen;
		}
	}
}
