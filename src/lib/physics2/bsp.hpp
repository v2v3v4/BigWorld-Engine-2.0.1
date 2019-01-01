/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@file
 */

#ifndef BSP_HPP
#define BSP_HPP

// This file implements a BSP tree. It does not own (and thus delete)
// the triangles it contains.

#include "math/planeeq.hpp"
#include "math/boundbox.hpp"
#include "cstdmf/smartpointer.hpp"
#include "worldpoly.hpp"
#include "worldtri.hpp"

#include <string>
#include <map>

/**
 *	This enumeration is used to indicate which side of a BSP node a triangle is
 *	on.
 */
enum BSPSide
{
	BSP_ON		= 0x0,	///< Lies on the partitioning plane.
	BSP_FRONT	= 0x1, 	///< Lies in front of the partitioning plane.
	BSP_BACK	= 0x2,	///< Lies behind the partitioning plane.
	BSP_BOTH	= 0x3	///< Crosses over the partitioning plane.
};


/**
 *	This interface is used by intersects.
 */
class CollisionVisitor
{
public:
	virtual ~CollisionVisitor() {};
	virtual bool visit( const WorldTriangle & hitTriangle, float dist ) = 0;
};


class ProgressTask;
class BSPAllocator;
class BSPConstructor;
class BSP;
class BSPFile;
class BinaryBlock;
typedef SmartPointer<BinaryBlock> BinaryPtr;

typedef std::vector< WorldTriangle::Flags > BSPFlagsMap;

/**
 *	This class is used to store a BSP tree. It is responsible for the triangles
 *	that are in its member nodes.
 */
class BSPTree
{
public:
	BSPTree( RealWTriangleSet & triangles );
	BSPTree();
	~BSPTree();

	bool load( BinaryPtr bp );
	bool save( const std::string & filename ) const;
	void remapFlags( BSPFlagsMap& flagsMap );

	const BSP * pRoot() const		{ return pRoot_; }

	uint32 size() const;
	bool empty() const { return triangles_.empty(); }

#ifdef COLLISION_DEBUG
	std::string name() const					{ return name_; }
	void name( const std::string & name )		{ name_ = name; }

	std::string name_;
#endif

	void generateBB();
	const BoundingBox& getBB();
	
	// Used by bsp regen to check if the generated bsp tree is the same
	// as the existing one
	const RealWTriangleSet& triangles() const { return triangles_; }
	
	enum UserDataKey
	{
		MD5_DIGEST,
		TIME_STAMP,
		
		USER_DEFINED = 1 << 16
	};
	
	BinaryPtr getUserData(UserDataKey type);
	void setUserData(UserDataKey type, BinaryPtr data);

	bool canCollide() const;

private:
	bool loadTrianglesForNode( BSPFile & bspFile,
		BSP & node, int numTriangles ) const;

	BSP * pRoot_;
	RealWTriangleSet triangles_;

	// Used for storage when loading.
	mutable uint16 * pIndices_;
	int indicesSize_;

	char * pNodeMemory_;

	BoundingBox bb_;

	typedef std::map<UserDataKey, BinaryPtr> UserDataMap;
	UserDataMap userData_;

	friend class BSP;
};


/**
 *	This class is used to implement a BSP (Binary Space Partitioning) tree.
 *	Objects of this type can be thought of as both a node of a BSP tree and the
 *	BSP tree that is root at that node.
 */
class BSP
{
public:
	bool intersects( const WorldTriangle & triangle,
		const WorldTriangle ** ppHitTriangle = NULL ) const;

	bool intersects( const Vector3 & start,
		const Vector3 & end,
		float & dist,
		const WorldTriangle ** ppHitTriangle = NULL,
		CollisionVisitor * pVisitor = NULL ) const;

	bool intersects( const WorldTriangle & triangle,
		const Vector3 & translation,
		CollisionVisitor * pVisitor = NULL ) const;

	void getNumNodes( int & numNodes, int & maxTriangles ) const;

	// Debugging
	void dump( int depth );
	uint32 size() const;

	bool load( const BSPTree & tree, BSPFile & bspFile,
		BSPAllocator & allocator );
	// TODO: This is only used by xbsync. We should really only compile it in
	// for it.
	bool save( FILE * pFile, const WorldTriangle * pFront ) const;

private:
	BSP();
	~BSP();

	BSPSide whichSide(const WorldTriangle * pTriangle) const;
	BSPSide whichSide(const WorldPolygon & poly) const;

	bool intersectsThisNode(const WorldTriangle & triangle,
		const WorldTriangle ** ppHitTriangle) const;

	bool intersectsThisNode(const Vector3 & start,
						const Vector3 &		end,
						float &				dist,
						const WorldTriangle ** ppHitTriangle,
						CollisionVisitor *  pVisitor ) const;

	bool intersectsThisNode( const WorldTriangle & triangle,
		const Vector3 & translation,
		CollisionVisitor * pVisitor ) const;

	void partition( WTriangleSet & triangles,
			WPolygonSet & polygons,
			BSPConstructor & constructor );

	BSP * pFront_;
	BSP * pBack_;
	PlaneEq planeEq_;
	WTriangleSet triangles_;
	bool partitioned_;

	static const int MAX_SIZE;
	static const float TOLERANCE;

	friend class BSPConstructor;
	friend class BSPAllocator;
	friend class BSPTree;
};

#ifdef CODE_INLINE
    #include "bsp.ipp"
#endif

#endif
