/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHARED_DATA_MANAGER_HPP
#define SHARED_DATA_MANAGER_HPP

#include "cstdmf/shared_ptr.hpp"

class BinaryIStream;
class BinaryOStream;
class Pickler;
class SharedData;

/**
 *	This class is responsible for handling the SharedData instances.
 */
class SharedDataManager
{
public:
	static shared_ptr< SharedDataManager > create( Pickler * pPickler );
	~SharedDataManager();

	void setSharedData( BinaryIStream & data );
	void delSharedData( BinaryIStream & data );

	void addToStream( BinaryOStream & stream );

private:
	SharedDataManager();

	bool init( Pickler * pPickler );

	SharedData *	pBaseAppData_;
	SharedData *	pGlobalData_;
};

#endif // SHARED_DATA_MANAGER_HPP
