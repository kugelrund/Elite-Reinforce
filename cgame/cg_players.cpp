
#define	CG_PLAYERS_CPP
#include "cg_local.h"
#include "fx_public.h"
#include "..\game\anims.h"
#include "..\game\boltOns.h"

#define	LOOK_SWING_SCALE	0.5

/*

player entities generate a great deal of information from implicit ques
taken from the entityState_t

*/

qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *headModelName, const char *headSkinName, 
									const char *torsoModelName, const char *torsoSkinName, 
									const char *legsModelName, const char *legsSkinName );

void CG_PlayerAnimSounds( animsounds_t *animSounds, int frame, int entNum );
extern void FX_BorgDeathSparkParticles( vec3_t origin, vec3_t angles, vec3_t vel, vec3_t user );
extern int PM_GetTurnAnim( gentity_t *gent, int anim );

extern animFileSet_t	knownAnimFileSets[MAX_ANIM_FILES];

//Basic set of custom sounds that everyone needs
const char	*cg_customBasicSoundNames[MAX_CUSTOM_BASIC_SOUNDS] = 
{
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25.wav",
	"*pain50.wav",
	"*pain75.wav",
	"*pain100.wav",
	"*gasp.wav",
	"*fall1.wav",
};

//Used as a supplement to the basic set for enemies and hazard team
const char	*cg_customCombatSoundNames[MAX_CUSTOM_COMBAT_SOUNDS] = 
{
	"*anger1.wav",	//Say when acquire an enemy when didn't have one before
	"*anger2.wav",
	"*anger3.wav",
	"*combat1.wav",	//Say when acquire a new enemy during combat
	"*combat2.wav",
	"*combat3.wav",
	"*victory1.wav",	//Say when killed an enemy
	"*victory2.wav",
	"*victory3.wav",
	"*settle1.wav",	//Say when killed an enemy anc can't find anymore (battle over)
	"*settle2.wav",
	"*settle3.wav",
};

//Used as a supplement to the basic set for crew and hazard team
const char	*cg_customExtraSoundNames[MAX_CUSTOM_EXTRA_SOUNDS] = 
{
	"*greeting1.wav",	//See player in hall, greet them
	"*greeting2.wav",
	"*greeting3.wav",
	"*response1.wav",	//Used by player when just standing around, no enemy
	"*response2.wav",
	"*response3.wav",
	"*busy1.wav",	//After responding a couple times, start playing busy
	"*busy2.wav",
	"*busy3.wav",
	"*mission1.wav",	//Say when used by player while in formation and no enemy
	"*mission2.wav",
	"*mission3.wav",
	"*blocked1.wav",	//Say when killed an enemy anc can't find anymore (battle over)
	"*blocked2.wav",
	"*blocked3.wav",
	"*ff_1a.wav",	//Friendly fire, level 1, subset A
	"*ff_1b.wav",
	"*ff_1c.wav",
	"*ff_2a.wav",	//Friendly fire, level 2, subset A
	"*ff_2b.wav",
	"*ff_2c.wav",
	"*ff_3a.wav",	//Friendly fire, level 3, subset A
	"*ff_3b.wav",
	"*ff_3c.wav",
};

//Scavenger combat sounds table
const char	*cg_customScavSoundNames[MAX_CUSTOM_SCAV_SOUNDS] =
{
	"*coverme.wav",		//"Cover me!"
	"*getdown.wav",		//"Get down!"
	"*mr_get.wav",		//"Get him!"
	"*mr_got.wav",		//"I got him!"
	"*mr_kill.wav",		//"Kill him!"
	"*mr_mine.wav",		//"He's mine!"
	"*mr_surround.wav",	//"Surround him!"
	"*overthere.wav",	//"Over there!"
	"*stop.wav",		//"Stop the intruder!"
	"*takecover.wav",	//"Take cover!"
	"*whatisthat.wav",	//"What's that?!"
	"*watchit.wav",		//"Watch it!"
	"*illgo.wav",		//"I'll go check it out..."
	"*mr_go.wav",		//"Where did he go?!"
};
/*
================
CG_CustomSound

  NOTE: when you call this, check the value.  If zero, do not try to play the sound.
		Either that or when a sound that doesn't exist is played, don't play the null 
		sound honk and don't display the error message

================
*/
static sfxHandle_t	CG_CustomSound( int entityNum, const char *soundName, int customSoundSet ) 
{
	clientInfo_t *ci;
	int			i;

	if ( soundName[0] != '*' ) 
	{
		return cgi_S_RegisterSound( soundName );
	}

	if ( !g_entities[entityNum].client )
	{
		// No client, this should never happen, so just use munro's sounds
		ci = &g_entities[0].client->clientInfo;
	}
	else
	{
		ci = &g_entities[entityNum].client->clientInfo;
	}

	//FIXME: if the sound you want to play could not be found, pick another from the same
	//general grouping?  ie: if you want ff_2c and there is none, try ff_2b or ff_2a...
	switch ( customSoundSet )
	{
	case CS_BASIC:
		// There should always be a clientInfo structure if there is a client, but just make sure...
		if ( ci )
		{
			for ( i = 0 ; i < MAX_CUSTOM_BASIC_SOUNDS && cg_customBasicSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customBasicSoundNames[i] ) ) 
				{
					return ci->sounds[i];
				}
			}
		}
		break;
	case CS_COMBAT:
		// There should always be a clientInfo structure if there is a client, but just make sure...
		if ( ci )
		{
			for ( i = 0 ; i < MAX_CUSTOM_COMBAT_SOUNDS && cg_customCombatSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customCombatSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS];
				}
			}
		}
		break;
	case CS_EXTRA:
		// There should always be a clientInfo structure if there is a client, but just make sure...
		if ( ci )
		{
			for ( i = 0 ; i < MAX_CUSTOM_EXTRA_SOUNDS && cg_customExtraSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customExtraSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS];
				}
			}
		}
		break;
	case CS_SCAV:
		// There should always be a clientInfo structure if there is a client, but just make sure...
		if ( ci )
		{
			for ( i = 0 ; i < MAX_CUSTOM_SCAV_SOUNDS && cg_customScavSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customScavSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS+MAX_CUSTOM_EXTRA_SOUNDS];
				}
			}
		}
		break;
	default:
		//no set specified, search all
		if ( ci )
		{
			for ( i = 0 ; i < MAX_CUSTOM_BASIC_SOUNDS && cg_customBasicSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customBasicSoundNames[i] ) ) 
				{
					return ci->sounds[i];
				}
			}
			for ( i = 0 ; i < MAX_CUSTOM_COMBAT_SOUNDS && cg_customCombatSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customCombatSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS];
				}
			}
			for ( i = 0 ; i < MAX_CUSTOM_EXTRA_SOUNDS && cg_customExtraSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customExtraSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS];
				}
			}
			for ( i = 0 ; i < MAX_CUSTOM_SCAV_SOUNDS && cg_customScavSoundNames[i] ; i++ ) 
			{
				if ( !Q_stricmp( soundName, cg_customScavSoundNames[i] ) ) 
				{
					return ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS+MAX_CUSTOM_EXTRA_SOUNDS];
				}
			}
		}
		break;
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}

void CG_TryPlayCustomSound( vec3_t origin, int entityNum, soundChannel_t channel, const char *soundName, int customSoundSet )
{
	sfxHandle_t	soundIndex = CG_CustomSound( entityNum, soundName, customSoundSet );
	if ( !soundIndex )
	{
		return;
	}

	cgi_S_StartSound( origin, entityNum, channel, soundIndex );
}
/*
======================
CG_NewClientinfo

  For player only, NPCs get them through NPC_stats and G_ModelIndex
======================
*/
void CG_NewClientinfo( int clientNum ) 
{
	clientInfo_t *ci;
	const char	*configstring;
	const char	*v;
	const char	*s;
	int			i;

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );

	if ( !configstring[0] ) 
	{
		return;		// player just left
	}
	//ci = &cgs.clientinfo[clientNum];
	if ( !(&g_entities[clientNum].client) )
	{
		return;
	}
	ci = &g_entities[clientNum].client->clientInfo;

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( ci->name, v, sizeof( ci->name ) );

	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	ci->handicap = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	ci->team = (team_t) atoi( v );

	// legsModel
	v = Info_ValueForKey( configstring, "legsModel" );
	
	Q_strncpyz(			g_entities[clientNum].client->renderInfo.legsModelName, v,
				sizeof(	g_entities[clientNum].client->renderInfo.legsModelName), qtrue);

	// torsoModel
	v = Info_ValueForKey( configstring, "torsoModel" );
	
	Q_strncpyz(			g_entities[clientNum].client->renderInfo.torsoModelName, v,
				sizeof(	g_entities[clientNum].client->renderInfo.torsoModelName), qtrue);	
	
	// headModel
	v = Info_ValueForKey( configstring, "headModel" );
	
	Q_strncpyz(			g_entities[clientNum].client->renderInfo.headModelName, v,
				sizeof(	g_entities[clientNum].client->renderInfo.headModelName), qtrue);

	// sounds
	cvar_t	*sex = gi.cvar( "sex", "male", 0 );
	if ( Q_stricmp("female", sex->string ) == 0 )
	{
		ci->customBasicSoundDir = "alexa";
	}
	else
	{
		ci->customBasicSoundDir = "munro";
	}

	//player uses only the basic custom sound set, not the combat or extra
	for ( i = 0 ; i < MAX_CUSTOM_BASIC_SOUNDS ; i++ ) 
	{
		s = cg_customBasicSoundNames[i];
		if ( !s ) 
		{
			break;
		}
		ci->sounds[i] = cgi_S_RegisterSound( va("sound/voice/%s/misc/%s", ci->customBasicSoundDir, s + 1) );
	}

	ci->infoValid = qfalse;
}

/*
CG_RegisterNPCCustomSounds
*/
extern sfxHandle_t CG_RegisterSexedSound(char* string);
void CG_RegisterNPCCustomSounds( clientInfo_t *ci )
{
	const char	*s;
	int			i;

	// sounds

	if ( ci->customBasicSoundDir && ci->customBasicSoundDir[0] )
	{
		for ( i = 0 ; i < MAX_CUSTOM_BASIC_SOUNDS ; i++ ) 
		{
			s = cg_customBasicSoundNames[i];
			if ( !s ) 
			{
				break;
			}

			char finalName[MAX_QPATH];

			Q_strncpyz(finalName, s+1, sizeof(finalName), qtrue);
			ci->sounds[i] = CG_RegisterSexedSound( va("sound/voice/%s/misc/%s", ci->customBasicSoundDir, finalName) );
		}
	}

	if ( ci->customCombatSoundDir && ci->customCombatSoundDir[0] )
	{
		for ( i = 0 ; i < MAX_CUSTOM_COMBAT_SOUNDS ; i++ ) 
		{
			s = cg_customCombatSoundNames[i];
			if ( !s ) 
			{
				break;
			}

			char finalName[MAX_QPATH];

			Q_strncpyz(finalName, s+1, sizeof(finalName), qtrue);
			ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS] = CG_RegisterSexedSound( va("sound/voice/%s/misc/%s", ci->customCombatSoundDir, finalName) );
		}
	}

	if ( ci->customExtraSoundDir && ci->customExtraSoundDir[0] )
	{
		for ( i = 0 ; i < MAX_CUSTOM_EXTRA_SOUNDS ; i++ ) 
		{
			s = cg_customExtraSoundNames[i];
			if ( !s ) 
			{
				break;
			}

			char finalName[MAX_QPATH];

			Q_strncpyz(finalName, s+1, sizeof(finalName), qtrue);
			ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS] = CG_RegisterSexedSound( va("sound/voice/%s/misc/%s", ci->customExtraSoundDir, finalName) );
		}
	}

	if ( ci->customScavSoundDir && ci->customScavSoundDir[0] )
	{
		for ( i = 0 ; i < MAX_CUSTOM_SCAV_SOUNDS ; i++ ) 
		{
			s = cg_customScavSoundNames[i];
			if ( !s ) 
			{
				break;
			}

			char finalName[MAX_QPATH];

			Q_strncpyz(finalName, s+1, sizeof(finalName), qtrue);
			ci->sounds[i+MAX_CUSTOM_BASIC_SOUNDS+MAX_CUSTOM_COMBAT_SOUNDS+MAX_CUSTOM_EXTRA_SOUNDS] = CG_RegisterSexedSound( va("sound/voice/%s/misc/%s", ci->customScavSoundDir, finalName) );
		}
	}
}

//=============================================================================

/*
void CG_RegisterNPCEffects( team_t team )

  This should register all the shaders, models and sounds used by a specific type
  of NPC's spawn, death and other miscellaneous effects.  NOT WEAPON EFFECTS, as those
  are taken care of in CG_RegisterWeapon
*/
void CG_RegisterNPCEffects( team_t team )
{
	int i;

	switch( team )
	{
	case TEAM_BORG:
		cgs.media.borgBeamInSound	= cgi_S_RegisterSound("sound/enemies/borg/borgbeam.wav");
		cgs.media.borgFlareShader	= cgi_R_RegisterShader( "gfx/misc/borgflare" );
		cgs.media.borgShieldShader	= cgi_R_RegisterShader( "borgShield" );
		cgs.media.borgRecycleSound	= cgi_S_RegisterSound("sound/enemies/borg/borgrecycle.wav");
		cgs.media.dnBoltShader		= cgi_R_RegisterShader( "gfx/misc/dnBolt" );
		cgs.media.spark1Sound		= cgi_S_RegisterSound( "sound/ambience/spark1.wav" );
		cgs.media.spark2Sound		= cgi_S_RegisterSound( "sound/ambience/spark2.wav" );
		cgs.media.spark3Sound		= cgi_S_RegisterSound( "sound/ambience/spark3.wav" );
		cgs.media.spark4Sound		= cgi_S_RegisterSound( "sound/ambience/spark4.wav" );
		cgs.media.spark5Sound		= cgi_S_RegisterSound( "sound/ambience/spark5.wav" );
		cgs.media.spark6Sound		= cgi_S_RegisterSound( "sound/ambience/spark6.wav" );
		break;

	case TEAM_SCAVENGERS:
	case TEAM_KLINGON:
	case TEAM_MALON:
	case TEAM_HIROGEN:
	case TEAM_IMPERIAL:
		cgs.media.scavBeamInSound		= cgi_S_RegisterSound( "sound/enemies/scav/transout.wav" );
		cgs.media.scavTransportShader	= cgi_R_RegisterShader( "powerups/scavBeamEffect" );
		//stab sounds for knife
		cgi_S_RegisterSound( "sound/enemies/harvester/stab1.wav" );
		cgi_S_RegisterSound( "sound/enemies/harvester/stab2.wav" );
		cgi_S_RegisterSound( "sound/enemies/harvester/stab3.wav" );
		break;

	case TEAM_STASIS:
		cgs.media.stasisBeamInSound		= cgi_S_RegisterSound("sound/movers/stasistransporter.wav");
		cgs.media.stasisAppearSound		= cgi_S_RegisterSound("sound/enemies/etherians/appear.wav");
		cgs.media.stasisBeamOutSound	= cgi_S_RegisterSound("sound/ambience/stasis/intostasis.wav");
		break;

	case TEAM_FORGE:
		cgs.media.borgFlareShader		= cgi_R_RegisterShader( "gfx/misc/borgflare" );//For the reaver
		cgs.media.forgeRemoveShader		= cgi_R_RegisterShader( "gfx/misc/forge_fade" );
		cgs.media.forgeRemovalSound		= cgi_S_RegisterSound( "sound/enemies/remove.wav" );
		cgs.media.forgeBeaminSound		= cgi_S_RegisterSound( "sound/movers/armytransin.mp3" );
		break;

	case TEAM_PARASITE:
		break;

	case TEAM_8472:
		cgs.media.speciesPortalShader	= cgi_R_RegisterShader( "gfx/effects/species_portal" ); 
		cgs.media.portalFlareShader		= cgi_R_RegisterShader( "gfx/effects/portal_flare" ); 
		cgs.media.speciesBeaminSound	= cgi_S_RegisterSound( "sound/enemies/species8472/8472in.wav" );
		cgs.media.speciesBeamoutSound	= cgi_S_RegisterSound( "sound/enemies/species8472/8472out.wav" );
		break;

	case TEAM_BOTS:
		// These chunks are actually used by every type of bot
		cgs.media.warriorBotChunks[0]		= cgi_R_RegisterModel( "models/chunks/warriorbot/head.md3" );
		cgs.media.warriorBotChunks[1]		= cgi_R_RegisterModel( "models/chunks/warriorbot/turret.md3" );
		cgs.media.warriorBotChunks[2]		= cgi_R_RegisterModel( "models/chunks/warriorbot/foot.md3" );
		cgs.media.warriorBotChunks[3]		= cgi_R_RegisterModel( "models/chunks/warriorbot/arm.md3" );
		cgs.media.warriorBotChunks[4]		= cgi_R_RegisterModel( "models/chunks/warriorbot/leg.md3" );
		cgs.media.warbotExplodeSounds[0]	= cgi_S_RegisterSound( "sound/enemies/warbot/explode1.wav" );
		cgs.media.warbotExplodeSounds[1]	= cgi_S_RegisterSound( "sound/enemies/warbot/explode2.wav" );
		cgs.media.hunterSeekerHoverSound	= cgi_S_RegisterSound( "sound/enemies/hunter/hover.wav" );
		for ( i = 0; i < 3; i++ )
			cgs.media.scoutbotExplodeSounds[i]	= cgi_S_RegisterSound( va("sound/enemies/scoutbot/explode%i.mp3", i+1 ) );

		break;

	case TEAM_STARFLEET:
		break;

	default:
		break;
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/

qboolean ValidAnimFileIndex ( int index )
{
	if ( index < 0 || index >= MAX_ANIM_FILES )
	{
		Com_Printf( S_COLOR_RED "Bad animFileIndex: %d\n", index );
		return qfalse;
	}

	return qtrue;
}



void ParseAnimationSndBlock(const char *filename, animsounds_t *animSounds, animation_t *animations, int *i,char **text_p) 
{
	char		*token;
	char		soundString[MAX_QPATH];
	int			lowestVal, highestVal;
	int			animNum, num, n;

	// get past starting bracket
	while(1) 
	{
		token = COM_Parse( text_p );
		if ( !Q_stricmp( token, "{" ) ) 
		{
			break;
		}
	}

	animSounds +=  *i;

	// read information for each frame
	while ( 1 ) 
	{
		// Get base frame of sequence
		token = COM_Parse( text_p );
		if ( !token || !token[0]) 
		{
			break;
		}

		if ( !Q_stricmp( token, "}" ) )		// At end of block 
		{
			break;
		}

		//Compare to same table as animations used 
		//	so we don't have to use actual numbers for animation first frames,
		//	just need offsets.
		//This way when animation numbers change, this table won't have to be updated,
		//	at least not much.
		animNum = GetIDForString(animTable, token);
		if(animNum == -1)
		{//Unrecognized ANIM ENUM name, or we're skipping this line, keep going till you get a good one
			Com_Printf(S_COLOR_YELLOW"WARNING: Unknown token %s in animSound file %s\n", token, filename );
			continue;
		}

		if ( animations[animNum].numFrames == 0 )
		{//we don't use this anim
			Com_Printf(S_COLOR_YELLOW"WARNING: %s animsounds.cfg: anim %s not used by this model\n", filename, token);
	
			// Get offset to frame within sequence
			token = COM_Parse( text_p );
			//get soundstring
			token = COM_Parse( text_p );
			//get lowest value
			token = COM_Parse( text_p );
			//get highest value
			token = COM_Parse( text_p );
			//get probability
			token = COM_Parse( text_p );

			continue;
		}

		animSounds->keyFrame = animations[animNum].firstFrame;

		// Get offset to frame within sequence
		token = COM_Parse( text_p );
		if ( !token ) 
		{
			break;
		}
		animSounds->keyFrame += atoi( token );

		//get soundstring
		token = COM_Parse( text_p );
		if ( !token ) 
		{
			break;
		}		
		strcpy(soundString, token);

		//get lowest value
		token = COM_Parse( text_p );
		if ( !token ) 
		{//WARNING!  BAD TABLE!
			break;
		}
		lowestVal = atoi( token );

		//get highest value
		token = COM_Parse( text_p );
		if ( !token ) 
		{//WARNING!  BAD TABLE!
			break;
		}
		highestVal = atoi( token );

		//Now precache all the sounds
		//NOTE: If we can be assured sequential handles, we can store sound indices
		//		instead of strings, unfortunately, if these sounds were previously
		//		registered, we cannot be guaranteed sequential indices.  Thus an array
		if(lowestVal && highestVal)
		{
			for ( n = lowestVal, num = 0; n <= highestVal && num < MAX_RANDOM_ANIMSOUNDS; n++, num++ )
			{
				animSounds->soundIndex[num] = cgi_S_RegisterSound( va( soundString, n ) );
			}
			animSounds->numRandomAnimSounds = num - 1;
		}
		else
		{
			animSounds->soundIndex[0] = cgi_S_RegisterSound( va( soundString ) );
#ifndef FINAL_BUILD
			if ( !animSounds->soundIndex[0] )
			{//couldn't register it - file not found
				Com_Printf( S_COLOR_RED "ParseAnimationSndBlock: sound %s does not exist (animsound.cfg %s)!\n", soundString, filename );
			}
#endif
			animSounds->numRandomAnimSounds = 0;
		}


		//get probability
		token = COM_Parse( text_p );
		if ( !token ) 
		{//WARNING!  BAD TABLE!
			break;
		}

		animSounds->probability = atoi( token );
		++animSounds;
		++*i;
	}	
}

/*
======================
CG_ClearAnimSndCache

resets all the soundcache so that a vid restart will recache them
======================
*/
void CG_ClearAnimSndCache( void )
{
	int i;
	for (i=0; i < numKnownAnimFileSets; i++) {
		knownAnimFileSets[i].soundsCached = qfalse;
	}
}

/*
======================
CG_ParseAnimationSndFile

Read a configuration file containing animation sounds 
models/players/munro/animsounds.cfg, etc

This file's presence is not required

======================
*/
void CG_ParseAnimationSndFile( const char *filename, int animFileIndex ) 
{
	char		*text_p;
	int			len;
	char		*token;
	char		text[20000];
	char		sfilename[MAX_QPATH];
	fileHandle_t	f;
	int			i, j, upper_i, lower_i;
	animsounds_t	*legsAnimSnds = knownAnimFileSets[animFileIndex].legsAnimSnds;
	animsounds_t	*torsoAnimSnds = knownAnimFileSets[animFileIndex].torsoAnimSnds;
	animation_t		*animations = knownAnimFileSets[animFileIndex].animations;

	if ( knownAnimFileSets[animFileIndex].soundsCached )
	{//already cached this one
		return;
	}

	//Mark this anim set so that we know we tried to load he sounds, don't care if the load failed
	knownAnimFileSets[animFileIndex].soundsCached = qtrue;

	// Load and parse animSounds.cfg file
	Com_sprintf( sfilename, sizeof( sfilename ), "models/players/%s/animsounds.cfg", filename );

	//initialize anim sound array
	for(i = 0; i < MAX_ANIM_SOUNDS; i++)
	{
		torsoAnimSnds[i].numRandomAnimSounds = 0;
		legsAnimSnds[i].numRandomAnimSounds = 0;
		for(j = 0; j < MAX_RANDOM_ANIMSOUNDS; j++)
		{
			torsoAnimSnds[i].soundIndex[j] = -1;
			legsAnimSnds[i].soundIndex[j] = -1;
		}
	}

	// load the file
	len = cgi_FS_FOpenFile( sfilename, &f, FS_READ );
	if ( len <= 0 ) 
	{//no file
		return;
	}
	if ( len >= sizeof( text ) - 1 ) 
	{
		CG_Printf( "File %s too long\n", sfilename );
		return;
	}

	cgi_FS_Read( text, len, f );
	text[len] = 0;
	cgi_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	upper_i =0;
	lower_i =0;

	// read information for batches of sounds (UPPER or LOWER)
	while ( 1 ) 
	{
		// Get base frame of sequence
		token = COM_Parse( &text_p );
		if ( !token || !token[0] ) 
		{
			break;
		}

		if ( !Q_stricmp(token,"UPPERSOUNDS") )	// A batch of upper sounds
		{
			ParseAnimationSndBlock( filename, torsoAnimSnds, animations, &upper_i, &text_p ); 
		}

		else if ( !Q_stricmp(token,"LOWERSOUNDS") )	// A batch of lower sounds
		{
			ParseAnimationSndBlock( filename, legsAnimSnds, animations, &lower_i, &text_p ); 
		}
	}
}
/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation ) 
{
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) 
	{
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	if ( !ValidAnimFileIndex( ci->animFileIndex ) )
	{
		CG_Error( "Bad animFileIndex: %i", ci->animFileIndex );
	}

	anim = &knownAnimFileSets[ci->animFileIndex].animations[ newAnimation ];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer ) 
	{
		CG_Printf( "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
qboolean CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float fpsMod, int entNum ) {
	int			f, animFrameTime;
	animation_t	*anim;
	qboolean	newFrame = qfalse;

	if(fpsMod > 2 || fpsMod < 0.5)
	{//should have been set right
		fpsMod = 1.0f;
	}

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) 
	{
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return qfalse;
	}

	// see if the animation sequence is switching
	//FIXME: allow multiple-frame overlapped lerping between sequences? - Possibly last 3 of last seq and first 3 of next seq?
	if ( newAnimation != lf->animationNumber || !lf->animation ) 
	{
		CG_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) 
	{
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		//Do we need to speed up or slow down the anim?
		/*if(fpsMod != 1.0)
		{//Note!  despite it's name, a higher fpsMod slows down the anim, a lower one speeds it up
			animFrameTime = ceil(lf->frameTime * fpsMod);
		}
		else*/
		{
			animFrameTime = fabs(anim->frameLerp);
			
			//special hack for player to ensure quick weapon change
			if ( entNum == 0 )
			{
				if ( lf->animationNumber == TORSO_DROPWEAP1 || lf->animationNumber == TORSO_RAISEWEAP1 )
				{
					animFrameTime = 50;
				}
			}
		}

		if ( cg.time < lf->animationTime ) 
		{
			lf->frameTime = lf->animationTime;		// initial lerp
		} 
		else 
		{
			lf->frameTime = lf->oldFrameTime + animFrameTime;
		}

		f = ( lf->frameTime - lf->animationTime ) / animFrameTime;
		if ( f >= anim->numFrames ) 
		{//Reached the end of the anim
			//FIXME: Need to set a flag here to TASK_COMPLETE
			f -= anim->numFrames;
			if ( anim->loopFrames != -1 ) //Before 0 meant no loop
			{
				if(anim->numFrames - anim->loopFrames == 0)
				{
					f %= anim->numFrames;
				}
				else
				{
					f %= (anim->numFrames - anim->loopFrames);
				}
				f += anim->loopFrames;
			} 
			else 
			{
				f = anim->numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}

		if ( anim->frameLerp < 0 )
		{
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else
		{
			lf->frame = anim->firstFrame + f;
		}

		if ( cg.time > lf->frameTime ) 
		{
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) 
			{
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}

		newFrame = qtrue;
	}

	if ( lf->frameTime > cg.time + 200 ) 
	{
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) 
	{
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) 
	{
		lf->backlerp = 0;
	} 
	else 
	{
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}

	return newFrame;
}


/*
===============
CG_ClearLerpFrame
===============
*/
void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) 
{
	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetLerpFrameAnimation( ci, lf, animationNumber );
	if ( lf->animation->frameLerp < 0 )
	{//Plays backwards
		lf->oldFrame = lf->frame = (lf->animation->firstFrame + lf->animation->numFrames);
	}
	else
	{
		lf->oldFrame = lf->frame = lf->animation->firstFrame;
	}
}

/*
===============
CG_PlayerAnimation
===============
*/
void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	clientInfo_t	*ci;
	int				clientNum;
	int				legsAnim;
	int				legsTurnAnim = -1;
	qboolean		newLegsFrame = qfalse;
	qboolean		newTorsoFrame = qfalse;

	clientNum = cent->currentState.clientNum;

	if ( cg_noPlayerAnims.integer ) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	ci = &cent->gent->client->clientInfo;
	//Changed this from cent->currentState.legsAnim to cent->gent->client->ps.legsAnim because it was screwing up our timers when we've just changed anims while turning
	legsAnim = (cent->gent->client->ps.legsAnim & ~ANIM_TOGGLEBIT);

	// do the shuffle turn frames locally (MAN this is an Fugly-ass hack!)
	
	if ( cent->pe.legs.yawing )
	{
		legsTurnAnim = PM_GetTurnAnim( cent->gent, legsAnim );
	}

	if ( legsTurnAnim != -1 )
	{
		newLegsFrame = CG_RunLerpFrame( ci, &cent->pe.legs, legsTurnAnim, cent->gent->client->renderInfo.legsFpsMod, cent->gent->s.number );
		//This line doesn't seem to serve any useful purpose, rather it
		//breaks things since any task waiting for a lower anim to complete
		//never will finish if this happens!!!
		//cent->gent->client->ps.legsAnimTimer = 0;
	} 
	else 
	{
		newLegsFrame = CG_RunLerpFrame( ci, &cent->pe.legs, legsAnim, cent->gent->client->renderInfo.legsFpsMod, cent->gent->s.number);
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

	if( newLegsFrame )
	{
		if ( ValidAnimFileIndex( ci->animFileIndex ) )
		{
			CG_PlayerAnimSounds( knownAnimFileSets[ci->animFileIndex].legsAnimSnds, cent->pe.legs.frame, cent->currentState.number );
		}
	}

	//HACKHACKHACK
	/*
	if ( cent->gent->client->playerTeam == TEAM_BORG &&
		cent->gent->client->ps.weapon == WP_BORG_ASSIMILATOR )
	{
		if ( cent->gent->NPC->attackHoldTime > cg.time )
		{
			if ( cent->pe.torso.frame >= 468 && cent->pe.torso.frame < 478 )
			{//Do not animate
				*torsoOld = *torso = cent->pe.torso.frame = 468;
				*torsoBackLerp = 0;
				if ( ValidAnimFileIndex( ci->animFileIndex ) )
				{
					CG_PlayerAnimSounds(knownAnimFileSets[ci->animFileIndex].torsoAnimSnds, cent->pe.torso.frame, cent->currentState.number );
				}
				return;
			}
		}
	}
	*/

	newTorsoFrame = CG_RunLerpFrame( ci, &cent->pe.torso, cent->gent->client->ps.torsoAnim, cent->gent->client->renderInfo.torsoFpsMod, cent->gent->s.number );

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;

	if( newTorsoFrame )
	{
		if ( ValidAnimFileIndex( ci->animFileIndex ) )
		{
			CG_PlayerAnimSounds(knownAnimFileSets[ci->animFileIndex].torsoAnimSnds, cent->pe.torso.frame, cent->currentState.number );
		}
	}
}


/*
void CG_PlayerAnimSounds( animsounds_t *animSounds, int frame, const vec3_t org, int entNum )

play any keyframed sounds - only when start a new frame
This func is called once for legs and once for torso
*/
void CG_PlayerAnimSounds( animsounds_t *animSounds, int frame, int entNum )
{
	int		i;
	int		holdSnd = -1;
	qboolean	playSound = qfalse;

	if ( entNum == 0 && !cg_thirdPerson.integer )
	{//player in first person view does not play any keyframed sounds
		return;
	}

	// Check for anim sound
	for (i=0;i<MAX_ANIM_SOUNDS;++i)
	{
		if (animSounds[i].soundIndex[0] == -1)	// No sounds in array
		{
			break;
		}

		if (animSounds[i].keyFrame == frame)
		{
			// are there variations on the sound?
			holdSnd = animSounds[i].soundIndex[ Q_irand( 0, animSounds[i].numRandomAnimSounds ) ];

			// Determine probability of playing sound
			if (!animSounds[i].probability)	// 100% 
			{
				playSound = qtrue;
			}
			else if (animSounds[i].probability > Q_irand(0, 99) )
			{
				playSound = qtrue;
			}
			break;
		}
	}

	// Play sound
	if (holdSnd != -1 && playSound)
	{
		if (holdSnd != 0)	// 0 = default sound, ie file was missing
		{
			cgi_S_StartSound( NULL, entNum, CHAN_AUTO, holdSnd );
		}

	}
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_UpdateAngleClamp
Turn curAngle toward destAngle at angleSpeed, but stay within clampMin and Max
==================
*/
void CG_UpdateAngleClamp( float destAngle, float clampMin, float clampMax, float angleSpeed, float *curAngle, float normalAngle) 
{
	float	swing;
	float	move;
	float	scale;
	float	actualSpeed;

	swing = AngleSubtract( destAngle, *curAngle );

	if(swing == 0)
	{//Don't have to turn
		return;
	}

	// modify the angleSpeed depending on the delta
	// so it doesn't seem so linear
	scale = fabs( swing );
	if (swing > 0)
	{
		if ( swing < clampMax * 0.25 ) 
		{//Pretty small way to go
			scale = 0.25;
		} 
		else if ( swing > clampMax * 2.0 )
		{//Way out of our range
			scale = 2.0;
		}
		else
		{//Scale it smoothly
			scale = swing/clampMax;
		} 
	}
	else// if (swing < 0)
	{
		if ( swing > clampMin * 0.25 ) 
		{//Pretty small way to go
			scale = 0.5;
		} 
		else if ( swing < clampMin * 2.0 )
		{//Way out of our range
			scale = 2.0;
		}
		else
		{//Scale it smoothly
			scale = swing/clampMin;
		} 
	}

	actualSpeed = scale * angleSpeed;
	// swing towards the destination angle
	if ( swing >= 0 ) 
	{
		move = cg.frametime * actualSpeed;
		if ( move >= swing ) 
		{//our turnspeed is so fast, no need to swing, just match
			*curAngle = destAngle;
		}
		else
		{
			*curAngle = AngleMod( *curAngle + move );
		}
	} 
	else if ( swing < 0 ) 
	{
		move = cg.frametime * -actualSpeed;
		if ( move <= swing ) 
		{//our turnspeed is so fast, no need to swing, just match
			*curAngle = destAngle;
		}
		else
		{
			*curAngle = AngleMod( *curAngle + move );
		}
	}
	
	swing = AngleSubtract( *curAngle, normalAngle );

	// clamp to no more than normalAngle + tolerance
	if ( swing > clampMax ) 
	{
		*curAngle = AngleMod( normalAngle + clampMax );
	} 
	else if ( swing < clampMin ) 
	{
		*curAngle = AngleMod( normalAngle + clampMin );
	}
}
/*
==================
CG_SwingAngles

  If the body is not locked OR if the upper part is trying to swing beyond it's
	range, turn the lower body part to catch up.

  Parms:	desired angle,		(Our eventual goal angle
			min swing tolerance,(Lower angle value threshold at which to start turning)
			max swing tolerance,(Upper angle value threshold at which to start turning)
			min clamp tolerance,(Lower angle value threshold to clamp output angle to)
			max clamp tolerance,(Upper angle value threshold to clamp output angle to)
			angle speed,		(How fast to turn)
			current angle,		(Current angle to modify)
			locked mode			(Don't turn unless you exceed the swing/clamp tolerance)
==================
*/
void CG_SwingAngles( float destAngle, 
					float swingTolMin, float swingTolMax, 
					float clampMin, float clampMax,
					float angleSpeed, float *curAngle, 
					lockMode_t lockMode, qboolean *turning ) 
{
	float	swing;
	float	move;
	float	scale;

	swing = AngleSubtract( destAngle, *curAngle );

	if(swing == 0)
	{//Don't have to turn
		*turning = qfalse;
	}
	else if ( lockMode == LM_LOCKED ) 
	{//Never turn
		*turning = qfalse;
	}
	else if ( lockMode == LM_CLAMP ) 
	{// see if beyond clamp tolerance
		if ( swing > clampMax || swing < clampMin ) 
		{//We're locked down, but trying to look somewhere beyond our range, so unlock
			*turning = qtrue;
		}
	}
	else if ( lockMode == LM_SWING ) 
	{// see if a swing should be started
		if ( swing > swingTolMax || swing < swingTolMin ) 
		{
			*turning = qtrue;
		}
	}
	else
	{
		*turning = qtrue;
	}

	//If we're not turning, then we're done
	if ( *turning == qfalse) 
		return;

	// modify the angleSpeed depending on the delta
	// so it doesn't seem so linear
	scale = fabs( swing );

	if (swing > 0)
	{
		if ( clampMax <= 0 )
		{
			*curAngle = destAngle;
			return;
		}

		if ( swing < swingTolMax * 0.5 ) 
		{//Pretty small way to go
			scale = 0.5;
		} 
		else if ( scale < swingTolMax ) 
		{//More than halfway to go
			scale = 1.0;
		} 
		else 
		{//Way out of our range
			scale = 2.0;
		}
	}
	else// if (swing < 0)
	{
		if ( clampMin >= 0 )
		{
			*curAngle = destAngle;
			return;
		}

		if ( swing > swingTolMin * 0.5 ) 
		{//Pretty small way to go
			scale = 0.5;
		} 
		else if ( scale > swingTolMin ) 
		{//More than halfway to go
			scale = 1.0;
		} 
		else 
		{//Way out of our range
			scale = 2.0;
		}
	}

	// swing towards the destination angle
	if ( swing >= 0 ) 
	{
		move = cg.frametime * scale * angleSpeed;
		if ( move >= swing ) 
		{//our turnspeed is so fast, no need to swing, just match
			move = swing;
		}
		*curAngle = AngleMod( *curAngle + move );
	} 
	else if ( swing < 0 ) 
	{
		move = cg.frametime * scale * -angleSpeed;
		if ( move <= swing ) 
		{//our turnspeed is so fast, no need to swing, just match
			move = swing;
		}
		*curAngle = AngleMod( *curAngle + move );
	}
	

	// clamp to no more than tolerance
	if ( swing > clampMax ) 
	{
		*curAngle = AngleMod( destAngle - (clampMax - 1) );
	} 
	else if ( swing < clampMin ) 
	{
		*curAngle = AngleMod( destAngle + (-clampMin - 1) );
	}
}

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}

//FIXME: Don't do this, use tag_eye instead?
//--------------------------------------------------------------
float CG_EyePointOfsForRace[RACE_HOLOGRAM+1][2] =
{
	0, 0,//RACE_NONE = 0,
	4, 8,//RACE_HUMAN,
	4, 8,//RACE_BORG,
	4, 8,//RACE_KLINGON,
	4, 8,//RACE_HIROGEN,
	4, 8,//RACE_MALON,
	0, 0,//RACE_STASIS,
	0, 0,//RACE_8472,
	0, 0,//RACE_BOT,
	8, 0,//RACE_HARVESTER,
	6, -6,//RACE_REAVER,
	4, 0,//RACE_AVATAR,
	4, 0,//RACE_PARASITE,
	4, 8,//RACE_VULCAN,
	4, 8,//RACE_BETAZOID,
	4, 8,//RACE_BOLIAN,
	4, 8,//RACE_TALAXIAN,
	4, 8,//RACE_BAJORAN,
	4, 8//RACE_HOLOGRAM
};

#define LOOK_DEFAULT_SPEED	0.15f
#define LOOK_TALKING_SPEED	0.15f	

static qboolean CG_CheckLookTarget( centity_t *cent, vec3_t	lookAngles, float *lookingSpeed )
{
	if ( !cent->gent->client->clientInfo.torsoModel || !cent->gent->client->clientInfo.headModel )
	{
		return qfalse;
	}

	//FIXME: also clamp the lookAngles based on the clamp + the existing difference between
	//		headAngles and torsoAngles?  But often the tag_torso is straight but the torso itself
	//		is deformed to not face straight... sigh...

	//Now calc head angle to lookTarget, if any
	if ( cent->gent->client->renderInfo.lookTarget >= 0 && cent->gent->client->renderInfo.lookTarget < ENTITYNUM_WORLD )
	{
		centity_t	*lookCent = &cg_entities[cent->gent->client->renderInfo.lookTarget];
		if ( lookCent && lookCent->gent )
		{
			vec3_t	lookDir, lookOrg, forward, eyeOrg;
			float	scaleFactor;
			
			if ( lookCent->gent != cent->gent->enemy )
			{//We turn heads faster than headbob speed, but not as fast as if watching an enemy
				*lookingSpeed = LOOK_DEFAULT_SPEED;
			}

			//FIXME: Ignore small deltas from current angles so we don't bob our head in synch with theirs?

			if ( cent->gent->client->renderInfo.lookTarget == 0 && !cg_thirdPerson.integer )
			{//Special case- use cg.refdef.vieworg if looking at player and not in third person view
				VectorCopy( cg.refdef.vieworg, lookOrg );
			}
			else if ( lookCent->gent->client )
			{
				VectorCopy( lookCent->gent->client->renderInfo.eyePoint, lookOrg );
			}
			else if ( lookCent->gent->s.pos.trType == TR_INTERPOLATE )
			{
				VectorCopy( lookCent->lerpOrigin, lookOrg );
			}
			else
			{
				VectorCopy( lookCent->gent->currentOrigin, lookOrg );
			}
			//Look in dir of lookTarget

			//NOTE: We have to calc the angle from the central axis of the head so the angle we get our of this is accurate for actual head model angles
			scaleFactor = (float)(cent->gent->client->renderInfo.scaleXYZ[0])/100.0f;
			AngleVectors( cent->gent->client->renderInfo.eyeAngles, forward, NULL, NULL );
			VectorMA( cent->gent->client->renderInfo.eyePoint, CG_EyePointOfsForRace[cent->gent->client->race][0]*scaleFactor*-1, forward, eyeOrg );

			VectorSubtract( lookOrg, eyeOrg, lookDir );
#if 1
			vectoangles( lookDir, lookAngles );
#else
			//FIXME: get the angle of the head tag and account for that when finding the lookAngles-
			//		so if they're lying on their back we get an accurate lookAngle...
			vec3_t	headDirs[3];
			vec3_t	finalDir;

			AnglesToAxis( cent->gent->client->renderInfo.headAngles, headDirs );
			VectorRotate( lookDir, headDirs, finalDir );
			vectoangles( finalDir, lookAngles );
#endif
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
CG_AddHeadBob
=================
*/
static qboolean CG_AddHeadBob( centity_t *cent ) 
{
	renderInfo_t	*renderInfo	= &cent->gent->client->renderInfo;
	int				volume		= gi.S_Override[cent->gent->s.clientNum];
	int				volChange	= volume - renderInfo->lastVoiceVolume;
	int				i;

	renderInfo->lastVoiceVolume = volume;

	if ( !volume )
	{
		// Not talking, set our target to be the normal head position
		VectorClear( renderInfo->targetHeadBobAngles );

		if ( VectorLengthSquared( renderInfo->headBobAngles ) < 1.0f )
		{
			// We are close enough to being back to our normal head position, so we are done for now
			return qfalse;
		}
	}
	else if ( volChange > 2 )
	{
		// a big positive change in volume
		for ( i = 0; i < 3; i++ )
		{
			// Move our head angle target a bit
			renderInfo->targetHeadBobAngles[i] += Q_flrand( -1.0 * volChange, 1.0 * volChange );

			// Clamp so we don't get too out of hand
			if ( renderInfo->targetHeadBobAngles[i] > 7.0f )
				renderInfo->targetHeadBobAngles[i] = 7.0f;

			if ( renderInfo->targetHeadBobAngles[i] < -7.0f )
				renderInfo->targetHeadBobAngles[i] = -7.0f;
		}
	}

	for ( i = 0; i < 3; i++ )
	{
		// Always try to move head angles towards our target
		renderInfo->headBobAngles[i] += ( renderInfo->targetHeadBobAngles[i] - renderInfo->headBobAngles[i] ) * ( cg.frametime / 150.0f );
	}

	// We aren't back to our normal position yet, so we still have to apply headBobAngles
	return qtrue;
}

/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso

===============
*/
void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) 
{
	vec3_t		legsAngles, torsoAngles, headAngles;
//	float		dest;
	float		speed;
	vec3_t		velocity;
	static	int	movementOffsets[8] = { 0, 0, 45, 0, 0, 0, -45, 0 };
	//static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		lookAngles, viewAngles;
	float		headYawClampMin, headYawClampMax;
	float		headPitchClampMin, headPitchClampMax;
	float		torsoYawSwingTolMin, torsoYawSwingTolMax;
	float		torsoYawClampMin, torsoYawClampMax;
	float		torsoPitchSwingTolMin, torsoPitchSwingTolMax;
	float		torsoPitchClampMin, torsoPitchClampMax;
	float		legsYawSwingTolMin, legsYawSwingTolMax;
	float		maxYawSpeed, yawSpeed, lookingSpeed;
	float		lookAngleSpeed = LOOK_TALKING_SPEED;//shut up the compiler
	float		swing, scale;
	int			i;
	qboolean	looking = qfalse, talking = qfalse;

	if ( cg.renderingThirdPerson && cent->gent && cent->gent->s.number == 0 )
	{
		// If we are rendering third person, we should just force the player body to always fully face
		//	whatever way they are looking, otherwise, you can end up with gun shots coming off of the
		//	gun at angles that just look really wrong.
		VectorClear( viewAngles );
		VectorCopy( cent->lerpAngles, lookAngles );
		lookAngles[PITCH] = 0;

		AnglesToAxis( lookAngles, legs );
		AnglesToAxis( viewAngles, torso );
		AnglesToAxis( viewAngles, head );

		return;
	}

	if(cent->currentState.eFlags & EF_NPC)
	{
		headYawClampMin = -cent->gent->client->renderInfo.headYawRangeLeft;
		headYawClampMax = cent->gent->client->renderInfo.headYawRangeRight;
		//These next two are only used for a calc below- this clamp is done in PM_UpdateViewAngles
		headPitchClampMin = -cent->gent->client->renderInfo.headPitchRangeUp;
		headPitchClampMax = cent->gent->client->renderInfo.headPitchRangeDown;

		torsoYawSwingTolMin = headYawClampMin * 0.3;
		torsoYawSwingTolMax = headYawClampMax * 0.3;
		torsoPitchSwingTolMin = headPitchClampMin * 0.5;
		torsoPitchSwingTolMax =  headPitchClampMax * 0.5;
		torsoYawClampMin = -cent->gent->client->renderInfo.torsoYawRangeLeft;
		torsoYawClampMax = cent->gent->client->renderInfo.torsoYawRangeRight;
		torsoPitchClampMin = -cent->gent->client->renderInfo.torsoPitchRangeUp;
		torsoPitchClampMax = cent->gent->client->renderInfo.torsoPitchRangeDown;

		legsYawSwingTolMin = torsoYawClampMin * 0.5;
		legsYawSwingTolMax = torsoYawClampMax * 0.5;

		if ( cent->gent && cent->gent->next_roff_time && cent->gent->next_roff_time >= cg.time )
		{//Following a roff, body must keep up with head, yaw-wise
			headYawClampMin = 
			headYawClampMax = 
			torsoYawSwingTolMin = 
			torsoYawSwingTolMax = 
			torsoYawClampMin = 
			torsoYawClampMax = 
			legsYawSwingTolMin = 
			legsYawSwingTolMax = 0;
		}

		yawSpeed = maxYawSpeed = cent->gent->NPC->stats.yawSpeed/150;//about 0.33 normally
		if(cent->gent->NPC->lookMode)
		{//We have a specific kind of looking we are doing,
			//lower the lookMode, slower the yawspeed
			yawSpeed *= (float)cent->gent->NPC->lookMode/(float)LT_FULLFACE;
		}
	}
	else
	{
		headYawClampMin = -70;
		headYawClampMax = 70;
		
		//These next two are only used for a calc below- this clamp is done in PM_UpdateViewAngles
		headPitchClampMin = -90;
		headPitchClampMax = 90;

		torsoYawSwingTolMin = -90;
		torsoYawSwingTolMax = 90;
		torsoPitchSwingTolMin = -90;
		torsoPitchSwingTolMax = 90;
		torsoYawClampMin = -90;
		torsoYawClampMax = 90;
		torsoPitchClampMin = -90;
		torsoPitchClampMax = 90;

		legsYawSwingTolMin = -90;
		legsYawSwingTolMax = 90;

		yawSpeed = maxYawSpeed = cg_swingSpeed.value;
	}
	
	if(yawSpeed <= 0)
	{//Just in case
		yawSpeed = 0.5f;	//was 0.33
	}

	lookingSpeed = yawSpeed;

	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	//Clamp and swing the legs
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ (int)cent->currentState.angles2[YAW] ];//(-45 to 45)
	
	if(cent->gent->client->renderInfo.renderFlags & RF_LOCKEDANGLE)
	{
		cent->pe.legs.yawAngle = cent->gent->client->renderInfo.lockYaw;
		cent->pe.legs.yawing = qfalse;
	}
	else
	{
		CG_SwingAngles( legsAngles[YAW], legsYawSwingTolMin, legsYawSwingTolMax, torsoYawClampMin, torsoYawClampMax, maxYawSpeed, &cent->pe.legs.yawAngle, cent->gent->client->renderInfo.legsYawLockMode, &cent->pe.legs.yawing );
	}
	
	legsAngles[YAW] = cent->pe.legs.yawAngle;
	
#if 0
	swing = AngleSubtract( legsAngles[YAW], headAngles[YAW] );
	scale = ( torsoYawClampMax ) ? ( fabs( swing ) / torsoYawClampMax ) : 1.0f;

	torsoAngles[YAW] = legsAngles[YAW] - ( swing * scale );
#else
	// torso
	// If applicable, swing the lower parts to catch up with the head
	CG_SwingAngles( headAngles[YAW], torsoYawSwingTolMin, torsoYawSwingTolMax, headYawClampMin, headYawClampMax, yawSpeed, &cent->pe.torso.yawAngle, cent->gent->client->renderInfo.torsoYawLockMode, &cent->pe.torso.yawing);
	torsoAngles[YAW] = cent->pe.torso.yawAngle;
#endif

	// ---------- pitch -----------

	//As the body twists to its extents, the back tends to arch backwards
	
#if 0
	swing = AngleSubtract( legsAngles[PITCH], headAngles[PITCH] );
	scale = ( torsoPitchClampMax ) ? ( fabs( swing ) / torsoPitchClampMax ) : 1.0f;

	torsoAngles[PITCH] = legsAngles[PITCH] - ( swing * scale );	
#else
	float dest;
	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) 
	{
		dest = (-360 + headAngles[PITCH]) * 0.75;
	} 
	else 
	{
		dest = headAngles[PITCH] * 0.75;
	}

	CG_SwingAngles( dest, torsoPitchSwingTolMin, torsoPitchSwingTolMax, torsoPitchClampMin, torsoPitchClampMax, 0.1f, &cent->pe.torso.pitchAngle, cent->gent->client->renderInfo.torsoPitchLockMode, &cent->pe.torso.pitching );
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;
#endif
	// --------- roll -------------

	// lean towards the direction of travel
	if ( cent->gent->s.eFlags & EF_BANK_STRAFE )
	{
		VectorCopy( cent->gent->client->ps.velocity, velocity );
		speed = VectorNormalize( velocity );

		if ( speed ) 
		{
			vec3_t	axis[3];
			float	side;

			// Magic number fun!  Speed is used for banking, so modulate the speed by a sine wave
			speed *= 0.4 * sin( (cg.time - cent->gent->fx_time + 200 ) * 0.003 );

			// Clamp to prevent harsh rolling
			if ( speed > 12 )
				speed = 12;

			AnglesToAxis( legsAngles, axis );
			side = speed * DotProduct( velocity, axis[1] );
			legsAngles[ROLL] -= side;
			torsoAngles[ROLL] -= side;


			side = speed * DotProduct( velocity, axis[0] );
			legsAngles[PITCH] += side;
			torsoAngles[PITCH] += side;
		}
	}

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	//----------- Special head looking ---------------

	//FIXME: to clamp the head angles, figure out tag_head's offset from tag_torso and add
	//	that to whatever offset we're getting here... so turning the head in an
	//	anim that also turns the head doesn't allow the head to turn out of range.

	//Start with straight ahead
	VectorCopy( headAngles, viewAngles );
	VectorCopy( headAngles, lookAngles );

	//Remember last headAngles
	VectorCopy( cent->gent->client->renderInfo.lastHeadAngles, headAngles );

	//See if we're looking at someone/thing
	looking = CG_CheckLookTarget( cent, lookAngles, &lookingSpeed );

	//Now add head bob when talking
	if ( cent->gent->client->clientInfo.extensions )
	{
		talking = CG_AddHeadBob( cent );
	}

	//Figure out how fast head should be turning
	if ( cent->pe.torso.yawing || cent->pe.torso.pitching )
	{//If torso is turning, we want to turn head just as fast
		lookAngleSpeed = yawSpeed;
	}
	else if ( talking )
	{//Slow for head bobbing
		lookAngleSpeed = LOOK_TALKING_SPEED;
	}
	else if ( looking )
	{//Not talking, set it up for looking at enemy, CheckLookTarget will scale it down if neccessary
		lookAngleSpeed = lookingSpeed;
	}
	else if ( cent->gent->client->renderInfo.lookingDebounceTime > cg.time )
	{//Not looking, not talking, head is returning from a talking head bob, use talking speed
		lookAngleSpeed = LOOK_TALKING_SPEED;
	}

	if ( looking || talking )
	{//Keep this type of looking for a second after stopped looking
		cent->gent->client->renderInfo.lookingDebounceTime = cg.time + 1000;
	}
	
	if ( cent->gent->client->renderInfo.lookingDebounceTime > cg.time )
	{
		//Calc our actual desired head angles
		for ( i = 0; i < 3; i++ )
		{
			lookAngles[i] = AngleMod( cent->gent->client->renderInfo.headBobAngles[i] + lookAngles[i] );
		}

		if( VectorCompare( headAngles, lookAngles ) == qfalse )
		{
			//FIXME: This clamp goes off viewAngles,
			//but really should go off the tag_torso's axis[0] angles, no?
			CG_UpdateAngleClamp( lookAngles[PITCH], headPitchClampMin/1.25, headPitchClampMax/1.25, lookAngleSpeed, &headAngles[PITCH], viewAngles[PITCH] );
			CG_UpdateAngleClamp( lookAngles[YAW], headYawClampMin/1.25, headYawClampMax/1.25, lookAngleSpeed, &headAngles[YAW], viewAngles[YAW] );
			CG_UpdateAngleClamp( lookAngles[ROLL], -10, 10, lookAngleSpeed, &headAngles[ROLL], viewAngles[ROLL] );
		}

		if ( !cent->gent->enemy || cent->gent->enemy->s.number != cent->gent->client->renderInfo.lookTarget )
		{
			//NOTE: Hacky, yes, I know, but necc.  
			//We want to turn the body to follow the lookTarget 
			//ONLY IF WE DON'T HAVE AN ENEMY OR OUR ENEMY IS NOT OUR LOOKTARGET
			//This is the piece of code that was making the enemies not face where
			//they were actually aiming.

			//Yaw change
			swing = AngleSubtract( legsAngles[YAW], headAngles[YAW] );
			scale = fabs( swing ) / ( torsoYawClampMax + 0.01 );	//NOTENOTE: Some ents have a clamp of 0, which is bad for division

			scale *= LOOK_SWING_SCALE;
			torsoAngles[YAW] = legsAngles[YAW] - ( swing * scale );

			//Pitch change
			swing = AngleSubtract( legsAngles[PITCH], headAngles[PITCH] );
			scale = fabs( swing ) / ( torsoPitchClampMax + 0.01 );	//NOTENOTE: Some ents have a clamp of 0, which is bad for division

			scale *= LOOK_SWING_SCALE;
			torsoAngles[PITCH] = legsAngles[PITCH] - ( swing * scale );	
		}
	}
	else
	{//Look straight ahead
		VectorCopy( viewAngles, headAngles );
	}

	//Remember current headAngles next time
	VectorCopy( headAngles, cent->gent->client->renderInfo.lastHeadAngles );

	//-------------------------------------------------------------

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================


/*
===============
CG_TrailItem
===============
*/
void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
	refEntity_t		ent;
	vec3_t			angles;
	vec3_t			axis[3];

	VectorCopy( cent->lerpAngles, angles );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	memset( &ent, 0, sizeof( ent ) );
	VectorMA( cent->lerpOrigin, -24, axis[0], ent.origin );
	ent.origin[2] += 20;
	VectorScale( cg.autoAxis[0], 0.75, ent.axis[0] );
	VectorScale( cg.autoAxis[1], 0.75, ent.axis[1] );
	VectorScale( cg.autoAxis[2], 0.75, ent.axis[2] );
	ent.hModel = hModel;
	cgi_R_AddRefEntityToScene( &ent );
}


/*
===============
CG_PlayerPowerups
===============
*/
void CG_PlayerPowerups( centity_t *cent ) 
{
/*
	if ( !cent->currentState.powerups ) {
		return;
	}

	// quad gives a dlight
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		cgi_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2, 0.2, 1 );
	}

	// flight plays a looped sound
	if ( cent->currentState.powerups & ( 1 << PW_HIROGEN_SHIELD ) ) {
		cgi_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, 
			cgi_S_RegisterSound( "sound/items/flight.wav" ) );
	}

	// powerball
	if ( cent->currentState.powerups & ( 1 << PW_BALL ) ) {
		CG_TrailItem( cent, cgs.media.ballModel );
	}

	// redflag
	if ( cent->currentState.powerups & ( 1 << PW_REDFLAG ) ) {
		CG_TrailItem( cent, cgs.media.redFlagModel );
		cgi_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1, 0.2, 0.2 );
	}

	// blueflag
	if ( cent->currentState.powerups & ( 1 << PW_BLUEFLAG ) ) {
		CG_TrailItem( cent, cgs.media.blueFlagModel );
		cgi_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2, 0.2, 1 );
	}
*/
}

/*
qboolean CG_ApplyBoltOnToRefEnt (refEntity_t *newBoltOn, boltOn_t *boltOn, boltOnInfo_t *bOInfo, const vec3_t org, refEntity_t *targModel)

 attaches boltOn refEnt to the proper model and tag
  
*/
qboolean CG_ApplyBoltOnToRefEnt (refEntity_t *newBoltOn, boltOn_t *boltOn, boltOnInfo_t *bOInfo, const vec3_t org, refEntity_t *targModel)
{
	int		i;
	vec3_t	dir;

	memset( newBoltOn, 0, sizeof(*newBoltOn) );

	newBoltOn->hModel = cgs.model_draw[boltOn->model.modelIndex];
	if (!newBoltOn->hModel) 
	{
		return qfalse;
	}

	newBoltOn->frame = bOInfo->frame;

	VectorCopy( org, newBoltOn->lightingOrigin );

	AnglesToAxis( boltOn->angleOffsets, newBoltOn->axis );
	for(i = 0; i < 3; i++)
	{
		if ( boltOn->model.scaleXYZ[i] != 100.0f )
		{
			VectorScale( newBoltOn->axis[i], boltOn->model.scaleXYZ[i]/100.0f, newBoltOn->axis[i] );
			newBoltOn->nonNormalizedAxes = qtrue;
		}
	}

	//NB!!! targetTag must exist in targetModel
	CG_PositionRotatedEntityOnTag( newBoltOn, targModel, targModel->hModel, boltOn->targetTag, NULL );

	vectoangles( newBoltOn->axis[0], bOInfo->lastAngles );
	//Add the origin offsets
	for( i = 0; i < 3; i++ )
	{
		if ( boltOn->originOffsets[i] )
		{
			VectorCopy( newBoltOn->axis[i], dir );
			
			if ( newBoltOn->nonNormalizedAxes )
			{//Need to normalize for this transform
				VectorNormalize( dir );
			}

			VectorMA( newBoltOn->origin, boltOn->originOffsets[i], dir, newBoltOn->origin );
		}
	}

	VectorCopy( newBoltOn->origin, bOInfo->lastOrigin );

	return qtrue;
}

/*
===============
CG_GetTeamFromUserinfo

Return team value based on client userinfo
===============
*/
int CG_GetTeamFromUserinfo( int clientNum ) {
	const char *info, *teamString;

	info = CG_ConfigString( CS_PLAYERS + clientNum );
	teamString = Info_ValueForKey( info, "t" );

	/*if ( !strcmp( teamString, "s" ) )
		return TEAM_SPECTATOR;

	if ( !strcmp( teamString, "r" ) ) 
		return TEAM_RED;

	if ( !strcmp( teamString, "b" ) )
		return TEAM_BLUE;*/

	return TEAM_FREE;
}

/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
	int				rf;
	refEntity_t		ent;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	cgi_R_AddRefEntityToScene( &ent );
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) {
	vec3_t		end, mins = {-7, -7, 0}, maxs = {7, 7, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= SHADOW_DISTANCE;

	cgi_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		cent->pe.legs.yawAngle, 1,1,1,alpha, qfalse, 16, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = cgi_CM_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = cgi_CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	cgi_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	cgi_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}


/*
===============
CG_LightningBolt
===============
*/
#define BEAM_VARIATION	25

void CG_LightningBolt( centity_t *cent, vec3_t origin ) 
{
	// FIXME:  This sound also plays when the weapon first fires which causes little sputtering sounds..not exactly cool
	// Must be currently firing
	if ( !( cent->currentState.eFlags & EF_FIRING ) )
		return;

	//Must be a durational weapon
	if ( ( cent->currentState.weapon == WP_DREADNOUGHT && !( cent->currentState.eFlags & EF_ALT_FIRING )) 
			|| cent->currentState.weapon == WP_PHASER
			/*|| cent->currentState.weapon == WP_BOT_LASER*/ )
	{	/*continue*/	}
	else
		return;

	trace_t		trace;
	gentity_t	*traceEnt;
	vec3_t		end, forward, org, angs;
	qboolean	spark = qfalse, impact = qtrue, weak = qfalse;

	// for lightning weapons coming from the player, it had better hit the crosshairs or else..
	if ( cent->gent->s.number || cg_thirdPerson.integer == 2 )
	{
		VectorCopy( origin, org );
	}
	else
	{
		VectorCopy( cg.refdef.vieworg, org );
	}

	// Find the impact point of the beam
	VectorCopy( cent->lerpAngles, angs );
	if ( cg_thirdPerson.integer == 2 )
	{
		angs[PITCH] = 0; 
		angs[ROLL] = 0;
	}
	AngleVectors( angs, forward, NULL, NULL );

	VectorMA( org, weaponData[cent->currentState.weapon].range, forward, end );
	
	// Add a subtle variation to the beam weapon's endpoint
	for ( int i = 0; i < 3; i ++ )
	{
		end[i] += crandom() * BEAM_VARIATION;
	}

	CG_Trace( &trace, org, vec3_origin, vec3_origin, end, cent->currentState.number, MASK_SHOT );
	traceEnt = &g_entities[ trace.entityNum ];

	// Make sparking be a bit less frame-rate dependent..also never add sparking when we hit a surface with a NOIMPACT flag
	if ( cent->gent->fx_time < cg.time && !(trace.surfaceFlags & SURF_NOIMPACT ))
	{
		spark = qtrue;
		cent->gent->fx_time = cg.time + random() * 100 + 100;
	}

	// Don't draw certain kinds of impacts when it hits a player and such..or when we hit a surface with a NOIMPACT flag
	if ( (traceEnt->takedamage && traceEnt->client) || (trace.surfaceFlags & SURF_NOIMPACT) )
	{
		impact = qfalse;
	}
	
	// Add in the effect
	switch ( cent->currentState.weapon )
	{
	case WP_PHASER:
		if ( cent->gent->client->ps.ammo[AMMO_PHASER] < 1 )
		{
			weak = qtrue;
		}

		if ( cent->currentState.eFlags & EF_ALT_FIRING && !weak )
			FX_PhaserAltFire( origin, trace.endpos, trace.plane.normal, spark, impact );
		else
			FX_PhaserFire( origin, trace.endpos, trace.plane.normal, spark, impact, weak );
		break;

	case WP_DREADNOUGHT:
		// Move the beam back a bit to help cover up the poly edges on the fire beam
		VectorMA( origin, -4, forward, org );
		FX_DreadnoughtFire( org, trace.endpos, trace.plane.normal, spark );
		break;

/*
	case WP_BOT_LASER:
		FX_BotLaser( origin, trace.endpos, trace.plane.normal, impact );
		break;
*/
	}
}


/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, int powerups, gentity_t *gent ) 
{
	if ( !gent )
	{
		cgi_R_AddRefEntityToScene( ent );
		return;
	}

	// FIXME:  What the heck!?!  The power flags don't clear themselves?!?
	if ( gent->client->ps.powerups[PW_QUAD] < cg.time )
	{
		gent->client->ps.powerups[PW_QUAD] = 0;
		if ( (powerups & ( 1 << PW_QUAD )) )
		{
			return;
		}
	}
	if ( gent->client->ps.powerups[PW_INVIS] < cg.time )
		gent->client->ps.powerups[PW_INVIS] = 0;

	if ( gent->client->ps.powerups[PW_DISINT_6] < cg.time )
		gent->client->ps.powerups[PW_DISINT_6] = 0;

	if ( gent->client->ps.powerups[PW_HASTE] < cg.time )
	{//scav beam effect
		gent->client->ps.powerups[PW_HASTE] = 0;
	}
	if ( gent->client->ps.powerups[PW_REGEN] < cg.time )
	{//disruptor
		gent->client->ps.powerups[PW_REGEN] = 0;
		if ( (powerups & ( 1 << PW_REGEN )) )
		{
			//stop drawing him after this effect
			gent->client->ps.eFlags |= EF_NODRAW;
			return;
		}
	}
	if ( gent->client->ps.powerups[PW_DISINT_1] < cg.time )
	{//arc welder disintigration
		gent->client->ps.powerups[PW_DISINT_1] = 0;
	}
	if ( gent->client->ps.powerups[PW_DISINT_2] < cg.time )
	{//arc welder disintigration
		gent->client->ps.powerups[PW_DISINT_2] = 0;
		if ( (powerups & ( 1 << PW_DISINT_2 )) )
		{
			//stop drawing him after this effect
			gent->client->ps.eFlags |= EF_NODRAW;
			return;
		}
	}
	if ( gent->client->ps.powerups[PW_DISINT_3] < cg.time )
	{//phaser alt disruptor
		gent->client->ps.powerups[PW_DISINT_3] = 0;
		if ( (powerups & ( 1 << PW_DISINT_3 )) )
		{
			//stop drawing him after this effect
			gent->client->ps.eFlags |= EF_NODRAW;
			return;
		}
	}
	if ( gent->client->ps.powerups[PW_BORG_SHIELD] < cg.time )
	{
		gent->client->ps.powerups[PW_BORG_SHIELD] = 0;
	}

	// Add in the base model if neither beaming state is set
	if ( !gent->client->ps.powerups[PW_INVIS] && 
		!gent->client->ps.powerups[PW_QUAD] && 
		!gent->client->ps.powerups[PW_DISINT_6] && 
		!gent->client->ps.powerups[PW_REGEN] && 
		!gent->client->ps.powerups[PW_DISINT_2] &&
		!gent->client->ps.powerups[PW_DISINT_3] )
	{
		cgi_R_AddRefEntityToScene( ent );
	}
	// If beaming out, don't add the base model after we've gone past the halfway point
	else if ( gent->client->ps.powerups[PW_QUAD] > cg.time + 2000 )
	{
		cgi_R_AddRefEntityToScene( ent );
	}
	// If beaming in, don't add the base model until we reach the halfway point
	else if ( gent->client->ps.powerups[PW_INVIS] > 0 && gent->client->ps.powerups[PW_INVIS] < cg.time + 2000 )
	{
		cgi_R_AddRefEntityToScene( ent );
	}
	else if ( gent->client->ps.powerups[PW_DISINT_6] > 0 )
	{
		if ( (powerups & ( 1 << PW_DISINT_6 )) )
		{// If beaming in (weapon), fade it in
			cgi_R_AddRefEntityToScene( ent );
		}
		else
		{
			cgi_R_AddRefEntityToScene( ent );
		}
	}
/*	else if ( gent->client->ps.powerups[PW_REGEN] > cg.time + 1000 )
	{//FIXME: Alpha out over time
		ent->shaderRGBA[3] = floor(float(gent->client->ps.powerups[PW_REGEN] - cg.time) / 2000.0f * 255.0f );
		cgi_R_AddRefEntityToScene( ent );
	}
	else if ( gent->client->ps.powerups[PW_DISINT_3] > cg.time + 1000 )
	{//FIXME: Alpha out over time
		ent->shaderRGBA[3] = floor(float(gent->client->ps.powerups[PW_DISINT_3] - cg.time) / 2000.0f * 255.0f );
		cgi_R_AddRefEntityToScene( ent );
	}
*/	
	// We are beaming in or out, so it's ok to add in the beaming effect
	if ( (powerups & ( 1 << PW_INVIS )) || (powerups & ( 1 << PW_QUAD )) ) 
	{
		ent->customShader = cgs.media.transportShader;
		ent->shaderTime = gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	if ( (powerups & ( 1 << PW_DISINT_6 )) ) 
	{
		ent->customShader = cgs.media.transportShader;
		ent->shaderTime = (gent->fx_time - 800.0f)/ 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	if ( (powerups & ( 1 << PW_HASTE )) ) 
	{
		ent->customShader = cgs.media.scavTransportShader;
		ent->shaderTime = gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	// Compression rifle
	if ( (powerups & ( 1 << PW_REGEN )) ) 
	{
		ent->shaderRGBA[0] = ent->shaderRGBA[1] = ent->shaderRGBA[2] = 255;
		ent->shaderRGBA[3] = 0;
		ent->customShader = cgs.media.disruptorShader;
		ent->shaderTime = gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	// Phaser alt
	if ( (powerups & ( 1 << PW_DISINT_3 )) ) 
	{
		ent->shaderRGBA[0] = ent->shaderRGBA[1] = ent->shaderRGBA[2] = 255;
		ent->shaderRGBA[3] = 0;
		ent->customShader = cgs.media.phaserDisruptorShader;
		ent->shaderTime = gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	// Electricity
	if ( (powerups & ( 1 << PW_DISINT_1 )) ) 
	{
		if ( (gent->client->ps.powerups[PW_DISINT_1] - cg.time ) > 1000 && random() > 0.4f )
		{
			// fade out over time
			int brightness = floor(float(gent->client->ps.powerups[PW_DISINT_1] - cg.time - 4000.0f) / 4000.0f * 255.0f );

			ent->shaderRGBA[0] = ent->shaderRGBA[1] = ent->shaderRGBA[2] = brightness;
			ent->shaderRGBA[3] = 255;
			ent->customShader = cgs.media.electricBodyShader;
			cgi_R_AddRefEntityToScene( ent );

			if ( random() > 0.9f )
				cgi_S_StartSound ( ent->origin, gent->s.number, CHAN_AUTO, cgi_S_RegisterSound( weaponData[WP_DREADNOUGHT].altmissileHitSound ) );
		}
	} 

	// Quantum
	if ( powerups & ( 1 << PW_DISINT_2 ) ) 
	{
		//expanding and disrupting
		ent->shaderRGBA[0] = ent->shaderRGBA[1] = ent->shaderRGBA[2] = 255;
		ent->shaderRGBA[3] = 255;

		ent->customShader = cgs.media.quantumDisruptorShader;
		ent->shaderTime = gent->fx_time / 1000.0f;
		cgi_R_AddRefEntityToScene( ent );
	} 

	if ( powerups & ( 1 << PW_BORG_SHIELD ) )
	{
		ent->customShader = cgs.media.borgFullBodyShieldShader;
		cgi_R_AddRefEntityToScene( ent );
	}

	if ( powerups & ( 1 << PW_HIROGEN_SHIELD ) )
	{
		ent->customShader = cgs.media.shieldShader;
		cgi_R_AddRefEntityToScene( ent );
	}

	if ( powerups & ( 1 << PW_FORGE_DEATH ) )
	{
		float percent;

		percent = (cg.time - gent->client->ps.powerups[PW_FORGE_DEATH]) / 3000.0f;

		if ( percent < 0 )
			percent = 0;

		if ( percent > 1 )
			percent = 1;

		ent->shaderRGBA[0] = 
		ent->shaderRGBA[1] = 
		ent->shaderRGBA[2] = percent * 255.0f;
		ent->shaderRGBA[3] = 255;

		ent->customShader = cgs.media.forgeRemoveShader;

		cgi_R_AddRefEntityToScene( ent );
	}
}

/*
-------------------------
CG_PlayerHeadExtension
-------------------------
*/

int CG_PlayerHeadExtension( centity_t *cent, refEntity_t *head )
{
	clientInfo_t	*ci = &cent->gent->client->clientInfo;;

	// if we have facial texture extensions, go get the sound override and add it to the face skin
	// if we aren't talking, then it will be 0
	if (ci->extensions && (gi.S_Override[cent->gent->s.clientNum] > 0))
	{//FIXME: When talking, look at talkTarget, if any
		//ALSO: When talking, add a head bob/movement on syllables - when gi.S_Override[] changes drastically
	
		if ( cent->gent->health <= 0 )
		{//Dead people close their eyes and don't make faces!  They also tell no tales...  BUM BUM BAHHHHHHH!
			//Make them always blink and frown
			head->customSkin = ci->headSkin + 3;
			return qtrue;
		}

		head->customSkin = ci->headSkin + gi.S_Override[cent->gent->s.clientNum];
		//reset the frown and blink timers
	}
	else
	// ok, we have facial extensions, but we aren't speaking. Lets decide if we need to frown or blink
	if (ci->extensions)
	{
		int	add_in = 0;

		// deal with blink first
		
		//Dead people close their eyes and don't make faces!  They also tell no tales...  BUM BUM BAHHHHHHH!
		if ( cent->gent->health <= 0 )
		{
			//Make them always blink and frown
			head->customSkin = ci->headSkin + 3;
			return qtrue;
		}

		if (!cent->gent->client->facial_blink)
		{	// reset blink timer
			cent->gent->client->facial_blink = cg.time + Q_flrand(3000.0, 5000.0);
			cent->gent->client->facial_frown = cg.time + Q_flrand(6000.0, 10000.0);
			cent->gent->client->facial_aux = cg.time + Q_flrand(6000.0, 10000.0);
		}


		// now deal with auxing
		// are we frowning ?
		if (cent->gent->client->facial_aux < 0)
		{
			// are we done frowning ?
			if (-(cent->gent->client->facial_aux) < cg.time)
			{
				// reset frown timer
				cent->gent->client->facial_aux = cg.time + Q_flrand(6000.0, 10000.0);
			}
			else
			{
				// yes so set offset to frown
				add_in = 4;
			}
		}
		// no we aren't frowning 
		else
		{
			// but should we start ?
			if (cent->gent->client->facial_aux < cg.time)
			{
				add_in = 4;
				// set blink timer
				cent->gent->client->facial_aux = -(cg.time + 3000.0);
			}
		}

		// now, if we aren't auxing - lets see if we should be blinking or frowning
		if (!add_in)
		{
			if( gi.S_Override[cent->gent->s.clientNum] == -1 )
			{//then we're talking and don't want to use blinking normal frames, force open eyes.
				add_in = 0;
				// reset blink timer
				cent->gent->client->facial_blink = cg.time + Q_flrand(3000.0, 5000.0);
			}
			// are we blinking ?
			else if (cent->gent->client->facial_blink < 0)
			{

				// yes so set offset to blink
				add_in = 1;

				// are we done blinking ?
				if (-(cent->gent->client->facial_blink) < cg.time)
				{
					add_in = 0;
					// reset blink timer
					cent->gent->client->facial_blink = cg.time + Q_flrand(3000.0, 5000.0);
				}
			}
			// no we aren't blinking 
			else
			{
				// but should we start ?
				if (cent->gent->client->facial_blink < cg.time)
				{
					add_in = 1;
					// set blink timer
					cent->gent->client->facial_blink = -(cg.time + 200.0);
				}
			}

			// now deal with frowning
			// are we frowning ?
			if (cent->gent->client->facial_frown < 0)
			{

				// yes so set offset to frown
				add_in += 2;

				// are we done frowning ?
				if (-(cent->gent->client->facial_frown) < cg.time)
				{
					add_in -= 2;
					// reset frown timer
					cent->gent->client->facial_frown = cg.time + Q_flrand(6000.0, 10000.0);
				}
			}
			// no we aren't frowning 
			else
			{
				// but should we start ?
				if (cent->gent->client->facial_frown < cg.time)
				{
					add_in += 2;
					// set blink timer
				 	cent->gent->client->facial_frown = -(cg.time + 3000.0);
				}
			}
		}

		// add in whatever we should
		head->customSkin = ci->headSkin + add_in;
	}
	// at this point, we don't have any facial extensions, so who cares ?
	else
	{
		head->customSkin = ci->headSkin;
	}

	return qtrue;
}

//--------------------------------------------------------------
// CG_GetTagWorldPosition
//
// Can pass in NULL for the axis
//--------------------------------------------------------------
void CG_GetTagWorldPosition( refEntity_t *model, char *tag, vec3_t pos, vec3_t axis[3] )
{
	orientation_t	orientation;

	// Get the requested tag
	cgi_R_LerpTag( &orientation, model->hModel, model->oldframe, model->frame,
		1.0f - model->backlerp, tag );

	VectorCopy( model->origin, pos );
	for ( int i = 0 ; i < 3 ; i++ ) 
	{
		VectorMA( pos, orientation.origin[i], model->axis[i], pos );
	}

	if ( axis )
	{
		MatrixMultiply( orientation.axis, model->axis, axis );
	}
}

extern void FX_HunterSeekerThruster( vec3_t origin, vec3_t normal, qboolean dead );
extern void FX_BotTurretBolt( vec3_t end, vec3_t start, vec3_t normal );

static qboolean calcedMp = qfalse;

//--------------------------------------------------------------
void CG_HunterSeekerEffects( centity_t *cent, refEntity_t *model )
{
	vec3_t		org, axis[3];
	vec3_t		end;
	trace_t		tr;

	// Alternately get one muzzle, then the other
	if ( cent->gent->count )
		CG_GetTagWorldPosition( model, "tag_weapon2", org, axis );
	else
		CG_GetTagWorldPosition( model, "tag_weapon3", org, axis );

	// Save off the muzzlepoint so the game side can know roughly where to do the damage trace from
	VectorCopy( org, cent->gent->client->renderInfo.muzzlePoint );	
	VectorCopy( axis[0], cent->gent->pos2 );						// stash forward vector
	cent->gent->client->renderInfo.mPCalcTime = cg.time;			// save off the time when the muzzle was last calc'ed
	calcedMp = qtrue;

	// Add in the effects cgame side, otherwise the effects will lag and not appear to come from the muzzle
	if ( cent->gent->s.eFlags & EF_ALT_FIRING )
	{
		VectorMA( org, 1024, axis[0], end );
		CG_Trace( &tr, org, NULL, NULL, end, cent->gent->s.number, MASK_SHOT );

		VectorMA( tr.endpos, 0.5, axis[2], end );
		VectorMA( org, 0.5, axis[2], org );

		FX_BotTurretBolt( end, org, tr.plane.normal );

		VectorMA( tr.endpos, -1, axis[2], end );
		VectorMA( org, -1, axis[2], org );

		FX_BotTurretBolt( end, org, tr.plane.normal );

		cent->gent->s.eFlags &= ~EF_ALT_FIRING;
	}

	// Add the thruster effects
	CG_GetTagWorldPosition( model, "tag_thruster1", org, axis );
	FX_HunterSeekerThruster( org, axis[0], (cent->gent->s.eFlags & EF_DEAD) );

	CG_GetTagWorldPosition( model, "tag_thruster2", org, axis );
	FX_HunterSeekerThruster( org, axis[0], (cent->gent->s.eFlags & EF_DEAD) );

	CG_GetTagWorldPosition( model, "tag_thruster3", org, axis );
	FX_HunterSeekerThruster( org, axis[0], (cent->gent->s.eFlags & EF_DEAD) );

	cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.hunterSeekerHoverSound );
}

//--------------------------------------------------------------
void CG_SlashTrailEffects( centity_t *cent, refEntity_t *model )
{
	vec3_t		org, org2, org3, org4;
	qboolean	two_handed = qfalse;

	// Keep the effect going for the specified frames
	// NOTE: if the animation frames change this will all fall apart again...sigh.
	if ( (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) == BOTH_MELEE2 
			&& cent->pe.torso.frame >= 228 && cent->pe.torso.frame <= 230 )
	{
		CG_GetTagWorldPosition( model, "tag_rhand", org, NULL );
		CG_GetTagWorldPosition( model, "tag_weapon2", org2, NULL );
	}
	// Keep the effect going for the specified frames
	else if ( (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) == BOTH_MELEE1 
			&& cent->pe.torso.frame >= 221 && cent->pe.torso.frame <= 223 )
	{
		two_handed = qtrue;

		CG_GetTagWorldPosition( model, "tag_rhand", org, NULL );
		CG_GetTagWorldPosition( model, "tag_weapon2", org2, NULL );

		CG_GetTagWorldPosition( model, "tag_lhand", org3, NULL );
		CG_GetTagWorldPosition( model, "tag_weapon", org4, NULL );
	}
	else
	{
		// This animation doesn't use slash trails, or it's just not time to start it yet
		return;
	}

	// If there is a set of old points that we can use...then we'll use 'em!
	if ( cent->gent->trigger_formation )
	{
		// Build a quadrilateral using the old set of points and the new points
		FX_AddTri( org, org2, cent->gent->pos1, 0.1f, 0.0, 400, cgs.media.solidWhiteShader );
		FX_AddTri( cent->gent->pos1, cent->gent->pos2, org2, 0.1f, 0.0, 400, cgs.media.solidWhiteShader );

		if ( two_handed )
		{
			FX_AddTri( org3, org4, cent->gent->pos3, 0.1f, 0.0, 400, cgs.media.solidWhiteShader );
			FX_AddTri( cent->gent->pos3, cent->gent->pos4, org4, 0.1f, 0.0, 400, cgs.media.solidWhiteShader );
		}
	}
	else
	{ 
		// Tell the system that we'll have a set of old points to use next frame
		cent->gent->trigger_formation = qtrue;
	}

	// Save these puppies for later so that we can connect up the slash trail properly
	VectorCopy( org, cent->gent->pos1 );
	VectorCopy( org2, cent->gent->pos2 );

	if ( two_handed )
	{
		VectorCopy( org3, cent->gent->pos3 );
		VectorCopy( org4, cent->gent->pos4 );
	}
}

//--------------------------------------------------------------
// CG_AddTagBasedEffects
//
// For reaver melee attacks, etc.
//--------------------------------------------------------------
void CG_AddTagBasedEffects( centity_t *cent, refEntity_t *legs, refEntity_t *torso )
{
	if ( cent->gent->client->race == RACE_REAVER )
	{
		CG_SlashTrailEffects( cent, torso );
	}
	else if ( cent->gent->client->race == RACE_BOT )
	{
		// I hate sticmps!
		if ( !_stricmp( cent->gent->NPC_type, "hunterseeker" ) )
		{
			CG_HunterSeekerEffects( cent, legs );
			return;
		}
	}
	else
	{
		// This race doesn't have any tag based effects
		return;
	}
}

/*
-------------------------
CG_GetPlayerLightLevel
-------------------------
*/

void CG_GetPlayerLightLevel( centity_t *cent )
{
	vec3_t	ambient, directed, lightDir;

	//Poll the renderer for the light level
	cgi_R_GetLighting( cent->lerpOrigin, ambient, directed, lightDir );

	//Get the maximum value for the player
	cent->gent->lightLevel = directed[0];

	if ( directed[1] > cent->gent->lightLevel )
		cent->gent->lightLevel = directed[1];

	if ( directed[2] > cent->gent->lightLevel )
		cent->gent->lightLevel = directed[2];
}

/*
-------------------------
CG_ScaleEffects

Apply scaling effects
-------------------------
*/

#define	STASIS_SCALE_DOWN_TIME	500.0f
#define	STASIS_SCALE_UP_TIME	500.0f

void CG_ScaleEffects( centity_t *cent )
{
	float scale = 0, max = 100;

	if ( cent->gent->s.eFlags & EF_SCALE_UP && 
		( cent->gent->client->race == RACE_STASIS 
			|| cent->gent->client->race == RACE_8472 
			|| cent->gent->client->playerTeam == TEAM_FORGE ) )
	{
		if ( cg.time > cent->gent->fx_time + STASIS_SCALE_UP_TIME )
		{
			// Kill the flag and then ensure that we are full size
			cent->gent->s.eFlags &= ~EF_SCALE_UP;
			VectorSet( cent->gent->client->renderInfo.scaleXYZ, max, max, max );
			
			if ( cent->gent->client != NULL )
				cent->gent->client->ps.eFlags &= ~EF_SCALE_UP;
		}
		else
		{
			// This assumes that full size is 100% along each axis
			scale = (float)( cg.time - cent->gent->fx_time ) / STASIS_SCALE_UP_TIME;
			scale *= max;

			VectorSet( cent->gent->client->renderInfo.scaleXYZ, scale, scale, max );
		}
	}

	if ( cent->gent->s.eFlags & EF_SCALE_DOWN && 
		( cent->gent->client->race == RACE_STASIS 
			|| cent->gent->client->race == RACE_8472 
			|| cent->gent->client->playerTeam == TEAM_FORGE ) )
	{
		if ( cg.time > cent->gent->fx_time + STASIS_SCALE_DOWN_TIME )
		{
			// Kill the flag and then ensure that we are fully shrunk
			cent->gent->s.eFlags &= ~EF_SCALE_DOWN;
			VectorSet( cent->gent->client->renderInfo.scaleXYZ, 1, 1, 1 );

			if ( cent->gent->client != NULL )
				cent->gent->client->ps.eFlags &= ~EF_SCALE_DOWN;
		}
		else
		{
			// This assumes that full size is 100% along each axis, though 
			scale = (float)( cg.time - cent->gent->fx_time ) / STASIS_SCALE_DOWN_TIME;
			scale = 100 - ( scale * 100 );

			if ( scale > 100 )
				scale = 100;
			VectorSet( cent->gent->client->renderInfo.scaleXYZ, scale, scale, 100 );
		}
	}
}

void CG_AssimilationTubules( vec3_t start, vec3_t end, vec3_t up, float scale )
{
	vec3_t	control1, control2;//, control1_vel, control2_vel;
	vec3_t	direction;
	float	len, timecycle;

	timecycle = (cg.time&1023)/1023.0f*(2*3.14);
	VectorSubtract( end, start, direction );
	len = VectorNormalize( direction );

	VectorMA(start, len*0.20f-(sin(timecycle)*0.20f), up, control1 );
	VectorMA(start, len*0.20f+(sin(timecycle)*0.20f), up, control2 );

	/*
	vectoangles( direction, control1_vel );
	control1_vel[ROLL] += crandom() * 360;
	AngleVectors( control1_vel, NULL, NULL, control1_vel );

	vectoangles( direction, control2_vel );
	control2_vel[ROLL] += crandom() * 360;
	AngleVectors( control2_vel, NULL, NULL, control2_vel );

	VectorScale( control1_vel, 10.0f + (100.0f * random()), control1_vel );
	VectorScale( control2_vel, -10.0f + (-100.0f * random()), control2_vel );
	*/

	FX_AddBezier( start, end, 
					control1, control2, NULL, NULL, NULL, NULL, 
					scale,				//scale
					0.0f,				//dscale
					1.0f,				//alpha
					0.0f,				//dalpha
					1.0f,			//killtime
					cgs.media.assimTubesShader );
}

void CG_AddFaceDisruption( refEntity_t *ent, int powerups )
{
	if ( powerups > cg.time )
	{
		vec3_t	org, end;
		float alpha = (powerups - cg.time)/2000.0f;//1.0 to 0.0
		float lengthScale = 1.0 - alpha;//0.0 to 1.0
		if ( alpha > 0.5 )
		{
			alpha = 1.0 - alpha;
		}

		//Right eye
		VectorMA( ent->origin, 4, ent->axis[0], org );//forward
		VectorMA( org, 1.5, ent->axis[1], org );//right
		VectorMA( org, 4, ent->axis[2], org );//up
		
		VectorMA( org, 4, ent->axis[2], end );//up
		FX_AddLine( org, end, 1.0, 5, 0.0, 1.0, 0.0, 1.0, cgs.media.dkorangeParticleShader );
		
		VectorMA( org, -4, ent->axis[0], org );//pull it back
		VectorMA( org, 56*lengthScale, ent->axis[0], end );//forward
		VectorMA( end, 6*lengthScale, ent->axis[1], end );//right
		VectorMA( end, 6*lengthScale, ent->axis[2], end );//up

		if( rand() & 1 )
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.orangeParticleShader );
		}
		else
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.yellowParticleShader );
		}

		//Left eye
		VectorMA( ent->origin, 4, ent->axis[0], org );//forward
		VectorMA( org, -1.5, ent->axis[1], org );//right
		VectorMA( org, 4, ent->axis[2], org );//up

		VectorMA( org, 4, ent->axis[2], end );//up
		FX_AddLine( org, end, 1.0, 5, 0.0, 1.0, 0.0, 1.0, cgs.media.dkorangeParticleShader );
		
		VectorMA( org, -4, ent->axis[0], org );//pull it back
		VectorMA( org, 56*lengthScale, ent->axis[0], end );//forward
		VectorMA( end, -6*lengthScale, ent->axis[1], end );//right
		VectorMA( end, 6*lengthScale, ent->axis[2], end );//up

		if( rand() & 1 )
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.orangeParticleShader );
		}
		else
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.yellowParticleShader );
		}

		//Mouth
		VectorMA( ent->origin, 4, ent->axis[0], org );//forward
		VectorMA( org, 1, ent->axis[2], org );//up

		VectorMA( org, 4, ent->axis[2], end );//up
		FX_AddLine( org, end, 1.0, 5, 0.0, 1.0, 0.0, 1.0, cgs.media.dkorangeParticleShader );
		
		VectorMA( org, -4, ent->axis[0], org );//pull it back
		VectorMA( org, 56*lengthScale, ent->axis[0], end );//forward
		VectorMA( end, -2*lengthScale, ent->axis[2], end );//up

		if( rand() & 1 )
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.orangeParticleShader );
		}
		else
		{
			FX_AddLine( org, end, 1.0, 2.0, 12.0, alpha, 0.0, 1.0, cgs.media.yellowParticleShader );
		}
	}
}

/*
===============
CG_StopWeaponSounds

Stops any weapon sounds as needed
===============
*/
void CG_StopWeaponSounds( centity_t *cent )
{
	qboolean		weak = qfalse;
	weaponInfo_t	*weapon = &cg_weapons[ cent->currentState.weapon ];

	if ( !( cent->currentState.eFlags & EF_FIRING ) )
	{
		if ( cent->pe.lightningFiring )
		{
			if ( weapon->stopSound )
			{
				cgi_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, weapon->stopSound );
			}

			cent->pe.lightningFiring = qfalse;
		}
		return;
	}

	if ( cent->gent && cent->gent->client )
	{
		if ( cent->gent->client->ps.ammo[AMMO_PHASER] < 1 )
			weak = qtrue;
	}

	if ( cent->currentState.eFlags & EF_ALT_FIRING && !weak )
	{
		if ( weapon->altFiringSound )
		{
			cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->altFiringSound );
		}
		cent->pe.lightningFiring = qtrue;
	}
	else if ( cent->currentState.eFlags & EF_FIRING )
	{
		cent->pe.lightningFiring = qtrue;
		if ( weapon->firingSound )
		{
			// Weak phaser sound should stutter
			if ( weak && (rand() & 1) )
				return;

			cgi_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound );
		}
	}
}

/*
===============
CG_Player

  FIXME: Extend this to be a render func for all multiobject entities in the game such that:

	You can have and stack multiple animated pieces (not just legs and torso)
	You can attach "bolt-ons" that either animate or don't (weapons, heads, borg pieces)
	You can attach any object to any tag on any object (weapon on the head, etc.)
	
	Basically, keep a list of objects:
		Root object (in this case, the legs) with this info:
			model
			skin
			effects
			scale
			if animated, frame or anim number
			drawn at origin and angle of master entity
		Animated objects, with this info:
			model
			skin
			effects
			scale
			frame or anim number
			object it's attached to
			tag to attach it's tag_parent to
			angle offset to attach it with
		Non-animated objects, with this info:
			model
			skin
			effects
			scale
			object it's attached to
			tag to attach it's tag_parent to
			angle offset to attach it with

  ALSO: 
	Move the auto angle setting back up to the game
	Implement 3-axis scaling
	Implement alpha
	Implement tint
	Implement other effects (generic call effect at org and dir (or tag), with parms?)

===============
*/
void CG_Player( centity_t *cent ) {
	clientInfo_t	*ci;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	refEntity_t		gun;
	refEntity_t		flash;
	refEntity_t		flashlight;
	int				renderfx, i, index;
	const weaponInfo_t	*weapon;
	const weaponData_t  *wData;
	qboolean		shadow, staticScale = qfalse;
	float			shadowPlane, alpha;
	vec3_t			scaleFactor;
	entityState_t *ent;

	calcedMp = qfalse;

	//Get the player's light level for stealth calculations
	CG_GetPlayerLightLevel( cent );

	if ( cent->currentState.eFlags & EF_NODRAW ) 
	{
		return;
	}

	ent = &cent->currentState;

	//FIXME: make sure this thing has a gent and client
	if(!cent->gent)
	{
		return;
	}

	if(!cent->gent->client)
	{
		return;
	}

	if ((in_camera) && !(cent->currentState.eFlags & EF_NPC))	// If player in camera then no need for shadow
	{
		return;
	}

	if(cent->currentState.number == 0 && !cg_thirdPerson.integer )
	{
		calcedMp = qtrue;
	}

	ci = &cent->gent->client->clientInfo;

	if ( !ci->infoValid ) 
	{
		return;
	}

	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );
	memset( &gun, 0, sizeof(gun) );
	memset( &flash, 0, sizeof(flash) );
	memset( &flashlight, 0, sizeof(flashlight) );

	CG_ScaleEffects( cent );

	// Weapon sounds may need to be stopped, so check now
	CG_StopWeaponSounds( cent );

	//Set the scale
	for(i = 0; i < 3; i++)
	{
		scaleFactor[i] = (float)(cent->gent->client->renderInfo.scaleXYZ[i])/100.0f;
	}

	//FIXME: pass in the axis/angles offset between the tag_torso and the tag_head?
	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	// NB: Also plays keyframed animSounds (Bob- hope you dont mind, I was here late and at about 5:30 Am i needed to do something to keep me awake and i figured you wouldn't mind- you might want to check it, though, to make sure I wasn't smoking crack and missed something important, it is pretty late and I'm getting pretty close to being up for 24 hours here, so i wouldn't doubt if I might have messed something up, but i tested it and it looked right.... noticed in old code base i was doing it wrong too, which explains why I was getting so many damn sounds all the time!  I had to lower the probabilities because it seemed like i was getting way too many sounds, and that was the problem!  Well, should be fixed now I think...)
	CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	cent->gent->client->renderInfo.legsFrame = cent->pe.legs.frame;
	cent->gent->client->renderInfo.torsoFrame = cent->pe.torso.frame;

	// add powerups floating behind the player
	CG_PlayerPowerups( cent );

	// add the shadow
	shadow = CG_PlayerShadow( cent, &shadowPlane );

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) 
	{
		renderfx = RF_THIRD_PERSON;			// only draw in mirrors
	}
	
	if ( cg_shadows.integer == 3 && shadow ) 
	{
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

	//
	// add the legs
	//
	legs.hModel = ci->legsModel;
	legs.customSkin = ci->legsSkin;

	VectorCopy( cent->lerpOrigin, legs.origin );

	//Scale applied to a refEnt will apply to any models attached to it... (except boltOns?)
	//This seems to copy the scale to every piece attached, kinda cool, but doesn't
	//allow the body to be scaled up without scaling a bolt on or whatnot...
	//Only apply scale if it's not 100% scale...
	if(scaleFactor[0] != 1.0)
	{
		VectorScale( legs.axis[0], scaleFactor[0], legs.axis[0] );
		legs.nonNormalizedAxes = qtrue;
	}

	if(scaleFactor[1] != 1.0)
	{
		VectorScale( legs.axis[1], scaleFactor[1], legs.axis[1] );
		legs.nonNormalizedAxes = qtrue;
	}

	if(scaleFactor[2] != 1.0)
	{
		VectorScale( legs.axis[2], scaleFactor[2], legs.axis[2] );
		legs.nonNormalizedAxes = qtrue;
		if ( !staticScale )
		{
			//FIXME:? need to know actual height of leg model bottom to origin, not hardcoded
			legs.origin[2] += 24 * (scaleFactor[2] - 1);
		}
	}

	VectorCopy( legs.origin, legs.lightingOrigin );
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

	CG_AddRefEntityWithPowerups( &legs, cent->currentState.powerups&~(1<<PW_DISINT_6), cent->gent );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) 
	{
		return;
	}

	//
	// add the torso
	//
	torso.hModel = ci->torsoModel;
	if (torso.hModel) 
	{
		orientation_t	tag_torso;

		torso.customSkin = ci->torsoSkin;

		VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

		CG_PositionRotatedEntityOnTag( &torso, &legs, legs.hModel, "tag_torso", &tag_torso );
		VectorCopy( torso.origin, cent->gent->client->renderInfo.torsoPoint );
		vectoangles( tag_torso.axis[0], cent->gent->client->renderInfo.torsoAngles );

		torso.shadowPlane = shadowPlane;
		torso.renderfx = renderfx;

		CG_AddRefEntityWithPowerups( &torso, cent->currentState.powerups&~(1<<PW_DISINT_6), cent->gent );

		// Do slash trails, etc if necessary
		CG_AddTagBasedEffects( cent, &legs, &torso );

		// Borg assimilator wires
		//If a Borg and weapon == assimilator and fire button down:
		if ( cent->currentState.weapon == WP_BORG_ASSIMILATOR && cent->gent->client->buttons & BUTTON_ATTACK && cent->gent->enemy && cent->gent->enemy->client )//&& cent->gent->enemy->s.number != 0 )
		{
			vec3_t	targetSpot, start, end, dir;
			refEntity_t	temp;

			CG_PositionEntityOnTag( &temp, &torso, torso.hModel, "tag_lhand");
			//find closer point: enemy tag_torso or enemy tag_head
			if ( cent->gent->enemy->s.number == 0 )
			{
				VectorCopy( cg.refdef.vieworg, targetSpot );
				targetSpot[2] -= 8;
			}
			else
			{
				float	dist;

				VectorSubtract( temp.origin, cent->gent->enemy->client->renderInfo.headPoint, dir );
				dist = VectorLengthSquared( dir );
				VectorSubtract( temp.origin, cent->gent->enemy->client->renderInfo.torsoPoint, dir );
				if ( dist > VectorLengthSquared( dir ) )
				{
					VectorCopy( cent->gent->enemy->client->renderInfo.torsoPoint, targetSpot );
				}
				else
				{
					VectorCopy( cent->gent->enemy->client->renderInfo.headPoint, targetSpot );
				}
			}

			//do a bezier curve of 2 assimilation tubules from tag_muzzle
			//to the closer enemy tag
			VectorSubtract( temp.origin, targetSpot, dir );
			if ( VectorLengthSquared( dir ) <= 576 )//24 squared
			{
				VectorMA( temp.origin, 1, temp.axis[1], start );
				VectorMA( targetSpot, 1, temp.axis[1], end );
				CG_AssimilationTubules( start, end, temp.axis[2], 0.25f );
				VectorMA( temp.origin, -1, temp.axis[1], start );
				VectorMA( targetSpot, -1, temp.axis[1], end );
				CG_AssimilationTubules( start, end, temp.axis[2], 0.25f );
			}
		}
		//
		// add the head
		//
		head.hModel = ci->headModel;
		if (head.hModel) 
		{
			orientation_t	tag_head;

			//Deal with facial expressions
			CG_PlayerHeadExtension( cent, &head );

			VectorCopy( cent->lerpOrigin, head.lightingOrigin );

			CG_PositionRotatedEntityOnTag( &head, &torso, torso.hModel, "tag_head", &tag_head );
			VectorCopy( head.origin, cent->gent->client->renderInfo.headPoint );
			vectoangles( tag_head.axis[0], cent->gent->client->renderInfo.headAngles );

			head.shadowPlane = shadowPlane;
			head.renderfx = renderfx;

			CG_AddRefEntityWithPowerups( &head, cent->currentState.powerups&~(1<<PW_DISINT_6), cent->gent );
			CG_AddFaceDisruption( &head, cent->gent->client->ps.powerups[PW_REGEN] );

			//Temp: Add special effects
			if(cent->gent->client->race == RACE_BORG && (cent->currentState.eFlags&EF_EYEBEAM) && cent->gent->client->ps.stats[STAT_HEALTH] > 0)
			{//Eyebeam, only if have an enemy?  Or only if awake and walking around?
				trace_t	trace;
				vec3_t	beamOrg, beamEnd, rgb = {1.0f,0.0f,0.0f};
				refEntity_t	temp;
				//FIXME: this should be attached to a eyepiece bolt-on eventually, not the head itself

				CG_PositionEntityOnTag( &temp, &head, head.hModel, "tag_ear");

				VectorCopy( temp.origin, beamOrg );
				VectorMA( beamOrg, 1024, temp.axis[0], beamEnd);//forward to end

				/*
				VectorMA(head.origin, 6.35, head.axis[2], beamOrg);//up
				VectorMA(beamOrg, 1.3, head.axis[1], beamOrg);//right
				VectorMA(beamOrg, 5.65, head.axis[0], beamOrg);//forward
				VectorMA(beamOrg, 1024, head.axis[0], beamEnd);//forward to end
				*/
				cgi_CM_BoxTrace( &trace, beamOrg, beamEnd, NULL, NULL, 0, MASK_SHOT );
				VectorCopy(trace.endpos, beamEnd);
				alpha = 1.0f - (random() * 0.5);
				FX_AddLine( beamOrg, beamEnd, 1.0f, 0.35f + ( crandom() * 0.1 ), 0.0f, alpha, alpha, rgb,rgb,1.0f, cgs.media.whiteLaserShader );
				VectorMA(beamOrg, 0.5, head.axis[0], beamOrg);//forward
				FX_AddSprite( beamOrg, NULL, NULL, 1.0f + (random() * 2.0f), 0.0f, 0.9f, 0.9f, 0.0f, 0.0f, 0.0f, cgs.media.borgEyeFlareShader );
				FX_AddQuad( beamEnd, trace.plane.normal, NULL, NULL, 2.0f + (crandom() * 1.0f), 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, cgs.media.borgEyeFlareShader );
			}

			if ( !calcedMp )
			{//First person player's eyePoint and eyeAngles should be copies from cg.refdef...
				//Calc this client's eyepoint
				VectorCopy( head.origin, cent->gent->client->renderInfo.eyePoint );
				VectorMA( cent->gent->client->renderInfo.eyePoint, CG_EyePointOfsForRace[cent->gent->client->race][1]*scaleFactor[2], head.axis[2], cent->gent->client->renderInfo.eyePoint );//up
				VectorMA( cent->gent->client->renderInfo.eyePoint, CG_EyePointOfsForRace[cent->gent->client->race][0]*scaleFactor[0], head.axis[0], cent->gent->client->renderInfo.eyePoint );//forward
				//Calc this client's eyeAngles
				vectoangles( head.axis[0], cent->gent->client->renderInfo.eyeAngles );
			}

			//FIXME: this should really be a function call
			//If have the Borg sparkies effect, apply it
			if ( cent->gent->s.eFlags & EF_BORG_SPARKIES && Q_irand( 0, cg.frametime ) > 5 )
			{
				vec3_t	angles, vec, spot;

				VectorAdd( head.origin, torso.origin, spot );
				VectorScale( spot, 0.5, spot );
				spot[0] += crandom() * 4;
				spot[1] += crandom() * 4;
				spot[2] += crandom() * 4;
				VectorSet( angles, Q_flrand(-360, 360), Q_flrand(-360, 360), Q_flrand(-360, 360) );
				AngleVectors( angles, vec, 0, 0 );
				if ( Q_irand( 0, 1) )
				{
					VectorScale( vec, Q_flrand( 300, 400 ), vec );
					FX_BorgDeathSparkParticles( spot, angles, vec, NULL );
					if ( !Q_irand( 0, 3 ) )
					{
						cgi_S_StartSound (NULL, cent->gent->s.number, CHAN_AUTO, cgi_S_RegisterSound ( va("sound/ambience/spark%d.wav", Q_irand(1, 6)) ) );
					}
				}
				/*else
				{
					vec3_t	end;
					float	duration = Q_irand( 50, 500 );

					VectorMA( spot, Q_flrand( 16, 52 ), vec, end );
					//NOTE: Maybe bezier curves from one tag to another?
					FX_AddElectricity( spot, end, 
							1.0f, Q_flrand(3, 9), -45, 
							1.0f, 0.0f, 
							duration * 2, cgs.media.borgLightningShaders[Q_irand(0, 3)], FXF_WRAP | FXF_TAPER, 0.6f );
					if ( !Q_irand( 0, 3 ) )
					{
						cgi_R_AddLightToScene( spot, Q_irand(100, 200), 100, 150, 50 );
					}
				}*/
			}
		}
		else
		{
			VectorCopy( torso.origin, cent->gent->client->renderInfo.eyePoint );
			cent->gent->client->renderInfo.eyePoint[2] += cent->gent->maxs[2] - 4;
			vectoangles( torso.axis[0], cent->gent->client->renderInfo.eyeAngles );
		}

		//
		// add the gun
		//
		CG_RegisterWeapon( cent->currentState.weapon );
		weapon = &cg_weapons[cent->currentState.weapon];
		wData = &weaponData[cent->currentState.weapon];

		gun.hModel = weapon->weaponWorldModel;
		if (gun.hModel) 
		{
			//FIXME: allow scale, animation and angle offsets
			VectorCopy( cent->lerpOrigin, gun.lightingOrigin );

			//FIXME: allow it to be put anywhere and move this out of if(torso.hModel)
			//Will have to call CG_PositionRotatedEntityOnTag
			
			CG_PositionEntityOnTag( &gun, &torso, torso.hModel, "tag_weapon");

			gun.shadowPlane = shadowPlane;
			gun.renderfx = renderfx;

			if ( cent->currentState.weapon != WP_BORG_TASER || Q_stricmp( "satan", cent->gent->NPC_type ) != 0 )
			{//uber-hack: satan's robot doesn't actually draw the weapon in-hand
				CG_AddRefEntityWithPowerups( &gun, 
					(cent->currentState.powerups & ( (1<<PW_INVIS)|(1<<PW_QUAD)|(1<<PW_BATTLESUIT)|(1<<PW_BORG_SHIELD)|(1<<PW_DISINT_6) ) ),
					cent->gent );
			}

			//
			// add the flash (even if invisible)
			//

			// impulse flash
			if ( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME )
			{
				if ( cent->gent->alt_fire && weapon->flashaModel ) {
					if (weapon->flasha2Model && ( rand() & 1 ) ) {
						flash.hModel = weapon->flasha2Model;
					} else {
						flash.hModel = weapon->flashaModel;
					}
				}
				else {
					// always fallback to main flash
					if (weapon->flash2Model && ( rand() & 1 ) ) {
						flash.hModel = weapon->flash2Model;
					} else {
						flash.hModel = weapon->flashModel;
					}
				}

				if (flash.hModel) 
				{
					CG_PositionEntityOnTag( &flash, &gun, gun.hModel, "tag_flash");
					flash.renderfx = renderfx | RF_NOSHADOW;
					cgi_R_AddRefEntityToScene( &flash );

					// make a dlight for the flash
					if ( wData->flashColor[0] || wData->flashColor[1] || wData->flashColor[2] ) {
						cgi_R_AddLightToScene( flash.origin, 200 + (rand()&31), wData->flashColor[0],
							wData->flashColor[1], wData->flashColor[2] );
					}

					if(!calcedMp)
					{
						// add lightning bolt
						CG_LightningBolt( cent, flash.origin );

						VectorCopy( flash.origin, cent->gent->client->renderInfo.muzzlePoint );
						cent->gent->client->renderInfo.mPCalcTime = cg.time;
						calcedMp = qtrue;
					}
				}
			}

			if (!calcedMp)
			{// Set the muzzle point
				orientation_t orientation;

				cgi_R_LerpTag( &orientation, weapon->weaponModel, gun.oldframe, gun.frame,
					1.0f - gun.backlerp, "tag_flash" );

				// FIXME: allow origin offsets along tag?
				VectorCopy( gun.origin, cent->gent->client->renderInfo.muzzlePoint );
				for ( i = 0 ; i < 3 ; i++ ) 
				{
					VectorMA( cent->gent->client->renderInfo.muzzlePoint, orientation.origin[i], gun.axis[i], cent->gent->client->renderInfo.muzzlePoint );
				}

				cent->gent->client->renderInfo.mPCalcTime = cg.time;
				// Weapon wasn't firing anymore, so ditch any weapon associated looping sounds.
				//cent->gent->s.loopSound = 0;
			}
			/*
			//Add a flashlight:
			flashlight.hModel = cgs.media.flashLightModel;
			if (flashlight.hModel) 
			{
				//Add the model to trace the stencil:
				CG_PositionEntityOnTag( &flashlight, &gun, weapon->weaponModel, "tag_flash");
				flashlight.renderfx = renderfx;
				
				AngleVectors(cent->lerpAngles, flashlight.lightDir, NULL, NULL);
				VectorScale(flashlight.lightDir, -1, flashlight.lightDir);

				cgi_R_AddRefEntityToScene( &flashlight );

				//Add a lightcone model with lightcone shader

				//Flag one of them to have a flare?
			}
			*/
		}
	}
	else
	{
		VectorCopy( legs.origin, cent->gent->client->renderInfo.eyePoint );
		cent->gent->client->renderInfo.eyePoint[2] += cent->gent->maxs[2] - 4;
		vectoangles( legs.axis[0], cent->gent->client->renderInfo.eyeAngles );
	}


	boltOnInfo_t	*boltOnInfo;
	//Bolt-Ons
	for ( i = 0; i < MAX_BOLT_ONS; i++ )
	{
		boltOnInfo = &cent->gent->client->renderInfo.boltOns[i]; 
		index = boltOnInfo->index;

 		if ( index >= 0 && index < numBoltOns )
		{
			boltOn_t		*boltOn;
			refEntity_t		boltOnRefEnt;
			qboolean		attached = qfalse;

			boltOn = &knownBoltOns[index];
			switch( boltOn->targetModel )
			{
			case MODEL_HEAD:
				if ( head.hModel )
				{
					attached = CG_ApplyBoltOnToRefEnt( &boltOnRefEnt, boltOn, &cent->gent->client->renderInfo.boltOns[i], cent->lerpOrigin, &head );
				}
				break;
			case MODEL_TORSO:
				if ( torso.hModel )
				{
					attached = CG_ApplyBoltOnToRefEnt( &boltOnRefEnt, boltOn, &cent->gent->client->renderInfo.boltOns[i], cent->lerpOrigin, &torso );
				}
				break;
			case MODEL_LEGS:
				if ( legs.hModel )
				{
					attached = CG_ApplyBoltOnToRefEnt( &boltOnRefEnt, boltOn, &cent->gent->client->renderInfo.boltOns[i], cent->lerpOrigin, &legs );
				}
				break;
			case MODEL_WEAPON1:
				if ( gun.hModel )
				{
					attached = CG_ApplyBoltOnToRefEnt( &boltOnRefEnt, boltOn, &cent->gent->client->renderInfo.boltOns[i], cent->lerpOrigin, &gun );
				}
				break;
			default:
				break;
			}

			if ( attached )
			{
				boltOnRefEnt.shadowPlane = shadowPlane;
				boltOnRefEnt.renderfx = renderfx;
				//need to apply effects to boltOns too
				CG_AddRefEntityWithPowerups( &boltOnRefEnt, cent->currentState.powerups&~(1<<PW_DISINT_6), cent->gent );

				// It would have been better to use AddRefEntWithPowerups, except boltons don't have their own powerup flags
				if ( boltOnInfo->fxFlags & BOLTON_BEAMIN ) 
				{
					boltOnRefEnt.customShader = cgs.media.transportShader;
					boltOnRefEnt.shaderTime = (boltOnInfo->startTime + 2000 ) * 0.001f; // Scale startTime to server time
					CG_AddRefEntityWithPowerups( &boltOnRefEnt, cent->currentState.powerups&~(1<<PW_DISINT_6), cent->gent );

					if ( boltOnInfo->startTime + 1900 < cg.time )
					{
						boltOnInfo->fxFlags &= ~BOLTON_BEAMIN;
					}
				}
			}
		}
	}
	//FIXME: for debug, allow to draw a cone of the NPC's FOV...
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info

FIXME: We do not need to do this, we can remember the last anim and frame they were
on and coontinue from there.
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;	

	if(cent->currentState.clientNum < MAX_CLIENTS)
	{
		CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim );
		CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim );
	}
	else if(cent->gent && cent->gent->client)
	{
		CG_ClearLerpFrame( &cent->gent->client->clientInfo, &cent->pe.legs, cent->currentState.legsAnim );
		CG_ClearLerpFrame( &cent->gent->client->clientInfo, &cent->pe.torso, cent->currentState.torsoAnim );
	}
	//else????

	EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.legs ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if ( cg_debugPosition.integer ) {
		CG_Printf("%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}
}

