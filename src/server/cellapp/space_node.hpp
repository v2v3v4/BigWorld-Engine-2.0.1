/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPACE_NODE_HPP
#define SPACE_NODE_HPP

class CellInfoVisitor;

#include "cstdmf/binary_stream.hpp"
#include "math/rectt.hpp"

class CellInfo;
class CellInfoVisitor;


/**
 *	This base class is used to represent the nodes in the BSP tree that is
 *	used to partition a space.
 */
class SpaceNode
{
public:
	virtual void deleteTree() = 0;

	virtual const CellInfo * pCellAt( float x, float z ) const = 0;
	virtual void visitRect(
			const BW::Rect & rect, CellInfoVisitor & visitor ) = 0;
	virtual void addToStream( BinaryOStream & stream ) const = 0;

protected:
	virtual ~SpaceNode() {};
};

#endif // SPACE_NODE_HPP
