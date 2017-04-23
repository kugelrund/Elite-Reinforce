// cg_text.c -- 

#include "cg_text.h"
#include "cg_local.h"
#include "cg_media.h"
#include "..\game\speakers.h"


int precacheWav_i;	// Current high index of precacheWav array
precacheWav_t precacheWav[MAX_PRECACHEWAV];


int precacheText_i;	// Current high index of precacheText array
precacheText_t precacheText[MAX_PRECACHETEXT];


extern vec4_t textcolor_caption;
extern vec4_t textcolor_center;
extern vec4_t textcolor_scroll;


#define	GAMETEXT_X_START	75.0f
#define	GAMETEXT_X_END		600.0f

#define	MAX_NUM_GAMELINES 4

void CG_GameText(int y, int charWidth )
{
	char	*s,*holds;
	int i, len;
	float	x, w;
	int		numChars;
	int text_i;
	char str[MAX_QPATH];
	int	holdCnt,playingTime;
	int totalLength,sound,max;

	Q_strncpyz (str, CG_Argv( 1 ), MAX_QPATH );

	cg.gameTextSpeaker = atoi(CG_Argv(2));
	cg.gameTextEntNum = atoi(CG_Argv(3));

	sound = cgs.sound_precache[atoi(CG_Argv(4))];

	text_i = CG_SearchTextPrecache(str);
	//ensure we found a match
	if (text_i == -1) 
	{	
		Com_Printf("WARNING: CG_GameText given invalid text key :'%s'",str);
		return; 
	}

	cg.gameTextTime = cg.time;
	cg.printTextY = 5 + SMALLCHAR_HEIGHT;

	cg.centerPrintCharWidth = charWidth;
	cg.gameTextCurrentLine = 0;

	// count the number of lines for centering
	cg.scrollTextLines = 1;


	memset (cg.printText, 0, sizeof(cg.printText));

	// Break into individual lines
	i = 0;
	len = 0;
	s = precacheText[text_i].text;
	holds = s;

	playingTime = cgi_S_GetSampleLength(sound);
	totalLength = strlen(s);
	if (totalLength == 0)
	{
		totalLength = 1;
	}
	cg.gameLetterTime = playingTime / totalLength;

	//We start at column 75 according to DrawGameText
	x = GAMETEXT_X_START;
	w = GAMETEXT_X_END - GAMETEXT_X_START;
	numChars = floor(w/SMALLCHAR_WIDTH);

	while( *s ) 
	{
		len++;
		if (*s == '\n')
		{//Being told explicitly to start a new line
			Q_strncpyz( cg.printText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}
		else if ( len == numChars )
		{//Reached max length of this line
			//step back until we find a space
			while( len && *s != ' ' )
			{
				s--;
				len--;
			}
			//break the line here
			Q_strncpyz( cg.printText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}
		s++;
	}

	len++;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( cg.printText[i], holds, len); // To get the last line

	//NOTE: This might be able to use the S_Override or TID_VOICE info from the cg.gameTextEntNum
	//		to decide when to drop the text...
	max = MAX_NUM_GAMELINES;
	if (max >cg.scrollTextLines)
	{
		max = cg.scrollTextLines;
	}

	holdCnt = 0;
	for (i=0;i<max;++i)
	{
		holdCnt += strlen(cg.printText[i]);
	}
	cg.gameNextTextTime = cg.time + (holdCnt * cg.gameLetterTime);	


	cg.scrollTextTime = 0;	// No scrolling during captions
}

void CG_DrawGameText(void)
{
	char	*start;
	int		l;
	int		i,max;
	int		x, y;
	char linebuffer[1024], string[1024];
	int	holdCnt;
	vec4_t		color; 

	if ( !cg.gameTextTime ) 
	{
		return;
	}

	// Advance to next line (if there are any) and calculate time to show
	if ( cg.gameNextTextTime < cg.time ) 
	{
		cg.gameTextCurrentLine += MAX_NUM_GAMELINES;

		if (cg.gameTextCurrentLine >= cg.scrollTextLines)
		{
			cg.gameTextTime = 0;
			return;
		}
		else
		{
			max = MAX_NUM_GAMELINES;
			if ((cg.scrollTextLines - cg.gameTextCurrentLine) < max)
			{
				max = cg.scrollTextLines - cg.gameTextCurrentLine;
			}

			// Loop through next lines to calc how long to show 'em
			holdCnt = 0;
			for (i=cg.gameTextCurrentLine;i<(cg.gameTextCurrentLine + max);++i)
			{
				holdCnt += strlen(cg.printText[i]);
			}
			cg.gameNextTextTime = cg.time + (holdCnt * cg.gameLetterTime);	
		}
	}

	// Give a color if one wasn't given
	if((textcolor_caption[0] == 0) && (textcolor_caption[1] == 0) && 
		(textcolor_caption[2] == 0) && (textcolor_caption[3] == 0))
	{
		Vector4Copy( colorTable[CT_WHITE], textcolor_caption );
	}


	color[0] = colorTable[CT_BLACK][0];
	color[1] = colorTable[CT_BLACK][1];
	color[2] = colorTable[CT_BLACK][2];
	color[3] = 0.350f;

	// Set Y of the first line
	y = cg.printTextY;
	x = GAMETEXT_X_START;

	// Background
	cgi_R_SetColor(color);	// Background, CLAMP TO 4 LINES
	CG_DrawPic( x - 4, y - SMALLCHAR_HEIGHT - 2, (70 * SMALLCHAR_WIDTH),(( ((cg.scrollTextLines>MAX_NUM_GAMELINES)?MAX_NUM_GAMELINES:cg.scrollTextLines) + 1) * SMALLCHAR_HEIGHT) + 4,	cgs.media.ammoslider );

	sprintf(string, "%s:", speakerTable[cg.gameTextSpeaker].stringID);

	CG_DrawStringExt( x, y - SMALLCHAR_HEIGHT, string, colorTable[CT_LTPURPLE1], qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT );

	for (i=	cg.gameTextCurrentLine;i< cg.gameTextCurrentLine + MAX_NUM_GAMELINES;++i)
	{
		start = cg.printText[i];
		while ( 1 ) 
		{

			for ( l = 0; l < 80; l++ ) 
			{
				if ( !start[l] || start[l] == '\n' ) 
				{
					break;
				}
				linebuffer[l] = start[l];
			}
			linebuffer[l] = 0;


			CG_DrawStringExt( x, y, linebuffer, textcolor_caption, qfalse, qtrue,
				SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT );

			y += SMALLCHAR_HEIGHT;

			while ( *start && ( *start != '\n' ) ) 
			{
				start++;
			}

			if ( !*start ) 
			{
				break;
			}
			start++;
		}
	}

	cgi_R_SetColor( NULL );

}

/*
===============================================================================

CAPTION TEXT

===============================================================================
*/
void CG_CaptionTextStop(void) 
{
	cg.captionTextTime = 0;
}

void CG_CaptionText( const char *str, int sound, int y, int charWidth ) 
{
	char	*s,*holds;
	int i, len;//, numChars;
	int text_i;
	int	holdTime,playingTime;
	int totalLength;

	text_i = CG_SearchTextPrecache((char *) str);
	//ensure we found a match
	if (text_i == -1) 
	{	
		Com_Printf("WARNING: CG_CaptionText given invalid text key :'%s'",str);
		return; 
	}

	cg.captionTextTime = cg.time;
	if (in_camera) {
		cg.captionTextY = SCREEN_HEIGHT;
	} else {	//get above the hud
		cg.captionTextY = SCREEN_HEIGHT - SMALLCHAR_HEIGHT * 3;
	}
	cg.centerPrintCharWidth = charWidth;
	cg.captionTextCurrentLine = 0;

	// count the number of lines for centering
	cg.scrollTextLines = 1;

	memset (cg.captionText, 0, sizeof(cg.captionText));

	// Break into individual lines
	i = 0;
	len = 0;
	s = precacheText[text_i].text;
	holds = s;

	playingTime = cgi_S_GetSampleLength(sound);
	totalLength = strlen(s);
	if (totalLength == 0)
	{
		totalLength = 1;
	}
	cg.captionLetterTime = playingTime / totalLength;

//	numChars = floor( (SCREEN_WIDTH - (cg.centerPrintCharWidth*2)) / cg.centerPrintCharWidth );	

	while( *s ) 
	{
		Q_strcat(cg.captionText[i],sizeof(cg.captionText[i]),va("%c",*s));
		len++;
		if (*s == '\n')
		{//being told explicitly to start a new line
			Q_strncpyz( cg.captionText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}
		else 
		if (CG_ProportionalStringWidth( cg.captionText[i], CG_SMALLFONT) >= SCREEN_WIDTH - (cg.centerPrintCharWidth*2))	//if ( len == numChars )
		{//Reached max length of this line
			//step back until we find a space
			while( len && *s != ' ' )
			{
				s--;
				len--;
			}
			Q_strncpyz( cg.captionText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}		
		s++;
	}

	len++;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( cg.captionText[i], holds, len); // To get the last line

	
	holdTime = strlen(cg.captionText[0]);
	if (cg.scrollTextLines > 1)
	{
		holdTime += strlen(cg.captionText[1]);
	}
	cg.captionNextTextTime = cg.time + (holdTime * cg.captionLetterTime);	

	cg.scrollTextTime = 0;	// No scrolling during captions

	//Echo to console in dev mode
	if ( cg_developer.integer )
	{
		Com_Printf( "%s\n", cg.captionText[i] );
	}
}


void CG_DrawCaptionText(void)
{
	char	*start;
	int		l;
	int		i;
	int		x, y, w;
	char linebuffer[1024];
	int	holdTime;

	if ( !cg.captionTextTime ) 
	{
		return;
	}


	if (cg_skippingcin.value != 0.0f)
	{
		cg.captionTextTime = 0;
		return;
	}

	if ( cg.captionNextTextTime < cg.time ) 
	{
		cg.captionTextCurrentLine += 2;

		if (cg.captionTextCurrentLine >= cg.scrollTextLines)
		{
			cg.captionTextTime = 0;
			return;
		}
		else
		{
			holdTime = strlen(cg.captionText[cg.captionTextCurrentLine]);
			if (cg.scrollTextLines >= cg.captionTextCurrentLine)
			{
				holdTime += strlen(cg.captionText[cg.captionTextCurrentLine + 1]);
			}

			// 50 miliseconds for every letter
			cg.captionNextTextTime = cg.time + (holdTime * cg.captionLetterTime);//50);
		}
	}

	// Give a color if one wasn't given
	if((textcolor_caption[0] == 0) && (textcolor_caption[1] == 0) && 
		(textcolor_caption[2] == 0) && (textcolor_caption[3] == 0))
	{
		Vector4Copy( colorTable[CT_WHITE], textcolor_caption );
	}

	cgi_R_SetColor(textcolor_caption);

	// Set Y of the first line
	y = cg.captionTextY - SMALLCHAR_HEIGHT * 3;

	for (i=	cg.captionTextCurrentLine;i< cg.captionTextCurrentLine + 2;++i)
	{
		start = cg.captionText[i];
		while ( 1 ) 
		{

			for ( l = 0; l < /*80*/128; l++ )	// should be some sensible limit I 'spose, but 80 is too small now
			{
				if ( !start[l] || start[l] == '\n' ) 
				{
					break;
				}
				linebuffer[l] = start[l];
			}
			linebuffer[l] = 0;

			w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

//			x = ( SCREEN_WIDTH - w ) / 2;
			x =   SCREEN_WIDTH       / 2;

//			CG_DrawStringExt( x, y, linebuffer, textcolor_caption, qfalse, qtrue,
//				SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT );

			CG_DrawProportionalString( x, y, linebuffer, CG_CENTER | CG_DROPSHADOW | CG_SMALLFONT, textcolor_caption );

			y += SMALLCHAR_HEIGHT * 1.5;

			while ( *start && ( *start != '\n' ) ) 
			{
				start++;
			}

			if ( !*start ) 
			{
				break;
			}
			start++;
		}
	}

	cgi_R_SetColor( NULL );

}

/*
===============================================================================

SCROLL TEXT

===============================================================================
*/
/*
CG_ScrollText - split text up into seperate lines
*/
void CG_ScrollText( const char *str, int y, int charWidth ) 
{
	char	*s,*holds;
	int i, len, numChars;
	int text_i;
	
	// Find text to match the str given
	text_i = CG_SearchTextPrecache((char *) str);

	cg.scrollTextTime = cg.time;
	cg.printTextY = SCREEN_HEIGHT;

	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.scrollTextLines = 1;

	// Break text string into individual lines
	if (text_i == -1)	// Didn't find a match so just print what you were given
	{
		s = (char *) str;
	}
	else
	{
		s = precacheText[text_i].text;
	}

	i = 0;
	len = 0;
	holds = s;

	numChars = floor( (SCREEN_WIDTH - (2*cg.centerPrintCharWidth)) / cg.centerPrintCharWidth );
	
	while( *s ) 
	{
		len++;
		if (*s == '\n')
		{//Being told explicitly to start a new line
			Q_strncpyz( cg.printText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}
		else if ( len == numChars )
		{//Reached max length of this line
			//step back until we find a space
			while( len && *s != ' ' )
			{
				s--;
				len--;
			}
			Q_strncpyz( cg.printText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.scrollTextLines++;
		}
		s++;
	}

	// To get the last line
	len++;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( cg.printText[i], holds, len);
	cg.captionTextTime = 0;		// No captions during scrolling
}

#define SCROLL_LPM (1/50.0) // 1 line per 50 ms
void CG_DrawScrollText(void)
{
	char	*start;
	int		l;
	int		i;
	int		y;
	char linebuffer[1024];
	const int	pad = BIGCHAR_HEIGHT * 1.75;

	if ( !cg.scrollTextTime ) 
	{
		return;
	}

	cgi_R_SetColor( textcolor_scroll );

	y = cg.printTextY - (cg.time - cg.scrollTextTime) * SCROLL_LPM;

	// See if text has finished scrolling off screen
	if ((y + cg.scrollTextLines * pad) < 1)
	{
		cg.scrollTextTime = 0;
		return;
	}

	for (i=0;i<cg.scrollTextLines;++i)
	{

		// Is this line off top of screen?
		if ((y + ((i +1) * pad)) < 1)
		{
			y += pad;
			continue;
		}
		// or past bottom of screen?
		else if (y > SCREEN_HEIGHT)
		{
			break;
		}

		start = cg.printText[i];

		while ( 1 ) 
		{

			for ( l = 0; l < 80; l++ ) 
			{
				if ( !start[l] || start[l] == '\n' ) 
				{
					break;
				}
				linebuffer[l] = start[l];
			}
			linebuffer[l] = 0;

			CG_DrawProportionalString(120, y, linebuffer, CG_BIGFONT | CG_DROPSHADOW, textcolor_scroll );
//			CG_DrawProportionalString((SCREEN_WIDTH / 2), y, linebuffer, CG_CENTER | CG_BIGFONT, textcolor_scroll );

			y += pad;

			while ( *start && ( *start != '\n' ) ) 
			{
				start++;
			}

			if ( !*start ) 
			{
				break;
			}
			start++;
		}
	}

	cgi_R_SetColor( NULL );

}


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;
	int		textI;

	// Find text to match the str given
	if (*str == '@')
	{
		textI = CG_SearchTextPrecache((char *) str);
		if (textI >= 0)
		{
			Q_strncpyz( cg.centerPrint, precacheText[textI].text, sizeof(cg.centerPrint) );
		}
		else
		{
			Com_Printf ("CG_CenterPrint: bad key for precached Text '%s'\n",str);	
		}
	}
	else
	{
		Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );
	}

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}

}


/*
===================
CG_DrawCenterString
===================
*/
void CG_DrawCenterString( void ) 
{
	char	*start;
	int		l;
	int		x, y, w;
	float	*color;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	if((textcolor_center[0] == 0) && (textcolor_center[1] == 0) && 
		(textcolor_center[2] == 0) && (textcolor_center[3] == 0))
	{
		Vector4Copy( colorTable[CT_WHITE], textcolor_center );
	}

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 40; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( SCREEN_WIDTH - w ) / 2;

		CG_DrawStringExt( x, y, linebuffer, textcolor_center, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5) );

		y += cg.centerPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

}


/*
===================
CG_SearchTextPrecache
===================
*/
int CG_SearchTextPrecache(char *key)
{
	int i;

	// Find text to match the str given
	if (*key == '@')
	{
		for (i=0;i<precacheText_i;++i)
		{
			if ( _stricmp( key, precacheText[i].key ) == 0 )
			{
				return(i);
			}
		}
	}

	return(-1);
}


/*
===================
CG_SearchWavPrecache
===================
*/
int CG_SearchWavPrecache(char *key)
{
	int i;

	// Find text to match the str given
	if (*key == '@')
	{
		for (i=0;i<precacheText_i;++i)
		{
			if ( _stricmp( key, precacheWav[i].textKey ) == 0 )
			{
				return(i);
			}
		}
	}

	return(-1);
}


/*
===============================================================================

LCARS TEXT

  LCARS Text is nothing fancy.  It just displays a couple lines in the center of the screen
  with an LCARS frame around it. 
===============================================================================
*/
#define	LCARS_X_START	87
#define	LCARS_X_END		610

void CG_LCARSText( const char *str, int y, int charWidth ) 
{
	int text_i,i,len;
	char *s,*holds;
	int w,numChars;
	int	holdTime;

	text_i = CG_SearchTextPrecache((char *) str);
	//ensure we found a match
	if (text_i == -1) 
	{	
		Com_Printf("WARNING: CG_LCARSText given invalid text key :'%s'",str);
		return; 
	}

	// count the number of lines
	cg.LCARSTextLines = 1;

	memset (cg.LCARSText, 0, sizeof(cg.LCARSText));

	// Break into individual lines
	i = 0;
	len = 0;
	s = precacheText[text_i].text;
	holds = s;

	//We start at column 75 according to DrawGameText
	w = LCARS_X_END - LCARS_X_START;
	numChars = floor(w/SMALLCHAR_WIDTH);

	while( *s ) 
	{
		len++;
		if (*s == '\n')
		{//Being told explicitly to start a new line
			Q_strncpyz( cg.LCARSText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.LCARSTextLines++;
		}
		else if ( len == numChars )
		{//Reached max length of this line
			//step back until we find a space
			while( len && *s != ' ' )
			{
				s--;
				len--;
			}
			//break the line here
			Q_strncpyz( cg.LCARSText[i], holds, len);
			i++;
			len = 0;
			holds = s;
			holds++;
			cg.LCARSTextLines++;
		}
		s++;
	}

	len++;  // So the NULL will be properly placed at the end of the string of Q_strncpyz
	Q_strncpyz( cg.LCARSText[i], holds, len); // To get the last line

	// Time it takes to view it
	holdTime = strlen(precacheText[text_i].text);

	holdTime = (holdTime * 50);
	if (holdTime < 5000)
	{
		holdTime = 5000;
	}

	cg.LCARSTextTime = cg.time + holdTime;

}


void CG_DrawLCARSText(void)
{
	char	*start;
	int		i;//,length;
	int		y,pad;
//	vec4_t newColor; 

	if ( cg.LCARSTextTime < cg.time ) 
	{
		return;
	}

//	Vector4Copy( colorTable[CT_BLACK], newColor );
//	newColor[3] = 0.5;

//	y = LCARS_Y_START - 42;	
	pad = SMALLCHAR_HEIGHT * 1.15;

	// Big Background
//	length =  (y + ((cg.LCARSTextLines + 1) * pad) + 15) - (LCARS_Y_START - 42); 
//	cgi_R_SetColor(newColor);
//	CG_DrawPic( 77, y,   534, (length + 27), cgs.media.whiteShader);	// Background

	// Left side column background
//	cgi_R_SetColor(newColor);
//	CG_DrawPic( 30, y + 10, 47, (length + 27), cgs.media.whiteShader);	// Background


	// Frame
//	cgi_R_SetColor( colorTable[CT_DKPURPLE3]);
//	CG_DrawPic( 30,  y,  64,   32, cgs.media.objcorner1);	// Top Corner

//	CG_DrawPic( 50,  y,  502,  22, cgs.media.whiteShader);	// Top line

//	CG_DrawProportionalString(611, y, ingame_text[IGT_LCARS], CG_RIGHT | CG_BIGFONT, colorTable[CT_LTBLUE1] );

	y = ((SCREEN_HEIGHT /2) - ((cg.LCARSTextLines * pad) /2)) - (3 * pad);
	for (i=	0;i< cg.LCARSTextLines;++i)
	{
		start = cg.LCARSText[i];
		CG_DrawProportionalString((SCREEN_WIDTH/2), y, cg.LCARSText[i], CG_PULSE | CG_SMALLFONT | CG_CENTER, colorTable[CT_LTGOLD1] );

		y += pad;
	}

//	cgi_R_SetColor( colorTable[CT_DKPURPLE3]);

//	length = (y + 15) - (LCARS_Y_START - 9);
//	CG_DrawPic( 30,  LCARS_Y_START - 14,  47, length, cgs.media.whiteShader);	// Left side bar
//	CG_DrawPic( 30,  y + 13,  64,   32, cgs.media.objcorner3);	// Bottom Corner
//	CG_DrawPic( 50,  y + 18 ,  561,  22, cgs.media.whiteShader);	// Bottomline

}
