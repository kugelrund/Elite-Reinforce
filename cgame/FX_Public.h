#ifndef	__FX_PUBLIC__
#define __FX_PUBLIC__

#include "cg_media.h"

#define FX_DEBUG	0
//#define _FXMEM

#include <list>

// FXPrimitive flags
#define	FXF_NONE			0x00000000
#define	FXF_BOUNCE			0x00000001
#define FXF_PULSE_ALPHA		0x00000002
#define FXF_ALPHA_NOISE		0x00000004
#define FXF_USE_ALPHA_CHAN	0x00000008	// will override doing an RGB fade and use alpha fade instead, not great for a RIVA though
										//		this should probably not be used unless you really have to
#define FXF_NON_LINEAR_FADE	0x00000010	// currently begins fade at 75% of total life; do not use with an RGB to RGB fade.  
										//		I may fix this later, but probably not unless it really needs to be...
#define FXF_TOUCHCALLBACK	0x00000020	// only valid for sprites
#define FXF_NODRAW			0x00000040	// only valid for particles, not used often
#define FXF_SPAWN_ONCE		0x00000080	// only valid for spawners, typically used with FXF_DELAY_SPAWN, 
#define FXF_DELAY_SPAWN		0x00000100	// only valid for spawners, for delaying a spawned effect
#define FXF_DRAWCAPS		0x00000200	// only valid for lines, specify an endcap shader when using this
#define FXF_SHRINK			0x00000400	// only valid for lines; line will shorten to zero length by the end of its life
#define FXF_TAPER			0x00000800	// only valid for electricity bolts, bolt will taper on one end
#define FXF_WRAP			0x00001000	// only valid for cylinders, bezier curves, or electricity; 
										//		texture will be stretched stScale times around cylinder or
										//		stScale times along the bezier curve or bolt
#define FXF_NO_LOD			0x00002000	// only valid for cylinders, always uses highest level of detail ( use at your own risk! )
#define FXF_ADJUST_ST		0x00004000	// only valid for lines, calculates ST coords based on length
#define FXF_STRETCH			0x00008000	// only valid for cylinders, allow control of texture repeat



//Memory management functions
extern void FXMEM_Init( void );
extern void FXMEM_Flush( void );
extern void *FXMEM_Allocate( size_t size );
extern void FXMEM_Free( void *ptr, size_t size );
extern void FXMEM_PrintDebugInfo( void );

#if _DEBUG

extern int FXMEMD_searchPeak;
extern int FXMEMD_allocListPeak;
extern int FXMEMD_freeListPeak;
extern int FXMEMD_allocatePeak;
extern int FXMEMD_allocateTotal;

#endif

#define	DEFAULT_SPAWNER_RADIUS	500

//-----------------------------
//	FXPrimitive	-	Parent class for all client effect types
//-----------------------------
class FXPrimitive
{

public:

	FXPrimitive();
	virtual ~FXPrimitive();

#ifdef _FXMEM
	
	//Memory manager overrides
	void*operator new ( size_t size )	{	return FXMEM_Allocate( size );				}
	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXPrimitive ) );	}

#endif

	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual bool Update( void );	//Move functions can be unique per effect
	virtual void Draw( void );		//Move functions can be unique per effect

	//FIXME: Not all primitives inherit these members, so ultimately, move them out to the individual children
	
	inline void SetOrigin( vec3_t origin )				{	if ( origin )		VectorCopy( origin, m_origin );				else VectorClear( m_origin );	}
	inline void SetAngles( vec3_t angles )				{	if ( angles )		VectorCopy( angles, m_angles );				else VectorClear( m_angles );	}
	inline void SetVelocity( vec3_t velocity )			{	if ( velocity )		VectorCopy( velocity, m_velocity );			else VectorClear( m_velocity );	}
	inline void SetAcceleration( vec3_t acceleration )	{	if ( acceleration )	VectorCopy( acceleration, m_acceleration );	else VectorClear( m_acceleration );	}
	
	inline void SetScaleDelta( float dscale )			{	md_scale = dscale;			}
	inline void SetScale( float scale )					{	m_scale = scale;			}

	inline void SetStartAlpha( float alphastart )		{	m_startalpha = alphastart;	}
	inline void SetEndAlpha( float alphaend )			{	m_endalpha = alphaend;		}
	inline void SetAlphaFreq( float frequency )			{	m_alphafreq = frequency;	}

	inline void SetStartRGB( vec3_t rgb )				{	if ( rgb )	{	VectorCopy(rgb, m_startRGB);
																			VectorCopy(rgb, m_RGB);}		}
	inline void SetEndRGB( vec3_t rgb )					{	if ( rgb )	VectorCopy(rgb, m_endRGB); }
	inline void SetElasticity( float elasticity )		{	m_elasticity = elasticity;	}
	inline void SetShader( qhandle_t shader )			{	m_shader = shader;			}
	inline void SetFlags( int flags )					{	m_flags = flags;			}

	//NOTENOTE: These were moved because there was just too many functions calling into this

	//Movestate information
	vec3_t		m_origin;
	vec3_t		m_angles;
	vec3_t		m_velocity;
	vec3_t		m_acceleration;

	int			m_flags;

	float		m_startalpha;
	float		m_endalpha;
	float		m_alpha;		// Current interpolated alpha
	float		m_alphafreq;	// sine wave freqency for pulsing alpha (pulses per second)

	vec3_t		m_startRGB;	
	vec3_t		m_endRGB;
	vec3_t		m_RGB;			// Current interpolated RGB

	float		md_scale;
	float		m_scale;
	float		m_elasticity;

	int			m_start_time;
	int			m_end_time;

	qhandle_t	m_shader;	//Graphics shader for the effect

	virtual void UpdateOrigin( void );
	virtual void UpdateVelocity( void );
	virtual void UpdateScale( void );
	virtual void UpdateAlpha( void );
	virtual void UpdateRGB( void );
	
	//FIXME: Not all primitives inherit these variables, so ultimately, move them out to the individual children
};
 
//-----------------------------
//	FXElectricity
//-----------------------------
#define	MAX_BOLT_SEGMENTS	16
#define DEFAULT_DEVIATION	0.5

class FXElectricity : public FXPrimitive
{
public:

	FXElectricity( void );
	~FXElectricity( void );

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXElectricity ) );	}

#endif

	void SetOrigin2( vec3_t origin2 )		{	if ( origin2 )	VectorCopy( origin2, m_origin2 );	else VectorClear( m_origin2 );}
	void SetSTScale( float scale )			{	m_stScale = scale;	}
	void SetDeviation( float deviation )	{	m_deviation = deviation; };
	
	virtual void Draw( void );		//Draw functions are unique per effect
	virtual bool Cull( void );		//Cull functions are unique per effect

	void	Build( void );

protected:
	
	inline void	DrawSegment( vec3_t start, vec3_t end, float scale, float tcStart, float tcEnd );
	void	Bolt( vec3_t start, vec3_t dir, float length, int num );

	vec3_t	m_origin2;
	float	m_stScale;

	vec3_t	m_boltVerts[ MAX_BOLT_SEGMENTS ];
	
	vec3_t	m_lastEnd[2];

	float	m_deviation;
	bool	m_tapered;
	bool	m_init;

};

//-----------------------------
//	FXParticle
//-----------------------------
class FXParticle : public FXPrimitive
{
public:

	FXParticle( void );
	~FXParticle( void );

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXParticle ) );	}

#endif

	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual bool Update( void );	//Move functions can be unique per effect
	virtual void Draw( void );		//Draw functions are unique per effect

	inline void SetThink( bool (*think)(FXPrimitive *, centity_t *owner) )	{ Think = think;	}

	void	SetRoll( float roll )		{	m_roll = roll;		}
	void	SetRollDelta( float droll )	{	md_roll = droll;	}
	inline void SetCOwner( centity_t *cowner )				{	m_cowner = cowner;			}
	inline const centity_t *GetCOwner( void )	const {	return (const centity_t *) m_cowner;	}


protected:

	void UpdateRoll( void );

	// The think function must update all of the particles attributes (velocity, alpha, etc.)
	bool (*Think)( FXPrimitive *object, centity_t *owner );

	float	m_roll;
	float	md_roll;
	centity_t	*m_cowner;	//cOwner of this effect

};

//-----------------------------
//	FXTri	-	Triangles
//-----------------------------
class FXTri : public FXPrimitive
{
	public:

	FXTri();
	~FXTri();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXTri ) );	}

#endif

	void SetOrigin2( vec3_t origin2 )		{	if ( origin2 )	VectorCopy( origin2, m_origin2 );	else VectorClear( m_origin2 );}
	void SetOrigin3( vec3_t origin3 )		{	if ( origin3 )	VectorCopy( origin3, m_origin3 );	else VectorClear( m_origin3 );}
	
	virtual void Draw( void );		//Draw functions are unique per effect
	virtual bool Cull( void );		//Cull functions are unique per effect

protected:

	vec3_t		m_origin2;
	vec3_t		m_origin3;
};

//-----------------------------
//	FXQuad	-	Quads
//-----------------------------
class FXQuad : public FXPrimitive
{

public:

	FXQuad( void );
	~FXQuad( void );

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXQuad ) );	}

#endif

	void	SetRoll( float roll )		{	m_roll = roll;	}
	void	SetRollDelta( float droll )	{	md_roll = droll;}

	void	SetNormal( vec3_t normal )	{	if ( normal )	VectorCopy( normal, m_normal );	else VectorClear( m_normal );	}

	virtual void Draw( void );		//Draw functions are unique per effect
	virtual bool Update( void );

protected:

	void UpdateRoll( void );

	float	m_roll;
	float	md_roll;
	vec3_t	m_normal;
};

//-----------------------------
//	FXSprite	-	Sprite
//-----------------------------
class FXSprite : public FXPrimitive
{

public:

	FXSprite();
	~FXSprite();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXSprite ) );	}

#endif

	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual bool Update( void );		//Move functions can be unique per effect
	virtual void Draw( void );		//Draw functions are unique per effect

	void	SetRoll( float roll )		{	m_roll = roll;		}
	void	SetRollDelta( float droll )	{	md_roll = droll;	}

protected:

	void	UpdateRoll( void );

	float	m_roll;	//Roll factor
	float	md_roll;

};

//-----------------------------
//	FXLine	-	Lines
//-----------------------------
class FXLine : public FXPrimitive
{

public:

	FXLine();
	virtual ~FXLine();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXLine ) );	}

#endif

	inline void SetEndcapShader( qhandle_t shader )			{	m_endcap_shader = shader;	}
	inline void SetOrigin2( vec3_t origin2 )		{	if ( origin2 ){	VectorCopy( origin2, m_origin2 );
																VectorCopy( origin2, m_work_org );
																}
	
												else		{	VectorClear( m_origin2 );
																VectorClear( m_work_org);
																}	
											}

	inline void SetSTScale( float scale )			{	m_stScale = scale;	}
	
	virtual void Draw( void );		//Draw functions are unique per effect
	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual void UpdateOrigin( void );

protected:

	vec3_t		m_origin2;
	vec3_t		m_work_org;
	float		m_stScale;
	qhandle_t	m_endcap_shader;
};

//-----------------------------
//	FXLine2	-	Lines
//-----------------------------
class FXLine2 : public FXLine
{

public:

	FXLine2();
	virtual ~FXLine2();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXLine ) );	}

#endif

	inline void SetScale2( float scale )		{	m_scale2 = scale; }
	inline void SetScale2Delta( float dscale )	{	m_dscale2 = dscale; }
		
	virtual void Draw( void );		//Draw functions are unique per effect
	virtual bool Cull( void );		//Cull functions are unique per effect

protected:

	float	m_scale2;
	float	m_dscale2;
};

//-----------------------------
//	FXSpawner - Effect spawner
//-----------------------------
class FXSpawner : public FXPrimitive
{

public:

	FXSpawner();
	~FXSpawner();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXSpawner ) );	}

#endif

	virtual bool Cull( void );			//Cull functions are unique per effect
	virtual bool Update( void );		//Move functions can be unique per effect
	virtual void Draw( void );			//Draw functions are unique per effect
	virtual void UpdateOrigin( void );	

	inline void SetDelay( int delay )			{	m_delay = delay;		}
	inline void SetVariance( float variance )	{	m_variance = variance;	}
	inline void SetRadius( int radius )			{	m_radius = radius;		}
	inline void SetOwner( localEntity_t *owner ){	m_owner = owner;		}

	void (*Think)( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user );

	int		m_nextThink;

protected:

	int				m_delay;
	float			m_variance;
	int				m_radius;
	localEntity_t	*m_owner;
};


//-----------------------------
//	FXBezier	-	Bezier curves
//-----------------------------
class FXBezier : public FXLine
{
public:

	FXBezier();
	~FXBezier();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXBezier ) );	}

#endif

	virtual bool Cull( void );
	virtual bool Update( void );		//Move functions can be unique per effect
	virtual void Draw( void );		//Draw functions are unique per effect

	void SetControl1( vec3_t control )	{	if ( control )	VectorCopy( control, m_control1 );	else VectorClear( m_control1 );}
	void SetControl2( vec3_t control )	{	if ( control )	VectorCopy( control, m_control2 );	else VectorClear( m_control2 );}

	void SetControl1_Velocity( vec3_t vel )	{	if ( vel )	VectorCopy( vel, m_control1_velocity );	else VectorClear( m_control1_velocity );}
	void SetControl2_Velocity( vec3_t vel )	{	if ( vel )	VectorCopy( vel, m_control2_velocity );	else VectorClear( m_control2_velocity );}
	
	void SetControl1_Acceleration( vec3_t accel )	{	if ( accel )	VectorCopy( accel, m_control1_acceleration );	else VectorClear( m_control1_acceleration );}
	void SetControl2_Acceleration( vec3_t accel )	{	if ( accel )	VectorCopy( accel, m_control2_acceleration );	else VectorClear( m_control2_acceleration );}

protected:

	void DrawSegment( vec3_t start, vec3_t end, float tc1, float tc2 );

	vec3_t	m_control1;
	vec3_t	m_control2;

	vec3_t	m_control1_velocity;
	vec3_t	m_control2_velocity;

	vec3_t	m_control1_acceleration;
	vec3_t	m_control2_acceleration;

	vec3_t	m_lastEnd[2];
	bool	m_init;
};


//-----------------------------
//	FXTrail
//-----------------------------
class FXTrail : public FXPrimitive
{

public:

	FXTrail();
	~FXTrail();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXTrail ) );	}

#endif

	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual bool Update( void );		//Move functions can be unique per effect
	virtual void Draw( void );		//Draw functions are unique per effect

	void SetOldOrigin( vec3_t oldorigin )	{ if (oldorigin)	VectorCopy( oldorigin, m_oldorigin );	else VectorClear( m_oldorigin );}
	void SetLength( float length )			{ m_length = length;	}
	void SetLengthDelta( float dlength )	{ md_length = dlength;	}

protected:

	void UpdateTailPoint( void );
	void UpdateTailLength( void );

	vec3_t	m_oldorigin;
	float	m_length;
	float	md_length;
};

//-----------------------------
//	FXCylinder
//-----------------------------
class FXCylinder : public FXPrimitive
{

public:

	FXCylinder();
	~FXCylinder();

#ifdef _FXMEM

	void operator delete( void *ptr )	{	FXMEM_Free( ptr, sizeof( FXCylinder ) );	}

#endif

	virtual bool Cull( void );		//Cull functions are unique per effect
	virtual bool Update( void );	//Move functions can be unique per effect
	virtual void Draw( void );		//Draw functions are unique per effect

	inline void SetScale2( float scale )			{	m_scale2 = scale;	}
	inline void SetScaleDelta2( float dscale )		{	md_scale2 = dscale;	}

	inline void SetHeight( float height )			{	m_height = height;	}
	inline void SetHeightDelta( float dheight )		{	md_height = dheight;}

	inline void SetStart( vec3_t start )		{	if ( start )	VectorCopy( start, m_origin );	else VectorClear( m_origin );	}
	inline void SetNormal( vec3_t normal )		{	if ( normal )	VectorCopy( normal, m_normal);	else VectorClear( m_normal );	}

	inline void SetBias( float bias)				{	m_bias = bias;	}
	inline void SetSTScale( float scale )			{	m_stScale = scale;	}
	
protected:

	virtual void UpdateScale( void );
	virtual void UpdateHeight( void );

	vec3_t	m_normal;
	float	m_bias;
	float	m_height;
	float	md_height;
	float	m_scale2;
	float	md_scale2;
	float	m_stScale;
};

//
//	Struct Declarations

typedef struct FX_state_s
{
	FXPrimitive		*effect;
	int				killTime;
} FX_state_t;

#define	MAX_EFFECTS		1024

extern FX_state_t	FX_renderList[ MAX_EFFECTS ];
extern FX_state_t	*FX_nextValid;

//	Function prototypes

extern	void FX_Add( void );
extern	int	FX_Init( void );

extern inline FXSprite *FX_AddSprite( vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float elasticity, float killTime, qhandle_t shader, int flags = 0);
extern inline FXSprite *FX_AddSprite( vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float elasticity, float killTime, qhandle_t shader, int flags = 0);

extern inline FXElectricity *FX_AddElectricity( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags = 0 );
extern inline FXElectricity *FX_AddElectricity( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags, float deviation );

extern inline FXParticle *FX_AddParticle( centity_t *owner, vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float elasticity, float killTime, qhandle_t shader, int flags, bool (*think)(FXPrimitive *, centity_t *) );
extern inline FXParticle *FX_AddParticle( centity_t *owner, vec3_t origin, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float elasticity, float killTime, qhandle_t shader, int flags, bool (*think)(FXPrimitive *, centity_t *) );

extern inline FXQuad *FX_AddQuad( vec3_t origin, vec3_t normal, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, float roll, float droll, float elasticity, float killTime, qhandle_t shader );
extern inline FXQuad *FX_AddQuad( vec3_t origin, vec3_t normal, vec3_t velocity, vec3_t acceleration, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float roll, float droll, float elasticity, float killTime, qhandle_t shader );

extern inline FXLine *FX_AddLine( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags = 0 );
extern inline FXLine *FX_AddLine( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, int flags = 0 );

extern inline FXLine2 *FX_AddLine2( vec3_t origin, vec3_t origin2, float stScale, float scale, float dscale, float scale2, float dscale2, float startalpha, float endalpha, float killtime, qhandle_t shader, int flags = 0 );

extern inline FXTri *FX_AddTri( vec3_t origin, vec3_t origin2, vec3_t origin3, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags = 0  );
extern inline FXTri *FX_AddTri( vec3_t origin, vec3_t origin2, vec3_t origin3, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, int flags = 0 );

extern inline FXSpawner *FX_AddSpawner( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user, int delay, float variance, float killTime, void *think, localEntity_t *owner = NULL, int radius = DEFAULT_SPAWNER_RADIUS );
extern inline FXSpawner *FX_AddSpawner( vec3_t origin, vec3_t angles, vec3_t velocity, vec3_t user, int delay, float variance, float killTime, int flags, void *think, localEntity_t *owner = NULL, int radius = DEFAULT_SPAWNER_RADIUS );

extern inline FXBezier *FX_AddBezier( vec3_t origin, vec3_t origin2, vec3_t control1, vec3_t control2, vec3_t control1_vel, vec3_t control2_vel, vec3_t control1_accel, vec3_t control2_accel, float scale, float dscale, float startalpha, float endalpha, float killTime, qhandle_t shader, int flags = 0 );
extern inline FXBezier *FX_AddBezier( vec3_t origin, vec3_t origin2, vec3_t control1, vec3_t control2, vec3_t control1_vel, vec3_t control2_vel, vec3_t control1_accel, vec3_t control2_accel, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, int flags = 0 );

extern inline FXTrail *FX_AddTrail( vec3_t origin, vec3_t velocity, vec3_t acceleration, float length, float dlength, float scale, float dscale, float startalpha, float endalpha, float elasticity, float killTime, qhandle_t shader, int flags = 0);
extern inline FXTrail *FX_AddTrail( vec3_t origin, vec3_t velocity, vec3_t acceleration, float length, float dlength, float scale, float dscale, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float elasticity, float killTime, qhandle_t shader, int flags = 0);

extern inline FXCylinder *FX_AddCylinder( vec3_t start, vec3_t normal, float height, float dheight, float scale, float dscale, float scale2, float dscale2, float startalpha, float endalpha, float killTime, qhandle_t shader, float bias = 1.0 );
extern inline FXCylinder *FX_AddCylinder( vec3_t start, vec3_t normal, float height, float dheight, float scale, float dscale, float scale2, float dscale2, float startalpha, float endalpha, vec3_t startRGB, vec3_t endRGB, float killTime, qhandle_t shader, float bias = 1.0 );

float FX_DetailLevel( vec3_t origin, float near_clip, float far_clip );

int FX_Free( void );

void FXE_Spray	( vec3_t direction, float speed, float variation, float cone, float gravity, FXPrimitive *effect );

#endif	//__FX_PUBLIC__

