/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NODE_CATALOGUE_HPP
#define NODE_CATALOGUE_HPP

#include "node.hpp"
#include "cstdmf/stringmap.hpp"
#include "cstdmf/singleton.hpp"
#include "cstdmf/concurrency.hpp"


namespace Moo
{

/**
 *	This class implements a global node catalogue.  It allows visuals
 *	and animations to share node storage
 *	This allows some memory and performance optimisations, as multiple
 *	models sharing a skeleton need only animate once, and multiple models
 *	using similarly named nodes do not have multiple copies of the node.
 *	These optimisations are made at the expense of having node traversal
 *	information overwritten by each successive traversal.  This symptom
 *	is most noticeable in that PyModelNodes need to copy world transforms
 *	from the nodes as they animate instead of pointing to the state of
 *	the existing node.
 */
class NodeCatalogue : public StringHashMap< NodePtr >,
					  public Singleton< NodeCatalogue >
{
public:
	static Node * findOrAdd( NodePtr pNode );
	static NodePtr find( const char * identifier );
	static void grab();
	static void give();

private:
	SimpleMutex nodeCatalogueLock_;
};

};

#endif //NODE_CATALOGUE_HPP
