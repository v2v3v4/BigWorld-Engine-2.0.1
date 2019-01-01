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

#include "node_catalogue.hpp"
#include "resmgr/datasection.hpp"

#ifndef MF_SERVER
#include "render_context.hpp"
#endif
#ifdef _WIN32
#include "math/blend_transform.hpp"
#endif

#include "node.hpp"

#ifndef CODE_INLINE
#include "node.ipp"
#endif

#include "cstdmf/memory_counter.hpp"
#include "cstdmf/guard.hpp"

memoryCounterDefine( node, Geometry );
PROFILER_DECLARE( Moo_Node_removeChild, "Moo Node removeChild" );

namespace Moo
{


/// Static initialiser
int Node::s_blendCookie_ = 1024;

/**
 * Default node constructor
 */
Node::Node() :
	transform_( Matrix::identity ),
	worldTransform_( Matrix::identity ),
	blendCookie_( s_blendCookie_ - 16 ),
	blendRatio_( 0.f ),
#ifdef _WIN32
	blendTransform_( transform_ ),
#endif
	transformInBlended_( false ),
	parent_( NULL )
{
	memoryCounterAdd( node );
	memoryClaim( this );
}

/**
 * Node destructor
 */
Node::~Node()
{
	memoryCounterSub( node );
	memoryClaim( this );
	memoryClaim( children_ );
	memoryClaim( identifier_ );
}


#ifndef MF_SERVER
/**
 * Traverse the hierarchy and update worldtransforms.
 */
void Node::traverse( )
{
	BW_GUARD;
	// Update transform
	Moo::rc().push();
	Moo::rc().preMultiply( this->transform() );
	this->worldTransform( Moo::rc().world() );	

	// Iterate and recurse through children
	NodePtrVector::iterator it = children_.begin();
	NodePtrVector::iterator end = children_.end();
	while (it != end)
	{
		(*it)->traverse();
		++it;
	}

	Moo::rc().pop();
}


/**
 *	This method copies this node's world transform into the
 *	appropriate copy in the global node catalogue and then
 *	continues down the hierarchy.
 */
void Node::loadIntoCatalogue( )
{
	BW_GUARD;
	NodePtr pGlobalNode = NodeCatalogue::find( identifier_.c_str() );

	pGlobalNode->worldTransform(worldTransform_);	

	NodePtrVector::iterator it = children_.begin();
	NodePtrVector::iterator end = children_.end();
	while (it != end)
	{
		(*it)->loadIntoCatalogue();
		++it;
	}
}
#endif // not MF_SERVER

/**
 * Add a child node
 */
void Node::addChild( NodePtr node )
{
	BW_GUARD;
	node->removeFromParent();
	children_.push_back( node );
	node->parent_ = this;
}

/**
 * Remove a child node
 */
void Node::removeChild( NodePtr node )
{
	BW_GUARD_PROFILER( Moo_Node_removeChild );
	NodePtrVector::iterator it = std::find( children_.begin(), children_.end(), node );
	MF_ASSERT_DEV( it != children_.end() );
	if( it != children_.end() )
	{
		(*it)->parent_ = NULL;
		children_.erase( it );
	}
}

/**
 * Remove this node from the parent
 */
void Node::removeFromParent( )
{
	BW_GUARD;
	if( parent_ )
	{
		parent_->removeChild( NodePtr( this ) );
	}
}


/**
 * Find a node recursively
 *
 * @param identifier name of node to find
 *
 * @return smartpointer to the node
 */
NodePtr Node::find( const std::string& identifier )
{
	BW_GUARD;
	// Is this the node we are looking for?
	if( identifier == identifier_ )
		return this;

	// Iterate through children, do a recursive find
	NodePtrVector::iterator it = children_.begin();
	NodePtrVector::iterator end = children_.end();
	while( it != end )
	{
		NodePtr ret = (*it)->find( identifier );
		if( ret )
			return ret;
		it++;
	}

	// The node is not in this branch.
	return NULL;
}


/**
 * Count the number of descendants this node has
 *
 * @return the number of descendants
 */
uint32 Node::countDescendants( ) const
{
	BW_GUARD;
	uint32 descendants = children_.size();
	NodePtrVector::const_iterator it = children_.begin();
	NodePtrVector::const_iterator end = children_.end();
	while( it != end )
	{
		descendants += (*it)->countDescendants();
		++it;
	}
	return descendants;
}

/**
 * Load nodes recursively
 *
 * @param nodeSection a DataSectionPtr to the node data to load
 *
 */
void Node::loadRecursive( DataSectionPtr nodeSection )
{
	BW_GUARD;
	memoryCounterAdd( node );

	identifier_ = nodeSection->readString( "identifier" );	
	memoryClaim( identifier_ );

	NodeCatalogue::findOrAdd(this);

	transform_ = nodeSection->readMatrix34( "transform", Matrix::identity );
	transformInBlended_ = false;

	std::vector< DataSectionPtr > nodeSections;
	nodeSection->openSections( "node", nodeSections );
	std::vector< DataSectionPtr >::iterator it = nodeSections.begin();
	std::vector< DataSectionPtr >::iterator end = nodeSections.end();

	while( it != end )
	{
		NodePtr childNode = new Node;
		childNode->loadRecursive( *it );
		this->addChild( childNode );
		++it;
	}
	memoryClaim( children_ );
}

/**
 * Set this nodes identifier
 *
 * @param identifier the nodes identifier
 */
void Node::identifier( const std::string& identifier )
{
	BW_GUARD;
	identifier_ = identifier;

	NodeCatalogue::findOrAdd(this);

	memoryCounterAdd( node );
	memoryClaim( identifier_ );
}

}

// node.cpp
