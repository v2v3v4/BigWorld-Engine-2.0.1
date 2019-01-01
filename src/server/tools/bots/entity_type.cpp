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

#include "entity_type.hpp"

#include "client_app.hpp"
#include "entity.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/md5.hpp"

#include "entitydef/constants.hpp"

#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )

// Static variable instantiations
std::vector< EntityType * > EntityType::s_types_;
EntityDescriptionMap		EntityType::entityDescriptionMap_;

/**
 *	Constructor (private)
 */
EntityType::EntityType( const EntityDescription & description,
			PyTypeObject * pType ):
	description_( description ),
	pType_( pType )
{
}

/**
 *	Destructor.
 */
EntityType::~EntityType()
{
}

/**
 *	This static method returns the EntityType instance for a given type id.
 */
EntityType * EntityType::find( uint type )
{
	if (type >= s_types_.size())
	{
		CRITICAL_MSG( "EntityType::find: "
			"No such type index %d\n", type );
		return NULL;
	}
	return s_types_[ type ];
}

/**
 *	This static method returns the EntityType instance for a given type name.
 */
EntityType * EntityType::find( const std::string & name )
{
	EntityTypeID type = 0;

	if (!entityDescriptionMap_.nameToIndex( name, type ))
	{
		CRITICAL_MSG( "EntityType::find: "
			"No entity type named %s\n", name.c_str() );
		return NULL;
	}

	return EntityType::find( type );
}


/**
 *	This static method initialises the entity type stuff.
 */
int EntityType::init( const std::string & standinEntity )
{
	// initialise BigWorld module
	PyObject * pBWModule = PyImport_AddModule( "BigWorld" );

	// add the "Entity" class to bigworld
	/*
	if ( PyObject_SetAttrString( pBWModule, "Entity",
			(PyObject *)&Entity::s_type_ ) == -1 )
	{
		return -1; // fatal error
	}
	*/
	if ( PyModule_AddObject( pBWModule, "Entity", 
			(PyObject *)&Entity::s_type_ ) == -1 )
	{
		return -1;
	}

	// load our standin entity first
	PyObject * pDefaultEntityModule = PyImport_ImportModule( 
		const_cast<char *>( standinEntity.c_str() ) );

	PyObject * pDefaultEntityClass = NULL;

	if (pDefaultEntityModule)
	{
		pDefaultEntityClass = PyObject_GetAttrString( pDefaultEntityModule,
			const_cast<char *>( standinEntity.c_str() ) );
	}

	if (pDefaultEntityClass == NULL ||
		PyObject_IsSubclass( pDefaultEntityClass, (PyObject *)&Entity::s_type_ ) != 1)
	{
		WARNING_MSG( "EntityType::init: Unable to load default standin entity type %s\n", 
			standinEntity.c_str() );
		PyErr_Print();
		pDefaultEntityClass = NULL;
	}

	// Initialise the entity descriptions.
	DataSectionPtr	pEntitiesList =
		BWResource::openSection( EntityDef::Constants::entitiesFile() );

	if (pEntitiesList && entityDescriptionMap_.parse( pEntitiesList ))
	{
		MD5 md5;
		entityDescriptionMap_.addToMD5( md5 );
		MD5::Digest digest;
		md5.getDigest( digest );
	}
	else
	{
		ERROR_MSG( "EntityType::init: "
				"EntityDescriptionMap::parse failed\n" );
		return -1;
	}

	// Create an EntityType for each EntityDescription read in.
	int numEntityTypes = entityDescriptionMap_.size();
	if (numEntityTypes <= 0)
	{
		ERROR_MSG( "EntityType::init: There are no entity descriptions!\n" );
		return -1;
	}

	s_types_.resize( numEntityTypes );

	bool containError = false;
	int numClientTypes = 0;

	for (int i = 0; i < numEntityTypes; i++)
	{
		const EntityDescription & currDesc =
			entityDescriptionMap_.entityDescription( i );

		if (!currDesc.isClientType())
		{
			// not on the client... move on
			continue;
		}

		PyObject * pClass = NULL;

		if (currDesc.name() == standinEntity)
		{
			pClass = pDefaultEntityClass;
		}
		else
		{
			PyObject * pModule = PyImport_ImportModule( 
				const_cast<char *>( currDesc.name().c_str() ) );

			if (pModule)
			{
				pClass = PyObject_GetAttrString( pModule,
					const_cast<char *>( currDesc.name().c_str() ) );
			}
		}

		if (pClass)
		{
			if (PyObject_IsSubclass( pClass, (PyObject *)&Entity::s_type_ ) != 1)
			{
				ERROR_MSG( "EntityType::init: Class %s is not derived from BigWorld.Entity\n",
					currDesc.name().c_str() );
				containError = true;
				pClass = NULL; // replace it with the default standin.
			}
			else if (!currDesc.checkMethods(
				currDesc.client().internalMethods(), pClass, false ))
			{
				WARNING_MSG( "EntityType::init: Class %s does not implement all client methods\n",
					currDesc.name().c_str() );
			}
		}
		else
		{
			ERROR_MSG( "EntityType::init: Could not find class %s\n", 
				currDesc.name().c_str() );
			containError = true;
		}

		if (pClass == NULL) // replace with a default entity type
		{
			if (pDefaultEntityClass)
			{
				INFO_MSG( "EntityType::init: use default %s type object for %s type\n",
					standinEntity.c_str(), currDesc.name().c_str() );

				pClass = pDefaultEntityClass;
			}
			else
			{
				return -1; // unrecoverable error.
			}
		}

		MF_ASSERT( PyType_Check( pClass ) );
		s_types_[currDesc.clientIndex()] =
			new EntityType( currDesc, (PyTypeObject *)pClass );

		numClientTypes =
			std::max( numClientTypes, currDesc.clientIndex() + 1 );
	}
	s_types_.resize( numClientTypes );

	if (containError)
	{
		WARNING_MSG( "Entity definition contain missing or incorrect "
			"entity script definitions. Standin entity is used in these cases.\n" );
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 *	This static method should be called on shut down to clean up the entity type
 *	resources.
 */
bool EntityType::fini()
{
	for (uint i = 0; i < s_types_.size(); i++)
	{
		if (s_types_[i])
			delete s_types_[i];
	}
	s_types_.clear();
	return true;
}


/**
 *	This function returns a brand new instance of a dictionary associated with
 *	this entity type. If a data section is passed in, the values are read from
 *	it, otherwise entries get their default values.
 */
PyObject * EntityType::newDictionary( DataSectionPtr pSection ) const
{
	PyObject * pDict = PyDict_New();

	for (uint i=0; i < description_.propertyCount(); i++)
	{
		DataDescription * pDD = description_.property(i);

		if (!pDD->isClientServerData())
		{
			// if we're not interested in it, don't set it.
			continue;
		}

		DataSectionPtr	pSubSection;

		PyObjectPtr pValue;

		// Can we get it from the DataSection?
		if (pSection &&
			(pSubSection = pSection->openSection( pDD->name() )))
		{
			// createFromSection returns a new reference.
			pValue = pDD->createFromSection( pSubSection );
			Py_XDECREF( pValue.getObject() );
		}
		else	// ok, resort to the default then
		{
			// pInitialValue returns a Smart pointer.
			pValue = pDD->pInitialValue();
		}

		PyDict_SetItemString( pDict,
			const_cast<char*>( pDD->name().c_str() ), pValue.getObject() );
	}

	return pDict;
}


/**
 *	This function returns a brand new instance of a dictionary associated with
 *	this entity type. It streams the properties from the input stream.
 *	This is only used for creating the player entity.
 */
PyObject * EntityType::newDictionary( BinaryIStream & stream,
									 StreamContents contents ) const
{
	// If there is no stream, return a default dictionary.
	if (stream.remainingLength() == 0)
		return this->newDictionary();

	PyObject * pDict = PyDict_New();

	if (contents < TAGGED_CELL_PLAYER_DATA)
	{
		const int dataDomainsToRead =
			((contents == BASE_PLAYER_DATA) ?
				EntityDescription::BASE_DATA :
				EntityDescription::CELL_DATA) |
			EntityDescription::CLIENT_DATA |
			EntityDescription::EXACT_MATCH;
		description_.readStreamToDict( stream, dataDomainsToRead, pDict );
	}
	else
	{
		bool allowOwnClientData = (contents == TAGGED_CELL_PLAYER_DATA);

		uint8 size;
		stream >> size;

		for (uint8 i = 0; i < size; i++)
		{
			uint8 index;
			stream >> index;

			DataDescription * pDD = description_.clientServerProperty( index );

			MF_ASSERT( pDD && (pDD->isOtherClientData() ||
				(allowOwnClientData && pDD->isOwnClientData())) );

			if (pDD != NULL)
			{
				PyObjectPtr pValue = pDD->createFromStream( stream, false );
				MF_ASSERT( pValue );

				if (PyDict_SetItemString( pDict,
					const_cast<char*>( pDD->name().c_str() ), &*pValue ) == -1)
				{
					ERROR_MSG( "EntityType::newDictionary: "
						"Failed to set %s\n", pDD->name().c_str() );
					PyErr_PrintEx(0);
				}
			}
		}
	}

	return pDict;
}


/**
 *	This function returns a brand new instance of an entity associated with
 *	this entity type. It streams the properties from the input stream.
 */
Entity * EntityType::newEntity( const ClientApp & clientApp, EntityID id, const Vector3 & pos,
	float yaw, float pitch, float roll,
	BinaryIStream & data, bool isBasePlayer )
{
	PyObject * pObject = PyType_GenericAlloc( this->pType_, 0 );

	if (pObject == NULL)
	{
		PyErr_Print();
		ERROR_MSG( "EntityType::newEntity: Allocation failed.\n" );
		return NULL;
	}

	return new (pObject) Entity( clientApp, id, *this, pos, yaw, pitch, roll, data, isBasePlayer );
}

// entity_type.cpp
