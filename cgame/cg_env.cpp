//This file contains environmental effects for the designers

#include "cg_local.h"
#include "fx_public.h"

extern void G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod);
/*
======================
CG_Spark

Creates a spark effect
======================
*/

void CG_Spark( vec3_t origin, vec3_t normal )
{
	FXTrail	*particle;
	vec3_t	dir, direction, start, end;
	vec3_t	velocity, accel;
	float	scale;
	int		numSparks;

	AngleVectors( normal, normal, NULL, NULL );

	for ( int j = 0; j < 3; j ++ )
		normal[j] = normal[j] + (0.15f * crandom());

	VectorNormalize( normal );

	//cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgi_S_RegisterSound( va( "sound/world/ric%d.wav", (rand() & 2)+1) ) );

	numSparks = 8 + (random() * 4.0f);
	
	scale = 0.2f + (random() *0.4);
	VectorMA( origin, 24.0f + (crandom() * 4.0f), normal, end );

	//One long spark
	FX_AddLine( origin,
				end,
				1.0f,
				scale,
				0.0f,
				1.0f,
				0.25f,
				125.0f,
				cgs.media.sparkShader );
	
	for ( int i = 0; i < numSparks; i++ )
	{	
		scale = 0.2f + (random() *0.4);

		for ( int j = 0; j < 3; j ++ )
			dir[j] = normal[j] + (0.25f * crandom());
		
		VectorNormalize(dir);

		VectorMA( origin, 0.0f + ( random() * 2.0f ), dir, start );
		VectorMA( start, 2.0f + ( random() * 16.0f ), dir, end );

		FX_AddLine( start,
					end,
					1.0f,
					scale,
					0.0f,
					1.0f,
					0.25f,
					125.0f,
					cgs.media.sparkShader );
	}

	if ( rand() & 1 )
	{
		numSparks = 1 + (random() * 2.0f);
		for ( int i = 0; i < numSparks; i++ )
		{	
			scale = 0.5f + (random() * 0.5f);

			VectorScale( normal, 250, velocity );
			VectorSet( accel, 0, 0, -512 );

			particle = FX_AddTrail( start,
									velocity,
									accel,
									8.0f,
									-32.0f,
									scale,
									-scale,
									1.0f,
									0.5f,
									0.25f,
									700.0f,
									cgs.media.sparkShader,
									FXF_BOUNCE );

		}
	}

	VectorMA( origin, 1, normal, direction );

	scale = 6.0f + (random() * 8.0f);
	float alpha = 0.1 + (random() * 0.4f);

	VectorSet( velocity, 0, 0, 8 );

	FX_AddSprite(	direction, 
					velocity, 
					NULL, 
					scale,
					scale,
					alpha,
					0.0f,
					random()*45.0f,
					0.0f,
					1000.0f,
					cgs.media.steamShader );
}

//------------------------------------------------------------------------------
void SteamThink( vec3_t position, vec3_t dest, vec3_t dir, vec3_t puff_scale )
{
	float	speed = 200;
	vec3_t	direction, facing, color = { 1.0f, 1.0f, 1.0f };
	vec3_t	velocity		= { 0, 0, 128 };
	vec3_t	acceleration	= { 0, 0, 256 };
	float	scale, dscale, len, detail;

	detail = FX_DetailLevel( position, 16, 1024 );

	if (detail == 0)
		return;

	VectorSubtract( dest, position, facing );
	len = VectorNormalize( facing );

	VectorCopy( dir, direction );

	if ( puff_scale[1] )
	{
		VectorSet( color, 0.9f + random() * 0.1f, 0.8f + random() * 0.2f, 0.4f + random() * 0.3f );
	}

	direction[0] += (2.0f + (crandom() * 2.0f));
	direction[1] += (2.0f + (crandom() * 2.0f));
	direction[2] += (2.0f + (crandom() * 2.0f));

	AngleVectors( direction, direction, NULL, NULL );

	VectorScale( direction, speed, velocity );
	VectorScale( velocity, -1.0f, acceleration );

	scale = 8.0f + (random() * 4.0f) * (puff_scale[0] / 10.0f);
	dscale = scale * 8.0 * (puff_scale[0] / 10.0f);

	FX_AddSprite(	position,
					velocity, 
					acceleration, 
					scale, 
					dscale, 
					1.0f, 
					0.0f,
					color,
					color,
					random() * 360,
					0.25f,
					(len / speed) * 1000, 
					cgs.media.steamShader );
}

//void SteamJetThink( vec3_t position, vec3_t dest, vec3_t dir, vec3_t user )
//{
//	FX_AddSpawner( position, dest, dir, NULL, 50, 0, 200 + ( random() * 500 ), (void *) SteamThink );
//}

/*
======================
CG_Steam

Creates a steam effect
======================
*/

void CG_Steam( vec3_t position, vec3_t dest, vec3_t dir, float scale, int flags )
{
	vec3_t user;

	// stuff some extra info into the user field
	user[0] = scale;
	user[1] = (flags & 4);

	if ( (flags & 2) )
	{
		FX_AddSpawner( position, dest, dir, user, 50, 0, 200 + ( random() * 500 ), (void *) SteamThink );
	}
	else
	{
		SteamThink( position, dest, dir, user );
	}
}

/*
======================
CG_CookingSteam

Creates a basic cooking steam effect
======================
*/
void CG_CookingSteam( vec3_t origin, float radius )
{
	vec3_t dir;

	VectorSet( dir, crandom()*2, crandom()*2, crandom() + radius); // always move mostly up
	VectorScale( dir, random() * 5 + 2, dir );

	FX_AddSprite( origin, dir, NULL, radius, radius * 2, 0.4F, 0.0, 0, 0, 1000, cgs.media.steamShader );
}

/*
======================
CG_Bolt

Creates a electricity bolt effect
======================
*/
#define DATA_EFFECTS	0
#define DATA_CHAOS		1
#define DATA_RADIUS		2

void BoltFireback( vec3_t start, vec3_t end, vec3_t velocity, vec3_t user )
{
	FX_AddElectricity( start, end, 1.0, user[DATA_RADIUS], 5.0, 1.0, 0.0, 200, cgs.media.bolt2Shader, 
						(int)user[DATA_EFFECTS], user[DATA_CHAOS] );

	if ( rand() & 1 )
		FX_AddElectricity( end, start, 1.0, user[DATA_RADIUS] * 2, 5.0, 1.0, 0.0, 200, cgs.media.bolt2Shader,
							(int)user[DATA_EFFECTS], user[DATA_CHAOS] );
}

//-----------------------------
void BorgBoltFireback( vec3_t start, vec3_t end, vec3_t velocity, vec3_t user )
{
	FX_AddElectricity( start, end, 1.0, user[DATA_RADIUS], 5.0, 1.0, 0.0, 100, cgs.media.borgLightningShaders[2],
						(int)user[DATA_EFFECTS], user[DATA_CHAOS]  );

	if ( rand() & 1 )
		FX_AddElectricity( end, start, 1.0, user[DATA_RADIUS] * 2, 5.0, 1.0, 0.0, 100, cgs.media.borgLightningShaders[3], 
							(int)user[DATA_EFFECTS], user[DATA_CHAOS] );
}

//-----------------------------
void BlackAndWhiteBoltFireback( vec3_t start, vec3_t end, vec3_t velocity, vec3_t user )
{
	FX_AddElectricity( start, end, 1.0, user[DATA_RADIUS], 5.0, 1.0, 0.0, 100, cgs.media.whiteLaserShader,
						(int)user[DATA_EFFECTS], user[DATA_CHAOS]  );

	if ( rand() & 1 )
		FX_AddElectricity( end, start, 1.0, user[DATA_RADIUS] * 2, 5.0, 1.0, 0.0, 100, cgs.media.whiteLaserShader, 
							(int)user[DATA_EFFECTS], user[DATA_CHAOS] );
}

//-----------------------------
// This looks a bit cheesy
void BoltSparkSpew( vec3_t origin, vec3_t dir, qhandle_t shader )
{
	FXTrail	*particle;

	// Create the sparks
	for ( int i = 0; i < 6; i++ )
	{	
		particle = FX_AddTrail( origin, NULL, NULL, 2.0f + random() * 3, -5.0f,
								0.5, -2.0, 0.3f, 0.0f, 0.25f, 500.0f, shader );
 
		if ( particle == NULL )
			return;

		FXE_Spray( dir, 25, 25, 0.6f, 300 + (rand() & 100), (FXPrimitive *) particle );
	}
}

//-----------------------------
void CG_Bolt( centity_t *cent )
{
	qboolean	borg, bw; // bw = black and white
	int			effects;
	float		chaos, radius;
	
	// Set up all of the parms
	borg = (cent->gent->spawnflags & 8);
	bw = (cent->gent->spawnflags & 64);
	effects = (cent->gent->spawnflags & 16) ? FXF_TAPER : 0;
	effects = (cent->gent->spawnflags & 32) ? (FXF_WRAP | effects) : effects;
	chaos = cent->gent->random;
	radius = cent->gent->radius;

	// Delayed bolt that should "work" a while
	if ( cent->gent->spawnflags & 2 )
	{
		vec3_t data;

		// This sucks, but the spawn function needs some extra bits of info
		data[DATA_EFFECTS] = effects;
		data[DATA_CHAOS] = chaos;
		data[DATA_RADIUS] = radius;

		if ( bw )
		{
			FX_AddSpawner( cent->lerpOrigin, cent->currentState.origin2, NULL, data, 70, random() * 25, 450, 
					(void *) BlackAndWhiteBoltFireback, NULL, 768 );
		}
		else if ( borg )
			FX_AddSpawner( cent->lerpOrigin, cent->currentState.origin2, NULL, data, 70, random() * 25, 450, (void *) BorgBoltFireback, NULL, 768 );
		else
			FX_AddSpawner( cent->lerpOrigin, cent->currentState.origin2, NULL, data, 70, random() * 25, 450, (void *) BoltFireback, NULL, 768 );
	}
	else
	{
		// Continuous bolts
		if ( bw )
		{
			FX_AddElectricity( cent->lerpOrigin, cent->currentState.origin2, 1.0, radius, 5.0, 1.0, 0.0, 200, cgs.media.whiteLaserShader, 
							effects, chaos );

			if ( rand() & 1 )
				FX_AddElectricity( cent->currentState.origin2, cent->lerpOrigin, 1.0, radius * 2, 5.0, 1.0, 0.0, 200, cgs.media.whiteLaserShader,
								effects, chaos );
		}
		else if ( borg )
		{
			FX_AddElectricity( cent->lerpOrigin, cent->currentState.origin2, 1.0, radius, 5.0, 1.0, 0.0, 200, cgs.media.borgLightningShaders[2], 
							effects, chaos );

			if ( rand() & 1 )
				FX_AddElectricity( cent->currentState.origin2, cent->lerpOrigin, 1.0, radius * 2, 5.0, 1.0, 0.0, 200, cgs.media.borgLightningShaders[3], 
								effects, chaos );
		}
		else
		{
			FX_AddElectricity( cent->lerpOrigin, cent->currentState.origin2, 1.0, radius, 5.0, 1.0, 0.0, 200, cgs.media.bolt2Shader, 
							effects, chaos );

			if ( rand() & 1 )
				FX_AddElectricity( cent->currentState.origin2, cent->lerpOrigin, 1.0, radius * 2, 5.0, 1.0, 0.0, 200, cgs.media.bolt2Shader, 
							effects, chaos );
		}
	}

	// Bolt that generates sparks at the impact point
	if ( cent->gent->spawnflags & 4 )
	{
		vec3_t	dir;

		VectorSubtract( cent->lerpOrigin, cent->currentState.origin2, dir );
		VectorNormalize( dir );

		if ( bw )
		{
			BoltSparkSpew( cent->currentState.origin2, dir, cgs.media.waterDropShader );
		}
		else if ( borg )
			BoltSparkSpew( cent->currentState.origin2, dir, cgs.media.borgFlareShader );
		else
			BoltSparkSpew( cent->currentState.origin2, dir, cgs.media.spark2Shader );
	}
}

/*
======================
CG_ForgeBolt

Creates an orange electricity bolt effect with a pulse that travels down the beam
======================
*/

void ForgeBoltFireback( vec3_t start, vec3_t end, vec3_t velocity, vec3_t user )
{
	FX_AddElectricity( start, end, 1.0, user[DATA_RADIUS], 5.0, 1.0, 0.0, 200, cgs.media.pjBoltShader, 
						(int)user[DATA_EFFECTS], user[DATA_CHAOS] );
}

//---------------------------------------------------
bool ForgeBoltPulse( FXPrimitive *fx, centity_t *ent )
{
	vec3_t			origin, new_org;
	trace_t			trace;
	qboolean		remove = qfalse;

	VectorCopy( fx->m_origin, origin );
	fx->UpdateOrigin();
	VectorCopy( fx->m_origin, new_org );

	CG_Trace( &trace, origin, NULL, NULL, new_org, -1, CONTENTS_SOLID );

	if ( trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid )
	{
		// The effect hit something, presumably a barrier, so kill it
		remove = qtrue;
		return false;
	}

	vec3_t		normal, rgb1 ={ 1.0F, 0.5F, 0.4F}, rgb2 ={ 1.0F, 1.0F, 0.3F};//, org;
	FXCylinder	*fxc;

	// Convert the direction of travel in to a normal;
	VectorCopy( fx->m_velocity, normal );
	VectorNormalize( normal );
	VectorScale( normal, -1, normal );

	fxc = FX_AddCylinder( new_org, normal, 16, 0, 16 - random() * 8, 0, 32 + random() * 24, 0, 0.2F, 0.2F, rgb1, rgb1, 1, cgs.media.psychicShader, 0.6F );

	if ( fxc == NULL )
		return false;

	fxc->SetFlags( FXF_WRAP );
	fxc->SetSTScale( Q_irand(1,3) );

	fxc = FX_AddCylinder( new_org, normal, 8, 0, 12 - random() * 8, 0, 24 + random() * 24, 0, 0.2F, 0.2F, rgb2, rgb2, 1, cgs.media.psychicShader, 0.6F );

	if ( fxc == NULL )
		return false;

	fxc->SetFlags( FXF_WRAP );
	fxc->SetSTScale( Q_irand(1,2) );

	return true;
}

//-----------------------------
void CG_ForgeBolt( centity_t *cent )
{
	qboolean	pulse;
	int			effects;
	float		chaos, radius;
	
	// Set up all of the parms
	pulse = (cent->gent->spawnflags & 8) ? qtrue : qfalse;
	effects = (cent->gent->spawnflags & 16) ? FXF_TAPER : 0;
	effects = (cent->gent->spawnflags & 32) ? (FXF_WRAP | effects) : effects;
	chaos = cent->gent->random;
	radius = cent->gent->radius;

	// Delayed bolt that should "work" a while
	if ( cent->gent->spawnflags & 2 )
	{
		vec3_t data;

		// This sucks, but the spawn function needs some extra bits of info
		data[DATA_EFFECTS] = effects;
		data[DATA_CHAOS] = chaos;
		data[DATA_RADIUS] = radius;

		FX_AddSpawner( cent->lerpOrigin, cent->currentState.origin2, NULL, data, 70, random() * 25, 450, (void *) ForgeBoltFireback );
	}
	else
	{
		FX_AddElectricity( cent->lerpOrigin, cent->currentState.origin2, 1.0, radius, 5.0, 1.0, 0.0, 200, cgs.media.pjBoltShader, 
						effects, chaos );

		if ( rand() & 1 )
			FX_AddElectricity( cent->currentState.origin2, cent->lerpOrigin, 1.0, radius * 2, 5.0, 1.0, 0.0, 200, cgs.media.pjBoltShader, 
						effects, chaos );

		if ( cg.time > cent->gent->delay && pulse )
		{
			vec3_t	dir;
			float	amt;

			VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
			VectorNormalize( dir );
			amt = 200 + random() * 100;
			VectorScale( dir, amt, dir );

			FX_AddParticle( cent, cent->lerpOrigin, dir, NULL, 16, 0.0, 1.0, 1.0,
						0.0, 0.0, 6000, cgs.media.ltblueParticleShader, FXF_NODRAW, ForgeBoltPulse );

			cent->gent->delay = cg.time + 500;
		}
	}

	// Bolt that generates sparks at the impact point
	if ( cent->gent->spawnflags & 4 )
	{
		vec3_t	dir;

		VectorSubtract( cent->lerpOrigin, cent->currentState.origin2, dir );
		VectorNormalize( dir );

		BoltSparkSpew( cent->currentState.origin2, dir, cgs.media.dkorangeParticleShader );
	}
}

/*
======================
CG_PsychoJism

Creates a psycho jism bolt effect
======================
*/

void CG_PsychoJism( vec3_t start, vec3_t end, qboolean open )
{
	vec3_t	dir, rev_dir, angles, pos1, pos2, ofdir, ofang;
	float	len, detail;
	int		i;

	detail = FX_DetailLevel( start, 16, 1024 );	//Technically this needs to find the average or closer...

	// don't draw if we are too far away
	if (detail == 0 || cg_freezeFX.integer)
	{
		return;
	}

	VectorSubtract( end, start, dir );
	len = VectorNormalize( dir );
	VectorScale( dir, -1, rev_dir );
	vectoangles( rev_dir, angles );

	if(open)
	{
		// Calculate a new start 
		VectorCopy( dir, ofdir );
		vectoangles( ofdir, ofang );
		
		ofang[2] = crandom() * 360;

		AngleVectors( ofang, NULL, ofdir, NULL );
		VectorMA( start, len * 0.06f, ofdir, pos1 );

		// Calculate a new end
		VectorMA( end, len * 0.08f, rev_dir, pos2 );
		for ( i=0; i < 3; i++ )
		{
			pos2[i] += crandom() * 2.0f;
		}
	}
	else
	{
		VectorCopy( start, pos1 );
		VectorCopy( end, pos2 );
	}

	FX_AddElectricity( pos1, pos2, 1.0f, 2.0f, 2.0f, 1.0f, 0.0f, 150.0f, cgs.media.pjBoltShader );

	if(!open)
	{
		VectorMA( start, -4, dir, pos1 );
		VectorMA( end, 4, dir, pos2 );
		FX_AddLine( pos2, pos1, 1.0, 2.0, 32.0 * random() + 24.0 , 0.5f, 0.0f, 100.0f, cgs.media.spark3Shader );

		CG_Spark(end, angles);
	}
}

/*
===========================
Plasma

Create directed and scaled plasma jet
===========================
*/

void CG_Plasma( vec3_t start, vec3_t end, vec4_t startRGBA, vec4_t endRGBA )
{
	vec3_t	v, sp, sRGB, eRGB;
	float	detail, len, salpha, ealpha;

	detail = FX_DetailLevel( start, 16, 1200 );
	if ( detail == 0 )
		return;

	salpha = Vector4to3( startRGBA, sRGB );
	ealpha = Vector4to3( endRGBA, eRGB );

	// Orient the plasma
	VectorSubtract( end, start, v );
	len = VectorNormalize( v );
	VectorMA( start, 0.5f, v, sp );

	// Stash a quad at the base to make the effect look a bit more solid
	FX_AddQuad( sp, v, NULL, NULL, len * 0.36f, 0.0f, salpha, salpha, sRGB, sRGB, 0.0f, 45.0f, 0.0f, 200, cgs.media.prifleImpactShader );
	
	// Add a subtle, random flutter to the cone direction
	v[0] += crandom() * 0.04;
	v[1] += crandom() * 0.04;
	v[2] += crandom() * 0.04;
	
	// Wanted the effect to be scalable based on the length of the jet. 
	FX_AddCylinder( start, v, len * 0.05, len * 2.0f, len * 0.16f, len * 0.32f, len * 0.40f, len * 0.64f, 
					salpha, ealpha, sRGB, eRGB, 200, cgs.media.plasmaShader, 0.3f );
	FX_AddCylinder( start, v, len * 0.05, len * 4.0f, len * 0.16f, len * 0.32f, len * 0.28f, len * 0.64f, 
					salpha, ealpha, sRGB, eRGB, 200, cgs.media.plasmaShader, 0.2f );
	FX_AddCylinder( start, v, len * 0.25, len * 8.0f, len * 0.20f, len * 0.32f, len * 0.02f, len * 0.32f, 
					salpha, ealpha, sRGB, eRGB, 200, cgs.media.plasmaShader, 0.1f );
}

/*
===========================
Drip

Create timed drip effect
===========================
*/

bool DripCallback( FXPrimitive *fx, centity_t *ent )
{
	vec3_t	org, dir = {0,0,-1};

	fx->UpdateOrigin();
	fx->UpdateAlpha();
	fx->UpdateRGB();

	VectorMA( fx->m_origin, -1.4, dir, org );
	FX_AddLine( fx->m_origin, org, 1.0, fx->m_scale * 0.75, -(fx->m_scale * 4), fx->m_alpha, 0.0, fx->m_RGB, fx->m_RGB, 
				25, fx->m_shader );

	return true;
}

//------------------------------------------------------------------------------
void DripSplash( vec3_t origin, vec3_t normal, vec3_t work, vec3_t user )
{
	FXTrail		*particle;
	float		detail;

	detail = FX_DetailLevel( origin, 16, 400 );
	if (detail == 0)
		return;

	for ( int i = 0; i < 5; i++ )
	{	
		particle = FX_AddTrail( origin, NULL, NULL, 1.5f, -2.0f, work[0], -work[0],
								work[1], 0, user, user, 0.25f, 350.0f, work[2], rand() & FXF_BOUNCE );

		if ( particle == NULL )
			return;

		FXE_Spray( normal, 60, 50, 0.4f, 512, (FXPrimitive *) particle );
	}
	
	switch( rand() & 3 )
	{
	case 1:
		cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AMBIENT, cgi_S_RegisterSound( "sound/ambience/waterdrop1.wav") );
		break;
	case 2:
		cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AMBIENT, cgi_S_RegisterSound( "sound/ambience/waterdrop2.wav") );
		break;
	default:
		cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AMBIENT, cgi_S_RegisterSound( "sound/ambience/waterdrop3.wav") );
		break;
	}
}

#define DRIP_ACCEL	370.0f

//------------------------------------------------------------------------------
void CG_Drip( vec3_t origin, vec4_t RGBA, float diameter, int thinktime, qhandle_t shader )
{
	FXSpawner	*spawner;
	trace_t		trace;
	vec3_t		vel, accel, end, RGB, work;
	float		grow, alpha, time, dis;

	alpha = Vector4to3( RGBA, RGB );

	// Calc how fast it should grow to reach drop size in the given amount of time
	grow = diameter / (float)((thinktime - cg.time) * 0.001);

	// Growing drop -- never moves
	FX_AddSprite( origin, NULL, NULL, 0.0, grow, alpha, alpha, RGB, RGB, 0.0, 0.0, thinktime - cg.time + cg.frametime, shader, 0 );

	// Ideally, zero should be used for vel...so just use something sufficiently close
	VectorSet( vel, 0, 0, -0.00001f );
	VectorSet( accel, 0, 0, -DRIP_ACCEL );

	// Find out where it will hit
	VectorMA( origin, 4, accel, end );
	gi.trace( &trace, origin, NULL, NULL, end, -1, MASK_SHOT );//ignore

	if ( trace.fraction < 1.0 )
	{
		VectorSubtract( trace.endpos, origin, end );
		dis = VectorNormalize( end );

		time = sqrt( 2  / DRIP_ACCEL * dis ) * 1000;	// Calculate how long the thing will take to travel that distance

		// Falling drop
		FX_AddParticle( NULL, origin, vel, accel, diameter, 0.0, alpha, alpha, RGB, RGB, 0.0, 0.0, time, 
						shader, 0, DripCallback );

		// Ok, so this is a little bit harsh, but all of this info needs to get to the spawner
		VectorSet( work, diameter, alpha, shader );

		spawner = FX_AddSpawner( trace.endpos, trace.plane.normal, work, RGB, time, 0, time + 100, 
									FXF_DELAY_SPAWN | FXF_SPAWN_ONCE, DripSplash, NULL );
	}
	else
	// Falling a long way so just send one that will fall for 2 secs, but don't spawn a splash
	{
		FX_AddSprite( origin, vel, accel, diameter, 0.0, alpha, alpha, RGB, RGB, 0, 0, 2000, shader );
	}
}

/*
======================
CG_ElectricFire

Creates an electric fire effect
======================
*/

void CG_ElectricFire( vec3_t origin, vec3_t normal )
{
	FXTrail	*particle;
	vec3_t	dir, direction, start, end;
	vec3_t	velocity, accel;
	float	scale, alpha;
	int		numSparks;

	AngleVectors( normal, normal, NULL, NULL);

	numSparks = 4 + (random() * 8.0f);
	
	for ( int i = 0; i < numSparks; i++ )
	{	
		scale = 0.3f + (random() *0.4);

		for ( int j = 0; j < 3; j ++ )
			dir[j] = normal[j] + (0.4f * crandom());
		
		VectorNormalize(dir);

		VectorMA( origin, -1.0f + ( random() * 2.0f ), dir, start );
		VectorMA( start, 2.0f + ( random() * 12.0f ), dir, end );

		FX_AddLine( start,
					end,
					1.0f,
					scale,
					0.0f,
					1.0f,
					0.0f,
					75.0f,
					cgs.media.sparkShader );
	}
 
	scale = 0.5f + (random() * 0.5f);

	VectorScale( normal, 300, velocity );
	VectorSet( accel, 0, 0, -600 );

	particle = FX_AddTrail( start,
							velocity,
							accel,
							6.0f,
							-24.0f,
							scale,
							-scale,
							1.0f,
							0.5f,
							0.0f,
							200.0f,
							cgs.media.sparkShader,
							FXF_BOUNCE );

	if ( particle == NULL )
		return;

	FXE_Spray( dir, 200, 200, 0.2f, 300, (FXPrimitive *) particle );

	VectorMA( origin, 1, normal, direction );
	VectorSet( velocity, 0, 0, 8 );

	for ( int i = 0; i < 3; i++)
	{
		scale = 6.0f + (random() * 8.0f);
		alpha = 0.1 + (random() * 0.4f);

		FX_AddSprite( direction, 
					velocity, 
					NULL, 
					scale,
					scale,
					alpha,
					0.0,
					random()*45.0f,
					0.0f,
					1000.0f,
					cgs.media.steamShader );

		VectorMA( velocity, 9.0, normal, velocity);
	}
}

/*
======================
CG_Teleporter

persistent teleporter fx for STASIS level
======================
*/

bool teleporter_think( FXPrimitive *fx, centity_t *ent )
{
	vec3_t	angvect;

	// Can generically update these . . even if the entity gets yanked.
	fx->UpdateAlpha();
	fx->UpdateScale();
	fx->UpdateRGB();

	// If the ent was somehow removed, it would be meaningless to continue any further.
	if ( !ent )
		return false;

	// Update the angle of the particle (position on the ellipsoid).
	VectorMA( fx->m_velocity, cg.frametime * 0.001, fx->m_acceleration, fx->m_velocity );

	// Update the actual position of the particle.
	AngleVectors( fx->m_velocity, angvect, NULL, NULL );
	fx->m_origin[0] = (4 * cos(cg.time * 0.004) + 20) * angvect[0] + ent->lerpOrigin[0];
	fx->m_origin[1] = (4 * sin(cg.time * 0.005) + 20) * angvect[1] + ent->lerpOrigin[1];
	fx->m_origin[2] = (4 * sin(cg.time * 0.006) + 30) * angvect[2] + ent->lerpOrigin[2];

	return true;
}

//------------------------------------------------------------------------------
void CG_Teleporter( centity_t *cent )
{
	vec3_t	vel, accel, angvect, org;

	// Velocity is actually the position on the sphere
	vel[ROLL]	= 0;
	vel[YAW]	= random() * 360.0;
	vel[PITCH]	= random() * 360.0;

	// Accel is actually the velocity around the sphere
	accel[ROLL] = 0;
	accel[YAW]	= crandom() * 180;
	accel[PITCH] = crandom() * 45;

	AngleVectors(vel, angvect, NULL, NULL);

	// Set the particle position
	org[0] = (4 * cos(cg.time * 0.004) + 20) * angvect[0] + cent->lerpOrigin[0];
	org[1] = (4 * sin(cg.time * 0.005) + 20) * angvect[1] + cent->lerpOrigin[1];
	org[2] = (4 * sin(cg.time * 0.006) + 30) * angvect[2] + cent->lerpOrigin[2];

	// Use a couple of different kinds to break up the monotony
	if ( rand() & 1 )
	{
		FX_AddParticle( cent, org, vel, accel, 1.0, 0.6f, 1.0, 0.3f,
						0.0, 0.0, 6000, cgs.media.ltblueParticleShader, 0, teleporter_think );
	}
	else
	{
		FX_AddParticle( cent, org, vel, accel, 1.0, 0.6f, 0.7f, 0.3f, 
						0.0, 0.0, 6000, cgs.media.purpleParticleShader, 0, teleporter_think );
	}
}

/*
======================
CG_ParticleStream

particle stream fx for STASIS level
======================
*/

bool particle_stream_think( FXPrimitive *fx, centity_t *ent )
{
	vec3_t old_org;

	// Make it flicker. . .always safe to do this
	fx->m_scale = random() * 12 + 2;
	fx->m_alpha = random() * 0.4 + 0.6;

	// If the ent was somehow removed, we don't want to continue any further.
	if ( !ent )
		return false;

	// Stash the old position so that we can draw a trailer line
	VectorCopy( fx->m_origin, old_org );

	// Update the position of the particle.
	fx->m_origin[0]  = cos(cg.time * 0.01 + fx->m_velocity[0]) * fx->m_velocity[1] + ent->lerpOrigin[0];
	fx->m_origin[1]  = sin(cg.time * 0.01 + fx->m_velocity[0]) * fx->m_velocity[1] + ent->lerpOrigin[1];
	fx->m_origin[2] += (fx->m_velocity[2] * cg.frametime * 0.001);

	FX_AddLine( fx->m_origin, old_org, 1.0f, 2.0f, -4.0f, 0.6f, 0.0, 500, cgs.media.IMOD2Shader );

	return true;
}

//------------------------------------------------------------------------------
void CG_ParticleStream( centity_t *cent )
{
	vec3_t	vel, org, dir;
	float	len, time;

	// This effect will currently only travel directly up or down--never sideways
	VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
	len = VectorNormalize( dir );

	// since the movement direction is limited, use the velocity var a bit more efficiently
	vel[0] = random() * 360;						// random position around the cylinder
	vel[1] = random() > 0.9  ? 20 : 6;				// random radius
	vel[2] = dir[2] * 120 + dir[2] * random() * 50;	// random velocity (up or down)

	// Set the particle position
	org[0] = cos(cg.time * 0.01 + vel[0]) * vel[1] + cent->lerpOrigin[0];
	org[1] = sin(cg.time * 0.01 + vel[0]) * vel[1] + cent->lerpOrigin[1];
	org[2] = cent->lerpOrigin[2];

	// Calculate how long the thing should live based on it's velocity and the distance it has to travel
	time = len / vel[2] * 1000;

	// Use a couple of different kinds to break up the monotony
	if ( rand() & 1 )
	{
		FX_AddParticle( cent, org, vel, NULL, 16, 0.0, 1.0, 1.0,
						0.0, 0.0, time, cgs.media.ltblueParticleShader, 0, particle_stream_think );
	}
	else
	{
		FX_AddParticle( cent, org, vel, NULL, 16, 0.0, 1.0, 1.0,
						0.0, 0.0, time, cgs.media.purpleParticleShader, 0, particle_stream_think );
	}
}

/*
======================
CG_ElectricalExplosion

Electrical Explosion
======================
*/

void smoke_puffs( vec3_t position, vec3_t dest, vec3_t dir, vec3_t user )
{
	vec3_t direc;

	direc[0] = crandom() * 7;
	direc[1] = crandom() * 7;
	direc[2] = random() * 6 + 8;

	FX_AddSprite( position, direc, NULL, 6.0f, 10.0f, 0.3f, 0.0f, 0.0f, 0.0f, 2200, cgs.media.steamShader );
}

//------------------------------------------------------------------------------
void electric_spark( vec3_t pos, vec3_t normal, vec3_t dir, vec3_t user )
{
	CG_Spark( pos, normal );
}

//------------------------------------------------------------------------------
void CG_ElectricalExplosion( vec3_t start, vec3_t dir, float radius )
{
	localEntity_t	*le;
	FXTrail			*particle;
	vec3_t			pos, temp, angles;
	int				i, numSparks;
	float			scale, dscale;

	// Spawn some delayed smoke
	FX_AddSpawner( start, dir, NULL, NULL, 150, 40, 9000, smoke_puffs );
	vectoangles( dir, angles );
	FX_AddSpawner( start, angles, NULL, NULL, 900, 800, 4000, FXF_DELAY_SPAWN, electric_spark );

	// Create the sparks for the explosion
	numSparks = 46 + (random() * 8.0f);
	 
	for ( i = 0; i < numSparks; i++ )
	{	
		scale = 0.2f + random();
		dscale = -scale*2;

		particle = FX_AddTrail( start,
								NULL,
								NULL,
								8.0f + random() * 6,
								-16.0f,
								scale,
								-scale,
								1.0f,
								0.0f,
								0.25f,
								700.0f,
								cgs.media.spark2Shader );
 
		if ( particle == NULL )
			return;

		FXE_Spray( dir, 200, 200, 0.3f, 500 + (rand() & 300), (FXPrimitive *) particle );
	}

	// Create some initial smoke puffs
	for (i = 0; i < 12; i++)
	{
		VectorCopy( dir, temp );
		temp[0] += crandom() * 0.5f;
		temp[1] += crandom() * 0.5f;
		temp[2] += crandom() * 0.5f;

		VectorMA( start, random() * 16 + 8, temp, pos );
		VectorScale( temp, random() * 4 + 5, temp );

		FX_AddSprite( pos, temp, NULL, 16.0, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2700 + random() * 600, cgs.media.steamShader );
	}

	// Now place a cool explosion model on top
	VectorSubtract( cg.refdef.vieworg, start, dir );
	VectorNormalize( dir );

	le = CG_MakeExplosion( start, dir, cgs.media.explosionModel, 6, cgs.media.electricalExplosionSlowShader, 500, qfalse, radius * 0.01f + ( crandom() * 0.3f)  );
	le->light = 150;
	
	le->refEntity.renderfx |= RF_NOSHADOW;

	VectorSet( le->lightColor, 0.8f, 0.8f, 1.0f );
}

/*
===========================
Laser

Create directed laser shot
===========================
*/

void CG_SmallSpark( vec3_t origin, vec3_t normal )
{
	vec3_t	dir, direction, start, end, velocity;
	float	scale;
	int		numSparks;

	AngleVectors( normal, normal, NULL, NULL );

	for ( int j = 0; j < 3; j ++ )
		normal[j] = normal[j] + (0.1f * crandom());

	VectorNormalize( normal );

	numSparks = 6 + (random() * 4.0f );
	
	for ( int i = 0; i < numSparks; i++ )
	{	
		scale = 0.1f + (random() *0.2f );

		for ( int j = 0; j < 3; j ++ )
			dir[j] = normal[j] + (0.7f * crandom());

		VectorMA( origin, 0.0f + ( random() * 0.5f ), dir, start );
		VectorMA( start, 1.0f + ( random() * 1.5f ), dir, end );

		FX_AddLine( start,
					end,
					1.0f,
					scale,
					0.0f,
					1.0f,
					0.7f,
					4.0f,
					cgs.media.sparkShader );
	}

	VectorMA( origin, 1, normal, direction );

	scale = 2.0f + (random() * 3.0f );
	float alpha = 0.6f + (random() * 0.4f );

	VectorSet( velocity, crandom() * 2, crandom() * 2, 8 + random() * 4 );
	VectorMA( velocity, 5, normal, velocity );

	FX_AddSprite(	direction, 
					velocity, 
					NULL, 
					scale,
					scale,
					alpha,
					0.0f,
					random() * 45.0f,
					0.0f,
					1000.0f,
					cgs.media.steamShader );
}

//------------------------------------------------------------------------------
void CG_FireLaser( vec3_t start, vec3_t end, vec3_t normal, vec4_t laserRGB, qboolean hit_ent )
{
	vec3_t	dir, right, up, angles, work, pos,
			sRGB, lRGB;
	float	scale = 1.0f, alpha;
	int		life = 0;

	if ( !(FX_DetailLevel( start, 16, 1200 ) ))
		return;

	// Orient the laser spray
	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	alpha = Vector4to3( laserRGB, lRGB );

	VectorMA( end, 0.5f, normal, pos );
	MakeNormalVectors( normal, right, up );
	
	VectorSet( sRGB, 1.0f, 0.8f, 0.8f );	

	FX_AddSprite( start, NULL, NULL, 
					1.75f, 1.0f, 
					alpha, 0.0f, 
					lRGB, lRGB, 
					0.0f, 
					0.0f, 
					200, 
					cgs.media.waterDropShader );

	FX_AddLine( start, end, 
					1.0f, 
					3.0f, 5.0f, 
					alpha, 0.0f, 
					lRGB, lRGB, 
					125, 
					cgs.media.whiteLaserShader );

	FX_AddLine( start, end, 
					1.0f, 
					0.3f, 5.0f, 
					random() * 0.4 + 0.4, 0.1f,
					125,
					cgs.media.whiteLaserShader );

	// Doing all of this extra stuff would look weird if it hits a player ent.
	if ( !hit_ent )
	{
		FX_AddQuad( pos, normal, NULL, NULL, 
					3.5f, 1.0f, 
					alpha, 0.0f, 
					lRGB, lRGB, 
					0.0f, 0.0f, 
					0.0f, 
					200, 
					cgs.media.waterDropShader );

		for ( int t=0; t < 2; t ++ )
		{
			VectorMA( pos, crandom() * 0.5f, right, work );
			VectorMA( work, crandom() * 0.5f, up, work );

			scale = crandom() * 0.5f + 1.75f;
			life = crandom() * 300 + 2100;

			VectorSet( sRGB, 1.0f, 0.7f, 0.2f );
			FX_AddQuad( work, normal, NULL, NULL, 
					scale, -0.1f, 
					1.0f, 0.0f, 
					sRGB, sRGB, 
					0, 0, 
					0, 
					life, 
					cgs.media.waterDropShader );
		}

		FX_AddQuad( pos, normal, NULL, NULL, 
					scale * 2.5f, 0.0f, 
					1.0f, 0.0f, 
					0, 0, 
					0, 
					life * 2, 
					cgs.media.smokeShader );
	
		vectoangles( normal, angles );
		CG_SmallSpark( end, angles );
	}
	else
	{
		// However, do add a little smoke puff
		FX_AddSprite( pos, NULL, NULL, 
						2.0f, 1.0f, 
						alpha, 0.0f, 
						lRGB, lRGB, 
						0.0f, 
						0.0f, 
						200, 
						cgs.media.waterDropShader );

		VectorMA( end, 1, normal, dir );
		scale = 1.0f + (random() * 3.0f);

		CG_Smoke( dir, normal, scale, 12.0f, cgs.media.steamShader );
	}
}

//------------------------------------------------------------------------------
void CG_AimLaser( vec3_t start, vec3_t end, vec3_t normal )
{
	vec3_t		lRGB = {1.0,0.0,0.0};

	// Beam
	FX_AddLine( start, end, 
					1.0f, 
					5.5f, 5.0f, 
					random() * 0.2 + 0.2, 0.1f,
					lRGB, lRGB,
					150,
					cgs.media.whiteLaserShader );

	FX_AddLine( start, end, 
					1.0f, 
					0.3f, 5.0f, 
					random() * 0.4 + 0.4, 0.1f,
					125,
					cgs.media.whiteLaserShader );

	// Flare at the start point
	FX_AddSprite( start, NULL, NULL, 
					1.5 + random() * 4, 0.0,
					0.1f,0.0, 
					0.0,
					0.0,
					100,
					cgs.media.borgEyeFlareShader );

	// endpoint flare
	FX_AddSprite( end, NULL, NULL, 
					2.5 + random() * 4, 0.0,
					0.1f,0.0, 
					0.0,
					0.0,
					100,
					cgs.media.borgEyeFlareShader );

	// oriented impact flare
	FX_AddQuad( end, normal, NULL, NULL, 
					1.5 + random() * 2, 0.0, 
					1.0, 0.0, 
					0.0, 0.0, 
					0.0, 
					120, 
					cgs.media.borgEyeFlareShader );
}
	
/*
======================
CG_TransporterStream

particle stream fx for forge level
The particles will accelerate up to the half-way point of the cylinder, then deccelerate til they hit their target
======================
*/

void CG_TransporterStream( centity_t *cent )
{
	vec3_t	vel, accel, dir, pos, right, up;
	float	len, time, acceleration, scale, dis, vf;

	VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
	len = VectorNormalize( dir );
	MakeNormalVectors( dir, right, up );

	for ( int t=0; t < 3; t++ )
	{
		// Create start offset within a circular radius
		VectorMA( cent->lerpOrigin, 8 * crandom(), right, pos );
		VectorMA( pos, 8 * crandom(), up, pos );

		acceleration = 80 + random() * 50;
		VectorScale( dir, acceleration, accel ); // acceleration vector
		VectorScale( dir, 0.0001, vel );		// Ideally, vel would be zero, so just make it really small

		dis = ( len * 0.8f );					// the two segs will be overlapping to cover up the middle

		// This is derived from dis = (vi)(t)  +  (1/2)(a)(t)^2 where the inital velocity (vi) = zero
		time = sqrt( 2  / acceleration * dis );	// Calculate how long the thing will take to travel that distance
		
		scale = 1.5f + random() * 4;

		// These will spawn at the base and accelerate towards the middle
		if ( rand() & 1 )
		{
			FX_AddSprite( pos, vel, accel, 
						scale, 0.0f, 
						1.0f, 0.0f, 
						0.0f, 
						0.0f, 
						time * 1000, 
						cgs.media.orangeParticleShader );
		}
		else
		{
			FX_AddSprite( pos, vel, accel, 
						scale, 0.0f, 
						1.0f, 0.0f, 
						0.0f, 
						0.0f, 
						time * 1000, 
						cgs.media.dkorangeParticleShader );
		}

		// These will be spawned somewhere in the middle and deccelerate till they reach the end of their target
		VectorMA( pos, len - dis, dir, pos );
		VectorScale( accel, -1, accel );

		vf = sqrt( 2 * dis * acceleration ); // calculate the how fast it would be moving at the end of its path
		VectorScale( dir, vf, vel );		//	this will be the _initial_ velocity for those starting in the middle

		if ( rand() & 1 )
		{
			FX_AddSprite( pos, vel, accel, 
						scale, 0.0f, 
						0.0f, 1.0f, 
						0.0f, 
						0.0f, 
						time * 1000, 
						cgs.media.orangeParticleShader );
		}
		else
		{
			FX_AddSprite( pos, vel, accel, 
						scale, 0.0f, 
						0.0f, 1.0f, 
						0.0f, 
						0.0f, 
						time * 1000, 
						cgs.media.dkorangeParticleShader );
		}
	}
}

/*
-------------------------
CG_Smoke
-------------------------
*/

void CG_Smoke( vec3_t origin, vec3_t dir, float radius, float speed, qhandle_t shader, int flags)
{
	vec3_t	velocity, accel;

	for ( int i = 0; i < 3; i++ )
	{
		velocity[i] = dir[i] + ( 0.2f * crandom());
	}

	VectorScale( velocity, speed, velocity );
	VectorScale( velocity, -0.25f, accel );
	accel[2] = random() * 12.0f + 6.0f;

	FX_AddSprite(	origin,
					velocity, 
					accel, 
					radius + (crandom() * radius * 0.5f ),  
					radius + (crandom() * radius), 
					0.9f + crandom(), 
					0.0f,
					16.0f + random() * 45.0f,
					0.5f,
					2000, 
					shader,
					flags );
}

/*
-------------------------
CG_ExplosionTrail
-------------------------
*/

bool explosionTrailThink( FXPrimitive *fx, centity_t *ent )
{
	localEntity_t	*le=0;
	vec3_t			direction, origin, new_org, angles, dir;
	trace_t			trace;
	float			scale;
	int				i;
	qboolean		remove = qfalse;

	VectorCopy( fx->m_origin, origin );
	fx->UpdateOrigin();
	VectorCopy( fx->m_origin, new_org );

	CG_Trace( &trace, origin, NULL, NULL, new_org, -1, CONTENTS_SOLID );

	if ( trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid )
	{
		// The effect hit something, presumably a barrier, so kill it
		// When the effect gets killed like this, it dies quickly and looks a bit thin.
		// Maybe something else should be done as well...
		remove = qtrue;
		//FIXME: FX_RemoveEffect( fx );
		return false;
	}

	scale = 80 * 0.03f;

	VectorSubtract( new_org, origin, dir );
	VectorNormalize( dir );
	vectoangles( dir, angles );
		
	//Orient the explosions to face the camera
	VectorSubtract( cg.refdef.vieworg, origin, direction );
	VectorNormalize( direction );

	for ( i = 0; i < 3 + (int)remove * 6; i++)
	{
		angles[2] = crandom() * 360;

		AngleVectors( angles, NULL, dir, NULL );
		VectorMA( origin, random() * 50.0f, dir, new_org );

		le = CG_MakeExplosion( new_org, direction, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 400 + (int)remove * 800, qfalse, random() * 1.0 + 0.8 );//random() * 1.0 + 1.0 );
	}

	le->light = 150;
	VectorSet( le->lightColor, 64, 192, 255 );

	//Shake the camera and damage everything in an area
	CG_ExplosionEffects( origin, 3.0f, 600 );
	G_RadiusDamage( origin, ent->gent, 150, 80, NULL, MOD_UNKNOWN );

	return true;
}

//------------------------------------------------------------------------------
void CG_ExplosionTrail( centity_t *cent )
{
	vec3_t			dir;
	float			len;

	VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, dir );
	len = VectorNormalize( dir );
	VectorScale( dir, 325, dir );

	FX_AddParticle( cent, cent->lerpOrigin, dir, NULL, 16, 0.0, 1.0, 1.0,
						0.0, 0.0, 6000, cgs.media.ltblueParticleShader, FXF_NODRAW, explosionTrailThink );
}

/*
----------------------
CG_BorgEnergyBeam

A scanning type beam
----------------------
*/

void CG_BorgEnergyBeam( centity_t *cent )
{
	vec3_t		normal, angles, base, dir, dir2, rgb;
	float		len, alpha;

	VectorSubtract( cent->currentState.origin2, cent->lerpOrigin, normal );
	len = VectorNormalize( normal );
	vectoangles( normal, angles );
	alpha = Vector4to3( cent->gent->startRGBA, rgb );

/*	// Code to make the thing "snap" when it's doing the beam slices
	if ( abs( cent->gent->pos2[0] ) >= cent->gent->radius )
	{
		// Snap back to start and move to the next slice
		cent->gent->pos2[0] = cent->gent->radius;
		cent->gent->pos2[1] -= ( cg.frametime * 0.0003 * cent->gent->speed );
	}

	// The slice has moved past the end so snap back to the first slice position
	if ( abs( cent->gent->pos2[1] ) >= cent->gent->radius )
	{
		cent->gent->pos2[1] = cent->gent->radius;
	}

	// Always move across the slice
	cent->gent->pos2[0] -= ( cg.frametime * 0.001 * cent->gent->speed );
*/

	if ( cent->gent->spawnflags & 2 )
	{
		// Trace a cone
		angles[2] = cent->gent->angle;
	}

	AngleVectors( angles, NULL, dir, dir2 );

	if ( cent->gent->spawnflags & 2 )
	{
		// Cone
		VectorMA( cent->currentState.origin2, cent->gent->radius, dir, base );
	}
	else
	{
		// Swinging pendulum
		VectorMA( cent->currentState.origin2, cent->gent->radius * ( sin( cent->gent->angle * 0.03f )), dir, base );
		VectorMA( base, cent->gent->radius * ( cos( cent->gent->angle * 0.003f )), dir2, base );
		// Do "snapping" beam slices
//		VectorMA( cent->currentState.origin2, cent->gent->pos2[0], dir, base );
//		VectorMA( base, cent->gent->pos2[1], dir2, base );
	}

	// Main trace laser
	FX_AddLine( cent->lerpOrigin, base, 64, 0.8f, 5.0f, alpha, 0.0, rgb, rgb, 120, cgs.media.whiteLaserShader );
	// Faint trail at base.  Is this really adding anything useful?
	FX_AddLine( cent->gent->pos1, base, 1, 1.0, 2.0, alpha * 0.2, 0.0, rgb, rgb, 1000, cgs.media.whiteLaserShader );
	// Faint trace cone, adds a bit of meat to the effect
	FX_AddTri( cent->lerpOrigin, cent->gent->pos1, base, alpha * 0.2, 0.0, rgb, rgb, 800, cgs.media.solidWhiteShader );
	// Laser impact point
	FX_AddSprite( base, NULL, NULL, random() * 2, 0.0, alpha, 0.0, rgb, rgb, 0.0, 0.0, 100, cgs.media.waterDropShader );

	VectorCopy( base, cent->gent->pos1 );
	cent->gent->angle += cent->gent->speed * 0.08f;
}

/*
----------------------
CG_ShimmeryThing

Creates column or cone of shimmering lines
Kind of looks like a teleporter effect
----------------------
*/

void CG_ShimmeryThing( vec3_t start, vec3_t end, float radius, qboolean taper )
{
	vec3_t	normal, angles, base, top, dir;
	float	len;

	VectorSubtract( end, start, normal );
	len = VectorNormalize( normal );
	vectoangles( normal, angles );

	for ( int i=0; i < 2; i++)
	{
		// Spawn the shards of light around a cylinder
		angles[2] = crandom() * 360;
		AngleVectors( angles, NULL, dir, NULL );

		// See if the effect should be tapered at the top
		if ( taper )
		{
			VectorMA( start, radius * 0.25f, dir, top );
		}
		else
		{
			VectorMA( start, radius, dir, top );
		}

		VectorMA( end, radius, dir, base );

		// Use a couple of different kinds to break up the monotony..
		if ( rand() & 1 )
		{
			FX_AddLine( top, base, 1.0f, len * 0.008f, len * 0.19f, 0.3f, 0.0f, random() * 200 + 600, cgs.media.ltblueParticleShader );
		}
		else
		{
			FX_AddLine( top, base, 1.0f, len * 0.008f, len * 0.19f, 0.2f, 0.0f, random() * 200 + 600, cgs.media.spark2Shader );
		}
	}
}

/*
-------------------------
CG_ShimmeryThing_Spawner
-------------------------
*/

void CG_Shimmer( vec3_t position, vec3_t dest, vec3_t dir, vec3_t other )
{
	CG_ShimmeryThing( position, dest, other[0], (qboolean) other[1] );
}

void CG_ShimmeryThing_Spawner( vec3_t start, vec3_t end, float radius, qboolean taper, int duration )
{
	vec3_t	packed = { radius, (float) taper, 0 };

	FX_AddSpawner( start, end, NULL, packed, 100, 0, duration, (void *) CG_Shimmer, NULL, 512 );
}

/*
----------------------
CG_Borg_Bolt

Yellow bolts that spark when the endpoints get close together
----------------------
*/
void CG_Borg_Bolt( centity_t *cent )
{
	vec3_t	diff, neworg, start, end;
	float	len;

	if (!cent->gent->enemy){
		return;//we lost him
	}
	VectorCopy( cent->gent->enemy->currentOrigin, end );
	
	if ( cent->gent->target2 )
	{
		VectorCopy( cent->gent->chain->currentOrigin, start );
	}
	else
	{
		VectorCopy( cent->lerpOrigin, start );
	}

	// Get the midpoint of the seg
	VectorSubtract( end, start, diff );
	len = VectorNormalize( diff );
	VectorMA( start, len * 0.5, diff, neworg );

	// If the length is pretty short, then spawn a glow spark
	if ( len > 0 && len < 12 )
	{
		int		ct;
		vec3_t	angles, dir;
		FXTrail	*particle;

		FX_AddSprite( neworg, NULL, NULL, random() * (128 / len) + 12, 16.0, 0.6f, 0.0, 0.0, 0.0, 300, 
					cgs.media.yellowParticleShader );

		vectoangles( dir, angles );

		ct = 12 - len;

		// fun sparks
		for ( int t=0; t < ct; t++ )
		{
			angles[1] = random() * 360;
			AngleVectors( angles, dir, NULL, NULL );
			dir[2] = random() * 0.3f;

			particle = FX_AddTrail( neworg, NULL, NULL, 8.0f + random() * 6, -16.0f, 1, -1,
							1.0f, 0.0f, 0.25f, 700.0f, cgs.media.yellowParticleShader );

			if ( particle == NULL )
				return;

			FXE_Spray( dir, 100, 150, 0.5f, 300 + (rand() & 300), (FXPrimitive *) particle );
		}

		// If it's really short, spark and make a noise.  Tried this without the if (len>0... and it was way
		//	too obnoxious
		if ( len <= 5 )
		{
			cgi_S_StartSound( neworg, 0, 0, cgi_S_RegisterSound( "sound/enemies/borg/borgtaser.wav") );
		}
	}

	// Use this to scale down the width of the bolts.  Otherwise, they will look pretty fairly nasty when they
	//	get too short.
	len = len / 32;

	FX_AddElectricity( start, end, 1.0, len, 5.0, 1.0, 0.0, 200, cgs.media.yellowBoltShader );

	if ( rand() & 1 )
	{
		FX_AddElectricity( end, start, 1.0, len, 5.0, 1.0, 0.0, 200, cgs.media.yellowBoltShader );
	}
}


/*
----------------------
CG_StasisTeleporter

New stasis teleporter effect
----------------------
*/
void CG_StasisTeleporter( vec3_t origin, float radius, qboolean exit )
{
	vec3_t		up = { 0,0,1 }, sRGB, eRGB, org;
	float		alpha = sin( cg.time * 0.005 ) * 0.2 + 0.3;
	FXCylinder	*fx;

	if ( exit )
	{
		// these are the exit colors
		VectorSet( sRGB, 0.0f, 0.8f, 1.0f ); // light-blue
		VectorSet( eRGB, 0.0f, 0.0f, 1.0f ); // blue
	}
	else
	{
		// these are the colors for the active type teleporters
		VectorSet( sRGB, 0.5f, 0.2f, 1.0f ); // bluish-purple
		VectorSet( eRGB, 1.0f, 0.0f, 0.4f ); // redish-purple
	}

	// Designers wanted the effects shifted down a bit, but still have the fx entity such that it gets placed so that it sits perfectly
	//	on top of the jump pads for ease of placement.
	VectorCopy( origin, org );
	org[2] -= 16;

	fx = FX_AddCylinder( org, up, 42.0f, 256.0f, 36.0f, 16.0f, 52.0f, 32.0f, alpha, 0.0f, sRGB, eRGB, 100, cgs.media.psychicShader, 0.25f );

	if ( fx == NULL )
		return;
	
	fx->SetSTScale( 1.0f );
	fx->SetFlags( FXF_WRAP );

	if ( exit )
	{
		// We are an exit type teleporter, so skip all of the proximity stuff
		return;
	}

	if ( radius < 256 )
	{
		vec3_t end, mid, vel, accel;
		float scale = ( 256 - radius ) * ( random() * 0.5f + 0.5f );

		VectorMA( origin, 80, up, end );

		FX_AddLine( org, end, 1, 1, scale * 4, 1.0, 0.0, 120, cgs.media.blueParticleShader );

		// take the average of the two points to get the midpoint
		VectorAdd( org, end, mid );
		VectorScale( mid, 0.5f, mid );

		for ( int i = 0; i < 2; i++ )
		{
			VectorSet( vel, crandom(), crandom(), crandom() * 2 );
			VectorScale( vel, random() * 24 + scale * 0.25f, vel );
			VectorScale( vel, -0.2f, accel );

			if ( rand() & 1 )
			{
				FX_AddSprite( mid, vel, accel, scale * 0.1f, scale * -0.2f, 0.3f, 0.0f, 0.0, 0.0, 500, cgs.media.ltblueParticleShader );
			}
			else
			{
				FX_AddSprite( mid, vel, accel, scale * 0.1f, scale * -0.2f, 0.3f, 0.0f, 0.0, 0.0, 500, cgs.media.purpleParticleShader );
			}
		}
	}
}

/*
----------------------
CG_StasisMushrooms

Ambient effect for when 
the stasis mushrooms get used.
----------------------
*/
bool MushroomThink( FXPrimitive *fx, centity_t *ent )
{
	fx->UpdateOrigin();
	fx->UpdateAlpha();
	fx->UpdateRGB();
	fx->UpdateScale();

	for ( int i = 0; i < 3; i++ )
	{
		fx->m_velocity[i] += crandom() * 300.0f * cg.frametime * 0.001f;
	}

	return true;
}

void CG_StasisMushrooms( vec3_t origin, int count )
{
	vec3_t	up, org;
	float	scale, alpha;

	for ( int i = 0; i < count; i++ )
	{
		// Make the thing move mostly up
		VectorSet( up, crandom() * 12.0f, crandom() * 12.0f, random() * 12.0f + 8.0f );
		VectorSet( org, origin[0] + crandom() * 4.0f, origin[1] + crandom() * 4.0f, origin[2] - 4.0f );

		scale = random() + 1.0f;
		alpha = random() * 0.4f + 0.3f;

		// Use a couple of different kinds to break up the monotony
		if ( rand() & 1 )
		{
			FX_AddParticle( NULL, org, up, NULL, scale, -1.0f, alpha, 0.1f, 0.0f, 0.0f, 600 + random() * 600, 
				cgs.media.ltblueParticleShader, 0, MushroomThink );
		}
		else
		{
			FX_AddParticle( NULL, org, up, NULL, scale, -1.0f, alpha, 0.1f, 0.0f, 0.0f, 600 + random() * 600, 
				cgs.media.blueParticleShader, 0, MushroomThink );
		}
	}
}

/*
----------------------
CG_DreadnoughtBeamGlow

Focusing beam effect
----------------------
*/
void CG_DreadnoughtBeamGlow( vec3_t origin, vec3_t normal, float radius )
{
	FX_AddQuad( origin, normal, NULL, NULL, radius + random() * 10, crandom() * 20, 1.0f, 0.1f, random() * 360, crandom() * 20, 0, 300, 
					cgs.media.bigBoomShader );
}

/*
----------------------
CG_MagicSmoke

Smoke effect for reaver
popping out of vats
----------------------
*/

void CG_MagicSmoke( vec3_t origin )
{
	int		i;
	float	v;
	vec3_t	org, vel, accel;

	for ( i = 0; i < 48; i++ )
	{
		org[0] = origin[0] + crandom() * 16;
		org[1] = origin[1] + crandom() * 16;
		org[2] = origin[2] + crandom() * 40;

		VectorSet( vel, crandom(), crandom(), crandom() );
		v = random() * 32 + 32;
		VectorScale( vel, v, vel );

		VectorScale( vel, -3, accel );

		FX_AddSprite( org, vel, accel, random() * 32 + 32, -(random() * 32 + 8), 1.0f, 0.0f, crandom() * 360, 0, random() * 600 + 400, cgs.media.steamShader );
	}
}

void CG_FountainSpurt( vec3_t org )
{
	int			i, t;
	vec3_t		org1, org2, cpt1, cpt2;
	vec3_t		dir, dir2;
	vec3_t		rgb = { 0.4f, 0.7f, 0.8f };
	FXBezier	*fxb;

	// offset table, could have used sin/cos, I suppose
	const float m[][2] = { 
						1, 0,
						0, 1,
						-1, 0,
						0, -1 };

	// The origin shouldn't be in solid, otherwise the ent won't think.  So, place the spawner above
	//	the solid object, then move the spout spawn points down to where they should be.
	org[2] -= 56; // magic number stuff

	// Create four spouts
	for ( i = 0; i < 4; i++ )
	{
		// Move the spout out from the exact center
		VectorCopy( org, org1 );
		org1[0] += 35 * m[i][0];
		org1[1] += 35 * m[i][1];

		// Create our Bezier path control points
		VectorSet( cpt1, 50 * m[i][0], 50 * m[i][1], 0 );
		VectorAdd( org1, cpt1, cpt1 );

		VectorSet( cpt2, 60 * m[i][0], 60 * m[i][1], -78 );
		VectorAdd( org1, cpt2, cpt2 );

		// Create the second endpoint--for now just try and use the last control point 
		VectorCopy( cpt2, org2 );

		// Add the main spout
		fxb = FX_AddBezier( org1, org2, cpt1, cpt2, NULL, NULL, NULL, NULL, 4, 15, 0.5f, 0.3f, 90, 
			cgs.media.fountainShader, FXF_WRAP );

		if ( fxb )
			fxb->SetSTScale( 0.7f );

		// Add a hazy faint spout
		fxb = FX_AddBezier( org1, org2, cpt1, cpt2, NULL, NULL, NULL, NULL, 10, 15, 0.1f, 0.0f, 200, 
			cgs.media.fountainShader, FXF_WRAP );

		if ( fxb )
			fxb->SetSTScale( 0.7f );

		// Create misty bits at the impact point
		VectorSet( dir, crandom(), crandom(), crandom() + 4 ); // always move mostly up
		VectorScale( dir, random() * 3 + 2, dir );
		FX_AddSprite( org2, dir, NULL, 20, -8, 0.3f, 0.0, 0, 0, 600, cgs.media.steamShader );

		// ripple shader
		VectorSet( dir, 0, 0, 1 ); // normal
		VectorSet( dir2, crandom() * 8, crandom() * 8, 0 ); // random drift
		FX_AddQuad( org2, dir, dir2, dir2, 14.0f, 6.0f + random() * 16.0f, 0.2f, 0.0f, 
					crandom() * 50, 0, 0, 800, cgs.media.rippleShader );

		// Spray from nozzle
		for ( t = 0; t < 2; t++ )
		{
			VectorSet( dir, 45 * m[i][0] + crandom() * 12, 45 * m[i][1] + crandom() * 12, crandom() * 16 );
			VectorSet( dir2, -5 * m[i][0], -5 * m[i][1], -95 );
			FX_AddSprite( org1, dir, dir2, 0.9f, 0.0f, 0.7f, 0.1f, rgb, rgb, 0.0f, 0.0f, 400.0f, cgs.media.waterDropShader );
		}

		// Impact splashes
		for ( t = 0; t < 3; t++ )
		{
			VectorCopy( org2, org1 );
			org1[0] += crandom() * 2;
			org1[1] += crandom() * 2;
			VectorSet( dir, m[i][0] * 14 + crandom() * 16, m[i][1] * 14 + crandom() * 16, 50 + random() * 50 );
			VectorSet( dir2, 0, 0, -250 );
			FX_AddSprite( org1, dir, dir2, 1.1f, -0.4f, 0.7f, 0.1f, rgb, rgb, 0.0f, 0.0f, 400.0f, cgs.media.waterDropShader );
		}
	}
}