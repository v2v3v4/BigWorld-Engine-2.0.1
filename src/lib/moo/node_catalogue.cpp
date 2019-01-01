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


BW_SINGLETON_STORAGE( Moo::NodeCatalogue );


namespace Moo
{


/**
 *	Static method to find or add a node in the node catalogue. 
 *	It implicitly locks the node catalogue, so you don't need to manually
 *	lock it before using this method.
 *
 *	@param	pNode			Source Node to use to find / add to the catalogue.
 *	@return	A pointer to the Node that was found / added. This method should never return NULL.
 */
Node * NodeCatalogue::findOrAdd( NodePtr pNode )
{
	BW_GUARD;
	Node * pStorageNode = NULL;

	SimpleMutexHolder smh( instance().nodeCatalogueLock_ );

	NodeCatalogue::iterator found =
		instance().StringHashMap<NodePtr>::find(
			pNode->identifier().c_str() );

	if (found != instance().end())
	{
		pStorageNode = found->second.getObject();
	}
	else
	{
		pStorageNode = pNode.getObject();

		instance().insert( std::make_pair(
			pNode->identifier(),
			pNode ) );
	}

	return pStorageNode;
}


/**
 *	Static method to find a node by name in the Model node catalogue.
 *	Note that this method hides the StringHashMap implementation of find.
 *	It implicitly locks the node catalogue, so you don't need to manually
 *	lock it before using this method.
 *
 *	@param	identifier		Name of the node to find.	
 *	@return NodePtr			Existing Node, or NULL if a node by that name 
 *							does not yet exist.
 */
NodePtr NodeCatalogue::find( const char * identifier )
{
	BW_GUARD;
	SimpleMutexHolder smh( instance().nodeCatalogueLock_ );

	NodeCatalogue::iterator found = 
		instance().StringHashMap<NodePtr>::find( identifier );

	if (found != instance().end()) return found->second;

	return NULL;
}


/**
 *	Static method to grab the lock for the node catalogue.
 *	The node catalogue must be locked when accessing it directly,
 *	e.g. through the [] operator.
 */
void NodeCatalogue::grab()
{
	BW_GUARD;
	instance().nodeCatalogueLock_.grab();
}


/**
 *	Static method to release the lock for the node catalogue.
 *	The node catalogue must be locked when accessing it directly,
 *	e.g. through the [] operator.
 */
void NodeCatalogue::give()
{
	instance().nodeCatalogueLock_.give();
}


};	//namespace Moo
