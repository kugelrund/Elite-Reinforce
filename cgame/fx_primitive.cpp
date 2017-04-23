//
//	Client Effects Primitive Class

//	jweier

#include "cg_local.h"
#include "FX_Public.h"

/*
-------------------------
FXPrimitive

Base class for all primitives
-------------------------
*/

FXPrimitive::FXPrimitive( void )
{
	VectorClear( m_origin );
	VectorClear( m_velocity );
	VectorClear( m_acceleration );
	m_flags = 0;
}

FXPrimitive::~FXPrimitive( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXPrimitive::Cull( void )
{
	vec3_t	dir;
	float	len;

	//Get the direction to the view
	VectorSubtract( m_origin, cg.refdef.vieworg, dir );

	//Check if it's behind the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) < 0 )
		return true;

	len = VectorLengthSquared( dir );

	//Can't be too close
	if ( len < 16 * 16 )
		return true;

	return false;
}

//	Movestate utility functions

/*
-------------------------
UpdateOrigin
-------------------------
*/

void FXPrimitive::UpdateOrigin( void )
{
	vec3_t	new_origin;//, clear = { 0, 0, 0 };
	float	ftime, time2;
	int		i;

	//If the velocity is dead, kill the upward acceleration (Huh?  I don't recall why this is here...)
//	if ( VectorCompare( m_velocity, clear ) )
//		m_acceleration[2] = 0;

	//Moved
	UpdateVelocity();

	//Calc the time differences
	ftime = cg.frametime * 0.001f;
	time2 = ftime * ftime * 0.5f;

	//Predict the new position
	for ( i = 0 ; i < 3 ; i++ ) 
		new_origin[i] = m_origin[i] + ftime * m_velocity[i] + time2 * m_velocity[i];

	//Only perform physics if this object is tagged to do so
	if ( ( m_flags & FXF_BOUNCE ) )
	{
		trace_t	trace;
		float	dot;

		CG_Trace( &trace, m_origin, NULL, NULL, new_origin, -1, CONTENTS_SOLID );

		//Hit something
		if ( trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid )
		{
			//Check for a bounce flag
			if ( m_flags & FXF_BOUNCE )
			{
				VectorMA( m_velocity, ftime*trace.fraction, m_acceleration, m_velocity );
				dot = DotProduct( m_velocity, trace.plane.normal );
				VectorMA( m_velocity, -2*dot, trace.plane.normal, m_velocity );

				VectorScale( m_velocity, m_elasticity, m_velocity );
			}

			//If the velocity is too low, terminate it
			if ( trace.plane.normal[2] > 0 && m_velocity[2] < 40 )
				VectorClear( m_velocity );

			//Done
			return;
		}
	}

	//No physics were done to this object, move it
	VectorCopy( new_origin, m_origin );
}

/*
-------------------------
Draw
-------------------------
*/

void FXPrimitive::Draw( void )
{
}

/*
-------------------------
UpdateVelocity
-------------------------
*/

void FXPrimitive::UpdateVelocity( void )
{
	VectorMA( m_velocity, cg.frametime * 0.001f, m_acceleration, m_velocity );
}

/*
-------------------------
UpdateScale
-------------------------
*/

void FXPrimitive::UpdateScale( void )
{
	m_scale += md_scale * cg.frametime * 0.001f;

	if (m_scale < 0.0f)
		m_scale = 0.0f;
}

/*
-------------------------
UpdateAlpha
-------------------------
*/

#define FREQ_CONVERSION 0.00628f	// ( 2 * pi ) / 1000 ms
#define FADE_PERCENT	0.75f

void FXPrimitive::UpdateAlpha( void )
{
	float perc;

	// Get percentage of completeness
	perc = (float)(cg.time - m_start_time) / (float)(m_end_time - m_start_time);

	if ( ( m_flags & FXF_NON_LINEAR_FADE ) )
	{
		if ( perc > FADE_PERCENT )
		{
			// Kill this flag, then set the fade to start now...this will cause an RGB to RGB fade to act weird 
			//	because the start time is being changed...so, just don't do it!  
			m_flags ^= FXF_NON_LINEAR_FADE; 
			m_start_time = cg.time;
		}

		// Fade shouldn't begin yet
		perc = 0;
	}

	m_alpha = m_startalpha * (1.0 - perc) + m_endalpha * perc;

	if ( m_flags & FXF_PULSE_ALPHA )
		// Use half of alpha as amplitude so it will end smoothly
		m_alpha += ( sin( ( cg.time - m_start_time ) * FREQ_CONVERSION * m_alphafreq ) * m_alpha * 0.5 );

	if (m_alpha < 0.0f)
		m_alpha = 0.0f;

	if (m_alpha > 1.0f)
		m_alpha = 1.0f;

	if ( m_flags & FXF_ALPHA_NOISE )
		m_alpha = random() * m_alpha;
}

/*
-------------------------
UpdateRGB
-------------------------
*/

void FXPrimitive::UpdateRGB( void )
{
	float perc;

	// Get percentage of completeness
	perc = (float)(cg.time - m_start_time) / (float)(m_end_time - m_start_time);

	float	invPerc = 1.0f - perc;

	for (int i=0; i < 3; i++)
	{
		m_RGB[i] = ( m_startRGB[i] * invPerc + m_endRGB[i] * perc );

		// Has been explicitely flagged to use the alpha channel
		if ( !(m_flags & FXF_USE_ALPHA_CHAN) )
		{
			m_RGB[i] *= m_alpha;
		}

		if (m_RGB[i] < 0.0f)
			m_RGB[i] = 0.0f;

		if (m_RGB[i] > 1.0f)
			m_RGB[i] = 1.0f;
	}
}

/*
-------------------------
Update
-------------------------
*/

bool FXPrimitive::Update( void )
{
	//Move the object
	UpdateOrigin();

	UpdateScale();
	UpdateAlpha();
	UpdateRGB();

	return true;
}

/*
-------------------------
FXQuad

Oriented quad with texture
-------------------------
*/

FXQuad::FXQuad( void )
{
}

FXQuad::~FXQuad( void )
{
}

const	int NUM_QUADVERTS	= 4;

const	vec3_t	quad_template[] = 
{
	{	-1.0f, -1.0f, 0.0f	},
	{	-1.0f,  1.0f, 0.0f	},
	{	 1.0f,  1.0f, 0.0f	},
	{	 1.0f, -1.0f, 0.0f	}
};

const	float	quad_st_template[][2] = 
{
	{   0.0f,  0.0f	},
	{   0.0f,  1.0f	},
	{   1.0f,  1.0f	},
	{   1.0f,  0.0f	}
};

/*
-------------------------
UpdateRoll
-------------------------
*/

void FXQuad::UpdateRoll( void )
{
	m_roll += md_roll * cg.frametime * 0.001f;

	if (m_roll > 360)
		m_roll = m_roll - 360;

	if (m_roll < 0)
		m_roll = m_roll + 360;
}

/*
-------------------------
Update
-------------------------
*/

bool FXQuad::Update( void )
{
	UpdateRoll();
	UpdateScale();
	UpdateAlpha();
	UpdateRGB();

	return true;
}

/*
-------------------------
Draw
-------------------------
*/

void FXQuad::Draw( void )
{
	polyVert_t	verts[NUM_QUADVERTS];
	vec3_t		vr, vu;
	vec3_t		axis[3];
	float		scale;
	int			i;

	scale = m_scale * 0.5f;

	MakeNormalVectors( m_normal, vr, vu );
	VectorCopy( m_normal, axis[0] );
	VectorCopy( vr, axis[1] );
	VectorCopy( vu, axis[2] );

	RotateAroundDirection( axis, m_roll );

	//Construct the quad
	for ( i = 0; i < NUM_QUADVERTS; i++ )
	{
		VectorMA( m_origin,		quad_template[i][0] * ( scale ), axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, quad_template[i][1] * ( scale ), axis[2], verts[i].xyz );
		
		verts[i].modulate[0] = m_RGB[0] * 255;
		verts[i].modulate[1] = m_RGB[1] * 255;
		verts[i].modulate[2] = m_RGB[2] * 255;
		if ( m_flags & FXF_USE_ALPHA_CHAN )
			verts[i].modulate[3] = (byte)(m_alpha * 255);
		else
			verts[i].modulate[3] = 255;
		
		verts[i].st[0] = quad_st_template[i][0];
		verts[i].st[1] = quad_st_template[i][1];
	}

	cgi_R_AddPolyToScene( m_shader, NUM_QUADVERTS, verts );
}

/*
-------------------------
FXSprite

Non-oriented sprite with texture
-------------------------
*/

FXSprite::FXSprite( void )
{
}

FXSprite::~FXSprite( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXSprite::Cull( void )
{
	return false;
}

//Sprite vertex template
const	vec3_t sprite_template[4] =
{
	{ -1, -1,  0,	},	//Top left
	{ -1,  1,  0,	},	//Bottom left
	{  1,  1,  0,	},	//Bottom right
	{  1, -1,  0,	},	//Top right
};


//Sprite UV template
const	float sprite_texture_template[][2] = 
{
	{  0.0f,  0.0f	},	//Top left
	{  0.0f,  1.0f	},	//Bottom left
	{  1.0f,  1.0f	},	//Bottom right
	{  1.0f,  0.0f	},	//Top right
};

/*
-------------------------
UpdateRoll
-------------------------
*/

void FXSprite::UpdateRoll( void )
{
	m_roll += md_roll * cg.frametime * 0.001f;

	if (m_roll > 360)
		m_roll = m_roll - 360;

	if (m_roll < 0)
		m_roll = m_roll + 360;
}

/*
-------------------------
Update
-------------------------
*/

bool FXSprite::Update( void )
{
	if (m_start_time > cg.time)
	{//i was created in the future, right after un-pausing
		return false;
	}
	//Move the object
	UpdateOrigin();

	UpdateRoll();
	UpdateScale();
	UpdateAlpha();
	UpdateRGB();

	return true;
}

/*
-------------------------
Draw
-------------------------
*/

void FXSprite::Draw( void )
{
	polyVert_t	verts[4];
	vec3_t		axis[3];
	float		scale;
	int			i;
	
	scale = m_scale * 0.5f;
	
	for ( i = 0; i < 3; i++ )
		VectorCopy( cg.refdef.viewaxis[i], axis[i] );	

	//This is done to spare non-rolling sprites the odd angle snapping <?>
	if ( m_roll )
		RotateAroundDirection( axis, m_roll );
	
	for ( i = 0; i < 4; i++ )
	{	
		VectorMA( m_origin,		sprite_template[i][0] * scale,	axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, sprite_template[i][1] * scale,	axis[2], verts[i].xyz );

		//Setup the UVs
		verts[i].st[0] = -sprite_texture_template[i][0];
		verts[i].st[1] = -sprite_texture_template[i][1];

		//Setup the vertex modulation
		verts[i].modulate[0] = m_RGB[0] * 255;
		verts[i].modulate[1] = m_RGB[1] * 255;
		verts[i].modulate[2] = m_RGB[2] * 255;
		if ( m_flags & FXF_USE_ALPHA_CHAN )
			verts[i].modulate[3] = (byte)(m_alpha * 255);
		else
			verts[i].modulate[3] = 255;
	}

	//Add it into the renderer
	cgi_R_AddPolyToScene( m_shader, 4, verts );
}

/*
-------------------------
FXElectricity

Generates a bolt of electricity
-------------------------
*/

FXElectricity::FXElectricity( void )
{
	m_init = false;
}

FXElectricity::~FXElectricity( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXElectricity::Cull( void )
{
	vec3_t	dir;

	VectorSubtract( m_origin, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;	//dont' cull

	VectorSubtract( m_origin2, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;	//dont' cull

	return true;
}

/*
-------------------------
Draw
-------------------------
*/

void FXElectricity::Bolt( vec3_t start, vec3_t dir, float length, int num )
{
	vec3_t	ofs, curPos;
	float	stepSize;
	
	stepSize = (float)( length / (float) MAX_BOLT_SEGMENTS );

	VectorCopy( start, curPos );

	for ( int i = 0; i < MAX_BOLT_SEGMENTS; i++ )
	{
		//Move along the bolt
		ofs[PITCH]	= dir[PITCH]+ (crandom() * ( m_deviation * 0.25 ) );
		ofs[YAW]	= dir[YAW]	+ (crandom() * ( m_deviation * 0.25 ) );
		ofs[ROLL]	= dir[ROLL] + (crandom() * ( m_deviation * 0.75 ) );

		VectorMA( curPos, stepSize, ofs, curPos );

		//Save the current position
		VectorCopy( curPos, m_boltVerts[i] );
	}
}

/*
-------------------------
Build
-------------------------
*/

void FXElectricity::Build( void )
{
	vec3_t	dir, ofs, curPos;
	float	length, stepSize;
	
	VectorSubtract( m_origin2, m_origin, dir );
	length = VectorNormalize( dir );

	stepSize = (float)( length / (float) MAX_BOLT_SEGMENTS );

	VectorCopy( m_origin, curPos );

	for ( int i = 0; i < MAX_BOLT_SEGMENTS; i++ )
	{
		//Save the current position
		VectorCopy( curPos, m_boltVerts[i] );

		//Move along the bolt
		ofs[PITCH]	= dir[PITCH]+ (crandom() * m_deviation );
		ofs[YAW]	= dir[YAW]	+ (crandom() * m_deviation );
		ofs[ROLL]	= dir[ROLL] + (crandom() * ( m_deviation * 0.5 ) );

		VectorMA( curPos, stepSize, ofs, curPos );
	}

	VectorCopy( m_origin2, m_boltVerts[MAX_BOLT_SEGMENTS - 1] );
}

/*
-------------------------
DrawSegment
-------------------------
*/

inline void FXElectricity::DrawSegment( vec3_t start, vec3_t end, float scale, float tcStart, float tcEnd )
{
	vec3_t		lineDir, cross, viewDir;
	polyVert_t	verts[4];

	VectorSubtract( end, start, lineDir );
	VectorSubtract( end, cg.refdef.vieworg, viewDir );

	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale *= 0.5;

	//Construct the oriented quad
	
	if ( m_init )
		VectorCopy( m_lastEnd[0], verts[0].xyz );
	else
		VectorMA( start, -scale, cross, verts[0].xyz );

	verts[0].st[0] = 0.0f;
	verts[0].st[1] = tcStart;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	if ( m_init )
		VectorCopy( m_lastEnd[1], verts[1].xyz );
	else
		VectorMA( start, scale, cross, verts[1].xyz );

	verts[1].st[0] = 1.0f;
	verts[1].st[1] = tcStart;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( end, scale, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = tcEnd;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( end, -scale, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = tcEnd;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 4, verts );

	VectorCopy( verts[2].xyz, m_lastEnd[1] );
	VectorCopy( verts[3].xyz, m_lastEnd[0] );

	m_init = true;
}

/*
-------------------------
Draw
-------------------------
*/

void FXElectricity::Draw( void )
{
	float	scale, incr = 0, detail = 0, tcStart = 0 , tcEnd = m_stScale;

	//Check for tapering
	if ( m_flags & FXF_TAPER )
	{
		// This overrides the default of zero
		incr = m_scale / MAX_BOLT_SEGMENTS;
	}

	scale	= m_scale;
	m_init	= false;

	// Set up to calculate the texture coords so the texture can be mapped across the length of the bolt
	if ( m_flags & FXF_WRAP )
	{
		detail = m_stScale / MAX_BOLT_SEGMENTS;
	}

	//Do all segments
	for ( int i = 0; i < MAX_BOLT_SEGMENTS - 1 ; i++ )
	{
		if ( m_flags & FXF_WRAP )
		{
			// This will override the defaults if necessary
			tcStart = detail * i;
			tcEnd = detail * (i+ 1);
		}

		scale -= incr;
		
		DrawSegment( m_boltVerts[i], m_boltVerts[i+1], scale, tcStart, tcEnd );
	}
}

/*
-------------------------
FXParticle

Crazy particle stuff
-------------------------
*/
FXParticle::FXParticle( void )
{
	Think = NULL;
}


FXParticle::~FXParticle( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXParticle::Cull( void )
{
	return qfalse;
}

/*
-------------------------
Draw
-------------------------
*/

void FXParticle::Draw( void )
{
	polyVert_t	verts[4];
	vec3_t		axis[3];
	float		scale;
	int			i;

	if ( m_flags & FXF_NODRAW )
		return;

	scale = m_scale * 0.5f;
	
	for ( i = 0; i < 3; i++ )
		VectorCopy( cg.refdef.viewaxis[i], axis[i] );	

	//This is done to spare non-rolling particles the odd angle snapping <?>
	if (m_roll)
		RotateAroundDirection( axis, m_roll );
	
	for ( i = 0; i < 4; i++ )
	{	
		VectorMA( m_origin,		sprite_template[i][0] * scale, axis[1], verts[i].xyz );
		VectorMA( verts[i].xyz, sprite_template[i][1] * scale, axis[2], verts[i].xyz );

		//Setup the UVs
		verts[i].st[0] = sprite_texture_template[i][0];
		verts[i].st[1] = sprite_texture_template[i][1];

		//Setup the vertex modulation
		verts[i].modulate[0] = (byte)(m_RGB[0] * 255);
		verts[i].modulate[1] = (byte)(m_RGB[1] * 255);
		verts[i].modulate[2] = (byte)(m_RGB[2] * 255);
		if ( m_flags & FXF_USE_ALPHA_CHAN )
			verts[i].modulate[3] = (byte)(m_alpha * 255);
		else
			verts[i].modulate[3] = 255;
	}

	//Add it into the renderer
	cgi_R_AddPolyToScene( m_shader, 4, verts );
}

/*
-------------------------
UpdateRoll
-------------------------
*/

void FXParticle::UpdateRoll( void )
{
	m_roll += md_roll * cg.frametime * 0.001f;

	if (m_roll > 360)
		m_roll = m_roll - 360;

	if (m_roll < 0)
		m_roll = m_roll + 360;
}

/*
-------------------------
Update
-------------------------
*/

bool FXParticle::Update( void )
{
	// There should be a think function, otherwise what's the point of using a particle?
	if ( Think )
	{
		// All particle attributes need to be updated in the think function since
		//	nothing is auto-updated.  This is done to ensure the most flexibility.
		return Think( this, m_cowner );
	}

	return true;
}

/*
-------------------------
FXTri

Triangle primitive
-------------------------
*/

FXTri::FXTri( void )
{
}

FXTri::~FXTri( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXTri::Cull( void )
{
	vec3_t	dir;
	vec3_t	mid;

	//Find the midpoint of the triangle
	for ( int i = 0; i < 3; i++ )
	{
		mid[ i ] = ( m_origin[ i ] + m_origin2[ i ] + m_origin3[ i ] ) * 0.333f;
	}
	VectorSubtract( mid, cg.refdef.vieworg, dir );

	//Check if it's behind the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) < 0 )
		return true;

	return false;
}

/*
-------------------------
Draw
-------------------------
*/

void FXTri::Draw( void )
{
	polyVert_t	verts[3];

	//Construct the tri
	VectorCopy( m_origin, verts[0].xyz );
	verts[0].st[0] = 0.0f;
	verts[0].st[1] = 0.0f;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	VectorCopy( m_origin2, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorCopy( m_origin3, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = 1.0f;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] = 
		verts[1].modulate[3] = 
		verts[2].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] = 
		verts[1].modulate[3] = 
		verts[2].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 3, verts );
}


/*
-------------------------
FXLine

Oriented line
-------------------------
*/

FXLine::FXLine( void )
{
}

FXLine::~FXLine( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXLine::Cull( void )
{
	vec3_t	dir;

	VectorSubtract( m_origin, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;	//dont' cull

	VectorSubtract( m_origin2, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;
	
	return true;	//all points behind
}

/*
-------------------------
Draw
-------------------------
*/

void FXLine::Draw( void )
{
	vec3_t		lineDir, cross, viewDir, neworg;
	polyVert_t	verts[4];
	float		scale;

	VectorSubtract( m_origin2, m_origin, lineDir );
	VectorSubtract( m_origin2, cg.refdef.vieworg, viewDir );
	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale = m_scale * 0.5;

	//Construct the oriented quad
	VectorMA( m_origin, -scale, cross, verts[0].xyz );
	verts[0].st[0] = 0.0f;
	verts[0].st[1] = 0.0f;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin, scale, cross, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, scale, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = m_stScale;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, -scale, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = m_stScale;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 4, verts );

	//Cap the lines
	if ( m_flags & FXF_DRAWCAPS )
	{
		//Construct an oriented quad for the endcap
		VectorMA( m_origin, scale, cross, verts[0].xyz );
		verts[0].st[0] = 0.0f;
		verts[0].st[1] = 0.02f;
		verts[0].modulate[0] = m_RGB[0] * 255;
		verts[0].modulate[1] = m_RGB[1] * 255;
		verts[0].modulate[2] = m_RGB[2] * 255;

		VectorMA( m_origin, -scale, cross, verts[1].xyz );
		verts[1].st[0] = 1.0f;
		verts[1].st[1] = 0.1f;
		verts[1].modulate[0] = m_RGB[0] * 255;
		verts[1].modulate[1] = m_RGB[1] * 255;
		verts[1].modulate[2] = m_RGB[2] * 255;

		VectorNormalize( lineDir );
		VectorMA( m_origin, -scale * 2, lineDir, neworg );
		VectorMA( neworg, -scale, cross, verts[2].xyz );
		verts[2].st[0] = 1.0f;
		verts[2].st[1] = 0.9f;
		verts[2].modulate[0] = m_RGB[0] * 255;
		verts[2].modulate[1] = m_RGB[1] * 255;
		verts[2].modulate[2] = m_RGB[2] * 255;

		VectorMA( neworg, scale, cross, verts[3].xyz );
		verts[3].st[0] = 0.0f;
		verts[3].st[1] = 0.9f;
		verts[3].modulate[0] = m_RGB[0] * 255;
		verts[3].modulate[1] = m_RGB[1] * 255;
		verts[3].modulate[2] = m_RGB[2] * 255;

		if ( m_flags & FXF_USE_ALPHA_CHAN )
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = (byte)(m_alpha * 255);
		}
		else
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = 255;
		}

		cgi_R_AddPolyToScene( m_endcap_shader, 4, verts );

		//Construct an oriented quad for the endcap
		VectorMA( m_origin2, -scale, cross, verts[0].xyz );
		verts[0].st[0] = 0.0f;
		verts[0].st[1] = 0.1f;
		verts[0].modulate[0] = m_RGB[0] * 255;
		verts[0].modulate[1] = m_RGB[1] * 255;
		verts[0].modulate[2] = m_RGB[2] * 255;

		VectorMA( m_origin2, scale, cross, verts[1].xyz );
		verts[1].st[0] = 1.0f;
		verts[1].st[1] = 0.05f;
		verts[1].modulate[0] = m_RGB[0] * 255;
		verts[1].modulate[1] = m_RGB[1] * 255;
		verts[1].modulate[2] = m_RGB[2] * 255;

		VectorMA( m_origin2, scale * 2, lineDir, neworg );
		VectorMA( neworg, scale, cross, verts[2].xyz );
		verts[2].st[0] = 1.0f;
		verts[2].st[1] = 0.9f;
		verts[2].modulate[0] = m_RGB[0] * 255;
		verts[2].modulate[1] = m_RGB[1] * 255;
		verts[2].modulate[2] = m_RGB[2] * 255;

		VectorMA( neworg, -scale, cross, verts[3].xyz );
		verts[3].st[0] = 0.0f;
		verts[3].st[1] = 0.9f;
		verts[3].modulate[0] = m_RGB[0] * 255;
		verts[3].modulate[1] = m_RGB[1] * 255;
		verts[3].modulate[2] = m_RGB[2] * 255;

		if ( m_flags & FXF_USE_ALPHA_CHAN )
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = (byte)(m_alpha * 255);
		}
		else
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = 255;
		}

		cgi_R_AddPolyToScene( m_endcap_shader, 4, verts );
	}
}

/*
-------------------------
UpdateOrigin
-------------------------
*/

void FXLine::UpdateOrigin( void )
{
	if ( m_flags & FXF_SHRINK )
	{
		vec3_t	dir;
		float	perc, len;

		VectorSubtract( m_work_org, m_origin, dir );
		len = VectorNormalize( dir );

		// Get percentage of completeness
		perc = (float)(cg.time - m_start_time) / (float)(m_end_time - m_start_time);

		VectorMA( m_origin, len * ( 1.0 - perc ), dir, m_origin2 );
	}
}

/*
-------------------------
FXLine

Oriented line
-------------------------
*/

FXLine2::FXLine2( void )
{
}

FXLine2::~FXLine2( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXLine2::Cull( void )
{
	vec3_t	dir;

	VectorSubtract( m_origin, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;	//dont' cull

	VectorSubtract( m_origin2, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;
	
	return true;	//all points behind
}

/*
-------------------------
Draw
-------------------------
*/

void FXLine2::Draw( void )
{
#if 0
	vec3_t		lineDir, cross, viewDir;
	polyVert_t	verts[4];
	float		scale, scale2;

	VectorSubtract( m_origin2, m_origin, lineDir );
	VectorSubtract( m_origin2, cg.refdef.vieworg, viewDir );
	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale	= m_scale * 0.5f;
	scale2	= m_scale2 * 0.5f;

	//Construct the oriented quad
	VectorMA( m_origin, -scale, cross, verts[0].xyz );
	verts[0].st[0] = 0.0f;
	verts[0].st[1] = 0.0f;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin, scale, cross, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, scale2, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = m_stScale;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, -scale2, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = m_stScale;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 4, verts );
#else
	vec3_t		lineDir, cross, viewDir;
	polyVert_t	verts[4];
	float		scale, scale2;

	VectorSubtract( m_origin2, m_origin, lineDir );
	VectorSubtract( m_origin2, cg.refdef.vieworg, viewDir );
	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale	= m_scale * 0.5f;
	scale2	= m_scale2 * 0.5f;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	//Construct the tapered, oriented quad, we have to use 3 tris to do this though
	VectorCopy( m_origin, verts[0].xyz );
	verts[0].st[0] = 0.5f;
	verts[0].st[1] = 0.0f;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, -scale2, cross, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = m_stScale;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, scale2, cross, verts[2].xyz );
	verts[2].st[0] = 0.0f;
	verts[2].st[1] = m_stScale;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	cgi_R_AddPolyToScene( m_shader, 3, verts );

	VectorCopy( m_origin, verts[0].xyz );
	verts[0].st[0] = 0.5f;
	verts[0].st[1] = 0.0f;

	VectorMA( m_origin2, -scale2, cross, verts[1].xyz );
	verts[1].st[0] = 0.0f;
	verts[1].st[1] = m_stScale;

	VectorMA( m_origin, -scale, cross, verts[2].xyz );
	verts[2].st[0] = 0.0f;
	verts[2].st[1] = m_stScale;

	cgi_R_AddPolyToScene( m_shader, 3, verts );

	VectorCopy( m_origin, verts[0].xyz );
	verts[0].st[0] = 0.5f;
	verts[0].st[1] = 0.0f;

	VectorMA( m_origin2, scale2, cross, verts[1].xyz );
	verts[1].st[0] = 0.0f;
	verts[1].st[1] = m_stScale;

	VectorMA( m_origin, scale, cross, verts[2].xyz );
	verts[2].st[0] = 0.0f;
	verts[2].st[1] = m_stScale;

	cgi_R_AddPolyToScene( m_shader, 3, verts );
/*
	VectorMA( m_origin, scale, cross, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, scale2, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = m_stScale;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin2, -scale2, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = m_stScale;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	cgi_R_AddPolyToScene( m_shader, 4, verts );
	*/
#endif
}

/*
-------------------------
FXSpawner

Effect spawner
-------------------------
*/

FXSpawner::FXSpawner( void )
{
	VectorClear( m_acceleration );
	m_owner = NULL;
}

FXSpawner::~FXSpawner( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXSpawner::Cull( void )
{
	vec3_t	dir;

	VectorSubtract( m_origin, cg.refdef.vieworg, dir );
	//Check if it's behind the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) < 0 )
		return true;

	float len = VectorLengthSquared( dir );
	//Check to see if we're outside the valid radius
	if ( len > m_radius*m_radius )
		return true;

	return false;
}

/*
-------------------------
Draw
-------------------------
*/

void FXSpawner::Draw( void )
{
}

/*
-------------------------
Update
-------------------------
*/

bool FXSpawner::Update( void )
{
	UpdateOrigin();
	UpdateVelocity();
	UpdateScale();
	UpdateAlpha();
	
	//FIXME: This is being double called...
	if ( Cull() == false )
	{
		if ( m_nextThink < cg.time )
		{
			if (Think != NULL)
				Think( m_origin, m_angles, m_velocity, m_startRGB );

			m_nextThink = cg.time + ( ( m_delay + ( m_variance * crandom() )) );

			if ( m_flags & FXF_SPAWN_ONCE )
				return false;
		}
	}
	
	return true;
}

/*
-------------------------
UpdateOrigin
-------------------------
*/

void FXSpawner::UpdateOrigin( void )
{
	//If attached to an owner, move with it
	if ( m_owner )
	{
		VectorCopy( m_owner->refEntity.origin, m_origin );
	}
}

/*
-------------------------
FXBezier

Bezier curve line
-------------------------
*/

FXBezier::FXBezier( void )
{
	m_init = false;
}

FXBezier::~FXBezier( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXBezier::Cull( void )
{
	vec3_t	dir;

	VectorSubtract( m_origin, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;	//don't cull

	VectorSubtract( m_origin2, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;

	VectorSubtract( m_control1, cg.refdef.vieworg, dir );

	//Check if it's in front of the viewer
	if ( (DotProduct( cg.refdef.viewaxis[0], dir )) >= 0 )
		return false;

	return true; //all points behind viewer
}

/*
-------------------------
Update
-------------------------
*/

bool FXBezier::Update( void )
{
	vec3_t	new_origin;
	float	ftime, time2;

	VectorMA( m_control1_velocity, cg.frametime * 0.001f, m_control1_acceleration, m_control1_velocity );
	VectorMA( m_control2_velocity, cg.frametime * 0.001f, m_control2_acceleration, m_control2_velocity );

	ftime = cg.frametime * 0.001f;
	time2 = ftime * ftime * 0.5f;

	for ( int i = 0 ; i < 3 ; i++ )
	{
		new_origin[i] = m_control1[i] + ftime * m_control1_velocity[i] + time2 * m_control1_velocity[i];
	}

	VectorCopy( new_origin, m_control1 );

	for ( int i = 0 ; i < 3 ; i++ )
	{
		new_origin[i] = m_control2[i] + ftime * m_control2_velocity[i] + time2 * m_control2_velocity[i];
	}

	VectorCopy( new_origin, m_control2 );

	UpdateAlpha();
	UpdateScale();
	UpdateRGB();

	return true;
}

/*
-------------------------
DrawSegment
-------------------------
*/

inline void FXBezier::DrawSegment( vec3_t start, vec3_t end, float texcoord1, float texcoord2 )
{
	vec3_t		lineDir, cross, viewDir;
	polyVert_t	verts[4];
	float		scale;

	VectorSubtract( end, start, lineDir );
	VectorSubtract( end, cg.refdef.vieworg, viewDir );
	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale = m_scale * 0.5;

	//Construct the oriented quad
	if ( m_init )
		VectorCopy( m_lastEnd[0], verts[0].xyz );
	else
		VectorMA( start, -scale, cross, verts[0].xyz );

	verts[0].st[0] = 0.0f;
	verts[0].st[1] = texcoord1;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	if ( m_init )
		VectorCopy( m_lastEnd[1], verts[1].xyz );
	else
		VectorMA( start, scale, cross, verts[1].xyz );

	verts[1].st[0] = 1.0f;
	verts[1].st[1] = texcoord1;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( end, scale, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = texcoord2;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( end, -scale, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = texcoord2;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 4, verts );

	VectorCopy( verts[2].xyz, m_lastEnd[1] );
	VectorCopy( verts[3].xyz, m_lastEnd[0] );

	m_init = true;
}

const	float	BEZIER_RESOLUTION	= 16.0f;

/*
-------------------------
Draw
-------------------------
*/

void FXBezier::Draw( void )	
{
	vec3_t	pos, old_pos;
    double	mu, mum1;
	float	incr = 1.0f / BEZIER_RESOLUTION, tex = 1.0, tc1, tc2;
	int		i;

	VectorCopy( m_origin, old_pos );

	m_init = false;	//Signify a new batch for vert gluing

	// Calculate the texture coords so the texture can stretch along the whole bezier
	if ( m_flags & FXF_WRAP )
	{
		tex = m_stScale / 1.0f;
	}

	for ( mu = 0.0; mu <= 1.0; mu += incr )
	{
		//Four point curve
#if 1
		double mum13, mu3, group1, group2;

		mum1	= 1 - mu;
		mum13	= mum1 * mum1 * mum1;
		mu3		= mu * mu * mu;
		group1	= 3 * mu * mum1 * mum1;
		group2	= 3 * mu * mu *mum1;

		for (i=0;i<3;i++)
			pos[i] = mum13 * m_origin[i] + group1 * m_control1[i] + group2 * m_control2[i] + mu3 * m_origin2[i];

#else
		
		//Three point curve

		double	mum12, mu2;
		mu2 = mu * mu;
		mum1 = 1 - mu;
		mum12 = mum1 * mum1;
		
		for (i=0;i<3;i++)
			pos[i] = m_origin[i] * mum12 + 2 * m_control1[i] * mum1 * mu + m_origin2[i] * mu2;

#endif		

		if ( m_flags & FXF_WRAP ) 
		{
			tc1 = mu * tex;
			tc2 = ( mu + incr ) * tex;
		}
		else
		{
			// Texture will get mapped onto each segement
			tc1 = 0.0f;
			tc2 = 1.0f;
		}

		//Draw it
		DrawSegment( old_pos, pos, tc1, tc2 );
		VectorCopy( pos, old_pos );
	}
}

/*
-------------------------
FXTrail

Leaves a trail behind it
-------------------------
*/

FXTrail::FXTrail( void )
{
}

FXTrail::~FXTrail( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXTrail::Cull( void )
{
	return FXPrimitive::Cull();
}

/*
-------------------------
Draw
-------------------------
*/

void FXTrail::Draw( void )
{
	vec3_t		lineDir, cross, viewDir;
	polyVert_t	verts[4];
	float		scale;

	VectorSubtract( m_oldorigin, m_origin, lineDir );
	VectorSubtract( m_oldorigin, cg.refdef.vieworg, viewDir );
	CrossProduct( lineDir, viewDir, cross );
	VectorNormalize( cross );

	scale = m_scale * 0.5;

	//Construct the oriented quad
	VectorMA( m_origin, -scale, cross, verts[0].xyz );
	verts[0].st[0] = 0.0f;
	verts[0].st[1] = 0.0f;
	verts[0].modulate[0] = m_RGB[0] * 255;
	verts[0].modulate[1] = m_RGB[1] * 255;
	verts[0].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_origin, scale, cross, verts[1].xyz );
	verts[1].st[0] = 1.0f;
	verts[1].st[1] = 0.0f;
	verts[1].modulate[0] = m_RGB[0] * 255;
	verts[1].modulate[1] = m_RGB[1] * 255;
	verts[1].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_oldorigin, scale, cross, verts[2].xyz );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = 1.0f;
	verts[2].modulate[0] = m_RGB[0] * 255;
	verts[2].modulate[1] = m_RGB[1] * 255;
	verts[2].modulate[2] = m_RGB[2] * 255;

	VectorMA( m_oldorigin, -scale, cross, verts[3].xyz );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = 1.0f;
	verts[3].modulate[0] = m_RGB[0] * 255;
	verts[3].modulate[1] = m_RGB[1] * 255;
	verts[3].modulate[2] = m_RGB[2] * 255;

	if ( m_flags & FXF_USE_ALPHA_CHAN )
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = (byte)(m_alpha * 255);
	}
	else
	{
		verts[0].modulate[3] =
		verts[1].modulate[3] =
		verts[2].modulate[3] =
		verts[3].modulate[3] = 255;
	}

	cgi_R_AddPolyToScene( m_shader, 4, verts );
}

/*
-------------------------
UpdateTailLength
-------------------------
*/

void FXTrail::UpdateTailLength( void )
{
	m_length += md_length * cg.frametime * 0.001f;

	if (m_length < 0.0f)
		m_length = 1.0f;
}

/*
-------------------------
UpdateTailPoint
-------------------------
*/

void FXTrail::UpdateTailPoint( void )
{
	vec3_t	dir;
	float	len;

	UpdateTailLength();

	VectorSubtract( m_oldorigin, m_origin, dir );
	len = VectorNormalize( dir );

	VectorMA( m_origin, m_length, dir, m_oldorigin);
}

/*
-------------------------
Update
-------------------------
*/

bool FXTrail::Update( void )
{
	//Move the object
	UpdateOrigin();

	UpdateTailPoint();

	UpdateScale();
	UpdateAlpha();
	UpdateRGB();

	return true;
}

/*
-------------------------
FXCylinder

Cylinder
-------------------------
*/

const int	NUM_CYLINDER_SEGMENTS	= 32;

FXCylinder::FXCylinder( void )
{
}

FXCylinder::~FXCylinder( void )
{
}

/*
-------------------------
Cull
-------------------------
*/

bool FXCylinder::Cull( void )
{
	return FXPrimitive::Cull();
}

/*
-------------------------
Draw
-------------------------
*/

void FXCylinder::Draw( void )
{
	polyVert_t	lower_points[NUM_CYLINDER_SEGMENTS], upper_points[NUM_CYLINDER_SEGMENTS], verts[4];
	vec3_t		vr, vu, vu2, midpoint, origin2;
	float		detail, length;
	int			i;
	int			segments;

	// allow for overriding the LOD mechanism, not recommended, but hey, flexibility is often cool...
	if ( m_flags & FXF_NO_LOD )
	{
		segments = NUM_CYLINDER_SEGMENTS;
	}
	else
	{
		//Work out the detail level of this cylinder
		VectorMA( m_origin, m_height * 0.5, m_normal, midpoint );
		VectorSubtract( midpoint, cg.refdef.vieworg, midpoint );
		length = VectorLengthSquared( midpoint );

		detail = 1 - (length / (1024.0*1024.0) );

		// FIXME: the bias doesn't really work all that great
		//Cylinder bias is simply implemented as a multiplier
		segments = NUM_CYLINDER_SEGMENTS * detail * m_bias;

		// 3 is the absolute minimum, but the pop between 3, 4 and 5 is too noticeable
		if ( segments < 7 )
		{
			segments = 7;
		}

		if ( segments > NUM_CYLINDER_SEGMENTS )
		{
			segments = NUM_CYLINDER_SEGMENTS;
		}
	}

	//Get the direction vector
	MakeNormalVectors( m_normal, vr, vu );

	VectorScale( vu, m_scale2 * 0.5, vu2 );
	VectorScale( vu, m_scale * 0.5, vu );

	VectorMA( m_origin, m_height, m_normal, origin2 );

	// Calculate the step around the cylinder
	detail = 360.0 / (float)segments;

	for ( i = 0; i < segments ; i++ )
	{
		//Upper ring
		RotatePointAroundVector( upper_points[i].xyz, m_normal, vu, detail * i );
		VectorAdd( upper_points[i].xyz, m_origin, upper_points[i].xyz );
		
		//Lower ring
		RotatePointAroundVector( lower_points[i].xyz, m_normal, vu2, detail * i );
		VectorAdd( lower_points[i].xyz, origin2, lower_points[i].xyz );
	}
	
	// Calculate the texture coords so the texture can wrap around the whole cylinder
	if ( m_flags & FXF_WRAP )
	{
		if ( m_flags & FXF_STRETCH )
			detail = 1.0f / (float)segments;
		else
			detail = m_stScale / (float)segments;
	}

	for ( i = 0; i < segments ; i++ )
	{
		int nextSegment = ( i + 1 == segments ) ? 0 : i + 1;

		if ( m_flags & FXF_WRAP )
		{
			verts[0].st[0] = detail * i;
			verts[1].st[0] = detail * i;
			verts[2].st[0] = detail * ( i + 1 );
			verts[3].st[0] = detail * ( i + 1 );
		}
		else
		{
			verts[0].st[0] = 0.0f;
			verts[1].st[0] = 0.0f;
			verts[2].st[0] = m_stScale;
			verts[3].st[0] = m_stScale;
		}

		if( m_flags & FXF_STRETCH )
		{
			verts[0].st[1] = m_stScale;
			verts[1].st[1] = 0.0f;
			verts[2].st[1] = 0.0f;
			verts[3].st[1] = m_stScale;
		}
		else
		{
			verts[0].st[1] = 1.0f;
			verts[1].st[1] = 0.0f;
			verts[2].st[1] = 0.0f;
			verts[3].st[1] = 1.0f;
		}
			
 		VectorCopy( upper_points[i].xyz, verts[0].xyz );
		
		verts[0].modulate[0] = m_RGB[0] * 255;
		verts[0].modulate[1] = m_RGB[1] * 255;
		verts[0].modulate[2] = m_RGB[2] * 255;

		VectorCopy( lower_points[i].xyz, verts[1].xyz );
		
		verts[1].modulate[0] = m_RGB[0] * 255;
		verts[1].modulate[1] = m_RGB[1] * 255;
		verts[1].modulate[2] = m_RGB[2] * 255;

		VectorCopy( lower_points[nextSegment].xyz, verts[2].xyz );
		
		verts[2].modulate[0] = m_RGB[0] * 255;
		verts[2].modulate[1] = m_RGB[1] * 255;
		verts[2].modulate[2] = m_RGB[2] * 255;

		VectorCopy( upper_points[nextSegment].xyz, verts[3].xyz );
		
		verts[3].modulate[0] = m_RGB[0] * 255;
		verts[3].modulate[1] = m_RGB[1] * 255;
		verts[3].modulate[2] = m_RGB[2] * 255;

		if ( m_flags & FXF_USE_ALPHA_CHAN )
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = (byte)(m_alpha * 255);
		}
		else
		{
			verts[0].modulate[3] =
			verts[1].modulate[3] =
			verts[2].modulate[3] =
			verts[3].modulate[3] = 255;
		}

		cgi_R_AddPolyToScene( m_shader, 4, verts );
	}
}

/*
-------------------------
UpdateScale
-------------------------
*/

void FXCylinder::UpdateScale( void )
{
	m_scale += md_scale * cg.frametime * 0.001f;

	if (m_scale < 0.0f)
		m_scale = 0.0f;

	m_scale2 += md_scale2 * cg.frametime * 0.001f;

	if (m_scale2 < 0.0f)
		m_scale2 = 0.0f;
}

/*
-------------------------
UpdateHeight
-------------------------
*/

void FXCylinder::UpdateHeight( void )
{
	m_height += md_height * cg.frametime * 0.001f;

	if (m_height < 0.0f)
		m_height = 0.0f;
}

/*
-------------------------
Update
-------------------------
*/

bool FXCylinder::Update( void )
{
	UpdateScale();
	UpdateHeight();
	UpdateAlpha();
	UpdateRGB();

	return true;
}
