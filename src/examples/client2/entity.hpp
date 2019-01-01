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
#include "cstdmf/binary_stream.hpp"
#include "entity_type.hpp"


class Entity
{
public:
	Entity( EntityID id, const EntityType & type, const Vector3& pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data , bool isBasePlayer );
	~Entity();

	EntityID id() const { return id_; }
	const EntityType & type() const { return type_; }

	void readCellPlayerData( BinaryIStream & stream );
	void updateProperties( BinaryIStream & stream,
		bool shouldCallSetMethod = true );

	void setProperty( const DataDescription * pDataDescription,
		PyObject * pValue,
		bool shouldCallSetMethod = true );


	typedef std::map<EntityID, Entity *> EntityMap;
	static EntityMap entities_;

	void handlePropertyChange( int messageID, BinaryIStream & data );
	void handleMethodCall( int messageID, BinaryIStream & data );

private:

	EntityID id_;
	const EntityType & type_;
	PyObject * pDict_;
};

#endif // ENTITY_HPP
