/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pickler.hpp"
#include "network/basictypes.hpp"
#include "cstdmf/binary_stream.hpp"
#include "entity_type.hpp"
#include "client_app.hpp"


/*~ class BigWorld.Entity
 *  @components{ bots }
 *
 *	This is the Bots component of the Distributed Entity Model. 
 *	It is responsible for handling an automated mimicing of similar Client
 *	entities. This includes interacting with other entities, and responding
 *	to updates from the server.
 *
 *	For more information refer to the Server Operations Guide, chapter
 *	Stress Testing With Bots.
 */
class Entity : public PyInstancePlus
{
	Py_InstanceHeader( Entity )

public:
	Entity( const ClientApp & clientApp, EntityID id, const EntityType & type,
		const Vector3 & pos, float yaw, float pitch, float roll,
		BinaryIStream & data , bool isBasePlayer );
	~Entity();

	void destroy();

	EntityID id() const { return id_; }
	const EntityType & type() const { return type_; }

	void readCellPlayerData( BinaryIStream & stream );
	void updateProperties( BinaryIStream & stream,
		bool shouldCallSetMethod = true );

	void setProperty( const DataDescription * pDataDescription,
		PyObjectPtr pValue,
		bool shouldCallSetMethod = true );

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	const CacheStamps & cacheStamps() const     { return cacheStamps_; }
	void cacheStamps( const CacheStamps & s )   { cacheStamps_ = s; }

	const ClientApp & getClientApp() const		{ return clientApp_; }

	void handlePropertyChange( int messageID, BinaryIStream & data );
	void handleMethodCall( int messageID, BinaryIStream & data );

	PY_RW_ATTRIBUTE_DECLARE( position_, position );
	const Position3D& position() { return position_; }
	void position( const Position3D& pos ) { position_ = pos; }

	PY_RO_ATTRIBUTE_DECLARE( pPyCell_, cell );
	PY_RO_ATTRIBUTE_DECLARE( pPyBase_, base );

	PY_RO_ATTRIBUTE_DECLARE( id_, id );

	PY_RO_ATTRIBUTE_DECLARE( &clientApp_, clientApp );

private:
	const ClientApp & clientApp_;
	Position3D position_;

	PyObject * pPyCell_;
	PyObject * pPyBase_;

	EntityID id_;
	CacheStamps cacheStamps_;
	const EntityType & type_;
};

#endif // ENTITY_HPP
