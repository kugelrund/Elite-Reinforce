#include "g_local.h"
#include "g_roff.h"
// The list of precached ROFFs
roff_list_t	roffs[MAX_ROFFS];
int			num_roffs = 0;

extern void	Q3_TaskIDComplete( gentity_t *ent, taskID_t taskType );


//-------------------------------------------------------
// G_LoadRoff
//
// Does the fun work of loading and caching a roff file
//	If the file is already cached, it just returns an
//	ID to the cached file.
//-------------------------------------------------------

int G_LoadRoff( const char *fileName )
{
	char	file[MAX_QPATH];
	byte	*data;
	int		len, i, roff_id = 0;

	// Before even bothering with all of this, make sure we have a place to store it.
	if ( num_roffs >= MAX_ROFFS )
	{
		Com_Printf( S_COLOR_RED"MAX_ROFFS count exceeded.  Skipping load of .ROF '%s'\n", fileName );
		return roff_id;
	}

	// The actual path
	sprintf( file, "%s/%s.rof", Q3_SCRIPT_DIR, fileName );

	// See if I'm already precached
	for ( i = 0; i < num_roffs; i++ )
	{
		if ( _stricmp( file, roffs[i].fileName ) == 0 )
		{
			// Good, just return me...avoid zero index
			return i + 1;
		}
	}

#ifdef _DEBUG
	Com_Printf( S_COLOR_GREEN"Caching ROF: '%s'\n", file );
#endif

	// Read the file in one fell swoop
	len = gi.FS_ReadFile( file, (void**) &data);

	if ( len <= 0 )
	{
		Com_Printf( S_COLOR_RED"Could not open .ROF file '%s'\n", fileName );
		return roff_id;
	}

	// Now let's check the header info...
	roff_hdr_t *header = (roff_hdr_t *)data;

	// ..and make sure it's reasonably valid
	if ( strncmp( header->sHeader, "ROFF", 4 ) !=0 || header->lVersion != ROFF_VERSION || header->fCount <= 0 )
	{
		Com_Printf( S_COLOR_RED"Bad header data in .ROF file '%s'\n", fileName );
	}
	else
	{
		// Cool, the file seems to be valid
		int	count = (int)header->fCount;

		// Ask the game to give us some memory to store this pooch
		move_rotate_t *mem = roffs[num_roffs].data = (move_rotate_t *) G_Alloc( count * sizeof( move_rotate_t ) );

		if ( mem )
		{
			// The allocation worked, so stash this stuff off so we can reference the data later if needed
			roffs[num_roffs].fileName	= G_NewString( file );
			roffs[num_roffs].frames		= count;

			// Step past the header to get to the goods
			move_rotate_t *roff_data = ( move_rotate_t *)&header[1];

			// Copy all of the goods into our ROFF cache
			for ( int i = 0; i < count; i++, roff_data++, mem++ )
			{
				// Copy just the delta position and orientation which can be applied to anything at a later point
				VectorCopy( roff_data->origin_delta, mem->origin_delta );
				VectorCopy( roff_data->rotate_delta, mem->rotate_delta );
			}

			// Done loading this roff, so save off an id to it..increment first to avoid zero index
			roff_id = ++num_roffs;
		}
	}

	gi.FS_FreeFile( data );

	return roff_id;
}


//-------------------------------------------------------
// G_Roff
//
// Handles applying the roff data to the specified ent
//-------------------------------------------------------

void G_Roff( gentity_t *ent )
{
	if ( !ent->next_roff_time || ent->next_roff_time > level.time )
	{
		// either I don't think or it's just not time to have me think yet
		return;
	}

	int roff_id = G_LoadRoff( ent->roff );

	if ( !roff_id )
	{
		// Couldn't cache this rof
		return;
	}

	// The ID is one higher than the array index
	move_rotate_t	*roff	= &roffs[roff_id - 1].data[ent->roff_ctr];
	int				frames	= roffs[roff_id - 1].frames;

#ifdef _DEBUG
	Com_Printf( S_COLOR_GREEN"ROFF dat: o:<%.2f %.2f %.2f> a:<%.2f %.2f %.2f>\n", 
					roff->origin_delta[0], roff->origin_delta[1], roff->origin_delta[2],
					roff->rotate_delta[0], roff->rotate_delta[1], roff->rotate_delta[2] );
#endif

	if ( ent->client )
	{
		// Set up the angle interpolation
		//-------------------------------------
		VectorAdd( ent->s.apos.trBase, roff->rotate_delta, ent->s.apos.trBase );
		ent->s.apos.trTime = level.time;
		ent->s.apos.trType = TR_INTERPOLATE;

		// Store what the next apos->trBase should be
		VectorCopy( ent->s.apos.trBase, ent->client->ps.viewangles );
		VectorCopy( ent->s.apos.trBase, ent->currentAngles );
		VectorCopy( ent->s.apos.trBase, ent->s.angles );
		if ( ent->NPC )
		{
			//ent->NPC->desiredPitch = ent->s.apos.trBase[PITCH];
			ent->NPC->desiredYaw = ent->s.apos.trBase[YAW];
		}

		// Set up the origin interpolation
		//-------------------------------------
		VectorAdd( ent->s.pos.trBase, roff->origin_delta, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
		ent->s.pos.trType = TR_INTERPOLATE;

		// Store what the next apos->trBase should be
		VectorCopy( ent->s.pos.trBase, ent->client->ps.origin );
		VectorCopy( ent->s.pos.trBase, ent->currentOrigin );
		//VectorCopy( ent->s.pos.trBase, ent->s.origin );
	}
	else
	{
		// Set up the angle interpolation
		//-------------------------------------
		VectorScale( roff->rotate_delta, ROFF_SAMPLE_RATE, ent->s.apos.trDelta );
		VectorCopy( ent->pos2, ent->s.apos.trBase );
		ent->s.apos.trTime = level.time;
		ent->s.apos.trType = TR_LINEAR;

		// Store what the next apos->trBase should be
		VectorAdd( ent->pos2, roff->rotate_delta, ent->pos2 );


		// Set up the origin interpolation
		//-------------------------------------
		VectorScale( roff->origin_delta, ROFF_SAMPLE_RATE, ent->s.pos.trDelta );
		VectorCopy( ent->pos1, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
		ent->s.pos.trType = TR_LINEAR;

		// Store what the next apos->trBase should be
		VectorAdd( ent->pos1, roff->origin_delta, ent->pos1 );
	}

	// See if the ROFF playback is done
	//-------------------------------------
	if ( ++ent->roff_ctr >= frames )
	{
		// We are done, so let me think no more, then tell the task that we're done.
		ent->next_roff_time = 0;

		// Stop any rotation or movement.
		VectorClear( ent->s.pos.trDelta );
		VectorClear( ent->s.apos.trDelta );

		Q3_TaskIDComplete( ent, TID_MOVE_NAV );

		return;
	}

	// Lock me to a 10hz update rate
	ent->next_roff_time = level.time + 100;
}


//-------------------------------------------------------
// G_SaveCachedRoffs
//
// Really fun savegame stuff
//-------------------------------------------------------

void G_SaveCachedRoffs()
{
	int i, len;

	// Write out the number of cached ROFFs
	gi.AppendToSaveGame( 'ROFF', (void *)&num_roffs, sizeof(num_roffs) );

	// Now dump out the cached ROFF file names in order so they can be loaded on the other end
	for ( i = 0; i < num_roffs; i++ )
	{
		// Dump out the string length to make things a bit easier on the other end...heh heh.
		len = strlen( roffs[i].fileName ) + 1;
		gi.AppendToSaveGame( 'SLEN', (void *)&len, sizeof(len) );
		gi.AppendToSaveGame( 'RSTR', (void *)(*roffs[i].fileName), len );
	}
}


//-------------------------------------------------------
// G_LoadCachedRoffs
//
// Really fun loadgame stuff
//-------------------------------------------------------

void G_LoadCachedRoffs()
{
	int		i, count, len;
	char	buffer[MAX_QPATH];

	// Get the count of goodies we need to revive
	gi.ReadFromSaveGame( 'ROFF', (void *)&count, sizeof(count), NULL );

	// Now bring 'em back to life
	for ( i = 0; i < count; i++ )
	{
		gi.ReadFromSaveGame( 'SLEN', (void *)&len, sizeof(len), NULL );
		gi.ReadFromSaveGame( 'RSTR', (void *)(buffer), len, NULL );
		G_LoadRoff( buffer );
	}
}
