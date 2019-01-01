/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_BRANCH_HPP

#include "space_node.hpp"

class Space;


/**
 *	This class is used to represent an internal node of the BSP. It
 *	corresponds to a partitioning plane.
 */
class SpaceBranch : public SpaceNode
{
public:
	SpaceBranch( Space & space, const BW::Rect & rect,
			BinaryIStream & stream, bool isHorizontal );
	virtual ~SpaceBranch();
	virtual void deleteTree();

	virtual const CellInfo * pCellAt( float x, float z ) const;
	virtual void visitRect( const BW::Rect & rect, CellInfoVisitor & visitor );
	virtual void addToStream( BinaryOStream & stream ) const;

private:
	float	position_;
	bool	isHorizontal_;
	SpaceNode *	pLeft_;
	SpaceNode *	pRight_;
};

#endif // SPACE_BRANCH_HPP
