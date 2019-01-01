/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PRIMITIVE_MANAGER_HPP
#define PRIMITIVE_MANAGER_HPP

#include <list>
#include "cstdmf/concurrency.hpp"
#include "primitive.hpp"
#include "device_callback.hpp"

namespace Moo
{

/**
 *	This singleton class keeps track of and loads Primitives.
 */
class PrimitiveManager : public Moo::DeviceCallback
{
public:
	typedef std::map< std::string, Primitive * > PrimitiveMap;

	~PrimitiveManager();

	static PrimitiveManager* instance();

	PrimitivePtr				get( const std::string& resourceID );

	virtual void				deleteManagedObjects( );
	virtual void				createManagedObjects( );

	static void init();
	static void fini();
private:
	PrimitiveManager();
	PrimitiveManager(const PrimitiveManager&);
	PrimitiveManager& operator=(const PrimitiveManager&);

	static void del( Primitive * pPrimitive );

	void addInternal( Primitive * pPrimitive );
	void delInternal( Primitive * pPrimitive );

	PrimitivePtr find( const std::string & resourceID );

	PrimitiveMap				primitives_;
	SimpleMutex					primitivesLock_;

	static PrimitiveManager*	pInstance_;

	friend Primitive::~Primitive();
};

}



#endif // PRIMITIVE_MANAGER_HPP
