/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity.hpp"

#include "entitydef/property_owner.hpp"
#include "network/msgtypes.hpp"

#include "Python.h"


DECLARE_DEBUG_COMPONENT2( "Entity", 0 )

// static variable instantiation
std::map<EntityID, Entity *> Entity::entities_;

extern EntityID g_playerID;

/**
 *	Constructor.
 */
Entity::Entity( EntityID id, const EntityType & type, const Vector3& pos,
		float yaw, float pitch, float roll,
		BinaryIStream & data , bool isBasePlayer ) :
	id_ ( id ),
	type_( type )
{
	Vector3 orientation( yaw, pitch, roll );

	std::cout << "Position:" << pos << ". Orientation:" << orientation
		<< "." << std::endl;

	if (isBasePlayer)
	{
		pDict_ = type_.newDictionary( data,
			EntityDescription::CLIENT_DATA |
			EntityDescription::BASE_DATA |
			EntityDescription::EXACT_MATCH );
	}
	else
	{
		pDict_ = type_.newDictionary();
		this->updateProperties( data, false );
	}

	PyObject * pStr = PyObject_Str( pDict_ );
	std::cout << PyString_AsString( pStr ) << std::endl;
	Py_DECREF( pStr );
}


/**
 *	Destructor.
 */
Entity::~Entity()
{
	Py_XDECREF( pDict_ );
	MF_VERIFY( entities_.erase( id_ ) );
}


/**
 *	Helper class to cast entity as a property owner
 */
class EntityPropertyOwner : public TopLevelPropertyOwner
{
public:
	EntityPropertyOwner( PyObject * dict, const EntityDescription & edesc ) :
		dict_( dict ), edesc_( edesc ) { }

	// called going to the root of the tree
	virtual void onOwnedPropertyChanged( PropertyChange & change )
	{
		// unimplemented
	}

	// called going to the leaves of the tree
	virtual int getNumOwnedProperties() const
	{
		return edesc_.clientServerPropertyCount();
	}

	virtual PropertyOwnerBase * getChildPropertyOwner( int ref ) const
	{
		DataDescription * pDD = edesc_.clientServerProperty( ref );
		PyObjectPtr pPyObj =
			PyDict_GetItemString( dict_, (char*)pDD->name().c_str() );
		if (!pPyObj)
		{
			PyErr_Clear();
			return NULL;
		}

		return pDD->dataType()->asOwner( &*pPyObj );
	}

	virtual PyObjectPtr setOwnedProperty( int ref, BinaryIStream & data )
	{
		DataDescription * pDD = edesc_.clientServerProperty( ref );
		if (pDD == NULL) return NULL;

		PyObjectPtr pNewObj = pDD->createFromStream( data, false );
		if (!pNewObj)
		{
			ERROR_MSG( "Entity::handleProperty: "
				"Error streaming off new property value\n" );
			return NULL;
		}

		PyObjectPtr pOldObj =
			PyDict_GetItemString( dict_, (char*)pDD->name().c_str() );
		if (!pOldObj)
		{
			PyErr_Clear();
			pOldObj = Py_None;
		}

		int err = PyDict_SetItemString(
			dict_, (char*)pDD->name().c_str(), &*pNewObj );
		if (err != 0)
		{
			ERROR_MSG( "Entity::handleProperty: "
				"Failed to set new property into Entity\n" );
			PyErr_Print();
		}

		return pOldObj;
	}

private:
	PyObject * dict_;
	const EntityDescription & edesc_;
};


/**
 *	This method is called when a message is received from the server telling us
 *	to change a property on this entity.
 */
void Entity::handlePropertyChange( int messageID, BinaryIStream & data )
{
	EntityPropertyOwner king( pDict_, type_.description() );

	PyObjectPtr pOldValue = Py_None;
	PyObjectPtr pChangePath = Py_None;

	int topLevelIndex = king.setPropertyFromExternalStream( data, messageID,
			&pOldValue, &pChangePath );

	// print it out
	DataDescription * pDD =
		type_.description().clientServerProperty( topLevelIndex );
	std::cout << "Setting " << pDD->name();

	PyObject * pValueStr = PyObject_Str( pOldValue.get() );
	PyObject * pPathStr = PyObject_Str( pChangePath.get() );
	std::cout << " from " << PyString_AsString( pValueStr );
	std::cout << ". Path " << PyString_AsString( pPathStr ) << std::endl;
	Py_DECREF( pValueStr );
	Py_DECREF( pPathStr );
}


/**
 *	This method is called when a message is received from the server telling us
 *	to call a method on this entity.
 */
void Entity::handleMethodCall( int messageID, BinaryIStream & data )
{
	MethodDescription * pMethodDescription =
		type_.description().clientMethod( messageID, data );

	MF_ASSERT( pMethodDescription != NULL );

	if (pMethodDescription != NULL)
	{
		//pMethodDescription->callMethod( this, true, data );
		// We would normally call the method for this methodDescription here
		// but no script objects have been set up on the client yet, so
		// we just print out the method name and arguments.
		std::cout << "Call method: " << pMethodDescription->name() << std::endl;
		SmartPointer<PyObject> spArgs(
				pMethodDescription->getArgsAsTuple( data ) );
		SmartPointer<PyObject> spStr( PyObject_Str( spArgs.getObject() ) );
		std::cout << "Arguments: " << PyString_AsString( spStr.getObject() )
			<< std::endl;
	}
	else
	{
		ERROR_MSG( "Entity::handleMethodCall: "
			"Do not have method starting with message id %d\n", messageID );
	}
}


/**
 *	This method reads the player data send from the cell. This is called on the
 *	player entity when it gets a cell entity associated with it.
 */
void Entity::readCellPlayerData( BinaryIStream & stream )
{
	PyObject * pCellData = type_.newDictionary( stream,
		EntityDescription::CLIENT_DATA |
		EntityDescription::CELL_DATA |
		EntityDescription::EXACT_MATCH );

	PyDict_Update( pDict_, pCellData );
	Py_DECREF( pCellData );

	std::cout << "Entity::readCellPlayerData:" << std::endl;
	PyObject * pStr = PyObject_Str( pDict_ );
	std::cout << PyString_AsString( pStr ) << std::endl;
	Py_DECREF( pStr );
}


/**
 *	This method sets a set of properties from the input stream.
 */
void Entity::updateProperties( BinaryIStream & stream,
	bool shouldCallSetMethod )
{
	uint8 size;
	stream >> size;

	for (uint8 i = 0; i < size; i++)
	{
		uint8 index;
		stream >> index;

		DataDescription * pDD =
			this->type().description().clientServerProperty( index );

		MF_ASSERT( pDD && pDD->isOtherClientData() );

		if (pDD != NULL)
		{
			PyObjectPtr pValue = pDD->createFromStream( stream, false );

			MF_ASSERT( pValue.hasObject() );

			this->setProperty( pDD, pValue.getObject(), shouldCallSetMethod );
		}
	}
}


/**
 *	This method sets the described property of the script.
 */
void Entity::setProperty( const DataDescription * pDataDescription,
	PyObject * pValue,
	bool shouldCallSetMethod )
{
	std::string	propName = pDataDescription->name();

	//	DEBUG_MSG( "Entity::setProperty( '%s' )\n", propName.c_str() );

	// now set the value
	PyDict_SetItemString( pDict_, const_cast<char*>( propName.c_str() ),
		pValue );

	PyObject * pStr = PyObject_Str( pValue );
	std::cout << "Setting " << propName << " to " <<
		PyString_AsString( pStr ) << std::endl;
	Py_DECREF( pStr );
}

// entity.cpp
