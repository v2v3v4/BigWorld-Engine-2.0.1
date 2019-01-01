/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "bsp.hpp"

#ifndef CODE_INLINE
	#include "bsp.ipp"
#endif

#include <list>
#include <vector>

#include "cstdmf/vectornodest.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/bwresource.hpp"


#ifndef MF_SERVER
#include "romp/progress.hpp"
#endif

DECLARE_DEBUG_COMPONENT2( "Physics", 0 );

template <class TYPE>
uint32 contentSize( const std::vector< TYPE >& vector )
{
	return vector.size() * sizeof( TYPE );
}

/*
 *	Description of file format:
 *
 *	In BNF format,
 *	<bsp_file>     ::= <header><triangle>*<node>*<userData>*
 *	<header>       ::= <magic><numTriangles><maxTriangles><numNodes>
 *	<triangle>     ::= <Vector3><Vector3><Vector3>  // Three points
 *	<Vector3>      ::= <float><float><float>  // x, y, z
 *	<node>         ::= <nodeFlags><planeEq><numIndexes><triangleIndex>*
 *	     // The nodes are written in prefix order. this, front tree, back tree
 *	<userData>     ::= <userDataKey><userDataBlob>
 *	<userDataBlob> ::= <blobSize><byte>*
 *	<magic>        ::= 0x00505342  // 32 bits
 *	<numTriangles> ::= uint32  // Number of triangles in <triangle>*
 *	<maxTriangles> ::= uint32  // Maximum size of all <triangleIndex>* lists
 *	<numNodes>     ::= uint32  // Number of nodes in <node>*
 *	<nodeFlags>    ::= <reserved><isPartitioned><hasFront><hasBack> // A byte
 *	<reserved>     ::= 5 bits  // Must be 10100
 *	<isPartitioned>::= 1 bit   // 1 if all triangles lie on this node's plane
 *	<hasFront>     ::= 1 bit   // 1 if node has a front child
 *	<hasBack>      ::= 1 bit   // 1 if node has a back child
 *	<planeEq>      ::= <normal><d>  // Plane is: p . normal == d
 *	<normal>       ::= <Vector3>  // Normal to plane in direction of front
 *	<d>            ::= float   // Such that plane is: p . normal == d
 *	<numIndexes>   ::= uint16  // Number of <triangleIndex> to follow
 *	<triangleIndex>::= uint16  // Index into the <triangle>* list of triangles
 *	<userDataKey>  ::= uint32  // User defined key associated with data
 *	<blobSize>     ::= uint32  // Number of bytes in following blob
 *
 *	In words:
 *	Magic number (including version number in last byte)
 *	Number of triangles as int32
 *	Max number of triangles for a single node as int32
 *	Number of nodes as int32
 *	List of triangles - (9 4-byte floats each. i.e. 3 Vector3s)
 *	All nodes of BSP in prefix order (front node before back node)
 *
 *	A node starts with 1 byte for flags. The first 5 bits should match 0xa0.
 *	The last three bits in order are:
 *		IS_PARTITIONED - set if all triangles lie on this node's plane
 *		HAS_FRONT - set if it has a front child
 *		HAS_BACK - set if it has a back child
 *	The plane equation for the node - Vector3 for normal, float for d.
 *		Plane is set of p such that: normal . p == d (where . is dot product)
 *		Normal is in direction of front.
 *	Number of triangles associated with this node as uint16
 *	List of triangle indexes as uint16s (index into file's global triangle list)
 *	At the end of the file is user data. Each user data is a 4 byte key followed
 *	by a uint32 size prefixed blob.
 */


// -----------------------------------------------------------------------------
// Section: BSP statics
// -----------------------------------------------------------------------------

/// This constant is the maximum size of an unpartitioned leaf node.
const int BSP::MAX_SIZE = 10;

/// This constant is the tolerance from the plane equation that a triangle is
/// added to the "on" set.
const float BSP::TOLERANCE = 0.01f;


// -----------------------------------------------------------------------------
// Section: BSPAllocator
// -----------------------------------------------------------------------------

/**
 *	This class is used to allocate and deallocate objects of type BSP. When we
 *	load from a file, we know how many nodes there are so we allocate them all
 *	at once. We also know that they are deleted together.
 */
class BSPAllocator
{
public:
	BSPAllocator( char * pMem ) : pNodeMemory_( pMem ) {};

	/**
	 *	This method creates a new BSP object. If this allocator has a pool of
	 *	memory, this is used for the allocation.
	 */
	BSP * newBSP()
	{
		BSP * pBSP;

		if (pNodeMemory_)
		{
			// If we have allocated them all together, get the memory from the
			// pool.
			pBSP = new (pNodeMemory_) BSP();
			pNodeMemory_ += sizeof( BSP );
		}
		else
		{
			pBSP = new BSP();
		}

		return pBSP;
	}

	/**
	 *	This method destroys the BSP object. If this object was allocated from
	 *	the pool, we only need to call the destructor and not delete the memory.
	 */
	void destroy( BSP * pBSP )
	{
		BSP * pCurr = pBSP;

		// Here we unroll one side of the recursion to speed things up. We
		// could look at fully removing the recursion.
		while (pCurr)
		{
			if (pCurr->pBack_)
				this->destroy( pCurr->pBack_ );

			BSP * pNext = pCurr->pFront_;

			// If it is from the allocator, just call the destructor and do
			// not delete the memory.
			if (pNodeMemory_)
				pCurr->~BSP();
			else
				delete pCurr;
			pCurr = pNext;
		}
	}

private:
	char * pNodeMemory_;
};


// -----------------------------------------------------------------------------
// Section: BSP Constructor
// -----------------------------------------------------------------------------

/**
 *	This class is used to get rid of the recursion in the construction of BSP
 *	trees. Basically, it keeps a stack of BSP nodes that are yet to be processed
 *	instead of keeping these on the call stack.
 */
class BSPConstructor
{
private:
	struct Element
	{
		BSP * pBSP_;
		WTriangleSet tris_;
		WPolygonSet polys_;
	};

public:
	BSPConstructor( BSPAllocator & allocator ) : allocator_( allocator ) {}

	/**
	 *	This method constructs a BSP tree rooted at the input node, containing
	 *	the input triangles. It avoids using recursion by using its own stack.
	 */
	BSP * construct( WTriangleSet & triangles )
	{
		WPolygonSet polygons( triangles.size() );
		BSP * pResult = this->addToPending( triangles, polygons );

		while (!stack_.empty())
		{
			Element & element = stack_.back();

			// Take a copy of the back element and pop it since partition may add more
			// elements to the stack.
			WPolygonSet polys;	polys.swap( element.polys_ );
			WTriangleSet tris;	tris.swap( element.tris_ );
			BSP * pCurrBSP = element.pBSP_;
			stack_.pop_back();

			pCurrBSP->partition( tris, polys, *this );
		}

		return pResult;
	}

	/**
	 *	This method adds a pending BSP node to the stack of pending nodes. This
	 *	new BSP node is returned and should be set as the appropriate child of
	 *	the parent node.
	 */
	BSP * addToPending( WTriangleSet & tris, WPolygonSet & polys )
	{
		stack_.push_back( Element() );
		Element & element = stack_.back();
		BSP * pBSP = allocator_.newBSP();
		element.pBSP_ = pBSP;
		element.tris_.swap( tris );
		element.polys_.swap( polys );

		//MF_ASSERT( !element.tris_.empty() );

		return pBSP;
	}

private:
	// TODO: Is a vector the best structure?
	typedef std::vector< Element > Stack;
	Stack stack_;
	BSPAllocator allocator_;
};


// -----------------------------------------------------------------------------
// Section: BSP
// -----------------------------------------------------------------------------

/**
 *	This private constructor creates a BSP node that is not yet partitioned.
 *
 *	@see BSP::partition
 */
BSP::BSP():
	pFront_(NULL),
	pBack_(NULL),
	planeEq_(),
	partitioned_(false)
{
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif
}


/**
 *	Destructor.
 */
BSP::~BSP()
{
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif
}


/**
 *	This method partitions the set of triangles across this BSP node. It creates
 *	child back and front nodes if necessary. The input polygon set must be the
 *	same size as the triangle set. Each polygon corresponds to the triangle at
 *	the same index. If the polygon is non-empty, it should be a subset of the
 *	triangle and this polygon will be used for deciding how the triangle is
 *	inserted into the BSP.
 *
 *	@param triangles		This triangles that this BSP subtree will contain.
 *	@param polygons			A set of polygons that describe the size of the
 *							triangles.
 *	@param shouldClearSets	If true, the input sets are cleared by this method.
 *							This is used to save memory.
 */
void BSP::partition( WTriangleSet & triangles,
					WPolygonSet & polygons,
					BSPConstructor & constructor )
{
	IF_NOT_MF_ASSERT_DEV( triangles.size() == polygons.size() )
	{
		MF_EXIT( "BSP::partition - triangle and polygon lists different sizes" );
	}

	if (int(triangles.size()) <= MAX_SIZE)
	{
		triangles_.swap( triangles );
		planeEq_.init( Vector3( 0.f, 0.f, 0.f ), Vector3( 1.f, 0.f, 0.f ) );
		return;
	}

	// Find the best partition.
	int bestFront = triangles.size();
	int bestBack  = triangles.size();
	int bestBoth  = triangles.size();
	int bestOn    = triangles.size();

	const int MAX_TESTS = 15;

	for (uint i = 0;
		i < uint(MAX_TESTS) && i < triangles.size();
		i++)
	{
		PlaneEq oldEq = planeEq_;

		int randIndex = (int(triangles.size()) < MAX_TESTS) ?
			i : rand() % triangles.size();

		MF_ASSERT( 0 <= randIndex && randIndex < int( triangles.size() ) );

		planeEq_.init( triangles[randIndex]->v0(),
			triangles[randIndex]->v1(),
			triangles[randIndex]->v2() );

		int tempFront = 0;
		int tempBack = 0;
		int tempBoth = 0;
		int tempOn = 0;

		for (uint j = 0; j < triangles.size() && tempBoth < bestBoth; j++)
		{
			const WorldTriangle * pTriangle = triangles[j];
			const WorldPolygon & poly = polygons[j];

			BSPSide side =
				poly.empty() ?	this->whichSide( pTriangle ) :
								this->whichSide( poly );

			switch (side)
			{
				case BSP_BOTH:	tempBoth++;		break;
				case BSP_FRONT:	tempFront++;	break;
				case BSP_BACK:	tempBack++;		break;
				case BSP_ON:	tempOn++;		break;
			}
		}

		// TODO: May want to change this.
		// #### Currently choose the partition with the least number of
		// duplicates.

		if (tempBoth < bestBoth)
		{
			bestBoth  = tempBoth;
			bestFront = tempFront;
			bestBack  = tempBack;
			bestOn    = tempOn;
		}
		else
		{
			planeEq_ = oldEq;
		}
	}

	this->partitioned_ = true;

	WTriangleSet frontSet;
	WPolygonSet	frontPolys;

	WTriangleSet backSet;
	WPolygonSet	backPolys;

	frontSet.reserve(bestFront + bestBoth);
	frontPolys.reserve(bestFront + bestBoth);

	backSet.reserve(bestBack + bestBoth);
	backPolys.reserve(bestBack + bestBoth);

#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

	triangles_.reserve(bestOn);

	for (uint index = 0; index < triangles.size(); index++)
	{
		const WorldTriangle * pTriangle = triangles[index];
		const WorldPolygon & poly = polygons[index];

		BSPSide side =
			poly.empty() ? this->whichSide( pTriangle ) :
							this->whichSide( poly );

		switch (side)
		{
		case BSP_ON:
			triangles_.push_back( pTriangle );
			break;

		case BSP_FRONT:
			frontSet.push_back( pTriangle );
			frontPolys.push_back( poly );
			break;

		case BSP_BACK:
			backSet.push_back( pTriangle );
			backPolys.push_back( poly );
			break;

		case BSP_BOTH:
			frontSet.push_back( pTriangle );
			frontPolys.push_back( WorldPolygon() );

			backSet.push_back( pTriangle );
			backPolys.push_back( WorldPolygon() );

			if (!poly.empty())
			{
				poly.split( planeEq_,
					frontPolys.back(),
					backPolys.back() );
			}
			else
			{
				WorldPolygon tempPoly(3);
				tempPoly[0] = pTriangle->v0();
				tempPoly[1] = pTriangle->v1();
				tempPoly[2] = pTriangle->v2();

				tempPoly.split( planeEq_,
					frontPolys.back(),
					backPolys.back() );
			}

			break;
		}
	}
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

//	uint	totSize = frontSet.size() + backSet.size();

	if (!frontSet.empty())
	{
		// Add a node to the constructing objects pending stack.
		pFront_ = constructor.addToPending( frontSet, frontPolys );
//		pFront_ = new BSP( frontSet, frontPolys, pt, minP, midP );
	}

	if (!backSet.empty())
	{
		// We know that this new node will be popped immediately but it's probably
		// simplest to do it this way anyway.
		pBack_ = constructor.addToPending( backSet, backPolys );
//		pBack_ = new BSP( backSet, backPolys, pt, midP, maxP );
	}
}

#if 0
/**
 *	@internal
 *	This is a simple helper function used by splitPoly. It returns whether or
 *	not two points are too close.
 */
inline static bool tooClose( const Vector3 & p1, const Vector3 & p2 )
{
	const float MIN_DIST = 0.001f; // A millimetre
	const float MIN_DIST2 = MIN_DIST * MIN_DIST;

	return (p1 - p2).lengthSquared() <= MIN_DIST2;
}


/**
 *	@internal
 *	This is a simple helper function used by splitPoly. It adds a point to the
 *	polygon if it is not too close to the previous point.
 */
inline static int addToPoly( WorldPolygon & poly, const Vector3 & point )
{
	if (poly.empty() || !tooClose( poly.back(), point ))
	{
		poly.push_back( point );

		return 0;
	}

	return 1;
}


/**
 *	@internal
 *	This is a simple helper function used by splitPoly. It removes some
 *	unnecessary points.
 */
inline static void compressPoly( WorldPolygon & poly )
{
	if (tooClose( poly.front(), poly.back() ))
	{
		poly.pop_back();
	}

	if (poly.size() < 3)
	{
		poly.clear();
	}
}


/**
 *	This method splits the input polygon into the two polygons based on this
 *	objects paritioning plane.
 *
 *	@param inPoly		The polygon to be split.
 *	@param frontPoly	The polygon in front of the partitioning plane is put in
 *						here.
 *	@param backPoly		The polygon behind the partitioning plane is put in
 *						here.
 */
void BSP::splitPoly( const WorldPolygon & inPoly,
					WorldPolygon & frontPoly, WorldPolygon & backPoly ) const
{
	MF_ASSERT( inPoly.size() >= 3 );

	frontPoly.clear();
	backPoly.clear();

	const Vector3 * pPrevPoint = &inPoly.back();
	float prevDist = planeEq_.distanceTo( *pPrevPoint );
	bool wasInFront = (prevDist > 0.f);
	int compressions = 0;

	WorldPolygon::const_iterator iter = inPoly.begin();

	while (iter != inPoly.end())
	{
		float currDist = planeEq_.distanceTo( *iter );
		bool isInFront = (currDist > 0.f);

		if (isInFront != wasInFront)
		{
			float ratio = fabs(prevDist / (currDist - prevDist));
			Vector3 cutPoint =
				(*pPrevPoint) + ratio * (*iter - *pPrevPoint);

			compressions += addToPoly( frontPoly, cutPoint );
			compressions += addToPoly( backPoly, cutPoint );

			MF_ASSERT( fabs( planeEq_.distanceTo( cutPoint ) ) < 0.01f );
		}

		if (isInFront)
		{
			compressions += addToPoly( frontPoly, *iter );
		}
		else
		{
			compressions += addToPoly( backPoly, *iter );
		}

		wasInFront = isInFront;
		pPrevPoint = &(*iter);
		prevDist = currDist;

		iter++;
	}

	MF_ASSERT( inPoly.size() + 4 == frontPoly.size() + backPoly.size() + compressions );

	compressPoly( frontPoly );
	compressPoly( backPoly );
}
#endif


/**
 *	This method returns which side of this BSP node the input polygon is on.
 */
BSPSide BSP::whichSide(const WorldPolygon & poly) const
{
	BSPSide side = BSP_ON;

	WorldPolygon::const_iterator iter = poly.begin();

	while (iter != poly.end())
	{
		float dist = planeEq_.distanceTo( *iter );

		if (dist > TOLERANCE)
		{
			// Is this point on the front side?
			side = (BSPSide)(side | 0x1);
		}
		else if (dist < -TOLERANCE)
		{
			// Is this point on the back side?
			side = (BSPSide)(side | 0x2);
		}

		iter++;
	}

	return side;
}


/**
 *	This method returns which side of this BSP node the input triangle is on.
 */
BSPSide BSP::whichSide( const WorldTriangle * pTriangle ) const
{
	BSPSide side = BSP_ON;

	float dist0 = planeEq_.distanceTo( pTriangle->v0() );
	float dist1 = planeEq_.distanceTo( pTriangle->v1() );
	float dist2 = planeEq_.distanceTo( pTriangle->v2() );

	// Is the triangle in front of the plane?

	if (dist0 > TOLERANCE ||
		dist1 > TOLERANCE ||
		dist2 > TOLERANCE)
	{
		side = (BSPSide)(side | 0x1);
	}

	// Is the triangle behind the plane?

	if (dist0 < -TOLERANCE ||
		dist1 < -TOLERANCE ||
		dist2 < -TOLERANCE)
	{
		side = (BSPSide)(side | 0x2);
	}

	return side;
}


//
// Helper methods for intersects.
//

/**
 *	This function returns the minimum of the input values.
 */
template <class T> inline float min(T a, T b, T c)
{
	return a < b ?
		(a < c ? a : c) :
		(b < c ? b : c);
}


/**
 *	This function returns the maximum of the input values.
 */
template <class T> inline float max(T a, T b, T c)
{
	return a > b ?
		(a > c ? a : c) :
		(b > c ? b : c);
}



/**
 *	This method returns whether the input triangle intersects any triangle in
 *	this node or any child nodes.
 *
 *	@param triangle			The triangle to test for collision with the BSP.
 *	@param ppHitTriangle	If not NULL and a collision occurs, the pointer
 *							pointed to by this variable is set to point to the
 *							triangle that was collided with. If the triangle
 *							that is hit first is important, you should pass in
 *							the "starting" vertex as the first vertex of the
 *							test triangle.
 *
 *	@return True if an intersection occurred, otherwise false.
 */
bool BSP::intersects(const WorldTriangle & triangle,
					 const WorldTriangle ** ppHitTriangle) const
{
	bool intersects = false;

	if (!partitioned_)
	{
		intersects = this->intersectsThisNode(triangle,
			ppHitTriangle);
	}
	else
	{
		float d0 = planeEq_.distanceTo(triangle.v0());
		float d1 = planeEq_.distanceTo(triangle.v1());
		float d2 = planeEq_.distanceTo(triangle.v2());

		float min = ::min(d0, d1, d2);
		float max = ::max(d0, d1, d2);

		// Check the side with the first point of the triangle first.

		if (d0 < 0.f)
		{
			if (pBack_ && min < TOLERANCE)
			{
				intersects = pBack_->intersects(triangle,
					ppHitTriangle);
			}

			if (min < TOLERANCE && max > -TOLERANCE && !intersects)
			{
				intersects = this->intersectsThisNode(triangle,
					ppHitTriangle);
			}

			if (pFront_ && max > -TOLERANCE && !intersects)
			{
				intersects = pFront_->intersects(triangle,
					ppHitTriangle);
			}
		}
		else
		{
			if (pFront_ && max > -TOLERANCE)
			{
				intersects = pFront_->intersects(triangle,
					ppHitTriangle);
			}

			if (min < TOLERANCE && max > -TOLERANCE && !intersects)
			{
				intersects = this->intersectsThisNode(triangle,
					ppHitTriangle);
			}

			if (pBack_ && min < TOLERANCE && !intersects)
			{
				intersects = pBack_->intersects(triangle,
					ppHitTriangle);
			}
		}
	}

	return intersects;
}


/**
 *	This method returns whether the input triangle intersects any triangle
 *	assigned to this node.
 */
bool BSP::intersectsThisNode(const WorldTriangle & triangle,
							 const WorldTriangle ** ppHitTriangle) const
{
	bool intersects = false;


	WTriangleSet::const_iterator iter = triangles_.begin();

	while (iter != triangles_.end() &&
		!intersects)
	{
		if ((*iter)->collisionFlags() != TRIANGLE_NOT_IN_BSP)
		{
			if ((*iter)->intersects(triangle))
			{
				intersects = true;

				if (ppHitTriangle != NULL)
				{
					*ppHitTriangle = (*iter);
				}
			}
		}

		iter++;
	}

	return intersects;
}


/**
 *	This method is necessary for all bsps exported using BigWorld 1.8,
 *	which will have a "bsp2" section in the primitves file.  The flags member
 *	of such primtives file is the material group ID and not the actual flags.
 *	This method converts material group IDs to WorldTriangle flags.
 *
 *	Note - it is entirely reasonable to have IDs in the BSP data that do not
 *	have entries in the flagsMap.  This occurs when customBSPs are used.  In this
 *	case, the customBSP was not given a material mapping, and so the default
 *	is used - the object's material kind (passed in via the defaultFlags param)
 */
void BSPTree::remapFlags( BSPFlagsMap& flagsMap )
{
	RealWTriangleSet::iterator iter = triangles_.begin();

	while (iter != triangles_.end())
	{
		WorldTriangle& tri = *iter++;
		uint32 idx = (uint32)tri.flags();
		tri.flags( flagsMap[idx % flagsMap.size()] );
	}
}


/**
 *	This is a helper class for the intersects method
 */
class NullCollisionVisitor : public CollisionVisitor
{
public:
	virtual bool visit( const WorldTriangle &, float )
	{
		return true;
	}
};

static NullCollisionVisitor	s_nullCV;

/**
 *	This struct is the stack node used for keeping track of the current state
 *	of bsp traversal.
 */
struct BSPStackNode
{
	BSPStackNode();
	BSPStackNode( const BSP * pBSP, int eBack, float sDist, float eDist ) :
		pNode_( pBSP ), eBack_( eBack ), sDist_( sDist ), eDist_( eDist )
	{ }

	const BSP	* pNode_;
	int			eBack_;		// or -1 for unseen
	float		sDist_;
	float		eDist_;
};

/**
 *	This method returns whether the input interval intersects any triangle in
 *	this node or any children.  If it does, dist is set to the fraction of the
 *	distance along the vector that the intersection point lies.
 *
 *	Note: In general, you will want to pass in a float that is set to 1 for the
 *	dist parameter.
 *
 *	@param start	The start point of the interval to test for collision.
 *	@param end		The end point of the interval to test for collision.
 *	@param dist		A reference to a float. Usually this should be set to 1
 *					before the method is called
 *
 *					The input value of this float is the fraction of the
 *					interval that should be considered. For example, if dist is
 *					0.5, only the interval from <i>start</i> to the midpoint of
 *					<i>start</i> and <i>end</i> is considered for collision.
 *
 *					After the call, if a collision occurred, this value is set
 *					to the fraction along the interval that the collision
 *					occurred. Thus, the collision point can be calculated as
 *					start + dist * (end - start).
 *	@param ppHitTriangle
 *					A pointer to a world triangle pointer. If not NULL, the
 *					pointer pointed to by this parameter is set to the pointer
 *					to the triangle that caused the collision.
 *	@param pVisitor
 *					A pointer to a visitor object. If not NULL, this object's
 *					'visit' method is called for each triangle hit.
 *
 *	@return			True if there was a collision, otherwise false.
 */
bool BSP::intersects(const Vector3 & start,
					 const Vector3 & end,
					 float & dist,
					 const WorldTriangle ** ppHitTriangle,
					 CollisionVisitor * pVisitor ) const
{
	const WorldTriangle * pHitTriangle = NULL;

	if (ppHitTriangle == NULL) ppHitTriangle = &pHitTriangle;
	if (pVisitor == NULL) pVisitor = &s_nullCV;

	float origDist = dist;
	const WorldTriangle * origHT = *ppHitTriangle;

	Vector3 delta = end - start;
	float tolerancePct = TOLERANCE / delta.length();

	static VectorNoDestructor< BSPStackNode > stack;
	stack.clear();
	stack.push_back( BSPStackNode( this, -1, 0, 1 ) );

	while (!stack.empty())
	{
		// get the next node to look at
		BSPStackNode cur = stack.back();
		stack.pop_back();

		// set default / initial values for the line segment range
		float sDist = cur.sDist_;
		float eDist = cur.eDist_;

		// set up for plane intersection
		float iDist = 0.f;
		const PlaneEq & pe = cur.pNode_->planeEq_;

		// variables saying is points are on back side of plane
		int sBack, eBack;

		// see if this is a really simple node
		if (!cur.pNode_->partitioned_)
		{
			// just look at the triangles and don't try to add to the stack
			sBack = -2;
			eBack = -2;
		}
		// see if this is the first time we've seen it
		else if (cur.eBack_ == -1)
		{
			// ok, this is the first time at this node
			float sOut = pe.distanceTo( start + delta * (sDist - tolerancePct) );
			float eOut = pe.distanceTo( start + delta * (eDist + tolerancePct) );
			sBack = int(sOut < 0.f);
			eBack = int(eOut < 0.f);

			// find which side the start is on
			BSP * pStartSide = (&cur.pNode_->pFront_)[sBack];

			// are they both on the same side?
			if (sBack == eBack)
			{
				// ok, easy - go down that side then, if it's there

				// but first check if either are within tolerance
				if (fabs(sOut) < TOLERANCE || fabs(eOut) < TOLERANCE)
				{
					// and come back to check the triangles later if they are,
					//  but don't bother with the back side
					stack.push_back( BSPStackNode(
						cur.pNode_, -2, sDist, eDist ) );
				}

				// now go down the start side
				if (pStartSide != NULL)
				{
					stack.push_back( BSPStackNode(
						pStartSide, -1, sDist, eDist ) );
				}

				continue;
			}

			// ok, points are on different sides. find intersect distance
			iDist = pe.intersectRayHalf( start, pe.normal().dotProduct( delta ) );

			// if there's anything on the start side we'll have to do it first
			if (pStartSide != NULL)
			{
				// remember to come to the end side
				stack.push_back( BSPStackNode(
					cur.pNode_, eBack, sDist, eDist ) );

				// and then look at start side
				stack.push_back( BSPStackNode(
					pStartSide, -1, sDist, iDist ) );

				continue;
			}

			// ok, there's nothing on the start side, so fall through to
			//  check the triangles on the plane (against the line between
			//  between sDist and eDist), and later add the end side
			//  between iDist and eDist, if it's not null (phew!)
		}
		// ok we've been here before
		else
		{
			sBack = cur.eBack_;
			eBack = cur.eBack_;

			iDist = pe.intersectRayHalf( start, pe.normal().dotProduct( delta ) );

			// fall through to check the triangles on the plane (against the
			//  line between sDist and eDist), and later add the end
			//  side between iDist and eDist, if it's not null
		}

		// ok, check the triangles on this node, and get out immediately
		//  if we get a (confirmed) hit
		if (cur.pNode_->intersectsThisNode( start, end, dist, ppHitTriangle, pVisitor ) &&
				dist <= (cur.eDist_+tolerancePct) )
		{
			return true;
		}

		// reset stuff (hmmm)
		dist = origDist;
		*ppHitTriangle = origHT;

		// now add the other side if it's not null (and eBack is ok)
		if (eBack >= 0)
		{
			BSP * pEndSide = (&cur.pNode_->pFront_)[eBack];
			if (pEndSide != NULL) stack.push_back( BSPStackNode(
				pEndSide, -1, iDist, eDist ) );
		}
	}

	return false;
}

/**
 *	This method returns whether the input interval intersects a triangle
 *	assigned to this node. If it does, dist is set to the fraction of the
 *	distance along the vector that the intersection point lies.
 *
 *	@see BSP::intersects
 */
bool BSP::intersectsThisNode(const Vector3 &	start,
						 const Vector3 &		end,
						 float &				dist,
						 const WorldTriangle ** ppHitTriangle,
						 CollisionVisitor *		pVisitor ) const
{
	bool intersects = false;

	WTriangleSet::const_iterator iter = triangles_.begin();
	const Vector3 direction(end - start);

	// We go through all triangles because we need to find the closest one.

	while (iter != triangles_.end())
	{
		if ((*iter)->collisionFlags() != TRIANGLE_NOT_IN_BSP)
		{
			float originalDist = dist;

			if ((*iter)->intersects(start, direction, dist) &&
				(!pVisitor || pVisitor->visit( **iter, dist )))
			{
				intersects |= true;

				if (ppHitTriangle != NULL)
				{
					*ppHitTriangle = (*iter);
				}
			}
			else
			{
				dist = originalDist;
			}
		}

		iter++;
	}

	return intersects;
}

/**
 *	This method intersects the volume formed by moving a triangle
 *	by a given translation, with the BSP tree.
 *
 *	Any triangles that intersect this volume are returned in
 *	the vector passed by reference as the last argument.
 *	The vector of triangles is not cleared.
 *	Triangles parallel to translation are not considered.
 */
bool BSP::intersects( const WorldTriangle & triangle,
	const Vector3 & translation,
	CollisionVisitor * pVisitor ) const
{
	// if we're not partitioned it's easy
	if (!partitioned_)
	{
		return this->intersectsThisNode( triangle, translation, pVisitor );
	}

	// ok, see if the volume crosses this plane
	float dA0 = planeEq_.distanceTo(triangle.v0());
	float dA1 = planeEq_.distanceTo(triangle.v1());
	float dA2 = planeEq_.distanceTo(triangle.v2());
	float dB0 = planeEq_.distanceTo(triangle.v0()+translation);
	float dB1 = planeEq_.distanceTo(triangle.v1()+translation);
	float dB2 = planeEq_.distanceTo(triangle.v2()+translation);

	float min = std::min( ::min(dA0, dA1, dA2), ::min(dB0, dB1, dB2) );
	float max = std::max( ::max(dA0, dA1, dA2), ::max(dB0, dB1, dB2) );

	// TODO: Unroll the recursion to increase performance.

	if (min < TOLERANCE && max > -TOLERANCE)
	{
		if (this->intersectsThisNode( triangle, translation, pVisitor ))
			return true;
	}

	if (pBack_ && min < TOLERANCE)
	{
		if (pBack_->intersects( triangle, translation, pVisitor ))
			return true;
	}

	if (pFront_ && max > -TOLERANCE)
	{
		if (pFront_->intersects( triangle, translation, pVisitor ))
			return true;
	}

	return false;
}


/**
 *	This method returns whether the volume formed by moving the input
 *	triangle by a given translation intersects any triangle assigned
 *	to this node.
 */
bool BSP::intersectsThisNode( const WorldTriangle & triangle,
	const Vector3 & translation,
	CollisionVisitor * pVisitor ) const
{
	for (WTriangleSet::const_iterator iter = triangles_.begin();
		iter != triangles_.end();
		iter++)
	{
		if ((*iter)->collisionFlags() != TRIANGLE_NOT_IN_BSP)
		{
			if ((*iter)->intersects(triangle,translation))
			{
				if (pVisitor == NULL || pVisitor->visit( **iter, 0.f )) return true;
			}
		}
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: BSPFile
// -----------------------------------------------------------------------------

// TODO: use BinarySection instead.

/**
 *	This class is a simple helper to implement the load method of BSP. It is not
 *	specific to BSPs and so it could be moved elsewhere.
 */
class BSPFile
{
public:
	BSPFile( BinaryPtr pData ) :
		pData_( static_cast<const char *>( pData.getObject()->data() ) ),
		size_( pData.getObject()->len() )
	{
	}

	~BSPFile()
	{
		if (size_ != 0)
		{
			ERROR_MSG( "BSPFile::~BSPFile: Size is not 0. (%d)\n", size_ );
		}
	}

	int read( void * pDst, int elementSize, int numElements )
	{
		const int readSize = elementSize * numElements;
		size_ -= readSize;

		if (size_ >= 0)
		{
			memcpy( pDst, pData_, readSize );
			pData_ += readSize;
			return readSize;
		}
		else
		{
			return 0;
		}
	}

	int size() const
	{
		return size_;
	}

	int close()
	{
		size_ = 0;

		return 0;
	}

private:
	const char * pData_;
	int size_;
};


// -----------------------------------------------------------------------------
// Section: Loading and saving - Saving used by xbsync.
// -----------------------------------------------------------------------------

namespace
{

inline bool isValidPlane( const PlaneEq & pe )
{
	float normLen = pe.normal().length();

	return 0.9f < normLen && normLen < 1.1f;
}

} // anon namespace


static const uint8 BSP_IS_PARTITIONED	= 0x01;
static const uint8 BSP_HAS_FRONT		= 0x02;
static const uint8 BSP_HAS_BACK			= 0x04;
static const uint8 BSP_MAGIC			= 0xa0;
static const uint8 BSP_MAGIC_MASK		= 0xf8;

/**
 *	This method loads the BSP node from the input file along with its
 *	descendants.
 */
bool BSP::load( const BSPTree & tree, BSPFile & bspFile,
			   BSPAllocator & allocator )
{
	uint8 flags;
	if (!bspFile.read( &flags, 1, 1 ))
	{
		return false;
	}

	if ((flags & BSP_MAGIC_MASK) != BSP_MAGIC)
	{
		ERROR_MSG( "BSP::load: Bad magic mask 0x%x.\n", (int)flags );
		return false;
	}

	partitioned_ = (flags & BSP_IS_PARTITIONED) ? 1 : 0;

	if (!bspFile.read( &planeEq_, sizeof( planeEq_ ), 1 ))
	{
		return false;
	}

	uint16 numTris;

	if (!bspFile.read( &numTris, sizeof( uint16 ), 1 ))
	{
		return false;
	}
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

	if (!tree.loadTrianglesForNode( bspFile, *this, numTris ))
	{
		return false;
	}
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/BSP", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

	if (partitioned_ && !isValidPlane( planeEq_ ))
	{
		ERROR_MSG( "BSP::load: Bad plane equation: n = (%f, %f, %f). d = %f\n",
			planeEq_.normal().x, planeEq_.normal().y, planeEq_.normal().z,
			planeEq_.d() );
		return false;
	}

	if (flags & BSP_HAS_FRONT)
	{
		pFront_ = allocator.newBSP();
		if (!pFront_->load( tree, bspFile, allocator ))
		{
			allocator.destroy( pFront_ );
			pFront_ = NULL;
			return false;
		}
	}

	if (flags & BSP_HAS_BACK)
	{
		pBack_ = allocator.newBSP();
		if (!pBack_->load( tree, bspFile, allocator ))
		{
			allocator.destroy( pBack_ );
			pBack_ = NULL;
			return false;
		}
	}

	return true;
}


/**
 *	This method saves this BSP node to the input file along with its
 *	descendants. A pointer to the front triangle is passed in so that each
 *	triangles index can be calculated.
 */
bool BSP::save( FILE * pFile, const WorldTriangle * pFront ) const
{
	if (!isValidPlane( planeEq_ ))
	{
		ERROR_MSG( "BSP::save: "
			"Invalid planeEq n = (%f, %f, %f) d = %f. len = %f\n",
			planeEq_.normal().x, planeEq_.normal().y, planeEq_.normal().z,
			planeEq_.d(), planeEq_.normal().length() );
		return false;
	}

	bool result = true;

	uint8 flags = partitioned_ ? BSP_IS_PARTITIONED : 0x0;
	if (pFront_ != NULL)
	{
		flags |= BSP_HAS_FRONT;
	}

	if (pBack_ != NULL)
	{
		flags |= BSP_HAS_BACK;
	}

	flags |= BSP_MAGIC;

	result &= (fwrite( &flags, 1, 1, pFile ) != 0);

	// TODO: We don't need to do this if the node is not partitioned.
	// Probably should not rely on the layout of PlaneEq.
	result &= (fwrite( &planeEq_, sizeof(planeEq_), 1, pFile ) != 0);

	// TODO: Should support more than 64k triangles.
	if (triangles_.size() >= 1 << 16)
	{
		ERROR_MSG( "BSP::save: There are too many triangles. %"PRIzu" >= %d\n",
			triangles_.size(), 1 << 16 );

		return false;
	}

	uint16 numTris = triangles_.size();
	result &= (fwrite( &numTris, sizeof(numTris), 1, pFile ) != 0);

	WTriangleSet::const_iterator iter = triangles_.begin();
	while (iter != triangles_.end() && result)
	{
		// Get the index number of the triangle.
		uint16 index = *iter - pFront;
		result &= (fwrite( &index,
			sizeof( uint16 ), 1, pFile ) != 0);
		iter++;
	}

	if (pFront_ != NULL && result)
	{
		result = pFront_->save( pFile, pFront );
	}

	if (pBack_ != NULL && result)
	{
		result = pBack_->save( pFile, pFront );
	}

	return result;
}

/**
 *	This method sets the input references to be the number of nodes and triangle
 *	in this subtree. This method expects numNodes to be initialised to some
 *	value ( zero for the first call ).
 */
void BSP::getNumNodes( int & numNodes, int & maxTriangles ) const
{
	numNodes++;
	if (triangles_.size() > size_t(maxTriangles))
	{
		maxTriangles = triangles_.size();
	}

	if (pFront_)
	{
		pFront_->getNumNodes( numNodes, maxTriangles );
	}

	if (pBack_)
	{
		pBack_->getNumNodes( numNodes, maxTriangles );
	}
}


// -----------------------------------------------------------------------------
// Section: Debugging
// -----------------------------------------------------------------------------

/**
 *	Debugging method to dump the contents of this BSP tree
 */
void BSP::dump( int depth )
{
	if (1 || triangles_.size() > 20)
	{
		std::string	spaces(depth,' ');
		INFO_MSG( "%ssize = %d. n = (%f, %f, %f). d = %f len = %f\n",
			spaces.c_str(), (int)triangles_.size(),
			planeEq_.normal().x,
			planeEq_.normal().y,
			planeEq_.normal().z,
			planeEq_.d(),
			planeEq_.normal().length() );
#if 0
		WTriangleSet::const_iterator iter = triangles_.begin();
		while (iter != triangles_.end())
		{
			const WorldTriangle * pTri = *iter;
			INFO_MSG( "%s(%f,%f,%f)(%f,%f,%f)(%f,%f,%f)%d\n",
				spaces.c_str(),
				pTri->v0().x, pTri->v0().y, pTri->v0().z,
				pTri->v1().x, pTri->v1().y, pTri->v1().z,
				pTri->v2().x, pTri->v2().y, pTri->v2().z, int(pTri->flags()) );
			iter++;
		}
#endif
	}

	if (pFront_ != NULL)
	{
		pFront_->dump( depth + 1 );
	}

	if (pBack_ != NULL)
	{
		pBack_->dump( depth + 1 );
	}
}


/**
 *	For debugging.
 */
uint32 BSP::size() const
{
	uint32 sz = sizeof( BSP );
	sz += triangles_.capacity() * sizeof( triangles_.front() );
	return sz;
}


// -----------------------------------------------------------------------------
// Section: BSPTree
// -----------------------------------------------------------------------------

const uint8 BSP_FILE_VERSION = 0;
const uint32 BSP_FILE_MAGIC = 0x505342;
const uint32 BSP_FILE_TOKEN = BSP_FILE_MAGIC | (BSP_FILE_VERSION << 24);
// char * BSPTree::s_pNodeMemory = NULL;
// int BSPTree::s_nodeMemorySize = 0;

/**
 *	This is the constructor that is used when the BSP is created from a set of
 *	world triangles.
 */
BSPTree::BSPTree( RealWTriangleSet & triangles ) : pRoot_( NULL ),
	pIndices_( NULL ),
	indicesSize_( 0 ),
	pNodeMemory_( NULL )
{
	triangles_.swap( triangles );

#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/Tree", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

	int size = triangles_.size();
	WTriangleSet tris( size );

	for (int i = 0; i < size; i++)
	{
		tris[i] = &triangles_[i];
	}

	BSPAllocator allocator( pNodeMemory_ );
	BSPConstructor constructor( allocator );
	pRoot_ = constructor.construct( tris );
}


/**
 *	Default constructor. This is the constructor that is used when reading the
 *	BSP tree from a file.
 */
BSPTree::BSPTree() : pRoot_( NULL ),
	pIndices_( NULL ),
	indicesSize_( 0 ),
	pNodeMemory_( NULL )
{
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/Tree", (uint)ResourceCounters::SYSTEM),
		size() )
#endif
}


/**
 *	Destructor.
 */
BSPTree::~BSPTree()
{
#if ENABLE_RESOURCE_COUNTERS
	RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/Tree", (uint)ResourceCounters::SYSTEM),
		size() )
#endif

	// We need to let the allocator free the child nodes.
	// Note: pNodeMemory_ may be NULL.
	BSPAllocator allocator( pNodeMemory_ );

	if (pRoot_)
		allocator.destroy( pRoot_ );
	if (pNodeMemory_ != NULL)
	{
		delete [] pNodeMemory_;
	}
}


/**
 *	This method loads a BSP tree from the input file.
 *
 *	@return True if successful, otherwise false.
 */
bool BSPTree::load( BinaryPtr bp )
{
	IF_NOT_MF_ASSERT_DEV( sizeof( WorldTriangle ) == 40 )
	{
		MF_EXIT( "sizeof( WorldTriangle ) must be 40!" );
	}

	bool result = false;

	if (pRoot_ == NULL)
	{
		BSPFile bspFile( bp );

		struct
		{
			uint32 magic;
			int32 numTriangles;
			int32 numNodes;
			int32 maxTriangles;
		} header;

		if (!bspFile.read( &header, sizeof(header), 1 ))
		{
			bspFile.close();
			return false;
		}

		if (header.magic != BSP_FILE_TOKEN)
		{
			if ((header.magic & 0xffffff) == BSP_FILE_MAGIC)
			{
				ERROR_MSG( "BSPTree::load: "
					"Bad version. Expected %d. Got %u.\n",
					BSP_FILE_VERSION, header.magic >> 24 );
			}
			else
			{
				ERROR_MSG( "BSPTree::load: Bad magic\n" );
			}
			bspFile.close();
			return false;
		}

		// This is a bit dodgy.
#if ENABLE_RESOURCE_COUNTERS
		RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool("BSPTree/Tree", (uint)ResourceCounters::SYSTEM),
			size() )
#endif

		triangles_.resize( header.numTriangles );
#if ENABLE_RESOURCE_COUNTERS
		RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool("BSPTree/Tree", (uint)ResourceCounters::SYSTEM),
			size() )
#endif
		bspFile.read( triangles_.empty() ? NULL : &triangles_.front(),
			sizeof( WorldTriangle ), header.numTriangles );

		pIndices_ = new uint16[ header.maxTriangles ];
		indicesSize_ = header.maxTriangles;

		pNodeMemory_ = new char[ header.numNodes * sizeof(BSP) ];

		BSPAllocator allocator( pNodeMemory_ );

		pRoot_ = allocator.newBSP();
		result = pRoot_->load( *this, bspFile, allocator );

		delete [] pIndices_;
		pIndices_ = NULL;
		indicesSize_ = 0;

		// read user data
		while (bspFile.size() > 0)
		{
			IF_NOT_MF_ASSERT_DEV( unsigned( bspFile.size() ) >=
				sizeof( UserDataKey ) + sizeof( int ) )
			{
				return false;
			}

			UserDataKey type;
			bspFile.read(&type, sizeof(UserDataKey), 1);

			int size = 0;
			bspFile.read(&size, sizeof(int), 1);

			IF_NOT_MF_ASSERT_DEV(bspFile.size() >= size)
			{
				return false;
			}
			char * data = new char[size];
			bspFile.read(data, sizeof(char), size);
			this->setUserData(type, new BinaryBlock(data, size, "BinaryBlock/BSPTree"));
			delete [] data;
		}

		bspFile.close();

		// Generate the bounding box for the BSP
		this->generateBB();

		if (!result)
		{
			ERROR_MSG( "BSPTree::load: Loading failed.\n" );
		}
	}
	else
	{
		ERROR_MSG( "BSPTree::load: Already been initialised.\n" );
		result = false;
	}

	return result;
}


/**
 *	This method saves this BSP tree to the input file.
 *
 *	@return True if successful, otherwise false.
 */
bool BSPTree::save( const std::string & filename ) const
{
	TRACE_MSG( "BSPTree::save: %s\n", filename.c_str() );
	IF_NOT_MF_ASSERT_DEV( sizeof( WorldTriangle ) == 40 )
	{
		MF_EXIT( "sizeof( WorldTriangle ) must be 40!" );
	}

	bool result = false;

	if (triangles_.size() > 0xffff)
	{
		ERROR_MSG( "BSPTree::save: "
				"Tree size (%"PRIzu") is bigger than max size (%d)\n",
				triangles_.size(), 0xffff );
	}
	else if (pRoot_ != NULL)
	{
		FILE * pFile = BWResource::instance().fileSystem()->posixFileOpen( filename, "wb" );

		if (pFile == NULL)
		{
			ERROR_MSG( "BSPTree::save: Could not open %s for writing.\n",
				filename.c_str() );
			return false;
		}

		int numTriangles = triangles_.size();
		int numNodes = 0;
		int maxTriangles = 0;

		pRoot_->getNumNodes( numNodes, maxTriangles );

		fwrite( &BSP_FILE_TOKEN, sizeof( BSP_FILE_TOKEN ), 1, pFile );
		fwrite( &numTriangles, sizeof( numTriangles ), 1, pFile );
		fwrite( &numNodes, sizeof( numNodes ), 1, pFile );
		fwrite( &maxTriangles, sizeof( maxTriangles ), 1, pFile );

		if( triangles_.size() )
			fwrite( &triangles_.front(),
				sizeof( WorldTriangle ), triangles_.size(), pFile );

		result = pRoot_->save( pFile,
				triangles_.empty() ? NULL : &triangles_.front() );

		UserDataMap::const_iterator dataIt  = this->userData_.begin();
		UserDataMap::const_iterator dataEnd = this->userData_.end();
		while (dataIt != dataEnd)
		{
			int len = dataIt->second->len();
			fwrite( &dataIt->first, sizeof(UserDataKey), 1, pFile );
			fwrite( &len, sizeof(int), 1, pFile );
			fwrite( dataIt->second->cdata(), 1, len, pFile );
			++dataIt;
		}

		fclose( pFile );
	}
	else
	{
		ERROR_MSG( "Has no root\n" );
	}

	return result;
}


/**
 *	This is a helper method used by BSP's load method.
 */
bool BSPTree::loadTrianglesForNode( BSPFile & bspFile,
									BSP & node, int numTriangles ) const
{
	if (numTriangles > indicesSize_)
	{
		ERROR_MSG( "BSPTree::loadTrianglesForNode: "
			"Wanted to load %d but can only handle %d\n", numTriangles, indicesSize_ );
		return false;
	}

	if (numTriangles == 0)
		return true;

	if (!bspFile.read( pIndices_, sizeof(uint16), numTriangles ))
	{
		ERROR_MSG( "BSPTree::loadTrianglesForNode: Failed to read %d indices.\n",
			numTriangles );
		return false;
	}

	WTriangleSet & nodeTris = node.triangles_;
	nodeTris.resize( numTriangles );
	const int maxSize = triangles_.size();

	for (int i = 0; i < numTriangles; i++)
	{
		uint16 index = pIndices_[i];
		if (index >= maxSize)
		{
			ERROR_MSG( "BSPTree::loadTrianglesForNode: "
				"Index too big %d >= %d.\n", index, maxSize );
			return false;
		}

		nodeTris[i] = &triangles_[ pIndices_[i] ];
	}

	return true;
}


/**
 *	Retrieves the user data entry identified by the given key,
 *	if present. Returns a NULL BinaryPtr if it entry was not found.
 */
BinaryPtr BSPTree::getUserData(UserDataKey type)
{
	UserDataMap::const_iterator dataIt = this->userData_.find(type);
	return dataIt != this->userData_.end() ? dataIt->second : BinaryPtr(NULL);
}


/**
 *	Sets the user data entry identified by the given key. The data
 *	will be persisted to disk when the BSPTree is saved to a file.
 */
void BSPTree::setUserData(UserDataKey type, BinaryPtr data)
{
	this->userData_.insert(std::make_pair(type, data));
}


/**
 *	For debugging - size of memory allocated to this object.
 */
uint32 BSPTree::size() const
{
	uint32 sz = sizeof( BSPTree );
	sz += triangles_.capacity() * sizeof( triangles_.front() );
	return sz;
}

/**
 *	Generates a bounding box for this BSP.
 */
void BSPTree::generateBB()
{
	bb_ = BoundingBox::s_insideOut_;

	RealWTriangleSet::const_iterator it;
	for(it = triangles_.begin();
		it != triangles_.end();
		it++)
	{
		const WorldTriangle & tri = *it;

		bb_.addBounds( tri.v0() );
		bb_.addBounds( tri.v1() );
		bb_.addBounds( tri.v2() );
	}
}

/**
 *	Returns a const reference to the bounding box for this BSP.
 */
const BoundingBox& BSPTree::getBB()
{
	return bb_;
}

/**
 *	Used to find out if one or more triangles in the BSP are collideable
 *
 *  @return false if no triangle is collideable, true otherwise
 */
bool BSPTree::canCollide() const
{
	for( RealWTriangleSet::const_iterator it = triangles_.begin();
		it != triangles_.end(); it++)
	{
		if ( (*it).collisionFlags() != TRIANGLE_NOT_IN_BSP )
		{
			return true;
		}
	}
	return false;
}

// bsp.cpp
