/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_CREATOR_HPP
#define ENTITY_CREATOR_HPP

#include "Python.h"

#include "cstdmf/shared_ptr.hpp"
#include "cstdmf/smartpointer.hpp"
#include "server/id_client.hpp"

class Base;
class EntityType;
class LoginHandler;

namespace Mercury
{
class ChannelOwner;
}

typedef SmartPointer< Base > BasePtr;
typedef SmartPointer< EntityType > EntityTypePtr;
typedef SmartPointer< PyObject > PyObjectPtr;
typedef Mercury::ChannelOwner DBMgr;


/**
 *	This class is responsible for creating entities.
 */
class EntityCreator
{
public:
	static shared_ptr< EntityCreator > create( DBMgr & idOwner,
			Mercury::NetworkInterface & intInterface,
			Mercury::NetworkInterface & extInterface );

	PyObject * createBaseRemotely( PyObject * args, PyObject * kwargs );
	PyObject * createBaseAnywhere( PyObject * args, PyObject * kwargs );
	PyObject * createBaseLocally( PyObject * args, PyObject * kwargs );
	PyObject * createBase( EntityType * pType, PyObject * pDict,
							PyObject * pCellData = NULL ) const;

	bool createBaseFromDB( const std::string& entityType,
					const std::string& name,
					PyObjectPtr pResultHandler );
	bool createBaseFromDB( const std::string& entityType, DatabaseID dbID,
					PyObjectPtr pResultHandler );
	PyObject* createRemoteBaseFromDB( const char * entityType,
					DatabaseID dbID,
					const char * name,
					const Mercury::Address* pDestAddr,
					PyObjectPtr pCallback,
					const char* origAPIFuncName );

	void createBaseFromDB( const Mercury::Address& srcAddr,
			const Mercury::UnpackedMessageHeader& header,
			BinaryIStream & data );

	void createBaseWithCellData( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			LoginHandler * pLoginHandler );

	void setCreateBaseInfo( BinaryIStream & data );

	void returnIDs()				{ idClient_.returnIDs(); }
	EntityID getID()				{ return idClient_.getID(); }
	void putUsedID( EntityID id )	{ idClient_.putUsedID( id ); }

private:
	// ---- Methods ----
	EntityCreator( Mercury::NetworkInterface & intInterface,
			Mercury::NetworkInterface & extInterface );
	bool init( DBMgr & idOwner );

	DBMgr & dbMgr();
	float getLoad() const;

	BasePtr createBaseFromStream( BinaryIStream & data,
			Mercury::Address * pClientAddr = NULL,
			std::string * pEncryptionKey = NULL );

	PyObject * createBaseCommon( const Mercury::Address * pAddr,
		PyObject * args, PyObject * kwargs, bool hasCallback = true );

	bool addCreateBaseData( BinaryOStream & stream,
					EntityTypePtr pType, PyObjectPtr pDict,
					PyObject * pCallback, EntityID * pNewID = NULL );
	EntityTypePtr consolidateCreateBaseArgs( PyObjectPtr pDestDict,
			PyObject * args, int headOffset, int tailOffset ) const;

	// ---- Data ----

	IDClient		idClient_;

	Mercury::NetworkInterface & intInterface_;
	Mercury::NetworkInterface & extInterface_;

	Mercury::Address	createBaseAnywhereAddr_;
};

#endif // ENTITY_CREATOR_HPP
