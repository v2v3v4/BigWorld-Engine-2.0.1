/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

// identifier was truncated to '255' characters in the browser information
#pragma warning ( disable: 4786 )

#include "entity_type.hpp"

#include "connection_control.hpp"
#include "entity.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/memory_counter.hpp"

#include "entitydef/constants.hpp"

#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )

memoryCounterDefine( entityDef, Entity );

// Static variable instantiations
std::vector<EntityType*>			EntityType::s_types_;
std::map<std::string,EntityTypeID>	EntityType::s_typeNames_;

/// Constructor (private)
EntityType::EntityType( const EntityDescription & description,
			PyObject * pModule,
			PyTypeObject * pClass ):
	description_( description ),
	pModule_( pModule ),
	pClass_( pClass ),
	pPlayerClass_( NULL )
{
	BW_GUARD;	
}


/// Destructor
EntityType::~EntityType()
{
	BW_GUARD;
	Py_XDECREF( pPlayerClass_ );
	Py_XDECREF( pClass_ );
}


/*static*/ void EntityType::fini()
{
	BW_GUARD;
	for (uint i = 0; i < s_types_.size(); i++)
	{
		delete s_types_[i];
	}
	s_types_.clear();
}


/**
 * Static function to return the EntityType instance for a given type id
 */
EntityType * EntityType::find( uint type )
{
	BW_GUARD;
	if (type >= s_types_.size())
	{
		CRITICAL_MSG( "EntityType::find: "
			"No such type index %d\n", type );
		return NULL;
	}
	return s_types_[ type ];
}


/**
 * Static function to return the EntityType instance for a given type name
 */
EntityType * EntityType::find( const std::string & name )
{
	BW_GUARD;
	EntityTypeID	type = 0;

	std::map<std::string,EntityTypeID>::iterator found =
		s_typeNames_.find( name );

	return (found != s_typeNames_.end()) ? s_types_[found->second] : NULL;
}

//extern void memNow( const std::string& token );

#include "network/remote_stepper.hpp"

/**
 * Static function to initialise the entity type stuff
 * Direct copy from Entity::init in the server.
 */
bool EntityType::init()
{
	BW_GUARD;
	bool succeeded = true;

	// Initialise the entity descriptions.
	DataSectionPtr	pEntitiesList =
		BWResource::openSection( EntityDef::Constants::entitiesFile() );

	RemoteStepper::step( "Parsing entities list" );

	static EntityDescriptionMap entityDescriptionMap;	// static to keep refs
	if (pEntitiesList && entityDescriptionMap.parse( pEntitiesList ))
	{
		MD5 md5;
		entityDescriptionMap.addToMD5( md5 );
		MD5::Digest digest;
		md5.getDigest( digest );
		ConnectionControl::serverConnection()->digest( digest );
	}
	else
	{
		CRITICAL_MSG( "EntityType::init: "
				"EntityDescriptionMap::parse failed\n" );
	}

	RemoteStepper::step( "Creating types array" );

	// Create an EntityType for each EntityDescription read in.
	int numEntityTypes = entityDescriptionMap.size();
	s_types_.resize( numEntityTypes );

	if (numEntityTypes <= 0)
	{
		ERROR_MSG( "EntityType::init: There are no entity descriptions!\n" );
		return false;
	}

//	memNow( "Before ET loop" );
	int numClientTypes = 0;
	for (int i = 0; i < numEntityTypes; i++)
	{
		const EntityDescription & currDesc =
			entityDescriptionMap.entityDescription( i );

		if (!currDesc.isClientType())
		{
			continue;
		}

		RemoteStepper::step( "Importing " + currDesc.name() );

		// Load the module
		PyObject * pModule = PyImport_ImportModule(
			const_cast<char*>( currDesc.name().c_str() ) );
		if (pModule == NULL)
		{
			PyErr_PrintEx(0);
			CRITICAL_MSG( "EntityType::init: Could not load module %s.py\n",
				currDesc.name().c_str() );
			return false;
		}

		// Find the class
		PyObject * pClass = NULL;
		if (pModule != NULL)
		{
			pClass = PyObject_GetAttrString( pModule,
				const_cast<char *>( currDesc.name().c_str() ) );
		}

		if (PyErr_Occurred())
		{
			PyErr_Clear();
		}

		if (pClass != NULL)
		{
			if (!currDesc.checkMethods( currDesc.client().internalMethods(), pClass ))
			{
				ERROR_MSG( "EntityType::init: class %s has a missing method.\n",
					currDesc.name().c_str() );
				succeeded = false;
			}
			else if (PyObject_IsSubclass( pClass, (PyObject *)&Entity::s_type_ ) != 1)
			{
				PyErr_Clear();
				ERROR_MSG( "EntityType::init: "
						"class %s does not derive from BigWorld.Entity.\n",
					currDesc.name().c_str() );
				succeeded = false;
			}
			else
			{
				EntityType * pType = new EntityType( currDesc,
						pModule, (PyTypeObject *)pClass );
				s_types_[ currDesc.clientIndex() ] = pType;
				s_typeNames_[currDesc.name()] = currDesc.clientIndex();
				numClientTypes =
					std::max( numClientTypes, currDesc.clientIndex() + 1 );
			}
		}
		else
		{
			succeeded = false;
			CRITICAL_MSG( "EntityType::init: Could not find class %s\n",
					currDesc.name().c_str() );
		}

//		memNow( "After type " + currDesc.name() );
	}
	s_types_.resize( numClientTypes );

//	memNow( "After ET loop" );

	RemoteStepper::step( "EntityType::init finished" );

	return succeeded;
}


/**
 * Static function to reload the entity scripts, and update all entity class
 * instances to be the new type
 */
bool EntityType::reload()
{
	BW_GUARD;
	// Go through all the classes we have and surreptitiously
	// swap any instances of them for instances of the new class
	for (uint i = 0; i < s_types_.size(); i++)
	{
		// types could only contain NULL if init failed
		MF_ASSERT_DEV( s_types_[i] != NULL && s_types_[i]->pModule_ != NULL );

		EntityType & et = *s_types_[i];

		// reload the module
		PyObject * pReloadedModule = PyImport_ReloadModule( et.pModule_ );
		if (pReloadedModule == NULL)
		{
			if (PyErr_Occurred())
			{
				PyErr_PrintEx(0);
				PyErr_Clear();
			}
			continue;
		}
		et.pModule_ = pReloadedModule;

		if (et.pClass_ != NULL)
		{
			et.swizzleClass( et.pClass_ );

			const EntityDescription & desc = et.description_;

			if (!desc.checkMethods( desc.client().internalMethods(),
				(PyObject *)et.pClass_ ))
			{
				ERROR_MSG( "EntityType::init: class %s has a missing method.\n",
					desc.name().c_str() );
			}
		}

		if (et.pPlayerClass_ != NULL)
		{
			et.swizzleClass( et.pPlayerClass_ );
		}
	}


	PyErr_Clear();

	return true;
}


/**
 *	Private method to swizzle references to pOldClass
 */
void EntityType::swizzleClass( PyTypeObject *& pOldClass )
{
	BW_GUARD;
	// ok, look for a newer class for this instance
	const char * className = pOldClass->tp_name;
	PyObject * pObjNewClass = PyObject_GetAttrString( pModule_, className );

	if (!pObjNewClass)
	{
		WARNING_MSG( "EntityType::reload: "
			"Class '%s' has disappeared!", className );
		PyErr_Clear();
		return;
	}

	if (!PyObject_IsSubclass( pObjNewClass, (PyObject *)&Entity::s_type_ ))
	{
		WARNING_MSG( "EntityType::reload: "
			"Class '%s' does not derive from Entity!", className );
		return;
	}

	PyTypeObject * pNewClass = (PyTypeObject*)pObjNewClass;

	// now find all old instances and replace with new ones
	// (we don't replace the class in existing method objects 'tho )
	/*
	// This would only work if Py_TRACE_REFS was on
	for (PyObject * i = Py_None->_ob_next; i != Py_None; i = i->_ob_next)
	{
		if (!PyInstance_Check( i )) continue;
		PyInstanceObject * pInst = (PyInstanceObject*)i;
		if (pInst->in_class == pClass) continue;

		Py_DECREF( pInst->in_class );
		pInst->in_class = pNewClass;
		Py_INCREF( pInst->in_class );
	}
	*/

	// Ask every entity to do it. No-one but the Entity objects themselves
	//  should actually have references to the instances that we're swizzling,
	//  so this is pretty thorough.
	for (Entity::Census::iterator iter = Entity::census_.begin();
		iter != Entity::census_.end();
		iter++)
	{
		(*iter)->swizzleClass( pOldClass, pNewClass );
	}

	// use this new class from now on
	Py_DECREF( pOldClass );
	pOldClass = pNewClass;
}


/**
 *	Get all the preloads from all the classes, and return them
 *	as a list of model names.
 */
PyObject * EntityType::getPreloads( void )
{
	BW_GUARD;
	PyObject * pList = PyList_New(0);
	PyObject * pArgs = Py_BuildValue("(O)", pList );

	for (uint i=0; i < s_types_.size(); i++)
	{
		PyObject * pFn = PyObject_GetAttrString(
			s_types_[i]->pModule_, "preload" );
		if (pFn == NULL)
		{
			PyErr_Clear();
			continue;
		}

		Py_INCREF( pArgs );
		Script::call( pFn, pArgs, "EntityType::getPreloads: ", true );
	}

	Py_DECREF( pArgs );
	return pList;
}


/**
 *	This function returns a brand new instance of a dictionary associated with
 *	this entity type. If a data section is passed in, the values are read from
 *	it, otherwise entries get their default values.
 */
PyObject * EntityType::newDictionary( DataSectionPtr pSection ) const
{
	BW_GUARD;
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

		PyObjectPtr pValue = NULL;

		// Can we get it from the DataSection?
		if (pSection &&
			(pSubSection = pSection->openSection( pDD->name() )))
		{
			pValue = pDD->createFromSection( pSubSection );
		}
		else	// ok, resort to the default then
		{
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
 *	This is only used for creating the player entity, and thus only the
 *	base-and-client data or the cell-and-[own|]client data is on the stream.
 */
PyObject * EntityType::newDictionary( BinaryIStream & stream,
	StreamContents contents ) const
{
	BW_GUARD;
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

			MF_ASSERT_DEV( pDD && (pDD->isOtherClientData() ||
				(allowOwnClientData && pDD->isOwnClientData())) );

			if (pDD != NULL && (pDD->isOtherClientData() ||
				(allowOwnClientData && pDD->isOwnClientData())))
			{
				PyObjectPtr pValue = pDD->createFromStream( stream, false );
				MF_ASSERT_DEV( pValue );

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
 *	This method returns the Python player class associated with this type. If no
 *	such class exists, NULL is returned.
 */
PyTypeObject * EntityType::pPlayerClass() const
{
	BW_GUARD;
	if (pPlayerClass_ == NULL)
	{
		std::string playerClassName("Player");
		playerClassName.append( this->name() );

		PyObject * pNewClass = PyObject_GetAttrString(
			pModule_, const_cast<char*>( playerClassName.c_str() ) );

		if (!pNewClass)
		{
			return NULL;	// Cannot have this kind of player.
		}

		if (!PyObject_IsSubclass( pNewClass, (PyObject *)&Entity::s_type_ ))
		{
			Py_DECREF( pNewClass );
			ERROR_MSG( "EntityType::pPlayerClass: "
					"%s does not derive from BigWorld.Entity\n",
				playerClassName.c_str() );
			return NULL;
		}

		pPlayerClass_ = (PyTypeObject *)pNewClass;
	}

	return pPlayerClass_;
}


/**
 *	This method creates a new entity of our type
 */
Entity * EntityType::newEntity( EntityID id, Vector3 & pos,
	float * pAuxVolatile, int enterCount, BinaryIStream & data,
	StreamContents contents, Entity * pSister )
{
	BW_GUARD;
	PyObject * pObject = PyType_GenericAlloc( pClass_, 0 );

	if (!pObject)
	{
		PyErr_PrintEx(0);
		ERROR_MSG( "EntityType::newEntity: Allocation failed.\n" );
		return NULL;
	}

	PyObject * pInitDict = this->newDictionary( data, contents );

	// Make sure that every property is set. If we are at a low LoD level, not
	// all properties will be set. Set them to their default value.
	for (uint i=0; i < description_.propertyCount(); i++)
	{
		DataDescription * pDD = description_.property(i);

		if (pDD->isOtherClientData())
		{
			if (PyDict_GetItemString( pInitDict,
				const_cast<char*>( pDD->name().c_str() ) ) == NULL)
			{
				PyObjectPtr pValue = pDD->pInitialValue();

				PyDict_SetItemString( pInitDict,
					const_cast<char*>( pDD->name().c_str() ), pValue.getObject() );
			}
		}
	}

	return new (pObject) Entity(
		*this, id, pos, pAuxVolatile, enterCount, pInitDict, pSister );
}

// entity_type.cpp
