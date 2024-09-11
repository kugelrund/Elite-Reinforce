//FX System memory manager

#include "CG_Local.h"
#include "FX_Public.h"

//STL Helper macros
#define STL_FREELIST( a )	(*a).second.second
#define STL_ALLOCLIST( a )	(*a).second.first

//STL typedefs
typedef std::list < void * >	void_l;
typedef	std::map< size_t, std::pair< void_l, void_l > >	memoryPair_m;	// size -> ( allocated list, freed list )

//Main pool listing
memoryPair_m	poolList;

//Debug information
DWORD	memSize = 0;
DWORD	numSlots = 0;

#if _DEBUG

int	FXMEMD_searchPeak		= 0;
int FXMEMD_allocListPeak	= 0;
int FXMEMD_freeListPeak		= 0;
int FXMEMD_allocatePeak		= 0;
int FXMEMD_wcSearchPeak		= -1;

#endif

/*
-------------------------
FXMEM_GetBlock
-------------------------
*/

inline void *FXMEM_AllocateBlock( memoryPair_m::iterator mpi, size_t size )
{
	void	*mem;

	//Check for an available memory position
	if ( STL_FREELIST(mpi).size() > 0 )
	{
		//Grab this address
		mem = STL_FREELIST(mpi).front();
		
		//Pop this off the free list and push it onto the allocated list
		STL_FREELIST(mpi).pop_front();
		STL_ALLOCLIST(mpi).push_back( mem );

		return mem;
	}

	//Otherwise, allocate space for it
	mem = malloc( size );	//FIXME: Z_Malloc( size ); ???
	
	//Make sure it's valid
	if ( mem == NULL )
		return NULL;

	//Keep this information for debugging
	memSize += size;

	//Store this off
	STL_ALLOCLIST(mpi).push_back( mem );

	return mem;
}

/*
-------------------------
FXMEM_Allocate
-------------------------
*/

void *FXMEM_Allocate( size_t size )
{
	memoryPair_m::iterator	mpi;

	//Attempt to find a memory block listing for this size
	mpi = poolList.find( size );

	//Found an entry for this block size
	if ( mpi != poolList.end() )
	{
		return FXMEM_AllocateBlock( mpi, size );
	}

	//Add an entry	//FIXME: This is NOT a good way to do this
	std::pair< void_l, void_l >	dummy;
	poolList[ size ] = dummy;

	numSlots++;

	//Recursively call for the memory block
	return FXMEM_Allocate( size );
}

/*
-------------------------
FXMEM_FreeBlock
-------------------------
*/

inline void FXMEM_FreeBlock( memoryPair_m::iterator mpi, void *ptr, size_t size )
{
	void_l::iterator	vli;
	
#ifdef _DEBUG

	int	iterations = 0;

#endif

	//Find the memory entry
	for ( vli = STL_ALLOCLIST( mpi ).begin(); vli != STL_ALLOCLIST( mpi ).end(); vli++ )
	{

#ifdef _DEBUG
		
		iterations++;

#endif
		if ( (*vli) != ptr )
			continue;

		//Move it to the free blocks for use later
		STL_FREELIST(mpi).push_front( (*vli) );

		//Remove it from the list of allocated blocks
		STL_ALLOCLIST(mpi).erase( vli );

#ifdef _DEBUG

		if ( iterations > FXMEMD_searchPeak )
		{
			FXMEMD_searchPeak = iterations;
			
			if ( FXMEMD_wcSearchPeak < FXMEMD_searchPeak )
				FXMEMD_wcSearchPeak = FXMEMD_searchPeak;
		}

#endif

		return;
	}
}

/*
-------------------------
FXMEM_Free
-------------------------
*/

void FXMEM_Free( void *ptr, size_t size )
{
	memoryPair_m::iterator	mpi;

	//Attempt to find a memory block listing for this size
	mpi = poolList.find( size );

	//Found an entry for this block size
	if ( mpi == poolList.end() )
	{
		//A rogue pointer is coming in, OR the memory table has been corrupted (read: BAD!!!)
		assert( 0 );
		return;
	}

	FXMEM_FreeBlock( mpi, ptr, size );
}

/*
-------------------------
FXMEM_Flush
-------------------------
*/

void FXMEM_Flush( void )
{
	memoryPair_m::iterator	mpi;
	void_l::iterator		vli;

	//Make sure all memory block are released
	for ( mpi = poolList.begin(); mpi != poolList.end(); mpi++ )
	{
		//Check for residual blocks
		if ( STL_ALLOCLIST(mpi).size() > 0 )
		{
			for ( vli = STL_ALLOCLIST(mpi).begin(); vli != STL_ALLOCLIST(mpi).end(); vli++ )
			{
				free( (*vli) );
			}
		}

		//Clear the list
		STL_ALLOCLIST(mpi).clear();

		//Check for residual blocks
		if ( STL_FREELIST(mpi).size() > 0 )
		{
			for ( vli = STL_FREELIST(mpi).begin(); vli != STL_FREELIST(mpi).end(); vli++ )
			{
				free( (*vli) );
			}
		}

		//Clear the list
		STL_FREELIST(mpi).clear();
	}

	//Clear the map
	poolList.clear();
}

/*
-------------------------
FXMEM_Init
-------------------------
*/

void FXMEM_Init( void )
{
#ifdef _FXMEM

	FXMEM_Flush();

#endif

	//Clear out the debug info
	memSize		= 0;
	numSlots	= 0;
}

/*
-------------------------
FXMEM_PrintDebugInfo
-------------------------
*/

void FXMEM_PrintDebugInfo( void )
{
	Com_Printf( "FXMEM Current Memory Usage: %d\n", memSize );

#if _DEBUG

	memoryPair_m::iterator	mpi;
	
	for ( mpi = poolList.begin(); mpi != poolList.end(); mpi++ )
	{
		Com_Printf( "FXMEM Block Size [%d]: Active (%d) Free (%d)\n", (*mpi).first, STL_ALLOCLIST(mpi).size(), STL_FREELIST(mpi).size() );
	}

	Com_Printf("FXMEM Search Peak: %d\n", FXMEMD_searchPeak );
	Com_Printf("FXMEM Worst Search Peak: %d\n", FXMEMD_wcSearchPeak );
//	Com_Printf("FXMEM Allocation Peak: %d\n", FXMEMD_allocatePeak );

#endif
}
