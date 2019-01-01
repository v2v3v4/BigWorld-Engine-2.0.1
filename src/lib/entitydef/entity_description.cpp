/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 *	This file provides the implementation of the EntityDescription class.
 *
 * 	@ingroup entity
 */

#include "pch.hpp"

#include "Python.h"

#include "constants.hpp"
#include "entity_description.hpp"

#include "cstdmf/md5.hpp"
#include "cstdmf/debug.hpp"

#include "cstdmf/binary_stream.hpp"
#include "cstdmf/watcher.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/multi_file_system.hpp"

#include "float.h"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )

#ifndef CODE_INLINE
#include "entity_description.ipp"
#endif


// -----------------------------------------------------------------------------
// Section: EntityDescription
// -----------------------------------------------------------------------------

/**
 *	The constructor.
 */
EntityDescription::EntityDescription() :
	BaseUserDataObjectDescription(),
	index_( INVALID_ENTITY_TYPE_ID ),
	clientIndex_( INVALID_ENTITY_TYPE_ID ),
	hasCellScript_( true ),
	hasBaseScript_( true ),
	hasClientScript_( true ),
	volatileInfo_(),
	internalNetworkCompressionType_( BW_COMPRESSION_DEFAULT_INTERNAL ),
	externalNetworkCompressionType_( BW_COMPRESSION_DEFAULT_EXTERNAL ),
	clientServerProperties_(),
	cell_(),
	base_(),
	client_(),
	numEventStampedProperties_( 0 )
{
}


/**
 *	Destructor.
 */
EntityDescription::~EntityDescription()
{
}


/**
 *	Static helper function to see if the named python file exists
 */
static bool pythonScriptExists( const std::string & path )
{
	MultiFileSystemPtr pFS = BWResource::instance().fileSystem();

	const char * exts[] = { ".py", ".pyc", ".pyo", ".pyd" };
	for (unsigned int i = 0; i < sizeof(exts)/sizeof(exts[0]); i++)
	{
		IFileSystem::FileInfo finfo;
		if (pFS->getFileType( path + exts[i], &finfo ) == pFS->FT_FILE &&
			finfo.size != 0)
				return true;
	}

	return false;
}

/**
 *	This method parses an entity description from a datasection.
 *
 *	@param name		The name of the Entity type.
 *	@param pSection	If not NULL, this data section is used, otherwise the
 *					data section is opened based on the name.
 *	@param isFinal	False if parsing an ancestor otherwise true.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parse( const std::string & name, bool isFinal )
{
	std::string filename = this->getDefsDir() + "/" + name + ".def";
	DataSectionPtr pSection = BWResource::openSection( filename );

	if (!pSection)
	{
		ERROR_MSG( "EntityDescription::parse: Could not open %s\n",
				filename.c_str() );
		return false;
	}

	std::string parentName = pSection->readString( "Parent" );

	if (!parentName.empty())
	{
		if (!this->parse( parentName, false ))
		{
			ERROR_MSG( "EntityDescription::parse: "
						"Could not parse %s, parent of %s\n",
					parentName.c_str(), name.c_str() );
			return false;
		}
	}

	name_ = name;

	// The ClientName tag is optional. It allows us to specify a different class
	// name for the client. If it is not present, it defaults to the same as the
	// server name.

	clientName_ = pSection->readString( "ClientName", clientName_ );

	if (clientName_.empty() && isFinal)
		clientName_ = name_;

#if defined( MF_SERVER ) || defined( EDITOR_ENABLED )

	hasCellScript_ = pythonScriptExists( this->getCellDir() + "/" + name_ );
	hasBaseScript_ = pythonScriptExists( this->getBaseDir() + "/" + name_ );

#else // neither the MF_SERVER nor the Tools

	// In the client, don't check for existence of base and cell script files,
	// just assume entities have cell and base scripts. This has to be done
	// because the final game should not have the server scripts shipped.
	hasCellScript_ = true;
	hasBaseScript_ = true;

#endif // MF_SERVER || EDITOR_ENABLED

	hasClientScript_ = pythonScriptExists( this->getClientDir() + "/" + clientName_ );

	if (!hasClientScript_ && isFinal)
		clientName_ = std::string();

	bool result = this->parseInterface( pSection, name_.c_str() );

#ifdef MF_SERVER
	if (isFinal)
	{
		// Adjust the detail levels. Because LoDs can be defined in any order
		// and derived interfaces can add more, we may not know the actual
		// level when we first parse a property. Here we convert from the index
		// to the actual level.

		int levels[ MAX_DATA_LOD_LEVELS + 1 ];

		for (int i = 0; i < MAX_DATA_LOD_LEVELS + 1; ++i)
		{
			levels[i] = lodLevels_.getLevel( i ).index();
		}

		for (unsigned int i = 0; i < this->propertyCount(); ++i)
		{
			DataDescription * pDD = this->property( i );

			switch (pDD->detailLevel())
			{
				case DataLoDLevel::NO_LEVEL:
					break;

				case DataLoDLevel::OUTER_LEVEL:
					pDD->detailLevel( lodLevels_.size() - 1 );
					break;

				default:
					pDD->detailLevel( levels[ pDD->detailLevel() ] );
					break;
			}
		}
	}

	// Check that entities without cell scripts don't have cell properties
	// This is important because DbMgr puts all persistent data (including
	// cell data) onto the stream regardless of whether they have cell script
	// or not. While other parts of the code skips the streaming of cell data
	// if it doesn't have a script.
	// But if it's client-only then we don't care. Important because client-only
	// entities have ALL_CLIENTS, OTHER_CLIENTS or OWN_CLIENT flags which
	// are also cell data.
	if (!this->hasCellScript() && !this->isClientOnlyType())
	{
		for ( Properties::const_iterator i = properties_.begin();
			i != properties_.end(); ++i )
		{
			if (i->isCellData())
			{
				ERROR_MSG( "Entity '%s' does not have a cell script but "
					"has cell property '%s'.\n", name_.c_str(),
					i->name().c_str() );
				result = false;
				break;
			}
		}
	}

	result &= initCompressionTypes( pSection->findChild( "NetworkCompression" ),
			internalNetworkCompressionType_, externalNetworkCompressionType_ );
#endif

	return result;
}


/**
 *	This method parses a data section for the properties and methods associated
 *	with this entity description.
 */
bool EntityDescription::parseInterface( DataSectionPtr pSection,
									const char * interfaceName )
{
	if (!pSection)
	{
		return false;
	}

	bool result =
#ifdef MF_SERVER
		lodLevels_.addLevels( pSection->openSection( "LoDLevels" ) ) &&
#endif

		this->BaseUserDataObjectDescription::parseInterface( pSection, 
			interfaceName ) &&

		volatileInfo_.parse( pSection->openSection( "Volatile" ) ) &&

		this->parseClientMethods( pSection->openSection( "ClientMethods" ),
			interfaceName ) &&
		this->parseCellMethods( pSection->openSection( "CellMethods" ),
			interfaceName ) &&
		this->parseBaseMethods( pSection->openSection( "BaseMethods" ),
			interfaceName ) &&
		this->parseTempProperties( pSection->openSection( "TempProperties" ),
			interfaceName );

	return result;
}


/**
 *	This method parses a data section for the properties associated with this
 *	entity description.
 *
 *	@param pProperties	The datasection containing the properties.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parseProperties( DataSectionPtr pProperties )
{
	if (!pProperties)
	{
		return true;
	}

	for (DataSectionIterator iter = pProperties->begin();
			iter != pProperties->end();
			++iter)
	{
		DataDescription dataDescription;

		if (!dataDescription.parse( *iter, name_ ))
		{
			WARNING_MSG( "Error parsing properties for %s\n",
					name_.c_str() );
			return false;
		}

#ifndef EDITOR_ENABLED
		if (dataDescription.isEditorOnly())
		{
			continue;
		}
#endif

		int index = properties_.size();
		int clientServerIndex = -1;

		PropertyMap::const_iterator propIter =
				propertyMap_.find( dataDescription.name() );

		if (propIter != propertyMap_.end())
		{
			INFO_MSG( "EntityDescription::parseProperties: "
					"property %s.%s is being overridden.\n",
				name_.c_str(), dataDescription.name().c_str() );
			index = propIter->second;
			if (dataDescription.isClientServerData())
			{
				clientServerIndex = 
					properties_[ index ].clientServerFullIndex();
			}
		}

		dataDescription.index( index );
		propertyMap_[dataDescription.name()] = dataDescription.index();

#ifdef EDITOR_ENABLED
		DataSectionPtr widget = (*iter)->openSection( "Widget" );
		if (!!widget)
		{
			dataDescription.widget( widget );
		}
#endif

		if (dataDescription.isClientServerData())
		{
			if (clientServerIndex != -1)
			{
				dataDescription.clientServerFullIndex( clientServerIndex );
				clientServerProperties_[ clientServerIndex ] =
					dataDescription.index();
			}
			else
			{
				dataDescription.clientServerFullIndex(
								clientServerProperties_.size() );
				clientServerProperties_.push_back(
								dataDescription.index() );
			}
		}

#ifdef MF_SERVER
		if (dataDescription.isOtherClientData())
		{
			int detailLevel;

			if (lodLevels_.findLevel( detailLevel,
						(*iter)->openSection( "DetailLevel" ) ))
			{
				dataDescription.detailLevel( detailLevel );
			}
			else
			{
				ERROR_MSG( "EntityDescription::parseProperties: "
										"Invalid detail level for %s.\n",
								dataDescription.name().c_str() );

				return false;
			}

			dataDescription.eventStampIndex( numEventStampedProperties_ );
			numEventStampedProperties_++;
		}
#endif

		if (index == int(properties_.size()))
		{
			properties_.push_back( dataDescription );
		}
		else
		{
			properties_[index] = dataDescription;
		}
	}

	return true;
}


/**
 *	This method parses a data section for the methods associated with this
 *	entity description that are implemented by the client.
 *
 *	@param pMethods The datasection containing the methods.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parseClientMethods( DataSectionPtr pMethods,
		const char * interfaceName )
{
	if (pMethods)
	{
		return client_.init( pMethods, MethodDescription::CLIENT,
			interfaceName );
	}
	/*
	else
	{
		// Not really the correct message since the data section may be an
		// interface. Also probably not worthwhile since it's fine for the files
		// not to have this section.
		WARNING_MSG( "%s has no ClientMethods section.\n", name_.c_str() );
	}
	*/

	return true;
}


/**
 *	This method parses a data section for the methods associated with this
 *	entity description that are implemented by the cell.
 *
 *	@param pMethods	The data section containing the methods.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parseCellMethods( DataSectionPtr pMethods,
		const char * interfaceName )
{
	if (!pMethods) return true;

	return cell_.init( pMethods, MethodDescription::CELL, interfaceName );
}


/**
 *	This method parses a data section for the methods associated with this
 *	entity description that are implemented by the base.
 *
 *	@param pMethods	The data section containing the methods.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parseBaseMethods( DataSectionPtr pMethods,
		const char * interfaceName )
{
	if (!pMethods) return true;

	return base_.init( pMethods, MethodDescription::BASE, interfaceName );
}


/**
 *	This method parses a data section for temporary properties.
 *
 *	@param pTempProps The data section containing the temporary properties.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescription::parseTempProperties( DataSectionPtr pTempProps,
		const char * interfaceName )
{
	if (!pTempProps)
	{
		return true;
	}


	int numProps = pTempProps->countChildren();

	for (int i = 0; i < numProps; ++i)
	{
		tempProperties_.insert( pTempProps->childSectionName( i ) );
	}

	return true;
}


/**
 *	This method supersedes this entity description with a newer one.
 */
void EntityDescription::supersede( MethodDescription::Component component )
{
	if (component == MethodDescription::BASE)
	{
		base_.supersede();
	}
	else if (component == MethodDescription::CELL)
	{
		cell_.supersede();
	}
	else
	{
		WARNING_MSG("only baseApp and cellApp can call supersede method. Ignored\n");
	}
}

/**
  * Tell entity description which directory it should try read
  * the .def files from
  */
const std::string EntityDescription::getDefsDir() const
{
	return EntityDef::Constants::entitiesDefsPath();
}


/**
  * Tell entity description which directory it should try read
  * the client script files from
  */
const std::string EntityDescription::getClientDir() const
{
	return EntityDef::Constants::entitiesClientPath();
}


/**
  * Tell entity description which directory it should try read
  * the cell server files from
  */
const std::string EntityDescription::getCellDir() const
{
	return EntityDef::Constants::entitiesCellPath();
}


/**
  * Tell entity description which directory it should try read
  * the base server files from
  */
const std::string EntityDescription::getBaseDir() const
{
	return EntityDef::Constants::entitiesBasePath();
}


/**
 *	This method is used for error checking. It checks whether the input class
 *	supports all of the necessary methods.
 */
bool EntityDescription::checkMethods( const MethodDescriptionList & methods,
		PyObject * pClass, bool warnOnMissing ) const
{
	MethodDescriptionList::const_iterator iter = methods.begin();
	bool isOkay = true;

	while (iter != methods.end())
	{
		PyObject * pMethod = PyObject_GetAttrString( pClass,
				const_cast< char * >( iter->name().c_str() ) );

		if (pMethod != NULL)
		{
			Py_DECREF( pMethod );
		}
		else
		{
			PyErr_Clear();

			if (warnOnMissing)
			{
				ERROR_MSG( "EntityDescription::checkMethods: "
					"class %s does not have method %s\n",
					this->name().c_str(), iter->name().c_str() );
			}

			isOkay = false;
		}

		iter++;
	}

	return isOkay;
}


/**
 *	This simple static helper function decides whether data should be sent based
 *	on the current pass.
 */
inline bool EntityDescription::shouldConsiderData( int pass,
		const DataDescription * pDD, int dataDomains )
{
	// This array is used to identify what data to add on each pass.
	const bool PASS_FILTER[4][2] =
	{
		{ true,  false },	// Base and not client
		{ true,  true },	// Base and client
		{ false, true },	// Cell and client
		{ false, false },	// Cell and not client
	};

	return (PASS_FILTER[ pass ][0] == pDD->isBaseData()) &&
			(PASS_FILTER[ pass ][1] == pDD->isClientServerData()) &&
			(pDD->isOtherClientData() ||
								!(dataDomains & ONLY_OTHER_CLIENT_DATA)) &&
			(pDD->isPersistent() || !(dataDomains & ONLY_PERSISTENT_DATA));
}


/**
 *	This simple static helper function decides whether a pass should be skipped
 *	based on the desired data domains that want to be streamed.
 *
 *	If the EXACT_MATCH flag is set, the pass is skipped unless all flags match
 *	exactly. If the EXACT_MATCH is not set, only one of the set flags passed in
 *	needs to match a flag for that pass.
 *
 *	The passes are as follows:
 *		0. Base data and not client data
 *		1. Base data and client data
 *		2. Cell data and client data
 *		3. Cell data and not client data
 */
inline bool EntityDescription::shouldSkipPass( int pass, int dataDomains )
{
	// This array is used to indicate whether or not we should skip this pass.
	// If one of the data domains is not set, we do not do that pass.
	const int PASS_JUMPER[4] =
	{
		EXACT_MATCH | BASE_DATA,				// done in pass 0
		EXACT_MATCH | BASE_DATA | CLIENT_DATA,	// done in pass 1
		EXACT_MATCH | CELL_DATA | CLIENT_DATA,	// done in pass 2
		EXACT_MATCH | CELL_DATA					// done in pass 3
	};

	// If the EXACT_MATCH flag is set, all of the flags must match.
	if (dataDomains & EXACT_MATCH)
	{
		return (dataDomains != PASS_JUMPER[ pass ]);
	}
	else
	{
		return ((dataDomains & PASS_JUMPER[ pass ]) == 0);
	}
}


static int NUM_PASSES = 4;


/**
 *	This interface is used by EntityDescription::addToStream.
 */
class AddToStreamVisitor
{
public:
	AddToStreamVisitor( const EntityDescription & entityDescription ):
			entityDescription_( entityDescription )
	{}

	virtual ~AddToStreamVisitor() {};

	virtual PyObject * getData( DataDescription & /*dataDesc*/ ) const
	{
		// Derived classes should implement either this method or addToStream.
		// If they derive from addToStream, it should no longer call getData.
		MF_EXIT( "getData not implemented or invalid call to getData" );
		return NULL;
	}

	virtual bool addToStream( DataDescription & dataDesc,
			BinaryOStream & stream, bool isPersistentOnly ) const
	{
		bool result = true;
		PyObjectPtr pValue( this->getData( dataDesc ),
							PyObjectPtr::STEAL_REFERENCE );

		if (!pValue)
		{
			// TODO: Could have a flag indicating whether there
			// should be an error here.
			pValue = dataDesc.pInitialValue();
			result = !this->isErrorOnNULL();
		}

		if (!dataDesc.isCorrectType( pValue.getObject() ))
		{
			ERROR_MSG( "EntityDescription::addToStream(%s): "
					"Data for %s (%s) is wrong type (%s)\n", 
				entityDescription_.name().c_str(),
				dataDesc.name().c_str(),
				dataDesc.dataType()->typeName().c_str(),
				pValue->ob_type->tp_name );
			pValue = dataDesc.pInitialValue();
			result = false;
		}
		else if (!dataDesc.addToStream( pValue.getObject(), stream, 
				isPersistentOnly ))
		{
			ERROR_MSG( "EntityDescription::addToStream(%s): "
					"Failed to write property %s (%s) to stream\n",
				entityDescription_.name().c_str(),
				dataDesc.name().c_str(), 
				dataDesc.dataType()->typeName().c_str() );
			result = false;
		}

		return result;
	}

	virtual bool isErrorOnNULL() const
	{
		return true;
	}

private:
	const EntityDescription & entityDescription_;
};


/**
 *	This class is used by EntityDescription::addSectionToStream.
 */
class AddToStreamSectionVisitor : public AddToStreamVisitor
{
public:
	AddToStreamSectionVisitor( const EntityDescription & entityDescription,
			DataSection * pSection ) : 
		AddToStreamVisitor( entityDescription ),
		pSection_( pSection )
	{}

protected:
	virtual bool addToStream( DataDescription & dataDesc,
			BinaryOStream & stream, bool isPersistentOnly ) const
	{
		DataSectionPtr pCurr = pSection_->openSection( dataDesc.name() );

		// pCurr == NULL is not an error.
		return dataDesc.fromSectionToStream( pCurr, stream, isPersistentOnly );
	}

private:
	DataSection * pSection_;
};



/**
 *	This class is used by EntityDescription::addDictionaryToStream.
 */
class AddToStreamDictionaryVisitor : public AddToStreamVisitor
{
public:
	AddToStreamDictionaryVisitor( const EntityDescription & entityDescription,
			PyObject * pDict ) :
		AddToStreamVisitor( entityDescription ),
		pDict_( pDict )
	{
		// Not ref counting pDict since the life of this object is short.
	}

protected:
	PyObject * getData( DataDescription & dataDesc ) const
	{
		PyObject * pObject = PyDict_GetItemString( pDict_,
				const_cast< char * >( dataDesc.name().c_str() ) );

		if (pObject != NULL)
		{
			Py_INCREF( pObject );
		}
		else
		{
			PyErr_Clear();
		}

		return pObject;
	}

	virtual bool isErrorOnNULL() const	{ return false; }

private:
	PyObject * pDict_;
};




/**
 *	This class is used by EntityDescription::addAttributesToStream.
 */
class AddToStreamAttributeVisitor : public AddToStreamVisitor
{
public:
	AddToStreamAttributeVisitor( const EntityDescription & entityDescription,
			PyObject * pObj ) : 
		AddToStreamVisitor( entityDescription ),
		pObj_( pObj )
	{
		// Not ref counting pObj since the life of this object is short.
	}

protected:
	PyObject * getData( DataDescription & dataDesc ) const
	{
		PyObject * ret = PyObject_GetAttrString( pObj_,
				const_cast< char * >( dataDesc.name().c_str() ) );
		if (ret == NULL)
		{
			PyErr_Print();
		}
		return ret;
	}

private:
	PyObject * pObj_;
};



/**
 *	This method adds information from the input section to the input stream.
 *	It may include base, client or cell data or any combination of these. This
 *	is specified by the dataDomains argument.
 *
 *	@param pSection		The data section containing the values to use. If any
 *						value is not in the data section, it gets the default.
 *	@param stream		The stream to add the data to.
 *	@param dataDomains	Indicates the type of data to be added to the stream.
 *						This is a bitwise combination of BASE_DATA, CLIENT_DATA
 *						and CELL_DATA.
 *
 *	@return True on success, otherwise false.
 */
bool EntityDescription::addSectionToStream( DataSectionPtr pSection,
		BinaryOStream & stream,
		int dataDomains ) const
{
	AddToStreamSectionVisitor visitor( *this, pSection.getObject() );

	return this->addToStream( visitor, stream, dataDomains );
}



/**
 *	This method adds information from the input section to the input PyDict.
 *	It may include base, client or cell data or any combination of these. This
 *	is specified by the dataDomains argument.
 *
 *	@param pSection		The data section containing the values to use. If any
 *						value is not in the data section, it gets the default.
 *	@param pDict		The Python dict to add the data to.
 *	@param dataDomains	Indicates the type of data to be added to the stream.
 *						This is a bitwise combination of BASE_DATA, CLIENT_DATA
 *						and CELL_DATA.
 *
 *	@return True on success, otherwise false.
 */
bool EntityDescription::addSectionToDictionary( DataSectionPtr pSection,
			PyObject * pDict,
			int dataDomains ) const
{

	class AddToDictSectionVisitor : public IDataDescriptionVisitor
	{
	public:
		AddToDictSectionVisitor( DataSectionPtr pSection, PyObject * pDict ) :
			pSection_( pSection ),
			pDict_( pDict )
		{};

		virtual bool visit( const DataDescription& propDesc )
		{
			DataSectionPtr pValueSection =
				pSection_->findChild( propDesc.name() );
			if (pValueSection)
			{
				PyObjectPtr pValue =
					propDesc.createFromSection( pValueSection );
				if (pValue)
				{
					PyDict_SetItemString( pDict_,
							propDesc.name().c_str(), pValue.get() );
				}
				else
				{
					WARNING_MSG( "EntityDescription::addSectionToDictionary: "
							"Could not add %s\n", propDesc.name().c_str() );
				}
			}

			return true;
		}

	private:
		DataSectionPtr pSection_;
		PyObject * pDict_;
	};

	AddToDictSectionVisitor visitor( pSection, pDict );
	this->visit( dataDomains, visitor );

	return true;
}



/**
 *	This method adds information from the input dictionary to the input stream.
 *	It may include base, client or cell data or any combination of these. This
 *	is specified by the dataDomains argument.
 *
 *	@param pDict		The dictionary containing the values to use. If any
 *						value is not in the dictionary, it gets the default.
 *	@param stream		The stream to add the data to.
 *	@param dataDomains	Indicates the type of data to be added to the stream.
 *						This is a bitwise combination of BASE_DATA, CLIENT_DATA
 *						and CELL_DATA.
 *
 *	@return True on success, otherwise false.
 */
bool EntityDescription::addDictionaryToStream( PyObject * pDict,
		BinaryOStream & stream,
		int dataDomains ) const
{
	if (!pDict || !PyDict_Check( pDict ))
	{
		ERROR_MSG( "EntityDescription::addDictionaryToStream: "
				"pDict is not a dictionary.\n" );
		return false;
	}

	AddToStreamDictionaryVisitor visitor( *this, pDict );

	return this->addToStream( visitor, stream, dataDomains );
}


/**
 *	This method adds information from the input entity to the input stream.
 *	It may include base, client or cell data or any combination of these. This
 *	is specified by the dataDomains argument.
 *
 *	@see EntityDescription::addAttributesToStream
 */
bool EntityDescription::addAttributesToStream( PyObject * pObject,
		BinaryOStream & stream,
		int dataDomains,
		int32 * pDataSizes,
		int numDataSizes ) const
{
	if (pObject == NULL)
	{
		ERROR_MSG( "EntityDescription::addAttributesToStream: "
				"pObject is NULL\n" );
		return false;
	}

	AddToStreamAttributeVisitor visitor( *this, pObject );

	return this->addToStream( visitor, stream, dataDomains,
			pDataSizes, numDataSizes );
}


/**
 *	This helper method is used by EntityDescription::addSectionToStream and
 *	EntityDescription::addDictionaryToStream.
 */
bool EntityDescription::addToStream( const AddToStreamVisitor & visitor,
		BinaryOStream & stream,
		int dataDomains,
		int32 * pDataSizes,
		int numDataSizes ) const
{
	int actualPass = 0;

	for (int pass = 0; pass < NUM_PASSES; pass++)
	{
		if (!EntityDescription::shouldSkipPass( pass, dataDomains ))
		{
			int initialStreamSize = stream.size();

			for (uint i = 0; i < this->propertyCount(); i++)
			{
				DataDescription * pDD = this->property( i );

				if (EntityDescription::shouldConsiderData( pass, pDD,
							dataDomains ))
				{
					// TRACE_MSG( "EntityDescription::addToStream: "
					//			"Adding property = %s\n", pDD->name().c_str() );

					if (!visitor.addToStream( *pDD, stream,
						(dataDomains & ONLY_PERSISTENT_DATA) != 0 ))
					{
						// TODO: Make sure that every caller handles the false
						// case correctly. The stream is now in an invalid state
						// so if it is still used it may cause problems remotely
						ERROR_MSG( "EntityDescription::addToStream(%s): "
								"Failed to add to stream while adding %s(%s). "
								"STREAM NOW INVALID!!\n",
							this->name().c_str(),
							pDD->name().c_str(),
							pDD->dataType()->typeName().c_str() );
						return false;
					}
				}
			}

			if ((pDataSizes != NULL) && (actualPass < numDataSizes))
			{
				pDataSizes[actualPass] = stream.size() - initialStreamSize;
			}

			actualPass++;
		}
	}

	MF_ASSERT_DEV( (numDataSizes == 0) || (numDataSizes == actualPass) ||
			(numDataSizes == actualPass - 1) );

	return true;
}


/**
 *	This method calls the visitor's visit method for each DataDescription
 *	matching dataDomains.
 *
 *	@param visitor	The object to have visit called on.
 *	@param dataDomains	Indicates the type of data to be added to the stream.
 *						This is a bitwise combination of BASE_DATA, CLIENT_DATA
 *						and CELL_DATA.
 */
bool EntityDescription::visit( int dataDomains,
		IDataDescriptionVisitor & visitor ) const
{
	for ( int pass = 0; pass < NUM_PASSES; pass++ )
	{
		if (!EntityDescription::shouldSkipPass( pass, dataDomains ))
		{
			for (uint i = 0; i < this->propertyCount(); i++)
			{
				DataDescription * pDD = this->property( i );

				if (EntityDescription::shouldConsiderData( pass, pDD,
					dataDomains ))
				{
					if (!visitor.visit( *pDD ))
						return false;
				}
			}
		}
	}

	return true;
}


/**
 *	This method removes the data on the input stream and sets values on the
 *	input dictionary.
 */
bool EntityDescription::readStreamToDict( BinaryIStream & stream,
	int dataDomains, PyObject * pDict ) const
{
	IF_NOT_MF_ASSERT_DEV( PyDict_Check( pDict ) )
	{
		return false;
	}

	class Visitor : public IDataDescriptionVisitor
	{
		BinaryIStream & stream_;
		PyObject * pDict_;
		bool onlyPersistent_;

	public:
		Visitor( BinaryIStream & stream, PyObject * pDict, bool onlyPersistent ) :
			stream_( stream ),
			pDict_( pDict ),
			onlyPersistent_( onlyPersistent ) {}

		bool visit( const DataDescription & dataDesc )
		{
			//TRACE_MSG( "EntityDescription::readStreamToDict: Reading "
			//			"property=%s\n", dataDesc.name().c_str() );

			PyObjectPtr pValue = dataDesc.createFromStream( stream_,
									onlyPersistent_ );

			MF_ASSERT_DEV( pValue );

			if (pValue)
			{
				if (PyDict_SetItemString( pDict_,
						const_cast< char * >( dataDesc.name().c_str() ),
						pValue.get() ) == -1)
				{
					ERROR_MSG( "EntityDescription::readStream: "
							"Failed to set %s\n", dataDesc.name().c_str() );
					PyErr_Print();
				}
			}
			else
			{
				ERROR_MSG( "EntityDescription::readStream: "
							"Could not create %s from stream.\n",
						dataDesc.name().c_str() );
				return false;
			}

			return !stream_.error();
		}
	};

	Visitor visitor( stream, pDict,
			((dataDomains & ONLY_PERSISTENT_DATA) != 0) );
	return this->visit( dataDomains, visitor );
}


/**
 *	This method adds the data on a stream to the input DataSection.
 */
bool EntityDescription::readStreamToSection( BinaryIStream & stream,
	int dataDomains, DataSectionPtr pSection ) const
{
	class Visitor : public IDataDescriptionVisitor
	{
		const EntityDescription & entityDescription_;
		BinaryIStream & stream_;
		DataSection * pSection_;
		bool onlyPersistent_;

	public:
		Visitor( const EntityDescription & entityDescription,
				BinaryIStream & stream,
				DataSectionPtr pSection, bool onlyPersistent ) :
			entityDescription_( entityDescription ),
			stream_( stream ),
			pSection_( pSection.getObject() ),
			onlyPersistent_( onlyPersistent ) {}

		bool visit( const DataDescription & dataDesc )
		{
			DataSectionPtr pCurr = pSection_->openSection( dataDesc.name(),
					true );

			MF_ASSERT_DEV( pCurr );

			if (pCurr)
			{
				if (!dataDesc.fromStreamToSection( stream_, pCurr, 
						onlyPersistent_ ))
				{
					ERROR_MSG( "EntityDescription::readStreamToSection(%s): "
							"Could not convert from stream to section for "
							"property %s (%s)\n",
						entityDescription_.name().c_str(),
						dataDesc.name().c_str(),
						dataDesc.dataType()->typeName().c_str() );

					return false;
				}
			}
			return true;
		}
	};

	Visitor visitor( *this, stream, pSection,
			((dataDomains & ONLY_PERSISTENT_DATA) != 0) );
	return this->visit( dataDomains, visitor );
}


/**
 *	This method returns the property that is the identifier for this Entity
 *	type.
 */
const DataDescription * EntityDescription::pIdentifier() const
{
	Properties::const_iterator iter = properties_.begin();

	while (iter != properties_.end())
	{
		if (iter->isIdentifier())
		{
			return &*iter;
		}

		++iter;
	}

	return NULL;
}


/**
 *	This method adds this object to the input MD5 object.
 */
void EntityDescription::addToMD5( MD5 & md5 ) const
{
	md5.append( name_.c_str(), name_.size() );

	Properties::const_iterator propertyIter = properties_.begin();

	while (propertyIter != properties_.end())
	{
		// Ignore the server side only ones.
		if (propertyIter->isClientServerData())
		{
			int csi = propertyIter->clientServerFullIndex();
			md5.append( &csi, sizeof( csi ) );
			propertyIter->addToMD5( md5 );
		}

		propertyIter++;
	}

	int count;
	MethodDescriptionList::const_iterator methodIter;

	count = 0;
	methodIter = client_.internalMethods().begin();

	while (methodIter != client_.internalMethods().end())
	{
		methodIter->addToMD5( md5, count );
		count++;

		methodIter++;
	}

	count = 0;
	const MethodDescriptionList & baseMethods = base_.internalMethods();
	methodIter = baseMethods.begin();

	while (methodIter != baseMethods.end())
	{
		if (methodIter->isExposed())
		{
			methodIter->addToMD5( md5, count );
			count++;
		}

		methodIter++;
	}

	count = 0;
	const MethodDescriptionList & cellMethods = cell_.internalMethods();
	methodIter = cellMethods.begin();

	while (methodIter != cellMethods.end())
	{
		if (methodIter->isExposed())
		{
			methodIter->addToMD5( md5, count );
			count++;
		}

		methodIter++;
	}
}


/**
 *	This method adds this object's persistent properties to the input MD5
 *	object.
 */
void EntityDescription::addPersistentPropertiesToMD5( MD5 & md5 ) const
{
	md5.append( name_.c_str(), name_.size() );

	Properties::const_iterator propertyIter = properties_.begin();

	while (propertyIter != properties_.end())
	{
		if (propertyIter->isPersistent())
		{
			propertyIter->addToMD5( md5 );
		}

		propertyIter++;
	}
}


// -----------------------------------------------------------------------------
// Section: Property related.
// -----------------------------------------------------------------------------

/**
 *	This method returns the number of client/server data properties of this
 *	entity class. Client/server data properties are those properties that can be
 *	sent between the server and the client.
 */
unsigned int EntityDescription::clientServerPropertyCount() const
{
	return clientServerProperties_.size();
}


/**
 *	This method returns a given data property for this entity class.
 */
DataDescription* EntityDescription::clientServerProperty( unsigned int n ) const
{
	IF_NOT_MF_ASSERT_DEV(n < clientServerProperties_.size())
	{
		MF_EXIT( "invalid property requested" );
	}

	return this->property( clientServerProperties_[n] );
}


// -----------------------------------------------------------------------------
// Section: Method related.
// -----------------------------------------------------------------------------

/**
 *	This method returns the number of client methods associated with this
 *	entity. "Client methods" refers to the methods implemented by the entity on
 *	the client that can be called by the server.
 *
 *	@return The number of client methods associated with this entity.
 */
unsigned int EntityDescription::clientMethodCount() const
{
	return client_.internalMethods().size();
}


/**
 *	This method returns the description of the client method associated with
 *	this entity that has the input index number.
 */
MethodDescription * EntityDescription::clientMethod( uint8 index, BinaryIStream & data ) const
{
	return client_.exposedMethod( index, data );
}


/**
 *	This method returns the description of the client method with the input
 *	name.
 */
MethodDescription * EntityDescription::findClientMethod( const std::string & name ) const
{
	return client_.find( name );
}


/**
 *	This method returns whether a name is a temporary property for this entity
 *	type.
 */
bool EntityDescription::isTempProperty( const std::string & name ) const
{
	return tempProperties_.find( name ) != tempProperties_.end();
}


#if ENABLE_WATCHERS
WatcherPtr EntityDescription::pWatcher()
{
	static WatcherPtr watchMe = NULL;

	if (watchMe == NULL)
	{
		watchMe = new DirectoryWatcher();
		EntityDescription *pNull = NULL;
		watchMe->addChild( "cellMethods", 
						   EntityMethodDescriptions::pWatcher(), 
						   &pNull->cell_ );
		watchMe->addChild( "baseMethods", 
						   EntityMethodDescriptions::pWatcher(), 
						   &pNull->base_ );
		watchMe->addChild( "clientMethods", 
						   EntityMethodDescriptions::pWatcher(), 
						   &pNull->client_ );
	}

	return watchMe;
}

#endif // ENABLE_WATCHERS

// entity_description.cpp
