#include "g_local.h"
#include "objectives.h"



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats->integer ) {
		gi.SendServerCommand( ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		gi.SendServerCommand( ent-g_entities, "print \"You must be alive to use this command.\n\"");
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	*arg;

	len = 0;
	c = gi.argc();
	for ( i = start ; i < c ; i++ ) {
		arg = gi.argv( i );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			gi.SendServerCommand( to-g_entities, "print \"Bad client slot: %i\n\"", idnum);
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			gi.SendServerCommand( to-g_entities, "print \"Client %i is not active\n\"", idnum);
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	gi.SendServerCommand( to-g_entities, "print \"User %s is not on the server\n\"", s);
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3) {
			ent->health = atoi(gi.argv(2));
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (MAX_PLAYER_WEAPONS+1)) - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}

	if ( !give_all && Q_stricmp(gi.argv(1), "weaponnum") == 0 )
	{
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(gi.argv(2)));
		return;
	}

	if ( Q_stricmp(name, "eweaps") == 0)	//for developing, gives you all the weapons, including enemy
	{
		ent->client->ps.stats[STAT_WEAPONS] = (unsigned)(1 << WP_NUM_WEAPONS) - ( 1 << WP_NONE ); // NOTE: this wasn't giving the last weapon in the list
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for ( i = 0 ; i < AMMO_MAX ; i++ ) {
			ent->client->ps.ammo[i] = ammoData[i].max;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(gi.argv(1), "armor") == 0)
	{
		if (gi.argc() == 3)
			ent->client->ps.stats[STAT_ARMOR] = atoi(gi.argv(2));
		else
		ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];

		if (!give_all)
			return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = FindItem (name);
		if (!it) {
			name = gi.argv(1);
			it = FindItem (name);
			if (!it) {
				gi.SendServerCommand( ent-g_entities, "print \"unknown item\n\"");
				return;
			}
		}

		it_ent = G_Spawn();
		VectorCopy( ent->currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.SendServerCommand( ent-g_entities, "print \"%s\"", msg);
}

/*
==================
Cmd_Undying_f

Sets client to undead mode

argv(0) undying
==================
*/
void Cmd_Undying_f (gentity_t *ent)
{
	char	*msg;

	if ( !CheatsOk( ent ) ) 
	{
		return;
	}

	ent->flags ^= FL_UNDYING;
	if (!(ent->flags & FL_UNDYING) )
	{
		msg = "undead mode OFF\n";
	}
	else
	{
		msg = "undead mode ON\n";
		ent->health = ent->max_health = 999;
		if ( ent->client )
		{
			ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH] = 999;
		}
	}

	gi.SendServerCommand( ent-g_entities, "print \"%s\"", msg);
}

/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.SendServerCommand( ent-g_entities, "print \"%s\"", msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	gi.SendServerCommand( ent-g_entities, "print \"%s\"", msg);
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	if ( !CheatsOk( ent ) ) {
		return;
	}

	gi.SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if( ( level.time - ent->client->respawnTime ) < 5000 ) {
		gi.SendServerCommand( ent-g_entities, "cp \"Only one kill every five seconds\n\"");
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}


/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	const char *s = gi.argv(1);
	const int len = strlen(s);
	gentity_t	*check;
	
	if ( gi.argc () < 2 ) {
		gi.Printf("usage: where classname\n");
		return;
	}
	for (int i = 0; i < globals.num_entities; i++) {
		check = &g_entities[i];
		if(!check || !check->inuse) {
			continue;
		}
		if (!Q_stricmpn(s, check->classname, len) ) {
			gi.SendServerCommand( ent-g_entities, "print \"%s %s\n\"", check->classname, vtos( check->s.pos.trBase ) );
		}
	}
}


/*
-------------------------
UserSpawn
-------------------------
*/

extern qboolean G_CallSpawn( gentity_t *ent );

void UserSpawn( gentity_t *ent, const char *name )
{
	vec3_t		origin;
	vec3_t		vf;
	vec3_t		angles;
	gentity_t	*ent2;
	
	//Spawn the ent
	ent2 = G_Spawn();
	ent2->classname = G_NewString( name );	//FIXME: This will leave floating memory...

	//TODO: This should ultimately make sure this is a safe spawn!

	//Spawn the entity and place it there
	VectorSet( angles, 0, ent->s.apos.trBase[YAW], 0 );
	AngleVectors( angles, vf, NULL, NULL );
	VectorMA( ent->s.pos.trBase, 96, vf, origin );	//FIXME: Find the radius size of the object, and push out 32 + radius

	origin[2] += 8;
	VectorCopy( origin, ent2->s.pos.trBase );
	VectorCopy( origin, ent2->s.origin );
	VectorCopy( ent->s.apos.trBase, ent2->s.angles );

	gi.linkentity( ent2 );

	//Find a valid spawning spot
	if ( G_CallSpawn( ent2 ) == qfalse )
	{
		gi.SendServerCommand( ent-g_entities, "print \"Failed to spawn '%s'\n\"", name );
		G_FreeEntity( ent2 );
		return;
	}
}

/*
-------------------------
Cmd_Spawn
-------------------------
*/

void Cmd_Spawn( gentity_t *ent )
{
	char	*name;

	name = ConcatArgs( 1 );

	gi.SendServerCommand( ent-g_entities, "print \"Spawning '%s'\n\"", name );

	UserSpawn( ent, name );
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	int			i;

	if ( !g_cheats->integer ) {
		gi.SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
		return;
	}
	if ( gi.argc() != 5 ) {
		gi.SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		origin[i] = atof( gi.argv( i+1 ) );
	}

	angles[YAW] = atof( gi.argv( 4 ) );

	TeleportPlayer( ent, origin, angles, STARFLEET_TELEPORT_FX );
}



/*
=================
Cmd_SetObjective_f
=================
*/
void Cmd_SetObjective_f( gentity_t *ent ) 
{
	int objectiveI,status,displayStatus;

	if ( gi.argc() != 4 ) {
		gi.SendServerCommand( ent-g_entities, va("print \"usage: setobjective <objective #>  <display status> <status>\n\""));
		return;
	}

	if ( !CheatsOk( ent ) ) 
	{
		return;
	}

	objectiveI = atoi(gi.argv(1));
	displayStatus = atoi(gi.argv(2));
	status = atoi(gi.argv(3));

	ent->client->sess.mission_objectives[objectiveI].display = displayStatus;
	ent->client->sess.mission_objectives[objectiveI].status = status;
}

/*
=================
Cmd_ViewObjective_f
=================
*/
void Cmd_ViewObjective_f( gentity_t *ent ) 
{
	int objectiveI;

	if ( gi.argc() != 2 ) {
		gi.SendServerCommand( ent-g_entities, va("print \"usage: viewobjective <objective #>\n\""));
		return;
	}

	objectiveI = atoi(gi.argv(1));

	gi.SendServerCommand( ent-g_entities, va("print \"Objective %d   Display Status(1=show): %d  Status:%d\n\"",objectiveI,ent->client->sess.mission_objectives[objectiveI].display,ent->client->sess.mission_objectives[objectiveI].status));
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	*cmd;

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "spawn") == 0)
	{
		Cmd_Spawn( ent );
		return;
	}
	
	if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "undying") == 0)
		Cmd_Undying_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
	else if (Q_stricmp (cmd, "setobjective") == 0)
		Cmd_SetObjective_f( ent );
	else if (Q_stricmp (cmd, "viewobjective") == 0)
		Cmd_ViewObjective_f( ent );
}
