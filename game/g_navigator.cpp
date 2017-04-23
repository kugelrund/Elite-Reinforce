#include "b_local.h"
#include "g_navigator.h"
#include "g_nav.h"

/*
-------------------------
CEdge
-------------------------
*/

CEdge::CEdge( void )
{
	CEdge( -1, -1, -1 );
}

CEdge::CEdge( int first, int second, int cost )
{
	m_first		= first;
	m_second	= second;
	m_cost		= cost;
}

CEdge::~CEdge( void )
{
}

/*
-------------------------
CNode
-------------------------
*/

CNode::CNode( void )
{
	m_numEdges		= 0;
	m_radius		= 0;
	m_ranks			= NULL;
}

CNode::~CNode( void )
{
	m_edges.clear();

	if ( m_ranks )
		delete [] m_ranks;
}

/*
-------------------------
Create
-------------------------
*/

CNode *CNode::Create( vec3_t position, int flags, int radius, int ID )
{
	CNode	*node = new CNode;

	VectorCopy( position, node->m_position );
	
	node->m_flags = flags;
	node->m_ID = ID;
	node->m_radius = radius;

	return node;
}

/*
-------------------------
Create
-------------------------
*/

CNode *CNode::Create( void )
{
	return new CNode;
}

/*
-------------------------
AddEdge
-------------------------
*/

void CNode::AddEdge( int ID, int cost, int flags )
{
	edge_t	edge;

	edge.ID		= ID;
	edge.cost	= cost;
	edge.flags	= flags;

	STL_INSERT( m_edges, edge );

	m_numEdges++;
}

/*
-------------------------
AddRank
-------------------------
*/

void CNode::AddRank( int ID, int rank )
{
	assert( m_ranks );

	m_ranks[ ID ] = rank;
}

/*
-------------------------
Draw
-------------------------
*/

void CNode::Draw( void )
{
	CG_DrawNode( m_position, NODE_NORMAL );
}

/*
-------------------------
GetEdge
-------------------------
*/

int CNode::GetEdge( int edgeNum )
{
	if ( edgeNum > m_numEdges )
		return -1;

	int count = 0;

	edge_v::iterator	ei;
	STL_ITERATE( ei, m_edges )
	{
		if ( count == edgeNum )
		{
			return (*ei).ID;
		}

		count++;
	}

	return -1;
}

/*
-------------------------
GetEdgeCost
-------------------------
*/

int CNode::GetEdgeCost( int edgeNum )
{
	if ( edgeNum > m_numEdges )
		return -1;

	int count = 0;

	edge_v::iterator	ei;
	STL_ITERATE( ei, m_edges )
	{
		if ( count == edgeNum )
		{
			return (*ei).cost;
		}

		count++;
	}

	return -1;
}

/*
-------------------------
GetEdgeFlags
-------------------------
*/

BYTE CNode::GetEdgeFlags( int edgeNum )
{
	if ( edgeNum > m_numEdges )
		return 0;

	int count = 0;

	edge_v::iterator	ei;
	STL_ITERATE( ei, m_edges )
	{
		if ( count == edgeNum )
		{
			return (*ei).flags;
		}

		count++;
	}
	
	return 0;
}

/*
-------------------------
InitRanks
-------------------------
*/

void CNode::InitRanks( int size )
{
	//Clear it if it's already allocated
	if ( m_ranks != NULL )
	{
		delete [] m_ranks;
		m_ranks = NULL;
	}

	m_ranks = new int[size];

	memset( m_ranks, -1, sizeof(int)*size );
}

/*
-------------------------
GetRank
-------------------------
*/

int CNode::GetRank( int ID )
{
	assert( m_ranks );

	return m_ranks[ ID ];
}


/*
-------------------------
Save
-------------------------
*/

int	CNode::Save( int numNodes, fileHandle_t file )
{
	//Write out the header
	unsigned long header = NODE_HEADER_ID;
	gi.FS_Write( &header, sizeof( header ), file );

	//Write out the basic information
	for ( int i = 0; i < 3; i++ )
		gi.FS_Write( &m_position[i], sizeof( float ), file );

	gi.FS_Write( &m_flags, sizeof( m_flags ), file );
	gi.FS_Write( &m_ID, sizeof( m_ID ), file );
	gi.FS_Write( &m_radius, sizeof( m_radius ), file );

	//Write out the edge information
	gi.FS_Write( &m_numEdges, sizeof( m_numEdges ), file );

	edge_v::iterator	ei;
	STL_ITERATE( ei, m_edges )
	{
		gi.FS_Write( &(*ei), sizeof( edge_t ), file );
	}

	//Write out the node ranks
	gi.FS_Write( &numNodes, sizeof( numNodes ), file );

	for ( int i = 0; i < numNodes; i++ )
	{
		gi.FS_Write( &m_ranks[i], sizeof( int ), file );
	}

	return true;
}

/*
-------------------------
Load
-------------------------
*/

int CNode::Load( int numNodes, fileHandle_t file )
{
	unsigned long header;
	gi.FS_Read( &header, sizeof(header), file );

	//Validate the header
	if ( header != NODE_HEADER_ID )
		return false;

	//Get the basic information
	for ( int i = 0; i < 3; i++ )
		gi.FS_Read( &m_position[i], sizeof( float ), file );

	gi.FS_Read( &m_flags, sizeof( m_flags ), file );
	gi.FS_Read( &m_ID, sizeof( m_ID ), file );
	gi.FS_Read( &m_radius, sizeof( m_radius ), file );

	//Get the edge information
	gi.FS_Read( &m_numEdges, sizeof( m_numEdges ), file );

	for ( int i = 0; i < m_numEdges; i++ )
	{
		edge_t	edge;

		gi.FS_Read( &edge, sizeof( edge_t ), file );

		STL_INSERT( m_edges, edge );
	}

	//Read the node ranks
	int	numRanks;

	gi.FS_Read( &numRanks, sizeof( numRanks ), file );

	//Allocate the memory
	InitRanks( numRanks );

	for ( int i = 0; i < numRanks; i++ )
	{
		gi.FS_Read( &m_ranks[i], sizeof( int ), file );
	}

	return true;
}

/*
-------------------------
CNavigator
-------------------------
*/

CNavigator::CNavigator( void )
{
}

CNavigator::~CNavigator( void )
{
}

/*
-------------------------
GetChar
-------------------------
*/

char CNavigator::GetChar( fileHandle_t file )
{
	char value;

	gi.FS_Read( &value, sizeof( value ), file );

	return value;
}

/*
-------------------------
GetInt
-------------------------
*/

int	CNavigator::GetInt( fileHandle_t file )
{
	int value;

	gi.FS_Read( &value, sizeof( value ), file );

	return value;
}

/*
-------------------------
GetFloat
-------------------------
*/

float CNavigator::GetFloat( fileHandle_t file )
{
	float value;

	gi.FS_Read( &value, sizeof( value ), file );

	return value;
}

/*
-------------------------
GetLong
-------------------------
*/

long CNavigator::GetLong( fileHandle_t file )
{
	long value;

	gi.FS_Read( &value, sizeof( value ), file );

	return value;
}

/*
-------------------------
Init
-------------------------
*/

void CNavigator::Init( void )
{
	Free();
}

/*
-------------------------
Free
-------------------------
*/

void CNavigator::Free( void )
{
	node_v::iterator	ni;

	STL_ITERATE( ni, m_nodes )
	{
		delete (*ni);
	}
}

/*
-------------------------
Load
-------------------------
*/

bool CNavigator::Load( const char *filename, int checksum )
{
	fileHandle_t	file;

	//Attempt to load the file
	gi.FS_FOpenFile( va( "maps/%s.nav", filename ), &file, FS_READ );

	//See if we succeeded
	if ( file == NULL )
		return false;

	//Check the header id
	long navID = GetLong( file );

	if ( navID != NAV_HEADER_ID )
	{
		gi.FS_FCloseFile( file );
		return false;
	}

	//Check the checksum to see if this file is out of date
	int check = GetInt( file );

	if ( check != checksum )
	{
		gi.FS_FCloseFile( file );
		return false;
	}

	int numNodes = GetInt( file );
	
	for ( int i = 0; i < numNodes; i++ )
	{
		CNode	*node = CNode::Create();

		if ( node->Load( numNodes, file ) == false )
		{
			gi.FS_FCloseFile( file );
			return false;
		}

		STL_INSERT( m_nodes, node );
	}

	gi.FS_FCloseFile( file );

	return true;
}

/*
-------------------------
Save
-------------------------
*/

bool CNavigator::Save( const char *filename, int checksum )
{
	fileHandle_t	file;

	//Attempt to load the file
	gi.FS_FOpenFile( va( "maps/%s.nav", filename ), &file, FS_WRITE );

	if ( file == NULL )
		return false;

	//Write out the header id
	unsigned long id = NAV_HEADER_ID;

	gi.FS_Write( &id, sizeof (id), file );

	//Write out the checksum
	gi.FS_Write( &checksum, sizeof( checksum ), file );

	int	numNodes = m_nodes.size();

	//Write out the number of nodes to follow
	gi.FS_Write( &numNodes, sizeof(numNodes), file );

	//Write out all the nodes
	node_v::iterator	ni;

	STL_ITERATE( ni, m_nodes )
	{
		(*ni)->Save( numNodes, file );
	}

	gi.FS_FCloseFile( file );

	return true;
}

/*
-------------------------
AddRawPoint
-------------------------
*/

int CNavigator::AddRawPoint( vec3_t point, int flags, int radius )
{
	CNode	*node	= CNode::Create( point, flags, radius, m_nodes.size() );

	if ( node == NULL )
	{
		Com_Error( ERR_DROP, "Error adding node!\n" );
		return -1;
	}
	
	//TODO: Validate the position
	//TODO: Correct stuck waypoints

	STL_INSERT( m_nodes, node );

	return node->GetID();
}

/*
-------------------------
GetEdgeCost
-------------------------
*/

int	CNavigator::GetEdgeCost( CNode *first, CNode *second )
{
	trace_t	trace;
	vec3_t	start, end;
	vec3_t	mins, maxs;

	//Setup the player size
	VectorSet( mins, -8, -8, -8 );
	VectorSet( maxs,  8,  8,  8 );

	//Setup the points
	first->GetPosition( start );
	second->GetPosition( end );

	gi.trace( &trace, start, mins, maxs, end, -1, MASK_SOLID );

	if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		return -1;

	//Connection successful, return the cost
	return Distance( start, end );
}

/*
-------------------------
ConnectNodes
-------------------------
*/

void CNavigator::ConnectNodes( void )
{
	node_v::iterator	ni, ni2;
	int					id = 0;
	int					cost;

	//Attempt to connect all nodes
	STL_ITERATE( ni, m_nodes )
	{
		//Attempt connection against all nodes in the system
		//TODO: Culling routines could speed this up
		STL_ITERATE( ni2, m_nodes )
		{
			if ( (*ni) == (*ni2 ) )
				continue;

			cost = GetEdgeCost( (*ni), (*ni2) );

			//No connection was made
			if ( cost < 0 )
				continue;

			//Connect the edges
			(*ni)->AddEdge( (*ni2)->GetID(), cost );
			(*ni2)->AddEdge( (*ni)->GetID(), cost );
		}

		id++;
	}
}

/*
-------------------------
AddNodeEdges
-------------------------
*/

void CNavigator::AddNodeEdges( CNode *node, int addDist, edge_l	&edgeList, bool *checkedNodes )
{
	//Add all edge
	for ( int i = 0; i < node->GetNumEdges(); i++ )
	{
		//Make sure we don't add an old edge twice
		if ( checkedNodes[ node->GetEdge( i ) ] == true )
			continue;

		//Get the node
		CNode	*nextNode = m_nodes[ node->GetEdge( i ) ];

		//This node has now been checked
		checkedNodes[ nextNode->GetID() ] = true;

		//Add it to the list
		STL_INSERT( edgeList, CEdge( nextNode->GetID(), node->GetID(), addDist + ( node->GetEdgeCost( i ) ) ) );
	}
}

/*
-------------------------
CalculatePath
-------------------------
*/

void CNavigator::CalculatePath( CNode *node )
{
	int	curRank = 0;

	edge_l	pathList;
	BYTE	*checked;

	//Init the completion table
	checked = new BYTE[ m_nodes.size() ];
	memset( checked, 0, m_nodes.size() );

	//Mark this node as checked
	checked[ node->GetID() ] = true;
	node->AddRank( node->GetID(), curRank++ );

	//Add all initial nodes
	for ( int i = 0; i < node->GetNumEdges(); i++ )
	{
		CNode	*nextNode = m_nodes[ node->GetEdge(i) ];
		assert(nextNode);

		checked[ nextNode->GetID() ] = true;

		STL_INSERT( pathList, CEdge( nextNode->GetID(), nextNode->GetID(), node->GetEdgeCost(i) ) );
	}

	float				minDist;
	edge_l::iterator	test;
	edge_l::iterator	ei;

	//Now flood fill all the others
	while ( pathList.size() )
	{
		minDist = 999999;
		test	= pathList.end();

		STL_ITERATE( ei, pathList )
		{
			if ( (*ei).m_cost < minDist )
			{
				minDist = (*ei).m_cost;
				test	= ei;
			}
		}

		CNode	*testNode = m_nodes[ (*test).m_first ];
		assert( testNode );

		node->AddRank( testNode->GetID(), curRank++ );

		//Add in all the new edges
		for ( int i = 0; i < testNode->GetNumEdges(); i++ )
		{
			CNode	*addNode = m_nodes[ testNode->GetEdge(i) ];
			assert( addNode );

			if ( checked[ addNode->GetID() ] )
				continue;

			int	newDist = (*test).m_cost + testNode->GetEdgeCost(i);
			STL_INSERT( pathList, CEdge( addNode->GetID(), (*test).m_second, newDist ) );

			checked[ addNode->GetID() ] = true;
		}

		pathList.erase( test );
	}

	pathList.clear();
	delete [] checked;
}

/*
-------------------------
CalculatePaths
-------------------------
*/

void CNavigator::CalculatePaths( void )
{
#if _HARD_CONNECT

#else

	ConnectNodes();

#endif	

	for ( int i = 0; i < m_nodes.size(); i++ )
	{
		//Allocate the needed memory
		m_nodes[i]->InitRanks( m_nodes.size() );
	}

	for ( int i = 0; i < m_nodes.size(); i++ )
	{
		CalculatePath( m_nodes[i] );
	}
}

/*
-------------------------
ShowNodes
-------------------------
*/

void CNavigator::ShowNodes( void )
{
	node_v::iterator	ni;

	vec3_t	position;

	STL_ITERATE( ni, m_nodes )
	{
		(*ni)->GetPosition( position );

		if ( gi.inPVS( g_entities[0].currentOrigin, position ) )
			(*ni)->Draw();
	}
}

/*
-------------------------
ShowEdges
-------------------------
*/

typedef	map < int, bool >		drawMap_m;

void CNavigator::ShowEdges( void )
{
	node_v::iterator	ni;
	vec3_t	start, end;

	drawMap_m	*drawMap;

	drawMap = new drawMap_m[ m_nodes.size() ];

	STL_ITERATE( ni, m_nodes )
	{
		//Attempt to draw each connection
		for ( int i = 0; i < (*ni)->GetNumEdges(); i++ )
		{
			int id = (*ni)->GetEdge( i );

			if ( id == -1 )
				continue;

			//Already drawn?
			if ( drawMap[(*ni)->GetID()].find( id ) != drawMap[(*ni)->GetID()].end() )
				continue;

			BYTE flags = (*ni)->GetEdgeFlags( i );

			CNode	*node = m_nodes[id];

			node->GetPosition( end );
			(*ni)->GetPosition( start );

			//Set this as drawn
			drawMap[id][(*ni)->GetID()] = true;

			if ( ( gi.inPVS( g_entities[0].currentOrigin, start ) == false ) && ( gi.inPVS( g_entities[0].currentOrigin, end ) == false ) )
				continue;

			if ( flags & EFLAG_BROKEN )
				CG_DrawEdge( start, end, EDGE_BROKEN );
			else
				CG_DrawEdge( start, end, EDGE_NORMAL );
		}
	}

	delete [] drawMap;
}

#if _HARD_CONNECT

/*
-------------------------
HardConnect
-------------------------
*/

void CNavigator::HardConnect( int first, int second, qboolean oneway )
{
	CNode	*start, *end;

	start	= m_nodes[first];
	end		= m_nodes[second];

	vec3_t	p1, p2;

	start->GetPosition( p1 );
	end->GetPosition( p2 );

	trace_t	trace;
	
	vec3_t	maxs = {  12,  12, 32 };
	vec3_t	mins = { -12, -12, 16 };

	int		flags = EFLAG_NONE;

	gi.trace( &trace, p1, mins, maxs, p2, -1, MASK_SOLID );

	int cost = Distance( p1, p2 );
	
	if ( trace.fraction != 1.0f )
		flags |= EFLAG_BROKEN;

	start->AddEdge( second, cost, flags );
	if ( !oneway )
	{
		end->AddEdge( first, cost, flags );
	}
}

#endif

/*
-------------------------
TestNodePath
-------------------------
*/

int CNavigator::TestNodePath( gentity_t *ent, vec3_t position )
{
	//Check the path
	if ( NAV_ClearPathToPoint( ent, ent->mins, ent->maxs, position, ent->clipmask&~CONTENTS_BODY ) == false )
		return false;
	
	return true;
}

/*
-------------------------
TestNodeLOS
-------------------------
*/

int CNavigator::TestNodeLOS( gentity_t *ent, vec3_t position )
{
	return NPC_ClearLOS( ent, position );
}

/*
-------------------------
TestBestFirst
-------------------------
*/

int	CNavigator::TestBestFirst( gentity_t *ent, int lastID, int flags )
{
	//Must be a valid one to begin with
	if ( lastID == NODE_NONE )
		return NODE_NONE;

	if ( lastID >= m_nodes.size() )
		return NODE_NONE;

	//Get the info
	vec3_t	nodePos;
	CNode	*node = m_nodes[ lastID ];
	CNode	*testNode;
	int		numEdges = node->GetNumEdges();
	float	dist;

	node->GetPosition( nodePos );

	//Setup our last node as our root, and search for a closer one according to its edges	
	int bestNode = ( TestNodePath( ent, nodePos ) ) ? lastID : NODE_NONE;
	float bestDist = ( bestNode == NODE_NONE ) ? 9999999.0f : DistanceSquared( ent->currentOrigin, nodePos );

	//Test all these edges first
	for ( int i = 0; i < numEdges; i++ )
	{
		//Get this node and its distance
		testNode = m_nodes[ node->GetEdge(i) ];

		testNode->GetPosition( nodePos );

		dist = DistanceSquared( ent->currentOrigin, nodePos );

		//Test against current best
		if ( dist < bestDist )
		{
			//See if this node is valid
			if ( TestNodePath( ent, nodePos ) )
			{
				bestDist = dist;
				bestNode = testNode->GetID();
			}
		}
	}

	return bestNode;
}

/*
-------------------------
CollectNearestNodes
-------------------------
*/

#define	NODE_COLLECT_MAX	16		//Maximum # of nodes collected at any time
#define NODE_COLLECT_RADIUS	512		//Default radius to search for nodes in
#define NODE_COLLECT_RADIUS_SQR		( NODE_COLLECT_RADIUS * NODE_COLLECT_RADIUS )

#if __NEWCOLLECT
int CNavigator::CollectNearestNodes( vec3_t origin, int radius, int maxCollect, nodeChain_l &nodeChain )
#else
int CNavigator::CollectNearestNodes( vec3_t origin, int radius, int maxCollect, int *nodeChain )
#endif	//__NEWCOLLECT
{
	node_v::iterator	ni;
	float				dist;
	vec3_t				position;
	int					collected = 0;
	bool				added = false;

	//Get a distance rating for each node in the system
	STL_ITERATE( ni, m_nodes )
	{
		//If we've got our quota, then stop looking
#if !__NEWCOLLECT

		if ( collected >= maxCollect )
			break;

#endif	//!__NEWCOLLECT

		//Get the distance to the node
		(*ni)->GetPosition( position );
		dist = DistanceSquared( position, origin );

		//Must be within our radius range
		if ( dist > (float) ( radius * radius ) )
			continue;
		
#if __NEWCOLLECT

		nodeList_t				nChain;
		nodeChain_l::iterator	nci;

		//Always add the first node
		if ( nodeChain.size() == 0 )
		{
			nChain.nodeID = (*ni)->GetID();
			nChain.distance = dist;

			nodeChain.insert( nodeChain.begin(), nChain );
			continue;
		}

		added = false;

		//Compare it to what we already have
		STL_ITERATE( nci, nodeChain )
		{
			//If we're less, than this entry, then insert before it
			if ( dist < (*nci).distance )
			{
				nChain.nodeID = (*ni)->GetID();
				nChain.distance = dist;

				nodeChain.insert( nci, nChain );
				collected = nodeChain.size();
				added = true;
				
				//If we've hit our collection limit, throw off the oldest one
				if ( nodeChain.size() > maxCollect )
				{
					nodeChain.pop_back();
				}

				break;
			}
		}

		//Otherwise, always pad out the collection if possible so we don't miss anything
		if ( ( added == false ) && ( nodeChain.size() < maxCollect ) )
		{
			nChain.nodeID = (*ni)->GetID();
			nChain.distance = dist;

			nodeChain.insert( nodeChain.end(), nChain );
		}

#else	//__NEWCOLLECT

		//Add it to our list
		nodeChain[collected++] = (*ni)->GetID();

#endif	//__NEWCOLLECT

	}

	return collected;
}

/*
-------------------------
GetNearestWaypoint
-------------------------
*/

int CNavigator::GetNearestNode( gentity_t *ent, int lastID, int flags )
{
	//Must have nodes
	if ( m_nodes.size() == 0 )
		return NODE_NONE;

	//Try and find an early match using our last node
	int bestNode = TestBestFirst( ent, lastID, flags );

	if ( bestNode != NODE_NONE )
		return bestNode;

/////////////////////////////////////////////////

#if __NEWCOLLECT

#define	MAX_Z_DELTA	18

/////////////////////////////////////////////////

	nodeChain_l				nodeChain;
	nodeChain_l::iterator	nci;

	//Collect all nodes within a certain radius
	CollectNearestNodes( ent->currentOrigin, NODE_COLLECT_RADIUS, NODE_COLLECT_MAX, nodeChain );

	vec3_t				position;
	int					radius;
	CNode				*node;

	//Look through all nodes
	STL_ITERATE( nci, nodeChain )
	{
		node = m_nodes[(*nci).nodeID];

		node->GetPosition( position );

		radius	= node->GetRadius();
		
		//Are we within the known clear radius of this node?
		if ( (*nci).distance < (radius*radius) )
		{
			//Do a z-difference sanity check
			if ( fabs( position[2] - ent->currentOrigin[2] ) < MAX_Z_DELTA )
			{
				//Found one		
				return (*nci).nodeID;
			}
		}
	
		//Do we need a clear path?
		if ( flags & NF_CLEAR_PATH )
		{
			if ( TestNodePath( ent, position ) == false )
				continue;
		}

		//Do we need a clear line of sight?
		if ( flags & NF_CLEAR_LOS )
		{
			if ( TestNodeLOS( ent, position ) == false )
				continue;
		}

		//Found one, we're done
		return (*nci).nodeID;
	}

/////////////////////////////////////////////////

#else	//__NEWCOLLECT

/////////////////////////////////////////////////

	static int	nodeChain[NODE_COLLECT_MAX];

	//Collect all nodes within a certain radius
	int collected = CollectNearestNodes( ent->currentOrigin, NODE_COLLECT_RADIUS, NODE_COLLECT_MAX, nodeChain );

	vec3_t				position;
	float				dist, bestDist = 99999999.0f;
	int					radius;
	CNode				*node;

	//Look through all nodes
	for ( int i = 0; i < collected; i++ )
	{
		node = m_nodes[nodeChain[i]];

		node->GetPosition( position );

		dist	= DistanceSquared( position, ent->currentOrigin );
		radius	= node->GetRadius();
		
		if ( dist < (radius*radius) )
		{
			//Do a z-difference sanity check
			if ( fabs( position[2]-ent->currentOrigin[2] ) < 18 )
			{
				bestDist = dist;
				bestNode = nodeChain[i];
			}

			break;
		}

		if ( dist < bestDist )
		{
			if ( flags & NF_CLEAR_PATH )
			{
				if ( TestNodePath( ent, position ) == false )
					continue;
			}

			if ( flags & NF_CLEAR_LOS )
			{
				if ( TestNodeLOS( ent, position ) == false )
					continue;
			}

			bestDist = dist;
			bestNode = nodeChain[i];
		}
	}

	//Brute force through the whole system
	if ( /*( bestNode == NODE_NONE ) &&*/ ( collected == NODE_COLLECT_MAX ) )	//This denotes a collection limit hit
	{
		//NOTENOTE: If you hit this assert, the entity has failed all other attempts to find a node.  Meaning that 
		//			he is either placed too far away from any surrounding nodes (256 away) or there are too many
		//			nodes, too densly packed into one area.  This can most likely be fixed by either reducing the
		//			node density of an area, or placing the NPC in question closer to a node.  This is not an error,
		//			but it is a full search of the system, which should be avoided!

		Com_Printf( "CNavigator::GetNearestNode() : WARNING : Full system search\n" );

		node_v::iterator	ni;

		STL_ITERATE( ni, m_nodes )
		{
			(*ni)->GetPosition( position );

			dist = DistanceSquared( position, ent->currentOrigin );

			if ( dist < bestDist )
			{
				if ( flags & NF_CLEAR_PATH )
				{
					if ( TestNodePath( ent, position ) == false )
						continue;
				}

				if ( flags & NF_CLEAR_LOS )
				{
					if ( TestNodeLOS( ent, position ) == false )
						continue;
				}

				bestDist = dist;
				bestNode = (*ni)->GetID();
			}
		}
	}

#endif	//__NEWCOLLECT

	return bestNode;
}

/*
-------------------------
ShowPath
-------------------------
*/

void CNavigator::ShowPath( int start, int end )
{	
	//Validate the start position
	if ( ( start < 0 ) || ( start > m_nodes.size() ) )
		return;

	//Validate the end position
	if ( ( end < 0 ) || ( end > m_nodes.size() ) )
		return;

	CNode	*startNode	= m_nodes[ start ];
	CNode	*endNode	= m_nodes[ end ];
	
	CNode	*moveNode = startNode;
	CNode	*testNode = NULL;

	int		bestNode;
	vec3_t	startPos, endPos;

	int		runAway = 0;

	//Draw out our path
	while ( moveNode != endNode )
	{
		bestNode = GetBestNode( moveNode->GetID(), end );
		
		//Some nodes may be fragmented
		if ( bestNode == -1 )
		{
			Com_Printf("No connection possible between node %d and %d\n", start, end );
			return;
		}

		//This is our next node on the path
		testNode = m_nodes[ bestNode ];

		//Get their origins
		moveNode->GetPosition( startPos );
		testNode->GetPosition( endPos );
		
		//Draw the edge
		CG_DrawEdge( startPos, endPos, EDGE_PATH );

		//Take a new best node
		moveNode = testNode;

		if ( runAway++ > 64 )
		{
			Com_Printf("Potential Run-away path!\n");
			return;
		}
	}
}

/*
-------------------------
GetBestNode
-------------------------
*/

int CNavigator::GetBestNode( int startID, int endID, int rejectID )
{
	//Validate the start position
	if ( ( startID < 0 ) || ( startID > m_nodes.size() ) )
		return WAYPOINT_NONE;

	//Validate the end position
	if ( ( endID < 0 ) || ( endID > m_nodes.size() ) )
		return WAYPOINT_NONE;

	if ( startID == endID )
		return startID;

	CNode	*start	= m_nodes[ startID ];
	CNode	*end	= m_nodes[ endID ];

	int		bestNode = -1;
	int		bestRank = 99999999;
	int		testRank, rejectRank = 0;

	if ( rejectID != WAYPOINT_NONE )
	{
		for ( int i = 0; i < start->GetNumEdges(); i++ )
		{
			if ( start->GetEdge(i) == rejectID )
			{
				rejectRank = end->GetRank( start->GetEdge(i) );
				break;
			}
		}
	}

	for ( int i = 0; i < start->GetNumEdges(); i++ )
	{
		int	edgeID = start->GetEdge(i);

		//Found one
		if ( edgeID == endID )
			return edgeID;

		testRank = end->GetRank( edgeID );

		//Found one
		if ( testRank <= rejectRank )
			continue;

		//No possible connection
		if ( testRank == NODE_NONE )
			return NODE_NONE;

		//Found a better one
		if ( testRank < bestRank )
		{
			bestNode = edgeID;
			bestRank = testRank;
		}
	}

	return bestNode;
}

/*
-------------------------
GetNodePosition
-------------------------
*/

int CNavigator::GetNodePosition( int nodeID, vec3_t out )
{
	//Validate the number
	if ( ( nodeID < 0 ) || ( nodeID >= m_nodes.size() ) )
		return false;

	CNode	*node = m_nodes[ nodeID ];

	node->GetPosition( out );

	return true;
}

/*
-------------------------
GetNodeNumEdges
-------------------------
*/

int CNavigator::GetNodeNumEdges( int nodeID )
{
	if ( ( nodeID < 0 ) || ( nodeID >=  m_nodes.size() ) )
		return -1;

	CNode	*node = m_nodes[ nodeID ];

	assert( node );

	return node->GetNumEdges();
}

/*
-------------------------
GetNodeEdge
-------------------------
*/

int CNavigator::GetNodeEdge( int nodeID, int edge )
{
	if ( ( nodeID < 0 ) || ( nodeID >=  m_nodes.size() ) )
		return -1;

	CNode	*node = m_nodes[ nodeID ];

	assert( node );

	return node->GetEdge( edge );
}

/*
-------------------------
Connected
-------------------------
*/

bool CNavigator::Connected( int startID, int endID )
{
	//Validate the start position
	if ( ( startID < 0 ) || ( startID > m_nodes.size() ) )
		return false;

	//Validate the end position
	if ( ( endID < 0 ) || ( endID > m_nodes.size() ) )
		return false;

	if ( startID == endID )
		return true;

	CNode	*start	= m_nodes[ startID ];
	CNode	*end	= m_nodes[ endID ];

	for ( int i = 0; i < start->GetNumEdges(); i++ )
	{
		int	edgeID = start->GetEdge(i);

		//Found one
		if ( edgeID == endID )
			return true;

		if ( ( end->GetRank( edgeID ) ) != NODE_NONE )
			return true;
	}

	return false;
}


/*
-------------------------
GetPathCost
-------------------------
*/

unsigned int CNavigator::GetPathCost( int startID, int endID )
{
	//Validate the start position
	if ( ( startID < 0 ) || ( startID > m_nodes.size() ) )
		return 0;

	//Validate the end position
	if ( ( endID < 0 ) || ( endID > m_nodes.size() ) )
		return 0;

	CNode	*startNode	= m_nodes[ startID ];
	CNode	*endNode	= m_nodes[ endID ];
	
	CNode	*moveNode = startNode;

	int		bestNode;
	int		pathCost = 0;
	int		bestCost;

	int		bestRank;
	int		testRank;

	//Draw out our path
	while ( moveNode != endNode )
	{
		bestRank = 99999999;
		bestNode = -1;
		bestCost = 0;

		for ( int i = 0; i < moveNode->GetNumEdges(); i++ )
		{
			int	edgeID = moveNode->GetEdge(i);

			//Done
			if ( edgeID == endID )
			{
				return pathCost + moveNode->GetEdgeCost( i );
			}	

			testRank = endNode->GetRank( edgeID );

			//No possible connection
			if ( testRank == NODE_NONE )
				return 0;

			//Found a better one
			if ( testRank < bestRank )
			{
				bestNode = edgeID;
				bestRank = testRank;
				bestCost = moveNode->GetEdgeCost( i );
			}
		}

		pathCost += bestCost;

		//Take a new best node
		moveNode = m_nodes[ bestNode ];
	}

	return pathCost;
}

/*
-------------------------
GetEdgeCost
-------------------------
*/

unsigned int CNavigator::GetEdgeCost( int startID, int endID )
{
	//Validate the start position
	if ( ( startID < 0 ) || ( startID > m_nodes.size() ) )
		return 0;

	//Validate the end position
	if ( ( endID < 0 ) || ( endID > m_nodes.size() ) )
		return 0;

	CNode	*start	= m_nodes[startID];
	CNode	*end	= m_nodes[endID];

	return GetEdgeCost( start, end );
}

/*
-------------------------
GetProjectedNode
-------------------------
*/

int CNavigator::GetProjectedNode( vec3_t origin, int nodeID )
{
	//Validate the start position
	if ( ( nodeID < 0 ) || ( nodeID > m_nodes.size() ) )
		return NODE_NONE;

	CNode	*node = m_nodes[nodeID];
	CNode	*tempNode;

	float	bestDot		= 0.0f;
	int		bestNode	= NODE_NONE;

	vec3_t	targetDir, basePos, tempDir, tempPos;
	float	dot;

	//Setup our target direction
	node->GetPosition( basePos );

	VectorSubtract( origin, basePos, targetDir );
	VectorNormalize( targetDir );

	//Go through all the edges
	for ( int i = 0; i < node->GetNumEdges(); i++ )
	{
		tempNode = m_nodes[node->GetEdge(i)];
		tempNode->GetPosition( tempPos );

		VectorSubtract( tempPos, basePos, tempDir );
		VectorNormalize( tempDir );	//FIXME: Retain the length here if you want it

		dot = DotProduct( targetDir, tempDir );

		if ( dot < 0.0f )
			continue;

		if ( dot > bestDot )
		{
			bestDot		= dot;
			bestNode	= tempNode->GetID();
		}
	}

	return bestNode;
}

