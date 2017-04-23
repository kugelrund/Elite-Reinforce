#include "cg_local.h"
#include "FX_Public.h"

vec3_t	WHITE = {1.0, 1.0, 1.0};
int		numFX	= 0;

void FX_PrintDebugInfo( void );

FX_state_t	FX_renderList[ MAX_EFFECTS ];
FX_state_t	*FX_nextValid = &FX_renderList[0];

#ifdef _DEBUG

#define HOP_RESOLUTION		32
#define ACTIVE_RESOLUTION	32	//Frames

int	hopTable[ HOP_RESOLUTION ];
int curHop = 0;
int	hopAverage = 0;

int	activeTable[ ACTIVE_RESOLUTION ];
int curActive = 0;
int	activeAverage = 0;

#endif

/*
===============
FX_Init

Allocates all FX
===============
*/

int	FX_Init( void )
{
	FX_Free();		//Make sure the system is cleaned out
	FXMEM_Init();	//Flush out the memory manager

	FX_nextValid = &FX_renderList[0];

	return true;
}

/*
-------------------------
FX_FreeMember
-------------------------
*/

void FX_FreeMember( FX_state_t *state )
{
	numFX--;

	delete state->effect;
	state->effect = NULL;
}

/*
===============
FX_Free

Frees all FX
===============
*/

int FX_Free( void )
{
	for ( int i = 0; i < MAX_EFFECTS; i++ )
	{
		if ( FX_renderList[ i ].effect == NULL )
			continue;

		FX_FreeMember( &FX_renderList[ i ] );
	}

	//Clear the FX memory
	FXMEM_Flush();

	return true;
}

/*
===============
FX_Add

Adds all client effects (within the FX wrapper) to the view
===============
*/

void FX_Add( void )
{
	FX_state_t	*state;

#ifdef _DEBUG

	FXMEMD_searchPeak		= 0;
	FXMEMD_allocListPeak	= 0;
	FXMEMD_freeListPeak		= 0;
	FXMEMD_allocatePeak		= 0;

#endif

	for ( int i = 0; i < MAX_EFFECTS; i++ )
	{
		if ( FX_renderList[ i ].effect == NULL )
			continue;

		state = &FX_renderList[ i ];
		
		if ( cg_freezeFX.integer )
			state->killTime += cg.frametime;	//Keep the effects alive

		//Clean up old events
		if ( ( ( state->killTime < cg.time ) && ( state->killTime != -1 ) ) )
		{
			//Free out the effect primitive
			FX_FreeMember( state );
			continue;
		}

		//Update the effect
		if ( !cg_freezeFX.integer && ( cg.frametime > 0 ) )	//not paused
		{
			//If the function returns false, this effect has been removed
			if ( state->effect->Update() == qfalse )
			{
				FX_FreeMember( state );
				continue;
			}
		}
	
		//Cull the effect
		if ( state->effect->Cull() == qfalse )
		{
			//Draw the effect
			state->effect->Draw();
		}
	}

#ifdef _DEBUG

	curActive++;

	if ( curActive > ( ACTIVE_RESOLUTION - 1 ) )
	{
		curActive = 0;
		activeAverage = 0;

		for ( int i = 0; i < ACTIVE_RESOLUTION; i++ )
			activeAverage += activeTable[ i ];

		activeAverage /= ACTIVE_RESOLUTION;

		for ( int i = 0; i < ACTIVE_RESOLUTION; i++ )
			activeTable[ i ] = 0;
	}

	activeTable[ curActive ] = numFX;

#endif

	//Print any debugging information, if requested
	if ( cg_debugFX.value )
		FX_PrintDebugInfo();
}

/*
-------------------------
FX_FindNextValid
-------------------------
*/

FX_state_t *FX_FindNextValid( void )
{

#ifdef _DEBUG

	//Compute the new hop average if applicable

	curHop++;

	if ( curHop > ( HOP_RESOLUTION - 1 ) )
	{
		curHop = 0;
		hopAverage = 0;

		for ( int i = 0; i < HOP_RESOLUTION; i++ )
			hopAverage += hopTable[ i ];

		hopAverage /= HOP_RESOLUTION;

		for ( int i = 0; i < HOP_RESOLUTION; i++ )
			hopTable[ i ] = 0;
	}

#endif

	for ( int i = 0; i < MAX_EFFECTS; i++ )
	{
		FX_nextValid = ( ( FX_nextValid + 1 ) > &FX_renderList[ MAX_EFFECTS - 1 ] ) ? &FX_renderList[ 0 ] : FX_nextValid + 1;

#ifdef _DEBUG

		hopTable[ curHop ]++;	//Increment the number of hops made

#endif

		if ( FX_nextValid->effect == NULL )
			return FX_nextValid;
	}

	return NULL;
}

/*
-------------------------
FX_AddPrimitive
-------------------------
*/

void FX_AddPrimitive( FXPrimitive *primitive, int killTime )
{
	//assert( cg.frametime >= 0 );

	if (cg.frametime <= 0)	//skip out if paused
	{
		delete primitive;
		return;
	}
	
	if ( ( FX_nextValid == NULL ) || ( FX_nextValid->effect ) )
	{
		//Com_Printf("MAX EFFECTS REACHED!\n");
		FX_nextValid = &FX_renderList[ 0 ];	//FIXME: Blah
		FX_FreeMember( FX_nextValid );
	}

	//If this effect is free, take it over
	if ( FX_nextValid->effect == NULL )
	{
		// Stash these in the primitive so it has easy access to the vals
		primitive->m_start_time = cg.time;
		primitive->m_end_time	= killTime;

		FX_nextValid->effect = primitive;
		FX_nextValid->killTime = killTime;

		FX_nextValid = FX_FindNextValid();

		numFX++;
	}
}
 
/*
===============
FX_AddSprite

Adds a sprite to the FX wrapper render list
===============
*/

inline FXSprite *FX_AddSprite( vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float elasticity, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddSprite( origin, velocity, acceleration, scale, dscale, 
							startalpha, endalpha, WHITE, WHITE, 
							roll, elasticity, killTime, shader, flags );
}

/*
===============
FX_AddSprite

Adds a sprite to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXSprite *FX_AddSprite( vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float elasticity, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXSprite	*sprite;

	sprite	= new FXSprite;

	sprite->SetOrigin( origin );
	sprite->SetVelocity( velocity );
	sprite->SetAcceleration( acceleration );
	sprite->SetShader( shader );
	sprite->SetScale( scale );
	sprite->SetStartAlpha( startalpha );
	sprite->SetElasticity( elasticity );

	sprite->SetScaleDelta( dscale );
	sprite->SetEndAlpha( endalpha );

	sprite->SetStartRGB( startRGB );
	sprite->SetEndRGB( endRGB );

	sprite->SetRoll( 0 );	//FIXME: Pass this in
	sprite->SetRollDelta( roll );

	sprite->SetFlags( flags );

	FX_AddPrimitive( sprite, killTime + cg.time );

	return sprite;
}

/*
===============
FX_AddElectricity

Adds a electricity bolt to the FX wrapper render list
===============
*/

inline FXElectricity *FX_AddElectricity( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, 
											float startalpha, float endalpha, float killTime, qhandle_t shader, 
											int flags, float deviation )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXElectricity *electricity;

	electricity = new FXElectricity;

	electricity->SetOrigin( origin );
	electricity->SetOrigin2( origin2 );
	electricity->SetShader( shader );
	electricity->SetScale( scale );
	electricity->SetStartAlpha( startalpha );

	electricity->SetStartRGB( WHITE );
	electricity->SetEndRGB( WHITE );

	electricity->SetScaleDelta( dscale );
	electricity->SetEndAlpha( endalpha );
	electricity->SetFlags( flags );

	electricity->SetSTScale( stScale );
	electricity->SetDeviation( deviation );

	electricity->Build();

	FX_AddPrimitive( electricity, killTime + cg.time );

	return electricity;
}

/*
===============
FX_AddElectricity

Adds a electricity bolt to the FX wrapper render list
overloaded to pass default deviation
===============
*/

inline FXElectricity *FX_AddElectricity( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, 
											float startalpha, float endalpha, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddElectricity( origin, origin2, stScale, scale, dscale, startalpha, endalpha, killTime, shader, flags, DEFAULT_DEVIATION );
}

/*
===============
FX_AddParticle

Adds a particle to the FX wrapper render list
===============
*/

inline FXParticle *FX_AddParticle( centity_t *owner, vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float elasticity, float killTime, qhandle_t shader, int flags, bool (*think)(FXPrimitive *, centity_t *) )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddParticle( owner, origin, velocity, acceleration, scale, dscale, 
									startalpha, endalpha, WHITE, WHITE, 
									roll, elasticity, killTime, shader, flags, think );
}

/*
===============
FX_AddParticle

Adds a particle to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXParticle *FX_AddParticle( centity_t *owner, vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float elasticity, float killTime, qhandle_t shader, int flags, bool (*think)(FXPrimitive *, centity_t *) )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXParticle	*particle;

	particle = new FXParticle;

	particle->SetOrigin( origin );
	particle->SetVelocity( velocity );
	particle->SetAcceleration( acceleration );
	particle->SetShader( shader );
	particle->SetScale( scale );
	particle->SetStartAlpha( startalpha );
	particle->SetElasticity( elasticity );

	particle->SetScaleDelta( dscale );
	particle->SetEndAlpha( endalpha );

	particle->SetStartRGB( startRGB );
	particle->SetEndRGB( endRGB );

	particle->SetRoll( 0 );	//FIXME: Pass this in
	particle->SetRollDelta( roll );

	particle->SetFlags( flags );
	particle->SetThink( think );
	particle->SetCOwner( owner );

	FX_AddPrimitive( particle, killTime + cg.time );

	return particle;
}

/*
===============
FX_AddTri

Adds a triangle to the FX wrapper render list
===============
*/

inline FXTri *FX_AddTri( vec3_t origin, vec3_t origin2, vec3_t origin3, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddTri( origin, origin2, origin3, startalpha, endalpha, WHITE, WHITE, killTime, shader, flags );
}

/*
===============
FX_AddTri

Adds a triangle to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXTri *FX_AddTri( vec3_t origin, vec3_t origin2, vec3_t origin3, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader,  int flags)
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXTri	*tri;

	tri	= new FXTri;

	tri->SetOrigin( origin );
	tri->SetOrigin2( origin2 );
	tri->SetOrigin3( origin3 );

	tri->SetShader( shader );

	tri->SetStartAlpha( startalpha );
	tri->SetEndAlpha( endalpha );

	tri->SetStartRGB( startRGB );
	tri->SetEndRGB( endRGB );

	tri->SetFlags( flags );

	FX_AddPrimitive( tri, killTime + cg.time );

	return tri;
}

/*
===============
FX_AddQuad

Adds a quad to the FX wrapper render list
===============
*/

inline FXQuad *FX_AddQuad( vec3_t origin, vec3_t normal, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float droll, float elasticity, float killTime, qhandle_t shader )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddQuad( origin, normal, velocity, acceleration, 
						scale, dscale, startalpha, endalpha, WHITE, WHITE, 
						roll, droll, elasticity, killTime, shader );
}

/*
===============
FX_AddQuad

Adds a quad to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXQuad *FX_AddQuad( vec3_t origin, vec3_t normal, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float droll, float elasticity, float killTime, qhandle_t shader )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXQuad	*quad;

	quad	= new FXQuad;

	quad->SetOrigin( origin );
	quad->SetNormal( normal );
	quad->SetVelocity( velocity );
	quad->SetAcceleration( acceleration );
	quad->SetShader( shader );
	quad->SetScale( scale );
	quad->SetStartAlpha( startalpha );

	quad->SetStartRGB( startRGB );
	quad->SetEndRGB( endRGB );

	quad->SetElasticity( elasticity );

	quad->SetScaleDelta( dscale * 4 );
	quad->SetEndAlpha( endalpha );

	quad->SetRoll( roll );
	quad->SetRollDelta( droll );

	FX_AddPrimitive( quad, killTime + cg.time );

	return quad;
}

/*
===============
FX_AddLine

Adds a line to the FX wrapper render list
===============
*/

inline FXLine *FX_AddLine( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddLine( origin, origin2, stScale, scale, dscale, 
						startalpha, endalpha, WHITE, WHITE, 
						killTime, shader, flags );
}

/*
===============
FX_AddLine

Adds a line to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXLine *FX_AddLine( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXLine	*line;

	// Check to see if we need to generate a new ST scale based on the line length...
	//	this will cure the problem of textures on lines getting stretched out ( dreadnought gun ).
	if ( flags & FXF_ADJUST_ST )
	{
		vec3_t	dis;
		float	len;

		VectorSubtract( origin, origin2, dis );
		len = VectorNormalize( dis );

		stScale = len / stScale;
	}

	line	= new FXLine;

	line->SetOrigin( origin );
	line->SetOrigin2( origin2 );
	line->SetShader( shader );
	line->SetScale( scale );
	line->SetStartAlpha( startalpha );

	line->SetStartRGB( startRGB );
	line->SetEndRGB( endRGB );

	line->SetScaleDelta( dscale );
	line->SetEndAlpha( endalpha );

	line->SetFlags( flags );
	line->SetSTScale( stScale );
	line->SetEndcapShader( 0 );

	FX_AddPrimitive( line, killTime + cg.time );

	return line;
}

/*
===============
FX_AddLine2

Adds a line to the FX wrapper render list
Overloaded for RGB
===============
*/

inline FXLine2 *FX_AddLine2( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float scale2, float dscale2, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXLine2	*line;

	line	= new FXLine2;

	line->SetOrigin( origin );
	line->SetOrigin2( origin2 );
	line->SetShader( shader );
	line->SetScale( scale );
	line->SetScale2( scale2 );
	line->SetStartAlpha( startalpha );

	line->SetStartRGB( WHITE );
	line->SetEndRGB( WHITE );

	line->SetScaleDelta( dscale );
	line->SetScale2Delta( dscale2 );
	line->SetEndAlpha( endalpha );

	line->SetFlags( flags );
	line->SetSTScale( stScale );
	line->SetEndcapShader( 0 );

	FX_AddPrimitive( line, killTime + cg.time );

	return line;
}

/*
===============
FX_AddSpawner
===============
*/

inline FXSpawner *FX_AddSpawner( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user, int delay, float variance, float killTime, void *think, localEntity_t *owner, int radius )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddSpawner( origin, angles, velocity, user, delay, variance, killTime, 0, think, owner, radius );
}

/*
===============
FX_AddSpawner
===============
*/

inline FXSpawner *FX_AddSpawner( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user, int delay, float variance, float killTime, int flags, void *think, localEntity_t *owner, int radius )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXSpawner	*spawner;

	spawner = new FXSpawner;

	spawner->SetOrigin( origin );
	spawner->SetAngles( angles );
	spawner->SetVelocity( velocity );
	spawner->SetStartRGB( user );

	spawner->SetDelay( delay );
	spawner->SetVariance( variance );

	spawner->SetFlags( flags );
	spawner->SetRadius( radius );

	spawner->SetOwner( owner );

	spawner->Think = (void (__cdecl *)(vec3_t, vec3_t, vec3_t, vec3_t)) think;
	
	if ( flags & FXF_DELAY_SPAWN )
	{
		spawner->m_nextThink = cg.time + ( ( delay + ( variance * crandom() )) );
	}
	else
	{
		spawner->m_nextThink = cg.time;
	}

	if ( killTime > 0 )
		FX_AddPrimitive( spawner, killTime + cg.time );
	else
		FX_AddPrimitive( spawner, killTime );

	return spawner;
}

/*
===============
FX_AddBezier

Adds a bezier curve
===============
*/

inline FXBezier *FX_AddBezier( vec3_t origin, vec3_t origin2, vec3_t control1, vec3_t control2, vec3_t control1_vel, vec3_t control2_vel, vec3_t control1_accel, vec3_t control2_accel, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddBezier( origin, origin2, control1, control2, 
							control1_vel, control2_vel, control1_accel, control2_accel, 
							scale, dscale, startalpha, endalpha, WHITE, WHITE, 
							killTime, shader, flags );
}

/*
===============
FX_AddBezier

Adds a bezier curve
Overloaded for RGB
===============
*/

inline FXBezier *FX_AddBezier( vec3_t origin, vec3_t origin2, vec3_t control1, vec3_t control2, vec3_t control1_vel, vec3_t control2_vel, vec3_t control1_accel, vec3_t control2_accel, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, int flags )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXBezier	*bezier;

	bezier	= new FXBezier;

	bezier->SetOrigin( origin );
	bezier->SetOrigin2( origin2 );
	bezier->SetShader( shader );
	bezier->SetScale( scale );
	bezier->SetStartAlpha( startalpha );

	bezier->SetStartRGB( startRGB );
	bezier->SetEndRGB( endRGB );

	bezier->SetScaleDelta( dscale );
	bezier->SetEndAlpha( endalpha );

	bezier->SetControl1( control1 );
	bezier->SetControl2( control2 );

	bezier->SetControl1_Velocity( control1_vel );
	bezier->SetControl2_Velocity( control2_vel );

	bezier->SetControl1_Acceleration( control1_accel );
	bezier->SetControl2_Acceleration( control2_accel );
	
	bezier->SetFlags( flags );

	FX_AddPrimitive( bezier, killTime + cg.time );

	return bezier;
}

/*
===============
FX_AddTrail

Adds a trail to the FX wrapper render list
===============
*/

inline FXTrail *FX_AddTrail( vec3_t origin, vec3_t velocity, vec3_t acceleration, float length, float dlength, float scale, float dscale, float startalpha, float endalpha, float elasticity, float killTime, qhandle_t shader, int flags)
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddTrail( origin, velocity, acceleration, length, dlength, scale, dscale, 
						startalpha, endalpha, WHITE, WHITE, elasticity, 
						killTime, shader, flags);
}

/*
===============
FX_AddTrail

Adds a trail to the FX wrapper render list
overloaded for RGB
===============
*/

inline FXTrail *FX_AddTrail( vec3_t origin, vec3_t velocity, vec3_t acceleration, float length, float dlength, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float elasticity, float killTime, qhandle_t shader, int flags)
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXTrail	*trail;

	trail	= new FXTrail;

	trail->SetLength( length );
	trail->SetLengthDelta( dlength );

	trail->SetOrigin( origin );
	trail->SetOldOrigin( origin );
	trail->SetVelocity( velocity );
	trail->SetAcceleration( acceleration );
	trail->SetShader( shader );
	trail->SetScale( scale );
	trail->SetStartAlpha( startalpha );
	trail->SetElasticity( elasticity );

	trail->SetStartRGB( startRGB );
	trail->SetEndRGB( endRGB );

	trail->SetScaleDelta( dscale );
	trail->SetEndAlpha( endalpha );

	trail->SetFlags( flags );

	FX_AddPrimitive( trail, killTime + cg.time );

	return trail;
}

/*
===============
FX_AddCylinder

Adds a cylinder to the FX wrapper render list
===============
*/

//NOTENOTE: The reigning king of parameters!

inline FXCylinder *FX_AddCylinder(	vec3_t start, 
									vec3_t normal,
									float height,
									float dheight,
									float scale,
									float dscale,
									float scale2,
									float dscale2,
									float startalpha, 
									float endalpha, 
									float killTime, 
									qhandle_t shader,
									float bias )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	return FX_AddCylinder( start, normal, height, dheight, scale, dscale, scale2, dscale2, 
							startalpha, endalpha, WHITE, WHITE, killTime, shader, bias );
}

/*
===============
FX_AddCylinder

Adds a cylinder to the FX wrapper render list
Overloaded for RGB
===============
*/

//NOTENOTE: The reigning king of parameters!
#define DEFAULT_ST_SCALE	1.0f

inline FXCylinder *FX_AddCylinder(	vec3_t start, 
									vec3_t normal,
									float height,
									float dheight,
									float scale,
									float dscale,
									float scale2,
									float dscale2,
									float startalpha, 
									float endalpha, 
									vec3_t startRGB,
									vec3_t endRGB,
									float killTime, 
									qhandle_t shader,
									float bias )
{
	if (cg.frametime <= 0)	//skip out if paused
		return NULL;

	FXCylinder	*cylinder;

	cylinder = new FXCylinder;

	cylinder->SetStart( start );
	cylinder->SetNormal( normal );

	cylinder->SetHeight( height );
	cylinder->SetHeightDelta( dheight );
	
	cylinder->SetScale( scale );
	cylinder->SetScaleDelta( dscale );
	
	cylinder->SetScale2( scale2 );
	cylinder->SetScaleDelta2( dscale2 );

	cylinder->SetStartAlpha( startalpha );
	cylinder->SetEndAlpha( endalpha );

	cylinder->SetStartRGB( startRGB );
	cylinder->SetEndRGB( endRGB );

	cylinder->SetShader( shader );

	cylinder->SetBias( bias );
	cylinder->SetSTScale( DEFAULT_ST_SCALE );

	if ( killTime > 0 )
		FX_AddPrimitive( cylinder, killTime + cg.time );
	else
		FX_AddPrimitive( cylinder, killTime );

	return cylinder;
}

/*
===============
FX_DetailLevel

Returns a value for the relative detail level an effect has
===============
*/

float FX_DetailLevel( vec3_t origin, float near_clip, float far_clip )
{
	vec3_t	dir;
	float	len;

	//TODO: Account for the FOV range

	VectorSubtract( origin, cg.refdef.vieworg, dir );
	len = VectorNormalize( dir );

	if ( len < near_clip )
		return 0.0f;

	if ( len > far_clip )
		return 0.0f;

	return (float) len / far_clip;
}

/*
-------------------------
FX_PrintDebugInfo
-------------------------
*/

void FX_PrintDebugInfo( void )
{
	Com_Printf( "Number of Effects Active: %d\n", numFX );

#ifdef _DEBUG

	Com_Printf( "Average Hops: %d\n", hopAverage );
	Com_Printf( "Average Active Effects: %d\n", activeAverage );

#endif

	if ( fx_memoryInfo.value )
		FXMEM_PrintDebugInfo();
}
