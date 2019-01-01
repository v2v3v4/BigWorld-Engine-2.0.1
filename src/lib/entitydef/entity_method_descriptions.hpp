/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_METHOD_DESCRIPTIONS_HPP
#define ENTITY_METHOD_DESCRIPTIONS_HPP

#include "method_description.hpp"

#include "cstdmf/watcher.hpp"

#include <map>
#include <vector>

class MethodDescription;

typedef std::vector< MethodDescription >	MethodDescriptionList;

/**
 *	This class is used to store descriptions of the methods of an entity.
 */
class EntityMethodDescriptions
{
public:
	bool init( DataSectionPtr pMethods,
		MethodDescription::Component component,
		const char * interfaceName );
	void checkExposedForSubSlots();
	void checkExposedForPythonArgs( const char * interfaceName );
	void supersede();

	unsigned int size() const;
	unsigned int exposedSize() const { return exposedMethods_.size(); }

	MethodDescription * internalMethod( unsigned int index ) const;
	MethodDescription * exposedMethod( uint8 topIndex, BinaryIStream & data ) const;
	MethodDescription * find( const std::string & name ) const;

	MethodDescriptionList & internalMethods()	{ return internalMethods_; }
	const MethodDescriptionList & internalMethods() const	{ return internalMethods_; }
#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif
private:
	typedef std::map< std::string, uint32 > Map;
	typedef MethodDescriptionList	List;

	Map		map_;
	List	internalMethods_;

	std::vector< unsigned int >	exposedMethods_;
};

#endif // ENTITY_METHOD_DESCRIPTIONS_HPP
