//NPC_combat.cpp
#include "b_local.h"

extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );
extern void G_SetEnemy( gentity_t *self, gentity_t *enemy );
extern qboolean NPC_CheckDisguise( gentity_t *ent );
extern qboolean NPC_CheckLookTarget( gentity_t *self );
extern void NPC_ClearLookTarget( gentity_t *self );

void G_ClearEnemy (gentity_t *self)
{
	NPC_CheckLookTarget( self );

	if ( self->enemy )
	{
		if(	self->client && self->client->renderInfo.lookTarget == self->enemy->s.number )
		{
			NPC_ClearLookTarget( self );
		}

		if ( self->NPC && self->enemy == self->NPC->goalEntity )
		{
			self->NPC->goalEntity = NULL;
		}
		//FIXME: set last enemy?
	}

	self->enemy = NULL;
}

/*
-------------------------
NPC_AngerAlert
-------------------------
*/

#define	ANGER_ALERT_RADIUS			512
#define	ANGER_ALERT_SOUND_RADIUS	256

static void G_AngerAlert( gentity_t *self )
{
	G_AlertTeam( self, self->enemy, ANGER_ALERT_RADIUS, ANGER_ALERT_SOUND_RADIUS );	
}

/*
-------------------------
G_TeamEnemy
-------------------------
*/

qboolean G_TeamEnemy( gentity_t *self )
{//FIXME: Probably a better way to do this, is a linked list of your teammates already available?
	int	i;
	gentity_t	*ent;

	if ( !self->client || self->client->playerTeam == TEAM_FREE )
	{
		return qfalse;
	}

	for( i = 1; i < MAX_GENTITIES; i++ )
	{
		ent = &g_entities[i];

		if ( !ent->client )
		{
			continue;
		}

		if ( ent->client->playerTeam != self->client->playerTeam )
		{//ent is not on my team
			continue;
		}

		if ( ent->enemy )
		{//they have an enemy
			if ( !ent->enemy->client || ent->enemy->client->playerTeam != self->client->playerTeam )
			{//the ent's enemy is either a normal ent or is a player/NPC that is not on my team
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
-------------------------
G_SetEnemy
-------------------------
*/

void G_SetEnemy( gentity_t *self, gentity_t *enemy )
{
	int	event = 0;
	
	//Must be valid
	if ( enemy == NULL )
		return;

	//Must be valid
	if ( enemy->inuse == 0 )
	{
		return;
	}

	//Don't take the enemy if in notarget
	if ( enemy->flags & FL_NOTARGET )
		return;

	if ( !self->NPC )
	{
		self->enemy = enemy;
		return;
	}
	
	if ( enemy->client && enemy->client->playerTeam == TEAM_DISGUISE )
	{//unmask the player
		enemy->client->playerTeam = TEAM_STARFLEET;
	}
	
	if ( self->enemy == NULL )
	{
		//FIXME: Have to do this to prevent alert cascading
		G_ClearEnemy( self );
		self->enemy = enemy;

		//Special case- if player is being hunted by his own people, set their enemy team correctly
		if ( self->client->playerTeam == TEAM_STARFLEET && enemy->s.number == 0 )
		{
			self->client->enemyTeam = TEAM_PLAYER;
		}

		//If have an anger script, run that instead of yelling
		if( self->behaviorSet[BSET_ANGER] )
		{
			G_ActivateBehavior( self, BSET_ANGER );
		}
		else if ( self->client && enemy->client && self->client->playerTeam != enemy->client->playerTeam )
		{
			//FIXME: Use anger when entire team has no enemy.
			//		 Basically, you're first one to notice enemies
			if ( !G_TeamEnemy( self ) )
			{//team did not have an enemy previously
				event = Q_irand(EV_ANGER1, EV_ANGER3);
			}
			else if ( !Q_irand( 0, 2 ) )
			{//Otherwise, just picking up another enemy
				event = Q_irand(EV_COMBAT1, EV_COMBAT3);
			}
			
			if ( event )
			{//yell
				G_AddVoiceEvent( self, event, 2000 );
			}
		}
		
		//Alert anyone else in the area
		if ( Q_stricmp( "desperado", self->NPC_type ) != 0 && Q_stricmp( "paladin", self->NPC_type ) != 0 )
		{//special holodeck enemies exception
			G_AngerAlert( self );
		}
		return;
	}
	
	//Otherwise, just picking up another enemy

	if ( Q_irand( 0, 2 ) == 0 )
	{//yell something
		event = Q_irand(EV_COMBAT1, EV_COMBAT3);
	}

	if ( event )
	{
		G_AddVoiceEvent( self, event, 2000 );
	}

	//Take the enemy
	G_ClearEnemy(self);
	self->enemy = enemy;
}

/*
int ChooseBestWeapon( void ) 
{
	int		n;
	int		weapon;

	// check weapons in the NPC's weapon preference order
	for ( n = 0; n < MAX_WEAPONS; n++ ) 
	{
		weapon = NPCInfo->weaponOrder[n];

		if ( weapon == WP_NONE ) 
		{
			break;
		}

		if ( !HaveWeapon( weapon ) ) 
		{
			continue;
		}

		if ( client->ps.ammo[weaponData[weapon].ammoIndex] ) 
		{
			return weapon;
		}
	}

	// check weapons serially (mainly in case a weapon is not on the NPC's list)
	for ( weapon = 1; weapon < WP_NUM_WEAPONS; weapon++ ) 
	{
		if ( !HaveWeapon( weapon ) ) 
		{
			continue;
		}

		if ( client->ps.ammo[weaponData[weapon].ammoIndex] ) 
		{
			return weapon;
		}
	}

	return client->ps.weapon;
}
*/

void ChangeWeapon( gentity_t *ent, int newWeapon ) 
{
	if ( !ent || !ent->client || !ent->NPC )
	{
		return;
	}

	ent->client->ps.weapon = newWeapon;
	ent->NPC->shotTime = 0;
	ent->NPC->burstCount = 0;
	ent->NPC->attackHold = 0;
	ent->NPC->currentAmmo = ent->client->ps.ammo[weaponData[newWeapon].ammoIndex];

	switch ( newWeapon ) 
	{
	case WP_IMOD://IMOD
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 3000;
		break;

	case WP_BORG_DRILL:
	case WP_BORG_ASSIMILATOR://Borg
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 5000;
		ent->NPC->attackHold = 5000;
		break;

	case WP_BORG_TASER://Borg
		ent->NPC->aiFlags |= NPCAI_BURST_WEAPON;
		ent->NPC->burstMin = 1;//0.5 sec
		ent->NPC->burstMean = 4;//1 second
		ent->NPC->burstMax = 7;//3 seconds
		ent->NPC->burstSpacing = 2000;//2 seconds
		break;

	case WP_BORG_WEAPON://Borg
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		//ent->NPC->aiFlags |= NPCAI_BURST_WEAPON;
		//ent->NPC->burstMin = 1;//0.5 sec
		//ent->NPC->burstMean = 1;//1 second
		//ent->NPC->burstMax = 2;//3 seconds
		ent->NPC->burstSpacing = 2000;//2 seconds
		break;

	case WP_COMPRESSION_RIFLE://prifle
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 1000;//attackdebounce
		break;

	case WP_IMPERIAL_BLADE:
	case WP_KLINGON_BLADE:
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 1000;//attackdebounce
		break;

	case WP_PHASER:
		ent->NPC->aiFlags |= NPCAI_BURST_WEAPON;
		ent->NPC->burstMin = 5;//0.5 sec
		ent->NPC->burstMean = 10;//1 second
		ent->NPC->burstMax = 20;//3 seconds
		ent->NPC->burstSpacing = 2000;//2 seconds
		ent->NPC->attackHold = 1000;//Hold attack button for a 1-second burst
		break;

	case WP_TRICORDER:
		ent->NPC->aiFlags |= NPCAI_BURST_WEAPON;
		ent->NPC->burstMin = 5;
		ent->NPC->burstMean = 10;
		ent->NPC->burstMax = 30;
		ent->NPC->burstSpacing = 1000;
		break;

	case WP_SCAVENGER_RIFLE://Scav weapon
	case WP_CHAOTICA_GUARD_GUN:
		//Check for special tutorial mode shooters
		if ( ( ent->client->playerTeam == TEAM_SCAVENGERS ) && ( ent->spawnflags & 2 ) )
		{
			ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
			ent->NPC->burstMin = 1;
			ent->NPC->burstMean = 1;
			ent->NPC->burstMax = 1;
			ent->NPC->burstSpacing = 1500;//attack debounce
		}
		else
		{
			ent->NPC->aiFlags |= NPCAI_BURST_WEAPON;
			ent->NPC->burstMin = 3;
			ent->NPC->burstMean = 3;
			ent->NPC->burstMax = 3;
			
			if ( g_spskill->integer == 0 )
				ent->NPC->burstSpacing = 1500;//attack debounce
			else if ( g_spskill->integer == 1 )
				ent->NPC->burstSpacing = 1000;//attack debounce
			else 
				ent->NPC->burstSpacing = 500;//attack debounce
		}

		break;

	case WP_STASIS:
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 2000;//attack debounce
		break;

	case WP_DESPERADO://winchester
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 2000;//attackdebounce
		break;

	case WP_PALADIN://crossbow
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		ent->NPC->burstSpacing = 2000;//attackdebounce
		break;

	default:
		ent->NPC->aiFlags &= ~NPCAI_BURST_WEAPON;
		break;
	}
}

void NPC_ChangeWeapon( int newWeapon ) 
{
	ChangeWeapon( NPC, newWeapon );
}
/*
void NPC_ApplyWeaponFireDelay(void)
How long, if at all, in msec the actual fire should delay from the time the attack was started
*/
void NPC_ApplyWeaponFireDelay(void)
{	
	if ( NPC->attackDebounceTime > level.time )
	{//Just fired, if attacking again, must be a burst fire, so don't add delay
		//NOTE: Borg AI uses attackDebounceTime "incorrectly", so this will always return for them!
		return;
	}
	
	switch(client->ps.weapon)
	{
	case WP_BORG_WEAPON:
		NPCInfo->burstCount = 0;
		client->fireDelay = 500;
		break;
	
	case WP_BORG_TASER:
		NPCInfo->burstCount = 0;
		if ( Q_stricmp( "satan", NPC->NPC_type ) != 0 )
		{//no fire delay for Satan's Robot
			client->fireDelay = 900;
		}
		break;
	
	case WP_BORG_DRILL:
	case WP_BORG_ASSIMILATOR:
		NPCInfo->burstCount = 0;
		client->fireDelay = 400;
		break;
	
	case WP_FORGE_PSYCH:
		NPCInfo->burstCount = 0;
		//TODO: Play charge up sound
		client->fireDelay = 700;
		break;
	
	case WP_FORGE_PROJ:
		NPCInfo->burstCount = 0;
		//TODO: Play charge up sound
		client->fireDelay = 500;
		break;

	case WP_PARASITE:
		NPCInfo->burstCount = 0;
		//TODO: Play charge up sound
		client->fireDelay = 400;
		break;

	case WP_STASIS_ATTACK:
		NPCInfo->burstCount = 0;
		client->fireDelay = 1000;
		break;

	case WP_IMPERIAL_BLADE:
	case WP_KLINGON_BLADE:
		NPCInfo->burstCount = 0;
		client->fireDelay = 600;
		break;

	default:
		client->fireDelay = 0;
		break;
	}
};

/*
-------------------------
ShootThink
-------------------------
*/
void ShootThink( void ) 
{
	int			delay;

	ucmd.buttons &= ~BUTTON_ATTACK;
/*
	if ( enemyVisibility != VIS_SHOOT) 
		return;
*/

	if ( client->ps.weapon == WP_NONE )
		return;

	if ( client->ps.weaponstate != WEAPON_READY && client->ps.weaponstate != WEAPON_FIRING && client->ps.weaponstate != WEAPON_IDLE) 
		return;

	if ( level.time < NPCInfo->shotTime ) 
	{
		return;
	}

	ucmd.buttons |= BUTTON_ATTACK;

	NPCInfo->currentAmmo = client->ps.ammo[weaponData[client->ps.weapon].ammoIndex];	// checkme

	NPC_ApplyWeaponFireDelay();

	if ( NPCInfo->aiFlags & NPCAI_BURST_WEAPON ) 
	{
		if ( !NPCInfo->burstCount ) 
		{
			NPCInfo->burstCount = Q_irand( NPCInfo->burstMin, NPCInfo->burstMax );
			/*
			NPCInfo->burstCount = erandom( NPCInfo->burstMean );
			if ( NPCInfo->burstCount < NPCInfo->burstMin ) 
			{
				NPCInfo->burstCount = NPCInfo->burstMin;
			}
			else if ( NPCInfo->burstCount > NPCInfo->burstMax ) 
			{
				NPCInfo->burstCount = NPCInfo->burstMax;
			}
			*/
			delay = 0;
		}
		else 
		{
			NPCInfo->burstCount--;
			if ( NPCInfo->burstCount == 0 ) 
			{
				delay = NPCInfo->burstSpacing;
			}
			else 
			{
				delay = 0;
			}
		}
	}
	else 
	{
		delay = NPCInfo->burstSpacing;
	}

	NPCInfo->shotTime = level.time + delay;
	NPC->attackDebounceTime = level.time + NPC_AttackDebounceForWeapon();
}

/*
static void WeaponThink( qboolean inCombat ) 
FIXME makes this so there's a delay from event that caused us to check to actually doing it

Added: hacks for Borg
*/
void WeaponThink( qboolean inCombat ) 
{
	if ( client->ps.weaponstate == WEAPON_RAISING || client->ps.weaponstate == WEAPON_DROPPING ) 
	{
		ucmd.weapon = client->ps.weapon;
		ucmd.buttons &= ~BUTTON_ATTACK;
		return;
	}

//MCG - Begin
	//For now, no-one runs out of ammo	
	if(NPC->client->ps.ammo[ weaponData[client->ps.weapon].ammoIndex ] < 10)	// checkme	
//	if(NPC->client->ps.ammo[ client->ps.weapon ] < 10)
	{
		Add_Ammo (NPC, client->ps.weapon, 100);
	}

	/*if ( NPC->playerTeam == TEAM_BORG )
	{//HACK!!!
		if(!(NPC->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_BORG_WEAPON )))
			NPC->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_BORG_WEAPON );

		if ( client->ps.weapon != WP_BORG_WEAPON ) 
		{
			NPC_ChangeWeapon( WP_BORG_WEAPON );
			Add_Ammo (NPC, client->ps.weapon, 10);
			NPCInfo->currentAmmo = client->ps.ammo[client->ps.weapon];
		}
	}
	else */
	
	/*if ( NPC->client->playerTeam == TEAM_SCAVENGERS )
	{//HACK!!!
		if(!(NPC->client->ps.stats[STAT_WEAPONS] & ( 1 << WP_SCAVENGER_RIFLE )))
			NPC->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_SCAVENGER_RIFLE );

		if ( client->ps.weapon != WP_SCAVENGER_RIFLE )
			 
		{
			NPC_ChangeWeapon( WP_SCAVENGER_RIFLE );
			Add_Ammo (NPC, client->ps.weapon, 10);
//			NPCInfo->currentAmmo = client->ps.ammo[client->ps.weapon];
			NPCInfo->currentAmmo = client->ps.ammo[weaponData[client->ps.weapon].ammoIndex];	// checkme
		}
	}
	else*/
//MCG - End
	{
		// if the gun in our hands is out of ammo, we need to change
		/*if ( client->ps.ammo[client->ps.weapon] == 0 ) 
		{
			NPCInfo->aiFlags |= NPCAI_CHECK_WEAPON;
		}

		if ( NPCInfo->aiFlags & NPCAI_CHECK_WEAPON ) 
		{
			NPCInfo->aiFlags &= ~NPCAI_CHECK_WEAPON;
			bestWeapon = ChooseBestWeapon();
			if ( bestWeapon != client->ps.weapon ) 
			{
				NPC_ChangeWeapon( bestWeapon );
			}
		}*/
	}

	ucmd.weapon = client->ps.weapon;
	ShootThink();
}

/*
HaveWeapon
*/

qboolean HaveWeapon( int weapon ) 
{
	return ( client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) );
}

qboolean EntIsGlass (gentity_t *check)
{
	if(check->classname &&
		!Q_stricmp("func_breakable", check->classname) &&
		check->count == 1 && check->health <= 100)
	{
		return qtrue;
	}

	return qfalse;
}

qboolean ShotThroughGlass (trace_t *tr, gentity_t *target, vec3_t spot, int mask)
{
	gentity_t	*hit = &g_entities[ tr->entityNum ];
	if(hit != target && EntIsGlass(hit))
	{//ok to shoot through breakable glass
		int			skip = hit->s.number;
		vec3_t		muzzle;

		VectorCopy(tr->endpos, muzzle);
		gi.trace (tr, muzzle, NULL, NULL, spot, skip, mask );
		return qtrue;
	}

	return qfalse;
}

/*
CanShoot
determine if NPC can directly target enemy

this function does not check teams, invulnerability, notarget, etc....

Added: If can't shoot center, try head, if not, see if it's close enough to try anyway.
*/
qboolean CanShoot ( gentity_t *ent, gentity_t *shooter ) 
{
	trace_t		tr;
	vec3_t		muzzle;
	vec3_t		spot, diff;
	gentity_t	*traceEnt;

	CalcEntitySpot( shooter, SPOT_WEAPON, muzzle );
	CalcEntitySpot( ent, SPOT_ORIGIN, spot );		//FIXME preferred target locations for some weapons (feet for R/L)

	gi.trace ( &tr, muzzle, NULL, NULL, spot, shooter->s.number, MASK_SHOT );
	traceEnt = &g_entities[ tr.entityNum ];

	// point blank, baby!
	if (tr.startsolid && (shooter->NPC) && (shooter->NPC->touchedByPlayer) ) 
	{
		traceEnt = shooter->NPC->touchedByPlayer;
	}
	
	if ( ShotThroughGlass( &tr, ent, spot, MASK_SHOT ) )
	{
		traceEnt = &g_entities[ tr.entityNum ];
	}

	// shot is dead on
	if ( traceEnt == ent ) 
	{
		return qtrue;
	}
//MCG - Begin
	else
	{//ok, can't hit them in center, try their head
		CalcEntitySpot( ent, SPOT_HEAD, spot );
		gi.trace ( &tr, muzzle, NULL, NULL, spot, shooter->s.number, MASK_SHOT );
		traceEnt = &g_entities[ tr.entityNum ];
		if ( traceEnt == ent) 
		{
			return qtrue;
		}
	}

	//Actually, we should just check to fire in dir we're facing and if it's close enough,
	//and we didn't hit someone on our own team, shoot
	VectorSubtract(spot, tr.endpos, diff);
	if(VectorLength(diff) < random() * 32)
	{
		return qtrue;
	}
//MCG - End
	// shot would hit a non-client
	if ( !traceEnt->client ) 
	{
		return qfalse;
	}

	// shot is blocked by another player

	// he's already dead, so go ahead
	if ( traceEnt->health <= 0 ) 
	{
		return qtrue;
	}

	// don't deliberately shoot a teammate
	if ( traceEnt->client && ( traceEnt->client->playerTeam == shooter->client->playerTeam ) ) 
	{
		return qfalse;
	}

	// he's just in the wrong place, go ahead
	return qtrue;
}


/*
void NPC_CheckPossibleEnemy( gentity_t *other, visibility_t vis ) 

Added: hacks for scripted NPCs
*/
void NPC_CheckPossibleEnemy( gentity_t *other, visibility_t vis ) 
{
	// is he is already our enemy?
	if ( other == NPC->enemy ) 
		return;

	if ( other->flags & FL_NOTARGET ) 
		return;

	// we already have an enemy and this guy is in our FOV, see if this guy would be better
	if ( NPC->enemy && vis == VIS_FOV ) 
	{
		if ( NPCInfo->enemyLastSeenTime - level.time < 2000 ) 
		{
			return;
		}
		if ( enemyVisibility == VIS_UNKNOWN ) 
		{
			enemyVisibility = NPC_CheckVisibility ( NPC->enemy, CHECK_360|CHECK_FOV );
		}
		if ( enemyVisibility == VIS_FOV ) 
		{
			return;
		}
	}

	if ( !NPC->enemy )
	{//only take an enemy if you don't have one yet
		G_SetEnemy( NPC, other );
	}

	if ( vis == VIS_FOV ) 
	{
		NPCInfo->enemyLastSeenTime = level.time;
		VectorCopy( other->currentOrigin, NPCInfo->enemyLastSeenLocation );
		NPCInfo->enemyLastHeardTime = 0;
		VectorClear( NPCInfo->enemyLastHeardLocation );
	} 
	else 
	{
		NPCInfo->enemyLastSeenTime = 0;
		VectorClear( NPCInfo->enemyLastSeenLocation );
		NPCInfo->enemyLastHeardTime = level.time;
		VectorCopy( other->currentOrigin, NPCInfo->enemyLastHeardLocation );
	}
}


//==========================================
//MCG Added functions:
//==========================================

/*
int NPC_AttackDebounceForWeapon (void)

DOES NOT control how fast you can fire
Only makes you keep your weapon up after you fire

*/
int NPC_AttackDebounceForWeapon (void)
{
	switch ( NPC->client->ps.weapon ) 
	{
	case WP_SCAVENGER_RIFLE://scav rifle
	case WP_CHAOTICA_GUARD_GUN:
		return 500;
		break;

	case WP_IMOD:
		return 3000;//IMOD
		break;

	case WP_BORG_WEAPON:
	case WP_BORG_TASER:
		return 1200;//BORG - guess- should be length of the attack anim
		break;

	case WP_BORG_ASSIMILATOR:
		return 200;
		break;

	case WP_COMPRESSION_RIFLE://prifle
		return 3000;
		break;

	case WP_PHASER:
		return 100;
		break;

	case WP_TRICORDER:
		return 0;//tricorder
		break;

	case WP_KLINGON_BLADE:
	case WP_IMPERIAL_BLADE:
		return 48;
		break;

	case WP_BOT_WELDER:
		
		if ( g_spskill->integer == 0 )
			return 2000;

		if ( g_spskill->integer == 1 )
			return 1500;

		return 1000;
		break;

	default:
		return 100;
		break;
	}
}

//FIXME: need a mindist for explosive weapons
float NPC_MaxDistSquaredForWeapon (void)
{
	if(NPCInfo->stats.shootDistance > 0)
	{//overrides default weapon dist
		return NPCInfo->stats.shootDistance * NPCInfo->stats.shootDistance;
	}

	switch ( NPC->s.weapon ) 
	{
	case WP_SCAVENGER_RIFLE://scav rifle
	case WP_CHAOTICA_GUARD_GUN:
		return 1024 * 1024;//should be shorter?
		break;

	case WP_IMOD:
		return 512 * 512;//IMOD
		break;

	case WP_BORG_WEAPON:
		return 512 * 512;//BORG
		break;

	case WP_BORG_TASER:
		return 256 * 256;//BORG
		break;

	case WP_BORG_DRILL:
	case WP_BORG_ASSIMILATOR:
		return 72 * 72;//BORG
		break;

	case WP_COMPRESSION_RIFLE://prifle
		return 1024 * 1024;
		break;

	case WP_PHASER:
		return 1024 * 1024;
		break;

	case WP_TRICORDER:
		return 0;//tricorder
		break;

	case WP_IMPERIAL_BLADE:
	case WP_KLINGON_BLADE:
		return 64*64;
		break;

	default:
		return 1024 * 1024;//was 0
		break;
	}
}

/*
-------------------------
ValidEnemy
-------------------------
*/

qboolean ValidEnemy(gentity_t *ent)
{
	if ( ent == NULL )
		return qfalse;

	//if team_free, maybe everyone is an enemy?
	if ( !NPC->client->enemyTeam )
		return qfalse;

	if( ent->client && !(ent->flags & FL_NOTARGET))
	{
		if( ent->health > 0 )
		{
			if( ent->client->playerTeam == NPC->client->enemyTeam )
			{
				return qtrue;
			}
		}
	}

	return qfalse;
}

qboolean NPC_EnemyTooFar(gentity_t *enemy, float dist, qboolean toShoot)
{
	vec3_t	vec;

	
	if ( !toShoot )
	{//Not trying to actually press fire button with this check
		if ( ( NPC->client->ps.weapon == WP_BORG_ASSIMILATOR ) || ( NPC->client->ps.weapon == WP_BORG_DRILL ) )
		{//Just have to get to him
			return qfalse;
		}
	}
	

	if(!dist)
	{
		VectorSubtract(NPC->currentOrigin, enemy->currentOrigin, vec);
		dist = VectorLengthSquared(vec);
	}

	if(dist > NPC_MaxDistSquaredForWeapon())
		return qtrue;

	return qfalse;
}

/*
NPC_PickEnemy

Randomly picks a living enemy from the specified team and returns it

FIXME: For now, you MUST specify an enemy team

If you specify choose closest, it will find only the closest enemy

If you specify checkVis, it will return and enemy that is visible

If you specify findPlayersFirst, it will try to find players first

You can mix and match any of those options (example: find closest visible players first)

FIXME: this should go through the snapshot and find the closest enemy
*/
gentity_t *NPC_PickEnemy (gentity_t *closestTo, int enemyTeam, qboolean checkVis, qboolean findPlayersFirst, qboolean findClosest)
{
	int			num_choices = 0;
	int			choice[128];//FIXME: need a different way to determine how many choices?
	gentity_t	*newenemy = NULL;
	gentity_t	*closestEnemy = NULL;
	int			entNum;
	vec3_t		diff;
	float		relDist;
	float		bestDist = Q3_INFINITE;
	qboolean	failed = qfalse;
	int			visChecks = (CHECK_360|CHECK_FOV|CHECK_VISRANGE);
	int			minVis = VIS_FOV;

	if (!enemyTeam)
	{
		return NULL;
	}

	if ( NPCInfo->behaviorState == BS_FORMATION || 
		NPCInfo->behaviorState == BS_STAND_AND_SHOOT || 
		NPCInfo->behaviorState == BS_HUNT_AND_KILL || 
		NPCInfo->behaviorState == BS_RUN_AND_SHOOT || 
		NPCInfo->behaviorState == BS_POINT_COMBAT )
	{//Formations guys don't require inFov to pick up a target
		//These other behavior states are active battle states and should not
		//use FOV.  FOV checks are for enemies who are patrolling, guarding, etc.
		visChecks &= ~CHECK_FOV;
		minVis = VIS_360;
	}

	if( findPlayersFirst || enemyTeam == TEAM_PLAYER )
	{//try to find a player first
		newenemy = &g_entities[0];
		if( newenemy->client && !(newenemy->flags & FL_NOTARGET) && !(newenemy->s.eFlags & EF_NODRAW))
		{
			if( newenemy->health > 0 )
			{
				if( enemyTeam == TEAM_PLAYER || newenemy->client->playerTeam == enemyTeam || 
					( enemyTeam == TEAM_STARFLEET && !NPC_CheckDisguise( newenemy ) ) )
				{//FIXME:  check for range and FOV or vis?
					if( newenemy != NPC->lastEnemy )
					{//Make sure we're not just going back and forth here
						if ( gi.inPVS(newenemy->currentOrigin, NPC->currentOrigin) )
						{
							if(NPCInfo->behaviorState == BS_INVESTIGATE ||	NPCInfo->behaviorState == BS_PATROL)
							{
								if(!NPC->enemy)
								{
									if(!InVisrange(newenemy))
									{
										failed = qtrue;
									}
									else if(NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) != VIS_FOV)
									{
										failed = qtrue;
									}
								}
							}

							if ( !failed )
							{
								VectorSubtract( closestTo->currentOrigin, newenemy->currentOrigin, diff );
								relDist = VectorLengthSquared(diff);
								if ( newenemy->client->hiddenDist > 0 )
								{
									if( relDist > newenemy->client->hiddenDist*newenemy->client->hiddenDist )
									{
										//out of hidden range
										if ( VectorLengthSquared( newenemy->client->hiddenDir ) )
										{//They're only hidden from a certain direction, check
											float	dot;
											VectorNormalize( diff );
											dot = DotProduct( newenemy->client->hiddenDir, diff ); 
											if ( dot > 0.5 )
											{//I'm not looking in the right dir toward them to see them 
												failed = qtrue;
											}
											else
											{
												Debug_Printf(debugNPCAI, DEBUG_LEVEL_INFO, "%s saw %s trying to hide - hiddenDir %s targetDir %s dot %f\n", NPC->targetname, newenemy->targetname, vtos(newenemy->client->hiddenDir), vtos(diff), dot );
											}
										}
										else
										{
											failed = qtrue;
										}
									}
									else
									{
										Debug_Printf(debugNPCAI, DEBUG_LEVEL_INFO, "%s saw %s trying to hide - hiddenDist %f\n", NPC->targetname, newenemy->targetname, newenemy->client->hiddenDist );
									}
								}

								if(!failed)
								{
									if(findClosest)
									{
										if(relDist < bestDist)
										{
											if(!NPC_EnemyTooFar(newenemy, relDist, qfalse))
											{
												if(checkVis)
												{
													if( NPC_CheckVisibility ( newenemy, visChecks ) == minVis )
													{
														bestDist = relDist;
														closestEnemy = newenemy;
													}
												}
												else
												{
													bestDist = relDist;
													closestEnemy = newenemy;
												}
											}
										}
									}
									else if(!NPC_EnemyTooFar(newenemy, 0, qfalse))
									{
										if(checkVis)
										{
											if( NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV )
											{
												choice[num_choices++] = newenemy->s.number;
											}
										}
										else
										{
											choice[num_choices++] = newenemy->s.number;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (findClosest && closestEnemy)
	{
		return closestEnemy;
	}

	if (num_choices)
	{
		return &g_entities[ choice[rand() % num_choices] ];
	}

	if ( enemyTeam == TEAM_PLAYER )
	{//couldn't find the player
		return NULL;
	}

	num_choices = 0;
	bestDist = Q3_INFINITE;
	closestEnemy = NULL;

	for ( entNum = 0; entNum < globals.num_entities; entNum++ )
	{
		newenemy = &g_entities[entNum];

		if ( (newenemy->client || newenemy->svFlags & SVF_NONNPC_ENEMY) && !(newenemy->flags & FL_NOTARGET) && !(newenemy->s.eFlags & EF_NODRAW))
		{
			if ( newenemy->health > 0 )
			{
				if ( (newenemy->client && newenemy->client->playerTeam == enemyTeam) || (!newenemy->client && newenemy->noDamageTeam == enemyTeam) )
				{//FIXME:  check for range and FOV or vis?
					if ( newenemy != NPC->lastEnemy )
					{//Make sure we're not just going back and forth here
						if(!gi.inPVS(newenemy->currentOrigin, NPC->currentOrigin))
						{
							continue;
						}

						if ( NPCInfo->behaviorState == BS_INVESTIGATE || NPCInfo->behaviorState == BS_PATROL )
						{
							if ( !NPC->enemy )
							{
								if ( !InVisrange( newenemy ) )
								{
									continue;
								}
								else if ( NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) != VIS_FOV )
								{
									continue;
								}
							}
						}

						VectorSubtract( closestTo->currentOrigin, newenemy->currentOrigin, diff );
						relDist = VectorLengthSquared(diff);
						if ( newenemy->client && newenemy->client->hiddenDist > 0 )
						{
							if( relDist > newenemy->client->hiddenDist*newenemy->client->hiddenDist )
							{
								//out of hidden range
								if ( VectorLengthSquared( newenemy->client->hiddenDir ) )
								{//They're only hidden from a certain direction, check
									float	dot;

									VectorNormalize( diff );
									dot = DotProduct( newenemy->client->hiddenDir, diff ); 
									if ( dot > 0.5 )
									{//I'm not looking in the right dir toward them to see them 
										continue;
									}
									else
									{
										Debug_Printf(debugNPCAI, DEBUG_LEVEL_INFO, "%s saw %s trying to hide - hiddenDir %s targetDir %s dot %f\n", NPC->targetname, newenemy->targetname, vtos(newenemy->client->hiddenDir), vtos(diff), dot );
									}
								}
								else
								{
									continue;
								}
							}
							else
							{
								Debug_Printf(debugNPCAI, DEBUG_LEVEL_INFO, "%s saw %s trying to hide - hiddenDist %f\n", NPC->targetname, newenemy->targetname, newenemy->client->hiddenDist );
							}
						}

						if ( findClosest )
						{
							if ( relDist < bestDist )
							{
								if ( !NPC_EnemyTooFar( newenemy, relDist, qfalse ) )
								{
									if ( checkVis )
									{
										//FIXME: NPCs need to be able to pick up other NPCs behind them,
										//but for now, commented out because it was picking up enemies it shouldn't
										//if ( NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_VISRANGE ) >= VIS_360 )
										if ( NPC_CheckVisibility ( newenemy, visChecks ) == minVis )
										{
											bestDist = relDist;
											closestEnemy = newenemy;
										}
									}
									else
									{
										bestDist = relDist;
										closestEnemy = newenemy;
									}
								}
							}
						}
						else if ( !NPC_EnemyTooFar( newenemy, 0, qfalse ) )
						{
							if ( checkVis )
							{
								//if( NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV )
								if ( NPC_CheckVisibility ( newenemy, CHECK_360|CHECK_VISRANGE ) >= VIS_360 )
								{
									choice[num_choices++] = newenemy->s.number;
								}
							}
							else
							{
								choice[num_choices++] = newenemy->s.number;
							}
						}
					}
				}
			}
		}
	}

	
	if (findClosest)
	{//FIXME: you can pick up an enemy around a corner this way.
		return closestEnemy;
	}

	if (!num_choices)
	{
		return NULL;
	}

	return &g_entities[ choice[rand() % num_choices] ];
}

/*
gentity_t *NPC_PickAlly ( void )

  Simply returns closest visible ally
*/

gentity_t *NPC_PickAlly ( qboolean facingEachOther, float range, qboolean ignoreGroup, qboolean movingOnly )
{
	gentity_t	*ally = NULL;
	gentity_t	*closestAlly = NULL;
	int			entNum;
	vec3_t		diff;
	float		relDist;
	float		bestDist = range;

	for ( entNum = 0; entNum < globals.num_entities; entNum++ )
	{
		ally = &g_entities[entNum];

		if ( ally->client )
		{
			if ( ally->health > 0 )
			{
				if ( ally->client && ( ally->client->playerTeam == NPC->client->playerTeam ||
					( (NPC->client->playerTeam == TEAM_SCAVENGERS||NPC->client->playerTeam==TEAM_IMPERIAL) && ally->client->playerTeam == TEAM_DISGUISE ) ) )
				{//if on same team or if player is disguised as your team
					if ( ignoreGroup )
					{
						if ( ally == NPC->client->leader )
						{
							//reject
							continue;
						}
						if ( ally->client && ally->client->leader && ally->client->leader == NPC )
						{
							//reject
							continue;
						}
					}

					if(!gi.inPVS(ally->currentOrigin, NPC->currentOrigin))
					{
						continue;
					}

					if ( movingOnly && ally->client && NPC->client )
					{//They have to be moving relative to each other
						if ( !DistanceSquared( ally->client->ps.velocity, NPC->client->ps.velocity ) )
						{
							continue;
						}
					}

					VectorSubtract( NPC->currentOrigin, ally->currentOrigin, diff );
					relDist = VectorNormalize( diff );
					if ( relDist < bestDist )
					{
						if ( facingEachOther )
						{
							vec3_t	vf;
							float	dot;

							AngleVectors( ally->client->ps.viewangles, vf, NULL, NULL );
							VectorNormalize(vf);
							dot = DotProduct(diff, vf);

							if ( dot < 0.5 )
							{//Not facing in dir to me
								continue;
							}
							//He's facing me, am I facing him?
							AngleVectors( NPC->client->ps.viewangles, vf, NULL, NULL );
							VectorNormalize(vf);
							dot = DotProduct(diff, vf);

							if ( dot > -0.5 )
							{//I'm not facing opposite of dir to me
								continue;
							}
							//I am facing him
						}

						if ( NPC_CheckVisibility ( ally, CHECK_360|CHECK_VISRANGE ) >= VIS_360 )
						{
							bestDist = relDist;
							closestAlly = ally;
						}
					}
				}
			}
		}
	}

	
	return closestAlly;
}

void NPC_CheckEnemy (qboolean findNew, qboolean tooFarOk)
{
	qboolean	forcefindNew = qfalse;
	gentity_t	*closestTo;
	//FIXME: have a "NPCInfo->persistance" we can set to determine how long to try to shoot
	//someone we can't hit?  Rather than hard-coded 10?

	//FIXME they shouldn't recognize enemy's death instantly

	//TEMP FIX:
	//if(NPC->enemy->client)
	//{
	//	NPC->enemy->health = NPC->enemy->client->ps.stats[STAT_HEALTH];
	//}

	if ( NPC->enemy )
	{
		if ( !NPC->enemy->inuse )
		{
			G_ClearEnemy( NPC );
		}
	}

	if ( NPC->svFlags & SVF_IGNORE_ENEMIES )
	{//We're ignoring all enemies for now
		G_ClearEnemy( NPC );
		return;
	}

	if ( NPC->svFlags & SVF_LOCKEDENEMY )
	{//keep this enemy until dead
		if ( NPC->enemy )
		{
			if ( (!NPC->NPC && !(NPC->svFlags & SVF_NONNPC_ENEMY) ) || NPC->enemy->health > 0 )
			{//Enemy never had health (a train or info_not_null, etc) or did and is now dead (NPCs, turrets, etc)
				return;
			}
		}
		NPC->svFlags &= ~SVF_LOCKEDENEMY;
	}

	if ( NPC->enemy )
	{
		if ( NPC_EnemyTooFar(NPC->enemy, 0, qfalse) )
		{
			if(findNew)
			{//See if there is a close one and take it if so, else keep this one
				forcefindNew = qtrue;
			}
			else if(!tooFarOk)//FIXME: don't need this extra bool any more
			{
				G_ClearEnemy( NPC );
			}
		}
		else if ( !gi.inPVS(NPC->currentOrigin, NPC->enemy->currentOrigin ) )
		{//FIXME: should this be a line-of site check?
			//FIXME: a lot of things check PVS AGAIN when deciding whether 
			//or not to shoot, redundant!
			//Should we lose the enemy?
			//FIXME: if lose enemy, run lostenemyscript
			if ( NPC->enemy->client && NPC->enemy->client->hiddenDist )
			{//He ducked into shadow while we weren't looking
				//Drop enemy and see if we should search for him
				NPC_LostEnemyDecideChase();
			}
			else
			{//If we're not chasing him, we need to lose him
				switch( NPCInfo->behaviorState )
				{
				case BS_HUNT_AND_KILL:
				case BS_COMBAT:
					//Okay to lose PVS, we're chasing them
					break;
				case BS_RUN_AND_SHOOT:
					//If he's not our goalEntity, we're running somewhere else, so lose him
					if ( NPC->enemy != NPCInfo->goalEntity )
					{
						G_ClearEnemy( NPC );
					}
					break;
				default:
					//We're not chasing him, so lose him as an enemy
					G_ClearEnemy( NPC );
					break;
				}
			}
		}
	}

	if ( NPC->enemy )
	{
		if ( NPC->enemy->health <= 0 || NPC->enemy->flags & FL_NOTARGET )
		{
			G_ClearEnemy( NPC );
		}
	}

	closestTo = NPC;
	//FIXME: check your defendEnt, if you have one, see if their enemy is different 
	//than yours, or, if they don't have one, pick the closest enemy to THEM?
	if ( NPCInfo->defendEnt )
	{//Trying to protect someone
		if ( NPCInfo->defendEnt->health > 0 )
		{//Still alive, We presume we're close to them, navigation should handle this?
			if ( NPCInfo->defendEnt->enemy )
			{//They were shot or acquired an enemy
				if ( NPC->enemy != NPCInfo->defendEnt->enemy )
				{//They have a different enemy, take it!
					G_SetEnemy( NPC, NPCInfo->defendEnt->enemy );
				}
			}
			else if ( NPC->enemy == NULL )
			{//We don't have an enemy, so find closest to defendEnt
				closestTo = NPCInfo->defendEnt;
			}
		}
	}

	if (!NPC->enemy || ( NPC->enemy && (NPC->enemy->health <= 0 || NPC->cantHitEnemyCounter >= 100)) || forcefindNew )
	{//FIXME: NPCs that are moving after an enemy should ignore the can't hit enemy counter- that should only be for NPCs that are standing still
		//NOTE: cantHitEnemyCounter >= 100 means we couldn't hit enemy for a full
		//	10 seconds, so give up.  This means even if we're chasing him, we would
		//	try to find another enemy after 10 seconds (assuming the cantHitEnemyCounter
		//	is allowed to increment in a chasing bState)
		gentity_t	*newenemy;
		qboolean	foundenemy = qfalse;

		if(!findNew)
		{
			NPC->lastEnemy = NPC->enemy;
			G_ClearEnemy(NPC);
			return;
		}

		//If enemy dead or unshootable, look for others on out enemy's team
		if ( NPC->client->enemyTeam )
		{
			//NOTE:  this only checks vis if can't hit enemy for 10 tries, which I suppose
			//			means they need to find one that in more than just PVS
			//newenemy = NPC_PickEnemy( closestTo, NPC->client->enemyTeam, (NPC->cantHitEnemyCounter > 10), qfalse, qtrue );//3rd parm was (NPC->enemyTeam == TEAM_STARFLEET)
			//For now, made it so you ALWAYS have to check VIS
			newenemy = NPC_PickEnemy( closestTo, NPC->client->enemyTeam, qtrue, qfalse, qtrue );//3rd parm was (NPC->enemyTeam == TEAM_STARFLEET)
			if(newenemy)
			{
				foundenemy = qtrue;
				G_SetEnemy( NPC, newenemy );
			}
		}
		
		if(!forcefindNew)
		{
			if(!foundenemy)
			{
				NPC->lastEnemy = NPC->enemy;
				G_ClearEnemy(NPC);
			}
			
			NPC->cantHitEnemyCounter = 0;
		}
		//FIXME: if we can't find any at all, go into INdependant NPC AI, pursue and kill
	}

	if ( NPC->enemy && NPC->enemy->client ) 
	{
		if(NPC->enemy->client->playerTeam)
		{
//			assert( NPC->client->playerTeam != NPC->enemy->client->playerTeam);
			if( NPC->client->playerTeam != NPC->enemy->client->playerTeam )
			{
				NPC->client->enemyTeam = NPC->enemy->client->playerTeam;
			}
		}
	}
}

/*
NPC_CheckAttack

Simply checks aggression and returns true or false
*/

qboolean NPC_CheckAttack (float scale)
{
	if(!scale)
		scale = 1.0;

	if(((float)NPCInfo->stats.aggression) * scale < Q_flrand(0, 4))
	{
		return qfalse;
	}

	if(NPCInfo->shotTime > level.time)
		return qfalse;

	return qtrue;
}

/*
NPC_CheckDefend

Simply checks evasion and returns true or false
*/

qboolean NPC_CheckDefend (float scale)
{
	if(!scale)
		scale = 1.0;

	if((float)(NPCInfo->stats.evasion) > random() * 4 * scale)
		return qtrue;

	return qfalse;
}
//========================================================================================
//OLD id-style hunt and kill
//========================================================================================
/*
IdealDistance

determines what the NPC's ideal distance from it's enemy should
be in the current situation
*/
float IdealDistance ( gentity_t *self ) 
{
	float	ideal;

	ideal = 225 - 20 * NPCInfo->stats.aggression;
	switch ( NPC->s.weapon ) 
	{
	case WP_QUANTUM_BURST:
		ideal += 200;
		break;

	case WP_GRENADE_LAUNCHER:
		ideal += 50;
		break;

	case WP_BORG_WEAPON:
		ideal = 256;
		break;

	case WP_BORG_DRILL:
	case WP_BORG_ASSIMILATOR:
		ideal = 36;
		break;

	case WP_BORG_TASER:
		ideal = 128;
		break;

	case WP_STASIS:
		ideal = 128;
		break;

	case WP_TRICORDER:
		ideal = 0;
		break;

	case WP_PHASER:
	case WP_COMPRESSION_RIFLE:
	case WP_IMOD:
	case WP_SCAVENGER_RIFLE:
	case WP_CHAOTICA_GUARD_GUN:
	case WP_TETRION_DISRUPTOR:
	case WP_DREADNOUGHT:
	default:
		break;
	}

	return ideal;
}

/*QUAKED point_combat (0.7 0 0.7) (-16 -16 -24) (16 16 28) DUCK FLEE INVESTIGATE SQUAD LEAN
NPCs in bState BS_COMBAT_POINT will find their closest empty combat_point

DUCK - NPC will duck and fire from this point
FLEE - Will choose this point when running
INVESTIGATE - Will look here if a sound is heard near it
*/

void SP_point_combat( gentity_t *self )
{
	if(level.numCombatPoints >= MAX_COMBAT_POINTS)
	{
		gi.Printf(S_COLOR_RED"ERROR:  Too many combat points, limit is %d\n", MAX_COMBAT_POINTS);
		G_FreeEntity(self);
		return;
	}

	G_SetOrigin(self, self->s.origin);
	gi.linkentity(self);

	VectorCopy(self->currentOrigin, level.combatPoints[level.numCombatPoints].origin);
	
	level.combatPoints[level.numCombatPoints].flags = self->spawnflags;
	level.combatPoints[level.numCombatPoints].occupied = qfalse;

	level.numCombatPoints++;

	G_FreeEntity(self);
};

/*
-------------------------
NPC_CollectCombatPoints
-------------------------
*/

typedef	std::map< int, int >	combatPoint_m;

int NPC_CollectCombatPoints( vec3_t origin, float radius, combatPoint_m	&points )
{
	float	radiusSqr = (radius*radius);
	float	distance;

	//Collect all nearest
	for ( int i = 0; i < level.numCombatPoints; i++ )
	{
		distance = 	DistanceSquared( origin, level.combatPoints[i].origin );

		if ( distance < radiusSqr )
		{
			//Using a map will sort nearest automatically
			points[ distance ] = i;
		}
	}

	return points.size();
}

/*
-------------------------
NPC_FindCombatPoint
-------------------------
*/

#define	MIN_AVOID_DOT				0.75f
#define MIN_AVOID_DISTANCE			128
#define MIN_AVOID_DISTANCE_SQUARED	( MIN_AVOID_DISTANCE * MIN_AVOID_DISTANCE )
#define	CP_COLLECT_RADIUS			512

int NPC_FindCombatPoint( vec3_t position, vec3_t avoidPosition, vec3_t enemyPosition, int flags )
{
	combatPoint_m	points;

	//Collect our nearest points
	NPC_CollectCombatPoints( position, CP_COLLECT_RADIUS, points );

	int	i;
	combatPoint_m::iterator	cpi;
	STL_ITERATE( cpi, points )
	{
		i = (*cpi).second;

		//Must be vacant
		if ( level.combatPoints[i].occupied == (int) qtrue )
			continue;

		//If we want a duck space, make sure this is one
		if ( ( flags & CP_DUCK ) && ( level.combatPoints[i].flags & CPF_DUCK ) )
			continue;

		//If we want a duck space, make sure this is one
		if ( ( flags & CP_FLEE ) && ( level.combatPoints[i].flags & CPF_FLEE ) )
			continue;

		///Make sure this is an investigate combat point
		if ( ( flags & CP_INVESTIGATE ) && ( level.combatPoints[i].flags & CPF_INVESTIGATE ) )
			continue;
		
		//Squad points are only valid if we're looking for them
		if ( ( level.combatPoints[i].flags & CPF_SQUAD ) && ( ( flags & CP_SQUAD ) == qfalse ) )
			continue;

		//If we need a cover point, check this point
		if ( ( flags & CP_COVER ) && ( NPC_ClearLOS( level.combatPoints[i].origin, NPC->enemy ) == qtrue ) )
			continue;

		//Need a clear LOS to our target
		if ( ( flags & CP_CLEAR ) && ( NPC_ClearLOS( level.combatPoints[i].origin, NPC->enemy ) == qfalse ) )
			continue;

		//Avoid this position?
		if ( ( flags & CP_AVOID ) && ( DistanceSquared( level.combatPoints[i].origin, enemyPosition ) < MIN_AVOID_DISTANCE_SQUARED ) )
			continue;

		//See if we're trying to avoid our enemy
		if ( flags & CP_AVOID_ENEMY  )
		{
			vec3_t	eDir, gDir;
			
			VectorSubtract( position, enemyPosition, eDir );
			VectorNormalize( eDir );

			VectorSubtract( position, level.combatPoints[i].origin, gDir );
			VectorNormalize( gDir );

			float	dot = DotProduct( gDir, eDir );
			
			//Don't want to run at enemy
			if ( dot >= MIN_AVOID_DOT )
				continue;

			//Can't be too close to the enemy
			if ( DistanceSquared( level.combatPoints[i].origin, enemyPosition ) < MIN_AVOID_DISTANCE_SQUARED )
				continue;
		}

		return i;
	}

	return -1;
}

//Overload

int NPC_FindCombatPoint( int flags )
{
	vec3_t	avoid;

	if ( NPC_ValidEnemy( NPC->enemy ) == qfalse )
	{
		flags &= ~CP_AVOID_ENEMY;
		VectorClear( avoid );
	}
	else	//FIXME: Hate else statements...
	{
		VectorCopy( NPC->enemy->currentOrigin, avoid );
	}

	return NPC_FindCombatPoint( NPC->currentOrigin, avoid, avoid, flags );
}

/*
-------------------------
NPC_FindSquadPoint
-------------------------
*/

int NPC_FindSquadPoint( vec3_t position )
{
	unsigned int	nearestDist = 99999999;
	int				nearestPoint = -1;

	//float			playerDist = DistanceSquared( g_entities[0].currentOrigin, NPC->currentOrigin );

	for ( int i = 0; i < level.numCombatPoints; i++ )
	{
		//Squad points are only valid if we're looking for them
		if ( ( level.combatPoints[i].flags & CPF_SQUAD ) == qfalse )
			continue;

		//Must be vacant
		if ( level.combatPoints[i].occupied == qtrue )
			continue;
		
		unsigned int dist = DistanceSquared( position, level.combatPoints[i].origin );

		//The point cannot take us past the player
		//if ( dist > ( playerDist * DotProduct( dirToPlayer, playerDir ) ) )	//FIXME: Retain this
		//	continue;

		//See if this is closer than the others
		if ( dist < nearestDist )
		{
			nearestPoint = i;
			nearestDist = dist;
		}
	}

	return nearestPoint;
}

/*
-------------------------
NPC_ReserveCombatPoint
-------------------------
*/

qboolean NPC_ReserveCombatPoint( int combatPointID )
{
	//Make sure it's valid
	if ( combatPointID > level.numCombatPoints )
		return qfalse;

	//Make sure it's not already occupied
	if ( level.combatPoints[combatPointID].occupied )
		return qfalse;

	//Reserve it
	level.combatPoints[combatPointID].occupied = qtrue;

	return qtrue;
}

/*
-------------------------
NPC_FreeCombatPoint
-------------------------
*/

qboolean NPC_FreeCombatPoint( int combatPointID )
{
	//Make sure it's valid
	if ( combatPointID > level.numCombatPoints )
		return qfalse;

	//Make sure it's currently occupied
	if ( level.combatPoints[combatPointID].occupied == qfalse )
		return qfalse;

	//Free it
	level.combatPoints[combatPointID].occupied = qfalse;
	
	return qtrue;
}

/*
-------------------------
NPC_SetCombatPoint
-------------------------
*/

qboolean NPC_SetCombatPoint( int combatPointID )
{
	//Free a combat point if we already have one
	if ( NPCInfo->combatPoint != -1 )
	{
		NPC_FreeCombatPoint( NPCInfo->combatPoint );
	}

	if ( NPC_ReserveCombatPoint( combatPointID ) == qfalse )
		return qfalse;

	NPCInfo->combatPoint = combatPointID;

	return qtrue;
}

