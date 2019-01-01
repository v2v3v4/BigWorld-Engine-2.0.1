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
	/*~ attribute Entity position
	 *  @components{ bots }
	 *
	 *	Current position of Entity in world
	 *
	 *	@type Read-Only Vector3
	 */
	PY_ATTRIBUTE( position )

	/*~ attribute Entity cell
	 *  @components{ bots }
	 *
	 *  This attribute provides a PyServer object. This can
	 *  be used to call functions on the base Entity.
	 *
	 *  @type Read-only PyServer
	 */
	PY_ATTRIBUTE( cell )

	/*~ attribute Entity base
	 *  @components{ bots }
	 *
	 *  This attribute provides a PyServer object. This can
	 *  be used to call functions on the base Entity.
	 *
	 *  @type Read-only PyServer
	 */
	PY_ATTRIBUTE( base )

	/*~ attribute Entity id
	 *  @components{ bots }
	 *  This is the unique identifier for the Entity. It is common across
	 *	bots, cell, and base applications.
	 *
	 *  @type Read-only Integer
	 */
	PY_ATTRIBUTE( id )

	/*~ attribute Entity clientApp
	 *  @components{ bots }
	 *
	 *  This attribute provides access to the ClientApp object the Entity was
	 *	created from.
	 *
	 *  @type Read-only ClientApp
	 */
	PY_ATTRIBUTE( clientApp )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
Entity::Entity( const ClientApp & clientApp, EntityID id, 
	const EntityType & type,
	const Vector3 & pos, float yaw, float pitch, float roll,
	BinaryIStream & data, bool isBasePlayer ) :
		PyInstancePlus( type.pType(), true ),
		clientApp_( clientApp ),
		position_( pos ),
		pPyCell_(NULL),
		pPyBase_(NULL),
		id_ ( id ),
		type_( type )
{
	Vector3 orientation( yaw, pitch, roll );

	//std::cout << "Position:" << pos << ". Orientation:" << orientation
	//	<< "." << std::endl;

	pPyCell_ = new PyServer( this, this->type().description().cell(), false );
	pPyBase_ = new PyServer( this, this->type().description().base(), true );

	if (isBasePlayer)
	{
		std::cout << "Position:" << pos << ". Orientation:" << orientation
			<< "." << std::endl;

		PyObject * pNewDict = type_.newDictionary( data, 
			EntityType::BASE_PLAYER_DATA );

		PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );

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
 *  This method destroys the entity.
 */
void Entity::destroy()
{
	// Sanity check to avoid calling this twice
	MF_ASSERT( pPyCell_ );

	if (pPyCell_)
	{
		((PyServer*)pPyCell_)->disown();
		Py_DECREF( pPyCell_ ); pPyCell_ = NULL;
	}
	if (pPyBase_)
	{
		((PyServer*)pPyBase_)->disown();
		Py_DECREF( pPyBase_ ); pPyBase_ = NULL;
	}

	// Clear out the __dict__ entries to release any objects that might be
	// referring back to this entity.
	PyObject * pDict = PyObject_GetAttrString( this, "__dict__" );
	if (pDict)
	{
		PyDict_Clear( pDict );
		Py_DECREF( pDict );
	}
	else
	{
		PyErr_Clear();
	}
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
	PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );

	if (pCurrDict != NULL)
	{
		PyObject * pCellData = type_.newDictionary( stream,
			EntityType::CELL_PLAYER_DATA );

		PyDict_Update( pCurrDict, pCellData );
		Py_DECREF( pCellData );
		Py_DECREF( pCurrDict );
	}
	else
	{
		ERROR_MSG( "Entity::readCellPlayerData: Could not get __dict__\n" );
		PyErr_PrintEx(0);
	}
}

/**
 *	This method sets a set of properties from the input stream.
 */
void Entity::updateProperties( BinaryIStream & stream,
	bool shouldCallSetMethod )
{
	// it's easy if we don't call the set method
	if (!shouldCallSetMethod)
	{
		PyObject * pMoreDict = this->type().newDictionary( stream,
			(this->id_ == clientApp_.id()) ?
				EntityType::TAGGED_CELL_PLAYER_DATA :
				EntityType::TAGGED_CELL_ENTITY_DATA );

		//TRACE_MSG( "Entity::updateProperties(under): "
		//	"%d props for id %d\n", PyDict_Size( pMoreDict ), id_ );

		PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );
		PyDict_Update( pCurrDict, pMoreDict );
		Py_DECREF( pCurrDict );
		Py_DECREF( pMoreDict );
	}
	// otherwise set them one by one
	else
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

				this->setProperty( pDD, pValue, shouldCallSetMethod );
			}
		}
	}
}


/**
 *	This method sets the described property of the script. It steals a reference
 *	to pValue.
 */
void Entity::setProperty( const DataDescription * pDataDescription,
	PyObjectPtr pValue,
	bool shouldCallSetMethod )
{
	std::string	propName = pDataDescription->name();

	PyObject * pOldValue = PyObject_GetAttrString(
		this, const_cast<char*>( propName.c_str() ) );

	// make it none (should only happen for OWN_CLIENT) properties
	// the first time they are set
	if (pOldValue == NULL)
	{
		// Actually, this can also happen when using LoDs. If an entity enters,
		// it may not have properties at higher LoDs set yet.
		//MF_ASSERT( this == Player::entity() );

		PyErr_Clear();
		pOldValue = Py_None;
		Py_INCREF( pOldValue );
	}


	// now set the value
	PyObject_SetAttrString( this, const_cast< char * >( propName.c_str() ),
		&*pValue );

	if (shouldCallSetMethod)
	{
		// then see if there's a set handler for it
		std::string methodName = "set_" + propName;
		PyObject * pMethod = PyObject_GetAttrString( this,
			const_cast< char * >( methodName.c_str() ) );

		// and call it if there is
		if (pMethod != NULL)
		{
			Script::call( pMethod, Py_BuildValue( "(O)", pOldValue ),
				"Entity::setProperty: " );
		}
		else
		{
			PyErr_Clear();
		}
	}

	Py_DECREF( pOldValue );
}


/**
 *	This method is called when script wants to set a property of this entity.
 *
 *	@return 0 on success, -1 on failure.
 */
int Entity::pySetAttribute( const char * attr, PyObject * value )
{
	PY_SETATTR_STD();
	return PyInstancePlus::pySetAttribute( attr, value );
}


/**
 *	This method is called when script wants to get a property of this entity.
 *
 *	@return The requested object on success, otherwise NULL.
 */
PyObject * Entity::pyGetAttribute( const char * attr )
{
	PY_GETATTR_STD();
	return PyInstancePlus::pyGetAttribute( attr );
}


// entity.cpp
