//g_objectives.cpp
//reads in ext_data\objectives.dat to objectives[]

#include "g_local.h"
#include "g_items.h"

#define	G_OBJECTIVES_CPP

#include "objectives.h"

qboolean G_ParseString( char **data, char **s ); 

char *mission_objective_text[MAX_OBJECTIVES];
char *tour_objective_text[MAX_TOUR_OBJECTIVES];

qboolean tactical_info[MAX_TACTICAL_TXT];
char *tactical_text[MAX_TACTICAL_TXT];
qboolean	missionInfo_Updated;


/*
============
OBJ_SetPendingObjectives
============
*/
void OBJ_SetPendingObjectives(gentity_t *ent)
{
	int i;

	for (i=0;i<MAX_OBJECTIVES;++i)
	{
		if ((ent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_PENDING) && 
			(ent->client->sess.mission_objectives[i].display))
		{
			ent->client->sess.mission_objectives[i].status = OBJECTIVE_STAT_FAILED;
		}
	}

	for (i=0;i<MAX_TOUR_OBJECTIVES;++i)
	{
		if ((ent->client->tourSess.tour_objectives[i].status == OBJECTIVE_STAT_PENDING) && 
			(ent->client->tourSess.tour_objectives[i].display))
		{
			ent->client->tourSess.tour_objectives[i].status = OBJECTIVE_STAT_FAILED;
		}
	}

}

/*
============
OBJ_SaveMissionObjectives
============
*/
void OBJ_SaveMissionObjectives( gclient_t *client )
{
	gi.AppendToSaveGame('OBJT', client->sess.mission_objectives, sizeof(client->sess.mission_objectives));
}

/*
============
OBJ_SaveTacticalInfo
============
*/
void OBJ_SaveTacticalInfo(void)
{
	gi.AppendToSaveGame('TACT', tactical_info, sizeof(tactical_info));
}

/*
============
OBJ_SaveObjectiveData
============
*/
void OBJ_SaveObjectiveData(void)
{
	gclient_t *client;

	client = &level.clients[0];

	OBJ_SaveMissionObjectives( client );
	OBJ_SaveTacticalInfo();
}

/*
============
OBJ_LoadMissionObjectives
============
*/
void OBJ_LoadMissionObjectives( gclient_t *client )
{
	gi.ReadFromSaveGame('OBJT', (void *) &client->sess.mission_objectives, sizeof(client->sess.mission_objectives), NULL);
}


/*
============
OBJ_LoadTacticalInfo
============
*/
void OBJ_LoadTacticalInfo(void)
{
	gi.ReadFromSaveGame('TACT', (void *) &tactical_info, sizeof(tactical_info), NULL);
}

/*
============
OBJ_LoadObjectiveData
============
*/
void OBJ_LoadObjectiveData(void)
{
	gclient_t *client;

	client = &level.clients[0];

	OBJ_LoadMissionObjectives( client );
	OBJ_LoadTacticalInfo();
}


const char* defstr = "?";

/*
============
OBJ_ParseObjectives
============
*/
void OBJ_ParseObjectives(char *buffer)
{
	char *holdBuf= buffer;
	char	*tokenStr;
	int	i=0;
	char *holdText;

	//initialize the text data jic we fall short
	_asm {
		lea edi, mission_objective_text
		mov ecx, MAX_OBJECTIVES
		mov eax, defstr
		rep stosd
	}

	_asm {
		lea edi, tour_objective_text
		mov ecx, MAX_TOUR_OBJECTIVES
		mov eax, defstr
		rep stosd
	}

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
			if (i<MAX_OBJECTIVES)
			{
				mission_objective_text[i] = G_NewString(tokenStr);
			}
			else
			{
				tour_objective_text[i-MAX_OBJECTIVES] = G_NewString(tokenStr);
			}

			++i;
			if (i>MAX_OBJECTIVES + MAX_TOUR_OBJECTIVES)
			{
				gi.Printf(S_COLOR_YELLOW"WARNING: Too many objectives in ext_dat/objectives.dat\n");
				break;
			}
		}
	}
}

/*
============
OBJ_LoadObjectives
============
*/
void OBJ_LoadObjectives (void)
{
	char	*buffer;
	char	finalName[MAX_QPATH];
	int		len;

	G_LanguageFilename("ext_data/objectives","dat",(char *) &finalName);

	len = gi.FS_ReadFile(finalName,(void **) &buffer);

	OBJ_ParseObjectives(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}


/*
============
OBJ_ParseTactical
============
*/
void OBJ_ParseTactical(char *buffer)
{
	char *holdBuf;
	char	*tokenStr;
	int	i=0;
	char *holdText;
	holdBuf = buffer;

	//init the bool array
	memset(&tactical_info,0,sizeof(tactical_info));

	//now initialize the text data as well.
	_asm {
		lea edi, tactical_text
		mov ecx, MAX_TACTICAL_TXT
		mov eax, defstr
		rep stosd
	}

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
			tactical_text[i] = G_NewString(tokenStr);
			++i;
			if (i>MAX_TACTICAL_TXT)
			{
				gi.Printf(S_COLOR_YELLOW"WARNING: Too many tactical strings in ext_dat/tactical.dat\nShould be %d\n",MAX_TACTICAL_TXT);
				break;
			}
		}
	}

}

/*
============
OBJ_LoadTactical
============
*/
void OBJ_LoadTactical (void)
{
	char	*buffer;
	char	finalName[MAX_QPATH];
	int		len;

	G_LanguageFilename("ext_data/tactical","dat",(char *) &finalName);

	len = gi.FS_ReadFile(finalName,(void **) &buffer);

	OBJ_ParseTactical(buffer);

	gi.FS_FreeFile( buffer );	//let go of the buffer
}
