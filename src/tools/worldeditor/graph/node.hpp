/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NODE_HPP
#define NODE_HPP


#include "cstdmf/smartpointer.hpp"


namespace Graph
{


// Forward declarations.
class Node;
typedef SmartPointer< Node > NodePtr;


/**
 *	This class stores info about an node in a Graph. In most cases it will be
 *	derived by application-specific edge classes.
 */
class Node : public ReferenceCount
{
public:
	Node();
	virtual ~Node();

private:
	Node( const Node & other );
};


} // namespace Graph


#endif // NODE_HPP
