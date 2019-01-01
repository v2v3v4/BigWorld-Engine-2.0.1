/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_COLLISION_HPP
#define SPEEDTREE_COLLISION_HPP

// BW Tech Hearders
#include "math/vector3.hpp"
#include "cstdmf/smartpointer.hpp"
#include <memory>
#include <vector>

class BSPTree;
class BoundingBox;
class CSpeedTreeRT;
typedef class std::vector<class WorldTriangle> RealWTriangleSet;


namespace speedtree {

typedef std::auto_ptr< BSPTree > BSPTreePtr;

void computeTreeGeometry(
	CSpeedTreeRT & speedTree,
	const char   * filename, 
	int            seed);

void deleteTreeGeometry(
	CSpeedTreeRT & speedTree);
	
BSPTree * getBSPTree( 
	const char  * filename, 
	int           seed,
	BoundingBox & o_bbox );
	
BSPTreePtr createBSPTree( 
	CSpeedTreeRT * speedTree, 
	const char   * filename, 
	int            seed,
	BoundingBox  & o_bbox );

void createBoxBSP(
	const float      * position, 
	const float      * dimension, 
	RealWTriangleSet & o_triangles);

const int SPT_COLLMAT_SOLID  = 0x78;
const int SPT_COLLMAT_LEAVES = 0x77;

} // namespace speedtree

#endif // SPEEDTREE_COLLISION_HPP
