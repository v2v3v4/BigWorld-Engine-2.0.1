/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"

#include "entity.hpp"

#include "py_server.hpp"

#include "common/simple_client_entity.hpp"

#include "network/msgtypes.hpp"

#include "pyscript/pyobject_base.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )

// scripting declarations
PY_BASETYPEOBJECT( Entity )

PY_BEGIN_METHODS( Entity )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Entity )
	PY_ATTRIBUTE( position )
	PY_ATTRIBUTE( cell )
	PY_ATTRIBUTE( base )
	PY_ATTRIBUTE( id )
PY_END_ATTRIBUTES()

// static variable instantiation
std::map<EntityID, Entity *> Entity::entities_;
std::map<EntityID, Entity *> Entity::cachedEntities_;


/**
 *	Constructor.
 */
Entity::Entity( EntityID id, const EntityType & type,
	const Vector3 & pos, float yaw, float pitch, float roll,
	BinaryIStream & data, bool isBasePlayer ) :
		PyInstancePlus( type.pType(), true ),
		pPyCell_(NULL),
		pPyBase_(NULL),
		id_ ( id ),
		type_( type )
{
	Vector3 orientation( yaw, pitch, roll );

	std::cout << "Position:" << pos << ". Orientation:" << orientation
		<< "." << std::endl;

	pPyCell_ = new PyServer( this, this->type().description().cell(), false );
	pPyBase_ = new PyServer( this, this->type().description().base(), true );

	if (isBasePlayer)
	{
		PyObject * pNewDict = type_.newDictionary( data,
			EntityDescription::CLIENT_DATA |
			EntityDescription::BASE_DATA |
			EntityDescription::EXACT_MATCH );
		PyObject * pCurrDict = PyObject_GetAttrString( this, "__dict__" );

		if ( !pNewDict || !pCurrDict ||
			PyDict_Update( pCurrDict, pNewDict ) < 0 )
		{
			PY_ERROR_CHECK();
		}

		Py_XDECREF( pNewDict );
		Py_XDECREF( pCurrDict );
	}
	else
	{
		// Start with all of the default values.
		PyObject * pNewDict = type_.newDictionary();
		PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );

		if ( !pNewDict || !pCurrDict ||
			PyDict_Update( pCurrDict, pNewDict ) < 0 )
		{
			PY_ERROR_CHECK();
		}

		Py_XDECREF( pNewDict );
		Py_XDECREF( pCurrDict );

		// now everything is in working order, create the script object
		this->updateProperties( data, false );
	}

	Script::call( PyObject_GetAttrString( this, "__init__" ),
		PyTuple_New(0), "Entity::Entity: ", true );
}


/**
 *	Destructor.
 */
Entity::~Entity()
{
}


/**
 *	This method is called when a message is received from the server telling us
 *	to change a property on this entity.
 */
void Entity::handlePropertyChange( int messageID, BinaryIStream & data )
{
	SimpleClientEntity::propertyEvent( this, this->type().description(),
		messageID, data, /*callSetForTopLevel:*/true );
}


/**
 *	This method is called when a message is received from the server telling us
 *	to call a method on this entity.
 */
void Entity::handleMethodCall( int messageID, BinaryIStream & data )
{
	SimpleClientEntity::methodEvent( this, this->type().description(),
		messageID, data );
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

	PyObject * pDict = PyObject_GetAttrString( this, "__dict__" );

	if ((pCellData == NULL) || (pDict == NULL))
	{
		ERROR_MSG( "Entity::readCellPlayerData: Could not create dict\n" );
		Py_XDECREF( pCellData );
		Py_XDECREF( pDict );
		return;
	}

	PyDict_Update( pDict, pCellData );
	Py_DECREF( pCellData );

	std::cout << "Entity::readCellPlayerData:" << std::endl;
	PyObject * pStr = PyObject_Str( pDict );
	std::cout << PyString_AsString( pStr ) << std::endl;
	Py_DECREF( pStr );

	Py_DECREF( pDict );
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

			MF_ASSERT( pValue );

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

	if (PyObject_SetAttrString( this,
		const_cast< char * >( propName.c_str() ), pValue ) == -1)
	{
		ERROR_MSG( "Entity::setProperty: Unable to set %s\n",
			propName.c_str() );
	}

	PyObject * pStr = PyObject_Str( pValue );
	std::cout << "Setting " << propName << " to " <<
		PyString_AsString( pStr ) << std::endl;
	Py_DECREF( pStr );
}


/**
 *	This method is called when script wants to set a property of this entity.
 *
 *	@return 0 on success, -1 on failure.
 */
int Entity::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return PyInstancePlus::pySetAttribute(attr,value);
}


/**
 *	This method is called when script wants to get a property of this entity.
 *
 *	@return The requested object on success, otherwise NULL.
 */
PyObject * Entity::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return PyInstancePlus::pyGetAttribute(attr);
}

// entity.cpp
