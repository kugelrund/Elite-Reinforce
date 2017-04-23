// Hunter Seeker weapon effects

#include "cg_local.h"
#include "fx_public.h"

#define	SMOKE_TRAIL_MIN_DISTANCE	2

//------------------------------------------------
void FX_BotRocketThink( centity_t *cent, const struct weaponInfo_s *weapon )
//------------------------------------------------
{
	vec3_t		vel, diff, org;
	float		len;
	FXSprite	*fx;

	// Make a smoke trail that's reasonably consistent and not so much based on frame rate.
	VectorSubtract( cent->lerpOrigin, cent->gent->pos1, diff );
	len = VectorNormalize( diff );

	if ( len > SMOKE_TRAIL_MIN_DISTANCE )
	{
		// smoke drift
		VectorSet( vel, crandom() * 24, crandom() * 24, crandom() * 12 );

		for ( int i = 0 ; i < len; i += SMOKE_TRAIL_MIN_DISTANCE )
		{
			VectorMA( cent->lerpOrigin, -6 - i, diff, org );

			// glowing crap
			if ( rand() & 1 )
				FX_AddSprite( org, NULL, NULL, random() * 6 + 6, -48, 0.5, 0.0, random() * 360, 0, 100, cgs.media.yellowParticleShader );
			else
				FX_AddSprite( org, NULL, NULL, random() * 6 + 6, -48, 0.5, 0.0, random() * 360, 0, 100, cgs.media.dkorangeParticleShader );

			// smoke
			VectorMA( cent->lerpOrigin, -6 - i, diff, org );
			VectorSet( vel, crandom() * 12, crandom() * 12, 0 );
			fx = FX_AddSprite( org, vel, vel, random() * 3 + 1, random() * 18 + 12, 0.3 + random() * 0.2, 0.0, random() * 360, 0, 400,cgs.media.steamShader );

			if ( fx == NULL )
				return;

			fx->SetRoll( random() * 360 );
		}

		VectorCopy( cent->lerpOrigin, cent->gent->pos1 );
	}
}

void FX_BotRocketHitPlayer( vec3_t origin, vec3_t dir )
//------------------------------------------------
{
	FX_BotRocketHitWall( origin, NULL );
	//Sound
//	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_BOT_ROCKET].missileHitSound );
}

void FX_BotRocketHitWall( vec3_t origin, vec3_t normal )
//------------------------------------------------
{
	vec3_t			dir;
	localEntity_t	*le;

	//Orient the explosions to face the camera
	VectorScale( cg.refdef.viewaxis[0], -1, dir );

	// Add an explosion and tag a light to it
	le = CG_MakeExplosion( origin, dir, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 600, qfalse, 0.6f + (random()*0.3f) );
	le->light = 150;
	le->refEntity.renderfx |= RF_NOSHADOW;
	VectorSet( le->lightColor, 1.0f, 1.0f, 0.6f );

	CG_Chunks( 0, origin, dir, 256, 8, MAT_METAL, 0, 0.15f );

	//Sound
	cgi_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cg_weapons[WP_BOT_ROCKET].missileHitSound );
}

void FX_HunterSeekerThruster( vec3_t origin, vec3_t normal, qboolean dead )
//------------------------------------------------
{
	vec3_t	dir, org, RGB1, RGB2;
	float	len = random() * 12.0f + 14.0f;
	float	alpha = random() * 0.2f + 0.1f;

	if ( !dead || Q_irand(0,12) > 5 )
	{
		VectorSet( RGB1, 0.3f, 0.7f + random() * 0.1f, 0.9f + random() * 0.1f );
		VectorSet( RGB2, 0.1f, 0.3f + random() * 0.1f, 0.7f + random() * 0.1f );

		VectorCopy( normal, dir );

		// Add a subtle, random flutter to the cone direction
		dir[0] += crandom() * 0.02;
		dir[1] += crandom() * 0.02;
		dir[2] += crandom() * 0.02;

		VectorMA( origin, 1, normal, org );

		FX_AddCylinder( org, dir, len * 0.07f, 0, len * 0.16f, 0, len * 0.40f, 0, 
						alpha * 3.5f, alpha * 3.5f, RGB1, RGB1, 1, cgs.media.plasmaShader, 0.3f );
		FX_AddCylinder( org, dir, len * 0.27f, 0, len * 0.16f, 0, len * 0.28f, 0, 
						alpha * 1.75f, alpha * 1.75f, RGB1, RGB1, 1, cgs.media.plasmaShader, 0.2f );
		FX_AddCylinder( org, dir, len * 0.9f, 0, len * 0.12f, 0, len * 0.02f, 0, 
						alpha, alpha, RGB2, RGB2, 1, cgs.media.plasmaShader, 0.1f );
	}

	if ( dead )
	{
		len = random() * 24 + 8;

		// Cheesy smoke
		VectorScale( dir, len, dir );
		FX_AddSprite( origin, dir, NULL, 1.0f, 4.0f + random() * 8.0f, 0.5f, 0.0f, crandom() * 90, 0, 1000 + random() * 500, cgs.media.steamShader );

		// Cheesy sparks
		if ( Q_irand( 0,12 ) > 11 )
		{
			VectorSet( dir, crandom(), crandom(), crandom() );

			len = random() * 160 + 50;
			VectorScale( dir, len, dir );
			dir[2] += 96;

			VectorSet( org, 0,0, -640 );
			FX_AddTrail( origin, dir, org, 2 + random() * 6, -12, 0.3f + random() * 0.4f, 0.0, 1.0f, 0.5f, 0.2f, 400, cgs.media.sparkShader );
		}
	}
}

void FX_BotTurretBolt( vec3_t start, vec3_t end, vec3_t dir )
//------------------------------------------------
{
	vec3_t	control1, control2, control1_vel, control2_vel;
	vec3_t	direction, org, vr, vu, RGB1;
	float	len;

	VectorSet( RGB1, 1.0f, 0.9f, 0.2f );

	FX_AddLine( start, end, 1.0f, 1.0f, -5.0f, 1.0f, 1.0f, 50.0f, cgs.media.whiteLaserShader );
	FX_AddLine( start, end, 1.0f, 2.0f, -10.0f, 1.0f, 1.0f, RGB1, RGB1, 50.0f, cgs.media.whiteLaserShader );
	
	VectorSubtract( end, start, direction );
	len = VectorNormalize( direction );

	MakeNormalVectors( direction, vr, vu );

	VectorMA( end, -5, direction, org );
	FX_AddSprite( org, NULL, NULL, 38.0f, -500.0f, 1.0f, 1.0f, 0.0f, 0.0f, 25, cgs.media.bigBoomShader );
	FX_AddSprite( start, NULL, NULL, 10.0f, -50.0f, 1.0f, 1.0f, 0.0f, 0.0f, 200, cgs.media.sunnyFlareShader );

	VectorMA(start, len * 0.66f, direction, control1 );
	VectorMA(start, len * 0.33f, direction, control2 );

	vectoangles( direction, control1_vel );
	control1_vel[ROLL] = 180;
	AngleVectors( control1_vel, NULL, NULL, control1_vel );

	vectoangles( direction, control2_vel );
	control2_vel[ROLL] = 180;
	AngleVectors( control2_vel, NULL, NULL, control2_vel );

	VectorScale( control1_vel, -30.0f + (-100.0f * random()), control1_vel );
	VectorScale( control2_vel, 20.0f + (50.0f * random()), control2_vel );

	FX_AddBezier( start, end, 
					control1, control2, control1_vel, control2_vel, NULL, NULL, 
					1.0f,				//scale
					2.0f,				//dscale
					0.15f,				//alpha
					0.0f,				//dalpha
					RGB1,
					RGB1,
					300.0f,				//killtime
					cgs.media.whiteLaserShader );
} 

void FX_BotExplosions( vec3_t origin, vec3_t dir, vec3_t accel, vec3_t user )
//------------------------------------------------
{
	vec3_t			view, org; 
	localEntity_t	*le;

	//Orient the explosions to face the camera
	VectorScale( cg.refdef.viewaxis[0], -1, view );

	for ( int i = 0; i < 3; i++ )
	{
		org[i] = origin[i] + crandom() * 28.0f;
	}
	// Add an explosion and tag a light to it
	le = CG_MakeExplosion( org, view, cgs.media.explosionModel, 6, cgs.media.surfaceExplosionShader, 600, qfalse, 0.6f + (random()*0.5f) );

	if ( rand() & 3 )
	{
		le->light = 150;
		le->refEntity.renderfx |= RF_NOSHADOW;
		VectorSet( le->lightColor, 1.0f, 1.0f, 0.6f );
	}
}

//------------------------------------------------
void FX_ChunkSmokeTrails( vec3_t origin, vec3_t angles, vec3_t vel, vec3_t scale )
{
	vec3_t dir;

	VectorSet( dir, crandom() * 12, crandom() * 12, random() * 16 + 8 );
	FX_AddSprite( origin, dir, NULL, 
					scale[0], scale[0] * 4, 
					1.0, 0.0, 
					crandom() * 90, 0, 
					1000, 
					cgs.media.smokeShader, FXF_USE_ALPHA_CHAN );

	VectorSet( dir, crandom() * 8, crandom() * 8, random() * 16 + 8 );
	FXSprite * fx = FX_AddSprite( origin, dir, NULL, 
					scale[0] * (1 + random()), scale[0] * 3 + random() * 2, 
					0.3f, 0.0f, 
					crandom() * 90, 0, 
					350, 
					cgs.media.splosionShader ); 

	if ( fx )
		fx->SetRoll( random() * 360 );

	scale[0] *= 0.98f;
}

#define CHUNK_SPEED		230
#define CHUNK_SCALE		1.0f

//------------------------------------------------
void FX_BotExplode( centity_t *cent )
{
	int				i, j;
	float			val, scale;
	vec3_t			norm={0,0,1};
	vec3_t			dir, work;
	localEntity_t	*le;
	refEntity_t		*re;

	FX_AddSpawner( cent->lerpOrigin, NULL, NULL, NULL, 75, 50, 600, 0, FX_BotExplosions, NULL, 1024 );

	if ( !cent->gent->owner )
		return;

	// Adjust the scale
	if ( ( !_stricmp( cent->gent->owner->NPC_type, "warriorbot" ) ) || ( !_stricmp( cent->gent->owner->NPC_type, "warriorbot_boss" ) ) )
	{
		scale = 0.8f;
		cgi_S_StartSound( cent->lerpOrigin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.warbotExplodeSounds[Q_irand(0,1)] );
	}
	else 
	{
		scale = 0.3f;
		cgi_S_StartSound( cent->lerpOrigin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.scoutbotExplodeSounds[Q_irand(0,2)] );
	}

	//Chunks
	for ( i = 0; i < 9; i++ )
	{
		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		le->leType = LE_FRAGMENT;
		le->endTime = cg.time + 5000;

		VectorCopy( cent->lerpOrigin, re->origin );

		for ( j = 0; j < 2; j++ )
		{
			re->origin[j] += crandom() * 16;
		}
		VectorCopy( re->origin, le->pos.trBase );

		//Velocity
		for ( j = 0; j < 3; j++ )
		{
			dir[j] = norm[j] + crandom() * 0.6f;
		}

		val = Q_flrand( CHUNK_SPEED * 0.5f, CHUNK_SPEED * 1.5f );
		VectorScale( dir, val, le->pos.trDelta );

		//Angular Velocity
		VectorSet( le->angles.trBase, random() * 360, random() * 360, random() * 360 );
		for ( j = 0; j < 3; j++ )
		{
			le->angles.trDelta[j] = Q_flrand(64.0f, 360.0f) * ( crandom() < 0 ? -1 : 1 );
		}

		AxisCopy( axisDefault, re->axis );

		le->radius = Q_flrand( CHUNK_SCALE * 0.7f, CHUNK_SCALE * 1.3f ) * scale;

		re->nonNormalizedAxes = qtrue;
		re->hModel = cgs.media.warriorBotChunks[(i>>1)];
		
		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;
		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->bounceFactor = 0.2f + random() * 0.3f;
		le->leFlags |= LEF_TUMBLE;
		le->ownerGentNum = NULL;//cent->gent->owner->s.number;

		// Make sure that we have the desired start size set
		for ( j = 0; j < 3; j++ )
		{
			VectorScale( le->refEntity.axis[j], le->radius, le->refEntity.axis[j] );
		}

		VectorSet( work, le->radius * 30, 0, 0 );
		if ( work[0] > 20 )
			work[0] = 20;

		FX_AddSpawner( re->origin, NULL, NULL, work, 20, 5, 3000, 0, FX_ChunkSmokeTrails, le, 1024 );
	}
}

