/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef QUAD_TREE_HPP
#define QUAD_TREE_HPP

/**
 *	This file implement the quadtree.
 *
 *	Possible memory reductions:
 *		- Reduce the depth of the tree that is being used.
 *		- In the calculateQTRange function, for large objects they can be scaled
 *			up so that they do not make as many leaf nodes.
 *		- When inserting objects, we do not need to go down to the maximum
 *			depth. We could stop when there are no other objects in the subtree
 *			(or a certain number).
 *		- We know that nodes at the maximum depth do not have any children so we
 *			could have a QuadTreeNode class without these pointers. This would
 *			involve casting (or the addition of virtual methods with the memory
 *			and small CPU costs).
 *		- The VC++7 implementation of std::vector takes 16 bytes when empty. We
 *			could look at having internal nodes having a fixed length array. If
 *			they had four, this would use the same amount of memory if the
 *			vectors were empty. If they overflowed, you would have to put them
 *			in child nodes. (This would cost 4 times as much).
 *		- An alternative to the previous option is for (internal nodes) they
 *			could keep a pointer to a vector. They only create it when
 *			necessary. This would cost 4 bytes for those nodes that are empty
 *			and 20 bytes for those that aren't. Nodes at the maximum depth would
 *			always have a collection.
 *		- We could just insert the objects until they would be split over nodes.
 *			This could be bad for nodes that overlap the centre point but the
 *			benefits include that we could keep the elements in a quadtree node
 *			as a linked-list with the next pointer actually in the
 *			ChunkObstacle. This would also get rid of the need for the 4 byte
 *			mark in ChunkObstacles.
 *
 *	Thoughts on speed:
 *		- We should look at how memory is allocated and maybe look at doing a
 *			custom allocator. Could also do batch adds.
 *
 *	Thoughts on cleanup:
 *		- The functions that need to be implemented based on the member type
 *			could be static methods of the DESC class.
 *
 *	Other thoughts:
 *		- It would be possible to use this for the chunk's hull tree.
 *
 *	TODO:	Need to implement deletion.
 */

#include <vector>

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "cstdmf/debug.hpp"

#include <float.h>
#include <string.h>
template <class MEMBER_TYPE> class QTTraversal;
template <class MEMBER_TYPE> class QuadTreeNode;
template <class MEMBER_TYPE> class QTTraversal;

typedef int QTIndex;

/**
 *	This enumeration is used to specify a child of a quadtree node.
 */
enum Quad
{
	QUAD_BL = 0,
	QUAD_BR = 1,
	QUAD_TL = 2,
	QUAD_TR = 3
};


// -----------------------------------------------------------------------------
// Section: QTCoord
// -----------------------------------------------------------------------------

/**
 *	This is a helper class used to specify a coord in the quadtree.
 */
class QTCoord
{
public:
	/**
	 *	This method makes sure that the coordinate is not outside the bottom and
	 *	and left sides.
	 */
	void clipMin()
	{
		x_ = std::max( 0, x_ );
		y_ = std::max( 0, y_ );
	}

	/**
	 *	This method makes sure that the coordinate is not outside the right and
	 *	top sides.
	 */
	void clipMax( int depth )
	{
		const int MAX = (1 << depth) - 1;
		x_ = std::min( x_, MAX );
		y_ = std::min( y_, MAX );
	}

	/**
	 *	This method returns the quadrant that a child coord is in.
	 *
	 *	@param depth	The depth that the child is at.
	 */
	int findChild( int depth ) const
	{
		const int MASK = (1 << depth);

		return ((x_ & MASK) + 2 * (y_ & MASK)) >> depth;
	}

	/**
	 *	This method converts this coord to indicate a child of the current node.
	 *
	 *	@param quad		The desired child quadrant.
	 *	@param depth	The depth of the child node.
	 */
	void offset( int quad, int depth )
	{
		x_ |= ((quad & 0x1) << depth);
		y_ |= ((quad > 1) << depth);
	}

	int x_;
	int y_;
};


// -----------------------------------------------------------------------------
// Section: QTRange
// -----------------------------------------------------------------------------

/**
 *	This is a helper class used to specify a range in the quadtree.
 */
class QTRange
{
public:
	/**
	 *	This method returns whether or not this range fills a node at the input
	 *	depth.
	 */
	bool fills( int depth ) const
	{
		const int MAX = (1 << depth) - 1;
		return left_ == 0 && bottom_ == 0 &&
			right_ == MAX && top_ == MAX;
	}

	/**
	 *	This method returns whether or not this range is valid. It is only used
	 *	for debugging.
	 */
	bool isValid( int depth ) const
	{
		const int MAX = 1 << depth;
		return
			(0 <= left_) && (left_ <= right_) && (right_ < MAX) &&
			(0 <= bottom_) && (bottom_ <= top_) && (top_ < MAX) &&
			depth >= 0;
	}

	/**
	 *	This method returns whether or not this range is in the quadtree with
	 *	the input depth.
	 */
	bool inQuad( int depth ) const
	{
		const int MAX = 1 << depth;

		return 0 <= right_ && 0 <= top_ &&
			left_ < MAX && bottom_ < MAX;
	}

	/**
	 *	This method clips this range so that it is entirely in the range of a
	 *	quadtree with the input depth.
	 */
	void clip( int depth )
	{
		corner_[0].clipMin();
		corner_[1].clipMax( depth );
	}

	// ---- Data ----
	union
	{
		QTIndex	ranges_[4];
		QTCoord	corner_[2];
		struct
		{
			QTIndex	left_;
			QTIndex	bottom_;
			QTIndex	right_;
			QTIndex	top_;
		};
	};
};


// -----------------------------------------------------------------------------
// Section: QuadTree
// -----------------------------------------------------------------------------

/**
 *	This class implements a quadtree. It is used to store the collision objects
 *	in a column.
 *
 *	MEMBER_TYPE indicates the type of object that is to be stored in the
 *	QuadTree. There needs to be a function matching the following prototype:
 *
 *	QTRange calculateQTRange( const MEMBER_TYPE & input, const Vector2 & origin,
 *						 float range, int depth )
 *
 *	DESC is a class that describes the QuadTree. It should have two static
 *	members. An integer DEPTH indicates the maximum depth of the tree and a
 *	floating point value, RANGE, that indicates the range that the QuadTree
 *	covers. It is assumed to be square.
 */
template <class MEMBER_TYPE>
class QuadTree
{
public:
	typedef QuadTree< MEMBER_TYPE > This;
	typedef QuadTreeNode< MEMBER_TYPE > Node;
	typedef QTTraversal< MEMBER_TYPE > Traversal;

	QuadTree( float x, float z, int depth, float range );
	~QuadTree();

	void add( const MEMBER_TYPE & element );
	void del( const MEMBER_TYPE & element );

	const MEMBER_TYPE * testPoint( const Vector3 & point ) const;

	Node *			pRoot()			{ return &root_; }
	const Node *	pRoot()	const	{ return &root_; }

	void print() const;
	long size() const;

	Traversal traverse( const Vector3 & source, const Vector3 & extent,
		float radius = 0.f ) const;

	int depth() const	{ return depth_; }
	float range() const	{ return range_; }

private:
	QuadTree( const QuadTree& );
	QuadTree& operator=( const QuadTree& );

	Node root_;

	const Vector2 origin_;	/// The coordinate of the bottom-left corner.
	const int depth_;
	const float range_;
};


// -----------------------------------------------------------------------------
// Section: QuadTreeNode
// -----------------------------------------------------------------------------

/**
 *	This class is used by QuadTree to implement its internal nodes.
 */
template <class MEMBER_TYPE>
class QuadTreeNode
{
public:
	typedef QuadTreeNode< MEMBER_TYPE > This;

	QuadTreeNode()
	{
		pChild_[0] = NULL;
		pChild_[1] = NULL;
		pChild_[2] = NULL;
		pChild_[3] = NULL;
	}
	~QuadTreeNode()
	{
		delete pChild_[0];
		delete pChild_[1];
		delete pChild_[2];
		delete pChild_[3];
	}

	void add( const MEMBER_TYPE & element, const QTRange & range, int depth );
	bool del( const MEMBER_TYPE & element, const QTRange & range, int depth );

	const MEMBER_TYPE * testPoint( const Vector3 & point,
		QTCoord coord, int depth ) const;

	typedef std::vector<const MEMBER_TYPE *> Elements;
	const Elements & elements() const	{ return elements_; }

	QuadTreeNode<MEMBER_TYPE> * pChild( int i ) const
	{
		return pChild_[i];
	}


	// For debugging
	int size() const;
	void print( int depth ) const;
	int countAt( QTCoord coord, int depth ) const;

private:
	QuadTreeNode<MEMBER_TYPE> * getOrCreateChild( int i )
	{
		if (pChild_[i] == NULL)
		{
			pChild_[i] = new QuadTreeNode;
		}

		return pChild_[i];
	}

	QuadTreeNode<MEMBER_TYPE> * pChild_[4];
	Elements	elements_;
};


// -----------------------------------------------------------------------------
// Section: QTTraversal
// -----------------------------------------------------------------------------

/**
 *	This class is used to traverse a QuadTree.
 */
template <class MEMBER_TYPE>
class QTTraversal
{
	static const int Q0 = 1 << 0;	// Bottom left
	static const int Q1 = 1 << 1;	// Bottom right
	static const int Q2 = 1 << 2;	// Top left
	static const int Q3 = 1 << 3;	// Top right

	typedef QuadTree<MEMBER_TYPE> Tree;
	typedef typename Tree::Node	Node;

public:
	const MEMBER_TYPE * next();

private:
	QTTraversal( const Tree * pTree, const Vector3 & src,
			const Vector3 & dst, float radius,
			const Vector2 & origin );

	struct StackElement
	{
		const Node * pNode;
		QTCoord coord;
		int depth;
	};

	inline void pushChild( const StackElement & curr, int quad );
	void push( const Node * pNode, QTCoord coord, int depth );
	StackElement pop();

	const MEMBER_TYPE * processHead();

	// TODO: Should not hard-code the stack size. It should be something like:
	//	3 * depth_ + 1
	static const int STACK_SIZE = 20;
	StackElement stack_[ STACK_SIZE ];
	int size_;

	size_t			headIndex_;

	// The following members are used to describe the query. We think of the
	// query interval parameterised as: src + t*dir, for t in [0,1]. Since the
	// query line has width, we end have a high side of the line and the low
	// side of the line.
	float			xLow0_;		///< Stores the t where the low line cuts x=0
	float			yLow0_;		///< Stores the t where the low line cuts y=0

	float			xHigh0_;	///< Stores the t where the high line cuts x=0
	float			yHigh0_;	///< Stores the t where the high line cuts y=0

	// The following store how much t changes for each grid square movement at
	// the lowest level along their relevant axis. The reason we keep separate
	// deltas for the low line and the high line is for the degenerate case when
	// the query line is along an axis.
	float			xLowDelta_;
	float			yLowDelta_;

	float			xHighDelta_;
	float			yHighDelta_;

	/**
	 *	Stores the direction of the traversal.
	 *	0 - Bottom-left to top-right
	 *	1 - Bottom-right to top-left
	 *	2 - Top-left to bottom-right
	 *	3 -	Top-right to bottom-left
	 */
	int				dirType_;

	friend class QuadTree< MEMBER_TYPE >;
};

#include "quad_tree.ipp"

#endif // QUAD_TREE_HPP
