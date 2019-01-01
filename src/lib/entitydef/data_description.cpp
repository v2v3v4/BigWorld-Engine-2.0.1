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

#include "data_description.hpp"
#include "constants.hpp"

#include "cstdmf/base64.h"
#include "cstdmf/debug.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/memory_stream.hpp"
#include "cstdmf/watcher.hpp"

#include "network/basictypes.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"

#ifndef CODE_INLINE
#include "data_description.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )


// -----------------------------------------------------------------------------
// Section: DataDescription
// -----------------------------------------------------------------------------

// TODO: DataLoDLevel shouldn't be needed by client.
#include "data_lod_level.hpp"

/**
 *	This is the default constructor.
 */
DataDescription::DataDescription() :
	pDataType_( NULL ),
	dataFlags_( 0 ),
	pInitialValue_( NULL ),
	pDefaultSection_( NULL ),
	index_( -1 ),
	localIndex_( -1 ),
	eventStampIndex_( -1 ),
	clientServerFullIndex_( -1 ),
	detailLevel_( DataLoDLevel::NO_LEVEL ),
	databaseLength_( DEFAULT_DATABASE_LENGTH )
#ifdef EDITOR_ENABLED
	, editable_( false )
#endif
{
}


// Anonymous namespace to make static to the file.
namespace
{
struct EntityDataFlagMapping
{
	const char *	name_;
	int				flags_;
};

EntityDataFlagMapping s_entityDataFlagMappings[] =
{
	{ "CELL_PRIVATE",		0 },
	{ "CELL_PUBLIC",		DATA_GHOSTED },
	{ "OTHER_CLIENTS",		DATA_GHOSTED|DATA_OTHER_CLIENT },
	{ "OWN_CLIENT",			DATA_OWN_CLIENT },
	{ "BASE",				DATA_BASE },
	{ "BASE_AND_CLIENT",	DATA_OWN_CLIENT|DATA_BASE },
	{ "CELL_PUBLIC_AND_OWN",DATA_GHOSTED|DATA_OWN_CLIENT },
	{ "ALL_CLIENTS",		DATA_GHOSTED|DATA_OTHER_CLIENT|DATA_OWN_CLIENT },
	{ "EDITOR_ONLY",		DATA_EDITOR_ONLY },
};


/*
 *	This function converts a string to the appropriate flags. It is used when
 *	parsing the properties in the def files.
 *
 *	@param name The string to convert.
 *	@param flags This value is set to the appropriate flag values.
 *	@param parentName The name of the entity type to be used if a warning needs
 *		to be displayed.
 *	@param propName The name of the property that these flags are associated
 *		with.
 *	@return True on success, otherwise false.
 */
bool setEntityDataFlags( const std::string & name, int & flags,
		const std::string & parentName, const std::string & propName )
{
	const int size = sizeof( s_entityDataFlagMappings )/
		sizeof( s_entityDataFlagMappings[0] );

	for (int i = 0; i < size; ++i)
	{
		const EntityDataFlagMapping & mapping = s_entityDataFlagMappings[i];

		if (name == mapping.name_)
		{
			flags = mapping.flags_;
			return true;
		}
	}

	return false;
};


/*
 *	This helper function returns the string that is associated with the input
 *	DataDescription flags.
 */
const char * getEntityDataFlagStr( int flags )
{
	const int size = sizeof( s_entityDataFlagMappings )/
		sizeof( s_entityDataFlagMappings[0] );

	for (int i = 0; i < size; ++i)
	{
		const EntityDataFlagMapping & mapping = s_entityDataFlagMappings[i];

		if (flags == mapping.flags_)
			return mapping.name_;
	}

	return NULL;
}

} // anonymous namespace


/**
 *	This method returns the data flags as a string (hopefully looking like
 * 	the one specified in the defs file).
 */
const char* DataDescription::getDataFlagsAsStr() const
{
	return getEntityDataFlagStr( dataFlags_ & DATA_DISTRIBUTION_FLAGS );
}


/**
 *	This method parses a data description.
 *
 *	@param pSection		The data section to parse.
 *	@param parentName	The name of the parent or an empty string if not parent
 *						exists.
 *	@param options		Additional parsing options. At the moment, the option
 *						PARSE_IGNORE_FLAGS is used by User Data Objects to
 *						ignore the 'Flags' section instead of failing to parse.
 *	@return				true if successful
 */
bool DataDescription::parse( DataSectionPtr pSection,
		const std::string & parentName,
		PARSE_OPTIONS options /*= PARSE_DEFAULT*/  )
{
	DataSectionPtr pSubSection;

	name_ = pSection->sectionName();

	DataSectionPtr typeSection = pSection->openSection( "Type" );

	pDataType_ = DataType::buildDataType( typeSection );

	if (!pDataType_)
	{
		ERROR_MSG( "DataDescription::parse: "
					"Unable to find data type '%s' for %s.%s\n",
				pSection->readString( "Type" ).c_str(),
				parentName.c_str(),
				name_.c_str() );

		return false;
	}

#ifdef EDITOR_ENABLED
	// try to get the default widget, if it's an alias and has one that is
	widget( DataType::findAliasWidget( typeSection->asString() ) );
#endif // EDITOR_ENABLED

	if ( options & PARSE_IGNORE_FLAGS )
	{
		dataFlags_ = 0;
	}
	else
	{
		// dataFlags_ = pSection->readEnum( "Flags", "EntityDataFlags" );
		if (!setEntityDataFlags( pSection->readString( "Flags" ), dataFlags_,
					parentName, name_ ))
		{
			ERROR_MSG( "DataDescription::parse: Invalid Flags section '%s' for %s\n",
					pSection->readString( "Flags" ).c_str(), name_.c_str() );
			return false;
		}
	}

	if (pSection->readBool( "Persistent", false ))
	{
		dataFlags_ |= DATA_PERSISTENT;
	}

	if (pSection->readBool( "Identifier", false ))
	{
		dataFlags_ |= DATA_ID;
	}

	// If the data lives on the base, it should not be on the cell.
	MF_ASSERT_DEV( !this->isBaseData() ||
			(!this->isGhostedData() && !this->isOtherClientData()) );

	pSubSection = pSection->findChild( "Default" );

	// If they include a <Default> tag, use it to create the
	// default value. Otherwise, just use the default for
	// that datatype.

#ifdef EDITOR_ENABLED
	editable_ = pSection->readBool( "Editable", false );
#endif

	if (pSubSection)
	{
		if (pDataType_->isConst())
		{
			pInitialValue_ = pDataType_->createFromSection( pSubSection );
		}
		else
		{
			pDefaultSection_ = pSubSection;
		}
	}
#ifdef EDITOR_ENABLED
	// The editor always pre loads the default value, so it won't try to make
	// it in the loading thread, which causes issues
	else if (editable())
	{
		pInitialValue_ = pDataType_->pDefaultValue();
//		Py_XDECREF( pInitialValue_.getObject() );
	}

//	MF_ASSERT( pInitialValue_ );
#endif

	databaseLength_ = pSection->readInt( "DatabaseLength", databaseLength_ );
	// TODO: If CLASS data type, then DatabaseLength should be default for
	// the individual members of the class if it is not explicitly specified
	// for the member.

	return true;
}


/**
 *	The copy constructor is needed for this class so that we can handle the
 *	reference counting of pInitialValue_.
 */
DataDescription::DataDescription(const DataDescription & description) :
	pInitialValue_( NULL )
{
	(*this) = description;
}


/**
 *	The destructor for DataDescription handles the reference counting.
 */
DataDescription::~DataDescription()
{
}


/**
 *	This method returns whether or not the input value is the correct type to
 *	set as new value.
 */
bool DataDescription::isCorrectType( PyObject * pNewValue )
{
	return pDataType_ ? pDataType_->isSameType( pNewValue ) : false;
}


/**
 *	This method adds this object to the input MD5 object.
 */
void DataDescription::addToMD5( MD5 & md5 ) const
{
	md5.append( name_.c_str(), name_.size() );
	int md5DataFlags = dataFlags_ & DATA_DISTRIBUTION_FLAGS;
	md5.append( &md5DataFlags, sizeof(md5DataFlags) );
	pDataType_->addToMD5( md5 );
}


/**
 * 	This method returns the initial value of this data item, as a Python
 * 	object. It returns a smart pointer to avoid refcounting problems.
 */
PyObjectPtr DataDescription::pInitialValue() const
{
	if (pInitialValue_)
	{
		return pInitialValue_;
	}
	else if (pDefaultSection_)
	{
		PyObjectPtr pResult = pDataType_->createFromSection( pDefaultSection_ );
		if (pResult)
			return pResult;
	}

	return pDataType_->pDefaultValue();
}

/**
 * 	This method returns the default value section of this property.
 */
DataSectionPtr DataDescription::pDefaultSection() const
{
	if (pDataType_->isConst())
	{
		// We didn't store the default section. Re-construct it from the
		// initial value.
		if (pInitialValue_)
		{
			DataSectionPtr pDefaultSection = new XMLSection( "Default" );
			if (!pDataType_->addToSection( pInitialValue_.getObject(),
					pDefaultSection ))
			{
				return NULL;
			}
			return pDefaultSection;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return pDefaultSection_;
	}
}

#ifdef EDITOR_ENABLED
/**
 *  This method sets the value "Widget" data section that describes
 *  specifics about how to show the property.
 */
void DataDescription::widget( DataSectionPtr pSection )
{
	pWidgetSection_ = pSection;
}

/**
 *  This method gets the value "Widget" data section that describes
 *  specifics about how to show the property.
 */
DataSectionPtr DataDescription::widget()
{
	return pWidgetSection_;
}
#endif // EDITOR_ENABLED


#if ENABLE_WATCHERS

WatcherPtr DataDescription::pWatcher()
{
	static WatcherPtr watchMe = NULL;

	if (!watchMe)
	{
		watchMe = new DirectoryWatcher();
		DataDescription * pNull = NULL;

		watchMe->addChild( "type",
							new SmartPointerDereferenceWatcher( 
								makeWatcher( &DataType::typeName ) ),
							&pNull->pDataType_);
		watchMe->addChild( "name", 
						   makeWatcher( pNull->name_ ));
		watchMe->addChild( "localIndex", 
						   makeWatcher( pNull->localIndex_ ));
		watchMe->addChild( "clientServerFullIndex", 
						   makeWatcher( pNull->clientServerFullIndex_ ));
		watchMe->addChild( "index",
							makeWatcher( &DataDescription::index ) );
		watchMe->addChild( "stats", EntityMemberStats::pWatcher(), 
						   &pNull->stats_ );
		watchMe->addChild( "persistent",
							makeWatcher( &DataDescription::isPersistent ) );
	}

	return watchMe;
}

#endif


// The following is a hack to make sure that data_types.cpp gets linked.
extern int DATA_TYPES_TOKEN;
int * pDataTypesToken = &DATA_TYPES_TOKEN;

// data_description.cpp
