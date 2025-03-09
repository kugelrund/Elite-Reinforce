#ifndef __G_NAVIGATOR__
#define __G_NAVIGATOR__

#define	__NEWCOLLECT	1

#define _HARD_CONNECT	1

//Node flags
#define	NF_ANY			0
#define	NF_CLEAR_LOS	0x00000001
#define NF_CLEAR_PATH	0x00000002

//Edge flags
#define	EFLAG_NONE		0
#define EFLAG_BROKEN	0x00000001

//Miscellaneous defines
#define	NODE_NONE		-1
#define	NAV_HEADER_ID	'JNV2'
#define	NODE_HEADER_ID	'NODE'

/*
-------------------------
CEdge
-------------------------
*/

class CEdge
{

public:

	CEdge( void );
	CEdge( int first, int second, int cost );
	~CEdge( void );

	int		m_first;
	int		m_second;
	int		m_cost;
};

/*
-------------------------
CNode
-------------------------
*/

class CNode
{
	typedef	struct edge_s
	{
		int		ID;
		int		cost;
		BYTE	flags;
	} edge_t;

	typedef	std::vector< edge_t >	edge_v;

public:

	CNode( void );
	~CNode( void );

	static CNode *Create( vec3_t position, int flags, int radius, int ID );
	static CNode *Create( void );

	void AddEdge( int ID, int cost, int flags = EFLAG_NONE );
	void AddRank( int ID, int rank );

	void Draw( void );

	int	GetID( void )					const	{	return m_ID;	}
	void GetPosition( vec3_t position )	const	{	if ( position )	VectorCopy( m_position, position );	}
 	
	int GetNumEdges( void )				const	{	return m_numEdges;	}
	int GetEdge( int edgeNum );
	int GetEdgeCost( int edgeNum );
	BYTE GetEdgeFlags( int edgeNum );
	int	GetRadius( void )				const	{	return m_radius;	}

	void InitRanks( int size );
	int GetRank( int ID );

	int	Save( int numNodes, fileHandle_t file );
	int Load( int numNodes, fileHandle_t file );

protected:

	vec3_t			m_position;
	int				m_flags;
	int				m_radius;
	int				m_ID;
	
	edge_v	m_edges;

	int		*m_ranks;
	int		m_numEdges;
};

/*
-------------------------
CNavigator
-------------------------
*/

class CNavigator
{
	typedef	std::vector < CNode * >			node_v;
	typedef	std::list < CEdge >				edge_l;

#if __NEWCOLLECT
	
	typedef struct nodeList_t
	{
		int				nodeID;
		unsigned int	distance;
	};

	typedef std::list < nodeList_t >		nodeChain_l;

#endif	//__NEWCOLLECT

public:

	CNavigator( void );
	~CNavigator( void );

	void Init( void );
	void Free( void );

	bool Load( const char *filename, int checksum );
	bool Save( const char *filename, int checksum );

	int  AddRawPoint( vec3_t point, int flags, int radius );
	void CalculatePaths( void );

#if _HARD_CONNECT

	void HardConnect( int first, int second, qboolean oneway );

#endif

	void ShowNodes( void );
	void ShowEdges( void );
	void ShowPath( int start, int end, bool draw_nodes = false );

	int GetNearestNode( gentity_t *ent, int lastID, int flags );

	int GetBestNode( int startID, int endID, int rejectID = NODE_NONE );

	int GetNodePosition( int nodeID, vec3_t out );
	int GetNodeNumEdges( int nodeID );
	int GetNodeEdge( int nodeID, int edge );
	float GetNodeLeadDistance( int nodeID );

	int GetNumNodes( void )		const	{	return m_nodes.size();		}
	
	bool Connected( int startID, int endID );

	unsigned int GetPathCost( int startID, int endID );
	unsigned int GetEdgeCost( int startID, int endID );

	int GetProjectedNode( vec3_t origin, int nodeID );

protected:

	int		TestNodePath( gentity_t *ent, vec3_t position );
	int		TestNodeLOS( gentity_t *ent, vec3_t position );
	int		TestBestFirst( gentity_t *ent, int lastID, int flags );
	
#if __NEWCOLLECT
	int		CollectNearestNodes( vec3_t origin, int radius, int maxCollect, nodeChain_l &nodeChain );
#else
	int		CollectNearestNodes( vec3_t origin, int radius, int maxCollect, int *nodeChain );
#endif	//__NEWCOLLECT

	char	GetChar( fileHandle_t file );
	int		GetInt( fileHandle_t file );
	float	GetFloat( fileHandle_t file );
	long	GetLong( fileHandle_t file );

	void	ConnectNodes( void );
	int		GetEdgeCost( CNode *first, CNode *second );
	void	AddNodeEdges( CNode *node, int addDist, edge_l &edgeList, bool *checkedNodes );

	void	CalculatePath( CNode *node );

	node_v			m_nodes;
};

#endif	//__G_NAVIGATOR__