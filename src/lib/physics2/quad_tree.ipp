/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// quad_tree.ipp


// -----------------------------------------------------------------------------
// Section: QuadTree
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param x	The x-coordinate of the left of the quadtree range.
 *	@param z	The z-coordinate of the bottom of the quadtree range.
 *	@param depth The maximum depth of the tree.
 *	@param range The range that the QuadTree covers. It is assumed to be square.
 */
template <class MEMBER_TYPE>
inline QuadTree<MEMBER_TYPE>::QuadTree( float x, float z,
		int depth, float range ) :
	origin_( x, z ),
	depth_( depth ),
	range_( range )
{
}


/**
 *	Destructor.
 */
template <class MEMBER_TYPE>
inline QuadTree<MEMBER_TYPE>::~QuadTree()
{
}


/**
 *	This method	adds the input element to the quadtree.
 */
template <class MEMBER_TYPE>
inline void QuadTree<MEMBER_TYPE>::add( const MEMBER_TYPE & element )
{
	QTRange range = calculateQTRange( element, origin_,
		range_, depth_ );

	if (!range.inQuad( depth_ ))
	{
		// This warning is known to be triggered by the current
		// incomplete system for handling very large objects.
		dprintf("QuadTree::add: Skipping - Out of range (%d, %d) - (%d, %d)\n",
			range.left_, range.bottom_, range.right_, range.top_ );
		return;
	}

	range.clip( depth_ );

	root_.add( element, range, depth_ );
}


/**
 *	This method removes the input element from this quadtree.
 */
template <class MEMBER_TYPE>
inline void QuadTree<MEMBER_TYPE>::del( const MEMBER_TYPE & element )
{
	QTRange range = calculateQTRange( element, range_, depth_ );
	root_.del( element, range );
}


/**
 *	This method returns member that contains the input point.
 */
template <class MEMBER_TYPE>
inline const MEMBER_TYPE *
QuadTree<MEMBER_TYPE>::testPoint( const Vector3 & point ) const
{
	return root_.testPoint( point );
}


/**
 *	For debugging. This method prints out this tree.
 */
template <class MEMBER_TYPE>
inline void QuadTree<MEMBER_TYPE>::print() const
{
	dprintf( "sizeof(QT) = %d\n", sizeof( *this ) );
	dprintf( "sizeof(QTN) = %d\n", sizeof( QuadTreeNode<MEMBER_TYPE> ) );
	dprintf( "sizeof(elements) = %d\n", sizeof( std::vector<const MEMBER_TYPE *> ) );
	dprintf( "Range = %f, Depth = %d\n", range_, depth_ );
	root_.print( 0 );

	const int width = 1 << depth_;
	QTCoord coord;

	for (coord.y_ = width - 1; coord.y_ >= 0; coord.y_--)
	{
		for (coord.x_ = 0; coord.x_ < width; coord.x_++)
		{
			dprintf( "%d", root_.countAt( coord, depth_ ) );
		}
		dprintf( "\n" );
	}
}


/**
 *	This method returns the number of bytes that this tree uses.
 */
template <class MEMBER_TYPE>
inline long QuadTree<MEMBER_TYPE>::size() const
{
	return sizeof( *this ) + root_.size();
}


/**
 *	This method returns a traversal object that can be used to traverse the
 *	quadtree and visit the members they may have been collided.
 */
template <class MEMBER_TYPE>
inline QTTraversal<MEMBER_TYPE> QuadTree<MEMBER_TYPE>::traverse(
	const Vector3 & src, const Vector3 & dst,
	float radius ) const
{
	// TODO: This method could be generalised so the we don't have to always
	// take Vector3s. We could have a conversion method to get Vector2s.
	return QTTraversal<MEMBER_TYPE>( this, src, dst, radius, origin_ );
}


// -----------------------------------------------------------------------------
// Section: QTTraversal
// -----------------------------------------------------------------------------

/**
 *	Constructor. It creates a traversal that 'points' to the first element.
 */
template <class MEMBER_TYPE>
inline QTTraversal<MEMBER_TYPE>::QTTraversal(
		const Tree * pTree,
		const Vector3 & src, const Vector3 & dst, float radius,
		const Vector2 & origin ) :
	size_( 0 ),
	headIndex_( 0 )
{
	const int DEPTH = pTree->depth();
	const float RANGE = pTree->range();
	QTCoord root;
	root.x_ = root.y_ = 0;

	// The root is the first node that we are going to consider.
	this->push( pTree->pRoot(), root, DEPTH );

	// This stores the interval from the src to dst. We can think of the line as
	// being: src + t * dir, for t in [0, 1]
	Vector2 dir( dst.x - src.x, dst.z - src.z );

	// dirType_ stores whether the line points up or down, left or right.
	const bool pointsLeft = (dir.x < 0);
	const bool pointsDown = (dir.y < 0);
	dirType_ = 2 * pointsDown + pointsLeft;

	// Since the line has width, we calculate a high line and a low line.
	Vector2 dirNorm = dir;
	dirNorm.normalise();
	dirNorm *= radius;
	Vector2 perp( -dirNorm.y, dirNorm.x );

	// Directions are flipped so that all calculations are done assuming the
	// query line points towards to top-right.

	// If the line points to top-left or bottom-right, the perpendicular is
	// actually in the opposite direction.
	if (pointsDown != pointsLeft)
	{
		perp *= -1.f;
	}

	dir += 2.f * dirNorm;

	Vector2 srcLow( src.x, src.z );
	srcLow -= dirNorm;
	srcLow -= perp;

	Vector2 srcHigh( src.x, src.z );
	srcHigh -= dirNorm;
	srcHigh += perp;

	Vector2 dstLow( dst.x, dst.z );
	dstLow += dirNorm;
	dstLow -= perp;
	Vector2 dstHigh( dst.x, dst.z );
	dstHigh += dirNorm;
	dstHigh += perp;

	const int WIDTH = 1 << DEPTH;

	// Needs to be big but when you divide by 0.5 (i.e. double) it must not go
	// to infinity.
	const float BIG_FLOAT = FLT_MAX / 4.f;

	if (dir.x != 0.f)
	{
		// Solving 0 = (srcLow.x - origin.x) + t * dir.x
		xLow0_	= (origin.x + pointsLeft * RANGE - srcLow.x) / dir.x;
		xHigh0_	= (origin.x + pointsLeft * RANGE - srcHigh.x) / dir.x;

		// How much t changes for each step in the grid
		xLowDelta_	= RANGE / WIDTH / fabsf( dir.x );
		xHighDelta_ = xLowDelta_;
	}
	else
	{
		// For the degenerate case, where we travel parallel to the y-axis, we
		// want t to be a big negative for coordinates less than the
		// y-coordinate and a big positive for coordinates greater.
		// Note: The degenerate case for the y-axis is slightly different
		//       to the x-axis as the high crossing has a smaller value
		//		 than the low crossing in the x case
		const float lowCrossing =
			floorf( WIDTH * (srcLow.x - origin.x) / RANGE ) + 0.5f;
		xLow0_ = -BIG_FLOAT;
		xLowDelta_ = -xLow0_/lowCrossing;

		// Note: The min here is due to a strange optimisation in VC+2005.
		// Even though srcLow.x >= srcHigh.x, the crossings could be the other
		// way around.
		const float highCrossing = std::min( lowCrossing,
			floorf( WIDTH * (srcHigh.x - origin.x) / RANGE ) + 0.5f );
		xHigh0_ = (highCrossing > 0.f) ? -BIG_FLOAT : BIG_FLOAT;
		xHighDelta_ = -xHigh0_/highCrossing;
	}
	
	// The y-coordinate done like the x-coordinate
	if (dir.y != 0.f)
	{
		yLow0_	= (origin.y + pointsDown * RANGE - srcLow.y) / dir.y;
		yHigh0_	= (origin.y + pointsDown * RANGE - srcHigh.y) / dir.y;
		yLowDelta_ = RANGE / WIDTH / fabsf( dir.y );
		yHighDelta_ = yLowDelta_;
	}
	else
	{
		const float lowCrossing =
			floorf( WIDTH * (srcLow.y - origin.y) / RANGE ) + 0.5f;
		yLow0_ = (lowCrossing > 0.f) ? -BIG_FLOAT : BIG_FLOAT;
		yLowDelta_ = -yLow0_/lowCrossing;

		// Note: The max here is due to a strange optimisation in VC+2005.
		// Even though srcLow.y <= srcHigh.y, the crossings could be the other
		// way around.
		const float highCrossing = std::max( lowCrossing,
			floorf( WIDTH * (srcHigh.y - origin.y) / RANGE ) + 0.5f );
		yHigh0_ = -BIG_FLOAT;
		yHighDelta_ = -yHigh0_/highCrossing;
	}
}


/**
 *	This method pushes a child onto the stack so that it will later be visited
 *	by the traversal.
 */
template <class MEMBER_TYPE>
inline void QTTraversal<MEMBER_TYPE>::pushChild(
	const StackElement & curr, int quad )
{
	// This is a bit tricky. We do the calculations assuming the interval points
	// from the bottom-left to the top-right. Here we calculate the actual
	// quadrant that we are talking about. It turns out that we can XOR the
	// virtual quadrant with the dirType.
	const Node * pChild = curr.pNode->pChild( quad ^ dirType_ );

	if (pChild)
	{
		QTCoord newCoord = curr.coord;

		// We still pretend it's in the flipped coord system
		newCoord.offset( quad, curr.depth - 1 );
		this->push( pChild, newCoord, curr.depth - 1 );
	}
}


/**
 *	This method returns the current elements in the traversal and moves on the
 *	the next element.
 */
template <class MEMBER_TYPE>
inline const MEMBER_TYPE * QTTraversal<MEMBER_TYPE>::next()
{
	if (size_ <= 0)
	{
		return NULL;
	}

#ifdef TIME_QUADTREE
	dwQTTraverse.start();
#endif

	while (size_ > 0)
	{
		// Check if the head has things to traverse.
		const MEMBER_TYPE * pResult = this->processHead();

		if (pResult)
		{
#ifdef TIME_QUADTREE
			dwQTTraverse.stop();
#endif
			return pResult;
		}

		// Pop the stack
		StackElement curr = this->pop();

		if (curr.depth > 0)
		{
			QTCoord & coord = curr.coord;
			// The number of grid square in half of this current node.
			const int halfWidth = 1 << (curr.depth - 1);

			// The following stores how much t changes across any of the child
			// nodes. Low and high are only different in the degenerate case
			// where the line is parallel to an axis.
			const float currLowXDelta = halfWidth * xLowDelta_;
			const float currLowYDelta = halfWidth * yLowDelta_;
			const float currHighXDelta = halfWidth * xHighDelta_;
			const float currHighYDelta = halfWidth * yHighDelta_;

			// Stores the t-value for the intersection of the lines with each of
			// the lines in the node.
			// (That is x=min, x=mid, x=max, y=min, y=mid, y=max).
			const float xMinLow = xLow0_ + coord.x_ * xLowDelta_;
			const float xMidLow = xMinLow + currLowXDelta;
//			const float xMaxLow = xMidLow + currLowXDelta;

			const float yMinLow = yLow0_ + coord.y_ * yLowDelta_;
			const float yMidLow = yMinLow + currLowYDelta;
			const float yMaxLow = yMidLow + currLowYDelta;

			const float xMinHigh = xHigh0_ + coord.x_ * xHighDelta_;
			const float xMidHigh = xMinHigh + currHighXDelta;
			const float xMaxHigh = xMidHigh + currHighXDelta;

			const float yMinHigh = yHigh0_ + coord.y_ * yHighDelta_;
			const float yMidHigh = yMinHigh + currHighYDelta;
//			const float yMaxHigh = yMidHigh + currHighYDelta;

			// The line goes above the mid-point?
			const bool aboveMidpoint = (yMidHigh <= xMidHigh);
			const bool belowMidpoint = (xMidLow <= yMidLow);

			const bool aboveQ0 = (yMidLow < xMinLow);
			const bool belowQ0 = (xMidHigh < yMinHigh);

			const bool aboveQ3 = (yMaxLow < xMidLow);
			const bool belowQ3 = (xMaxHigh < yMidHigh);

			// Is the interval entirely in 1 half of the region?
			const bool toLeft	= (xMidLow > 1.f);
			const bool toRight	= (xMidHigh < 0.f);
			const bool isAbove	= (yMidLow < 0.f);
			const bool isBelow	= (yMidHigh > 1.f);

			// Push the child that we intersect.
			// Note: Quad 3 is pushed first, so done last and Quad 0 pushed last
			//	so that it is done first.
			if (!aboveQ3 && !belowQ3 && !toLeft && !isBelow)
				this->pushChild( curr, 3 );

			if (belowMidpoint && !toLeft && !isAbove)
				this->pushChild( curr, 1 );

			if (aboveMidpoint && !toRight && !isBelow)
				this->pushChild( curr, 2 );

			if (!aboveQ0 && !belowQ0 && !toRight && !isAbove)
				this->pushChild( curr, 0 );
		}
	}

#ifdef TIME_QUADTREE
	dwQTTraverse.stop();
#endif
	return NULL;
}


/**
 *	This method pushs an entry onto the stack.
 */
template <class MEMBER_TYPE>
inline void QTTraversal<MEMBER_TYPE>::push(
	const Node * pNode, QTCoord coord, int depth )
{
	MF_ASSERT( size_ < STACK_SIZE );
	MF_ASSERT( headIndex_ == 0 );

	StackElement & curr = stack_[ size_ ];
	curr.pNode = pNode;
	curr.coord = coord;
	curr.depth = depth;
	size_++;
}


/**
 *	This method pushs an entry onto the stack.
 */
template <class MEMBER_TYPE>
inline typename QTTraversal<MEMBER_TYPE>::StackElement
	QTTraversal<MEMBER_TYPE>::pop()
{
	headIndex_ = 0;

	return stack_[ --size_ ];
}


/**
 *	This method looks at the current head of the stack. If there are still
 *	elements that have not yet been visited, visit them.
 *
 *	@return The next member to visit. If this node has none, NULL is returned.
 */
template <class MEMBER_TYPE>
inline const MEMBER_TYPE * QTTraversal<MEMBER_TYPE>::processHead()
{
	StackElement & head = stack_[ size_ - 1 ];
	const Node * pCurr = head.pNode;

	const typename Node::Elements & elements = pCurr->elements();

	while (headIndex_ < elements.size())
	{
		const MEMBER_TYPE * pNext = elements[ headIndex_ ];
		headIndex_++;

		// TODO: Test whether the height is correct.
		const bool shouldConsider = true;

		if (shouldConsider)
		{
			return pNext;
		}
	}

	return NULL;
}


// -----------------------------------------------------------------------------
// Section: QuadTreeNode
// -----------------------------------------------------------------------------

/**
 *	This method add a member to this node.
 */
template <class MEMBER_TYPE>
inline void QuadTreeNode<MEMBER_TYPE>::add( const MEMBER_TYPE & element,
	const QTRange & range, int depth )
{
	MF_ASSERT( range.isValid( depth ) );

	if (range.fills( depth ))
	{
		elements_.push_back( &element );
	}
	else
	{
		const int newDepth = depth - 1;
		const int MASK = (1 << newDepth);
		const int CLEAR = ~(1 << newDepth);
		const int MAX = (1 << newDepth) - 1;

		int inLeft		= !(range.left_		& MASK);
		int inBottom	= !(range.bottom_	& MASK);
		int inRight		= range.right_	& MASK;
		int inTop		= range.top_	& MASK;

		QTRange origRange = range;
		for (int i = 0; i < 4; i++)
		{
			origRange.ranges_[i] &= CLEAR;
		}
		QTRange newRange = origRange;

		if (inLeft)
		{
			if (inRight) newRange.right_ = MAX; // Right clipped

			if (inTop)
			{
				if (inBottom)
				{
					newRange.bottom_ = 0; // Left and bottom clipped
				}
				MF_ASSERT( newRange.isValid( newDepth ) );
				this->getOrCreateChild( QUAD_TL )->
						add( element, newRange, newDepth );
				newRange.bottom_ = origRange.bottom_;
				newRange.top_ = MAX;
				// Now right and top clipped
			}

			if (inBottom)
			{
				MF_ASSERT( newRange.isValid( newDepth ) );
				this->getOrCreateChild( QUAD_BL )->
					add( element, newRange, newDepth );
			}

			newRange.right_ = origRange.right_; // Now just top clipped
		}

		if (inRight)
		{
			if (inLeft) newRange.left_ = 0;
			if (inTop)	newRange.top_ = MAX;
			// Left and top clipped

			if (inBottom)
			{
				MF_ASSERT( newRange.isValid( newDepth ) );
				this->getOrCreateChild( QUAD_BR )->
					add( element, newRange, newDepth );
				newRange.bottom_ = 0;
				// Left, top and bottom clipped
			}

			if (inTop)
			{
				newRange.top_ = origRange.top_;
				// Left and bottom clipped.
				MF_ASSERT( newRange.isValid( newDepth ) );
				this->getOrCreateChild( QUAD_TR )->
					add( element, newRange, newDepth );
			}
		}
	}
}


/**
 *	This method deletes an element from this node.
 */
template <class MEMBER_TYPE>
inline bool QuadTreeNode<MEMBER_TYPE>::del( const MEMBER_TYPE & element,
	const QTRange & range,
	int depth )
{
	// TODO: Implement this!
	return false;
}


/**
 *	This method returns a member in this subtree that contains the input point.
 *	If no such member exists, NULL is returned.
 */
template <class MEMBER_TYPE>
inline const MEMBER_TYPE *
	QuadTreeNode<MEMBER_TYPE>::testPoint(
		const Vector3 & point, QTCoord coord, int depth ) const
{
	const MEMBER_TYPE * pResult = NULL;

	if (depth > 0)
	{
		const int quad = coord.findChild( depth -1 );
		This * pChild = pChild_[ quad ];

		if (pChild)
		{
			pResult = pChild->testPoint( point, coord, depth );
		}
	}

	if (pResult == NULL)
	{
		typename Elements::iterator iter = elements_.begin();

		while (iter != elements_.end())
		{
			const MEMBER_TYPE * pCurr = (*iter);

			if (containsTestPoint( *pCurr, point ))
			{
				return pCurr;
			}

			iter++;
		}
	}

	return pResult;
}


/**
 *	This method returns the number of bytes used by the quadtree rooted at this
 *	node.
 */
template <class MEMBER_TYPE>
inline int QuadTreeNode<MEMBER_TYPE>::size() const
{
	int childrenSize = 0;

	for (int i = 0; i < 4; i++)
	{
		childrenSize += (pChild_[i] ? pChild_[i]->size() : 0);
	}

	return childrenSize + sizeof( *this ) +
		sizeof( const MEMBER_TYPE * ) * elements_.capacity();
}


/**
 *	This function is used for debugging. It prints out info about this node.
 */
template <class NODE>
void printQTNode( const NODE & node, const char * prefix )
{
	dprintf( "%s%d\n", prefix, node.elements().size() );
}


/**
 *	This method is used for debugging. It prints out the quadtree rooted at this
 *	node.
 */
template <class MEMBER_TYPE>
inline void QuadTreeNode<MEMBER_TYPE>::print( int depth ) const
{
	char prefix[128];
	memset( prefix, ' ', 2 * depth );
	prefix[ 2 * depth ] = '\0';
	printQTNode( *this, prefix );

	for (int i = 0; i < 4; i++)
	{
		if (pChild_[i])
		{
			dprintf( "%s Child %d\n", prefix, i );
			pChild_[i]->print( depth + 1 );
		}
	}
}


/**
 *	This method is used for debugging. It returns the number of elements that
 *	are at the input coordinate.
 */
template <class MEMBER_TYPE>
inline int QuadTreeNode<MEMBER_TYPE>::countAt( QTCoord coord, int depth ) const
{
	int count = elements_.size();

	if (depth > 0)
	{
		int quad = coord.findChild( depth - 1 );

		QuadTreeNode<MEMBER_TYPE> * pChild = this->pChild( quad );

		if (pChild)
		{
			count += pChild->countAt( coord, depth -1 );
		}
	}

	return count;
}

// quad_tree.ipp
