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

#include "data_type.hpp"

#include "constants.hpp"
#include "meta_data_type.hpp"

#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )


// -----------------------------------------------------------------------------
// Section: DataType static methods
// -----------------------------------------------------------------------------

DataType::SingletonMap * DataType::s_singletonMap_;
DataType::Aliases DataType::s_aliases_;
#ifdef EDITOR_ENABLED
DataType::AliasWidgets DataType::s_aliasWidgets_;
#endif // EDITOR_ENABLED
static bool s_aliasesDone = false;

/**
 *	Default implementation for pDefaultSection().
 */
DataSectionPtr DataType::pDefaultSection() const
{
	DataSectionPtr pDefaultSection = new XMLSection( "Default" );

	if (!this->addToSection( this->pDefaultValue().getObject(), pDefaultSection ))
	{
		return NULL;
	}
	return pDefaultSection;
}

/**
 *	This factory method returns the DataType derived object associated with the
 *	input data section.
 *
 *	@param pSection	The data section describing the data type.
 *
 *	@return		A pointer to the data type. NULL if an invalid id is entered.
 *
 *	@ingroup entity
 */
DataTypePtr DataType::buildDataType( DataSectionPtr pSection )
{
	if (!pSection)
	{
		WARNING_MSG( "DataType::buildDataType: No <Type> section\n" );
		return NULL;
	}

	if (!s_aliasesDone)
	{
	   	s_aliasesDone = true;
		DataType::initAliases();
	}

	std::string typeName = pSection->asString();

	// See if it is an alias
	Aliases::iterator found = s_aliases_.find( typeName );
	if (found != s_aliases_.end())
	{
		if ( pSection->findChild( "Default" ) )
		{
			WARNING_MSG( "DataType::buildDataType: New default value for "
					"aliased data type '%s' is ignored. The default value of an"
					" aliased data type can only be overridden by the "
					"default value of an entity property.\n",
					typeName.c_str() );
		}
		return found->second;
	}

	// OK look for the MetaDataType then
	MetaDataType * pMetaType = MetaDataType::find( typeName );
	if (pMetaType == NULL)
	{
		ERROR_MSG( "DataType::buildDataType: "
			"Could not find MetaDataType '%s'\n", typeName.c_str() );
		return NULL;
	}

	// Build a DataType from the contents of the <Type> section
	DataTypePtr pDT = pMetaType->getType( pSection );

	if (!pDT)
	{
		ERROR_MSG( "DataType::buildDataType: "
			"Could not build %s from spec given\n", typeName.c_str() );
		return NULL;
	}

	pDT->setDefaultValue( pSection->findChild( "Default" ) );

	// And return either it or an existing one if this is a dupe
	return DataType::findOrAddType( pDT.getObject() );
}


/**
 *	Static method to find an equivalent data type in our set and delete
 *	the given one, or if there is no such data type then add this one.
 */
DataTypePtr DataType::findOrAddType( DataTypePtr pDT )
{
	if (s_singletonMap_ == NULL) s_singletonMap_ = new SingletonMap();

	SingletonMap::iterator found = s_singletonMap_->find(
										SingletonPtr( pDT.getObject() ) );

	if (found != s_singletonMap_->end())
		return found->pInst_;

	s_singletonMap_->insert( SingletonPtr( pDT.getObject() ) );
	return pDT;
}


/**
 *	This static method initialises the type aliases from alias.xml.
 *
 *	Note that these are full instances of DataType here, not just alternative
 *	labels for MetaDataTypes.
 */
bool DataType::initAliases()
{
	// Add internal aliases
	MetaDataType::addAlias( "FLOAT32", "FLOAT" );

	DataSectionPtr pAliases =
		BWResource::openSection( EntityDef::Constants::aliasesFile() );

	if (pAliases)
	{
		DataSection::iterator iter;
		for (iter = pAliases->begin(); iter != pAliases->end(); ++iter)
		{
			DataTypePtr pAliasedType = DataType::buildDataType( *iter );

			if (pAliasedType)
			{
				s_aliases_.insert( std::make_pair(
					(*iter)->sectionName().c_str(), pAliasedType ) );

#ifdef EDITOR_ENABLED
				s_aliasWidgets_.insert( std::make_pair(
					(*iter)->sectionName().c_str(),
					(*iter)->findChild( "Widget" ) ) );
#endif // EDITOR_ENABLED
			}
			else
			{
				ERROR_MSG( "DataType::initAliases: Failed to add %s\n",
					(*iter)->sectionName().c_str() );
			}
		}
	}
	else
	{
		WARNING_MSG( "Couldn't open aliases file '%s'\n",
			EntityDef::Constants::aliasesFile() );
	}

	return true;
}


/**
 *	This static method clears our internal statics in preparation for a full
 *	reload of all entitydef stuff.
 */
void DataType::clearStaticsForReload()
{
	// only need to clear the singleton map for UserDataTypes
	DataType::SingletonMap * oldMap = s_singletonMap_;
	s_singletonMap_ = NULL;	// set to NULL first for safety
	delete oldMap;

	s_aliases_.clear();
	s_aliasesDone = false;

	IF_NOT_MF_ASSERT_DEV( s_singletonMap_ == NULL )
	{
		MF_EXIT( "something is really wrong (NULL is no longer NULL)" );
	}
}


/**
 *	This method returns whether it is safe to ignore assigning a new value.
 */
bool DataType::canIgnoreAssignment( PyObject * pOldValue,
		PyObject * pNewValue ) const
{
	return (pOldValue == pNewValue) && this->canIgnoreSelfAssignment();
}


/**
 *	This method returns whether a value has actually changed or not.
 */
bool DataType::hasChanged( PyObject * pOldValue, PyObject * pNewValue ) const
{
	return (pOldValue == NULL) ||
				!this->canIgnoreSelfAssignment() ||
				(PyObject_Compare( pOldValue, pNewValue ) != 0);
}


std::string DataType::typeName() const
{
	return pMetaDataType_->name();
}


void DataType::callOnEach( void (DataType::*fn)() )
{
	if (s_singletonMap_)
	{
		DataType::SingletonMap::iterator iter = s_singletonMap_->begin();
		const DataType::SingletonMap::iterator endIter = s_singletonMap_->end();

		while (iter != endIter)
		{
			(iter->pInst_->*fn)();

			++iter;
		}
	}
}


// -----------------------------------------------------------------------------
// Section: DataType base class methods
// -----------------------------------------------------------------------------

/**
 *	Destructor
 */
DataType::~DataType()
{
	//SingletonPtr us( this );
	//SingletonMap::iterator found = s_singletonMap_->find( us );
	//if (found->pInst_ == this)
	//	s_singletonMap_->erase( found );

	// TODO: Make this more like code above than code below.
	// Unfortunately, code above doesn't work, because by the time we get here
	// in Windows, our virtual fn table has already been changed back to the
	// base class DataType one, and we can no longer call operator< on ourself.

	if (s_singletonMap_ != NULL)
	{
		for (SingletonMap::iterator it = s_singletonMap_->begin();
			it != s_singletonMap_->end();
			++it)
		{
			if (it->pInst_ == this)
			{
				s_singletonMap_->erase( it );
				break;
			}
		}
	}
}


/**
 *	This method reads this data type from a stream and adds it to a data
 *	section. The default implementation uses createFromStream then
 *	addToSection. DEPRECATED
 *
 *	@param stream		The stream to read from.
 *	@param pSection		The section to add to.
 *	@param isPersistentOnly Indicates whether only persistent data should be
 *		considered.
 *	@return true for success
 */
bool DataType::fromStreamToSection( BinaryIStream & stream,
	DataSectionPtr pSection, bool isPersistentOnly ) const
{
	PyObjectPtr pValue = this->createFromStream( stream, isPersistentOnly );

	if (!pValue)
	{
		return false;
	}
	else if (!this->addToSection( pValue.get(), pSection ))
	{
		return false;
	}

	return true;
}


/**
 *	This method reads this data type from a data section and adds it to
 *	a stream. The default implementation uses createFromSection then
 *	addToStream. DEPRECATED
 *
 *	@param pSection		The section to read from.
 *	@param stream		The stream to write to.
 *	@param isPersistentOnly Indicates whether only persistent data should be
 *		considered.
 *	@return true for success
 */
bool DataType::fromSectionToStream( DataSectionPtr pSection,
							BinaryOStream & stream, bool isPersistentOnly ) const
{
	if (!pSection)
		return false;

	PyObjectPtr pValue = this->createFromSection( pSection );
	if (!pValue)
		return false;

	this->addToStream( pValue.get(), stream, isPersistentOnly );

	return true;
}


/**
 *	This method first checks the type of the given object. If it fails, then
 *	it returns NULL. If it succeeds then it tells the object who its owner is,
 *	if it needs to know (i.e. if it is mutable and needs to tell its owner when
 *	it is modified). This base class implementation assumes that the object
 *	is const. Finally it returns a pointer to the same object passed in. Some
 *	implementations will need to copy the object if it already has another
 *	owner. In this case the newly-created object is returned instead.
 */
PyObjectPtr DataType::attach( PyObject * pObject,
	PropertyOwnerBase * pOwner, int ownerRef )
{
	if (this->isSameType( pObject ))
		return pObject;

	return NULL;
}


/**
 *	This method detaches the given object from its present owner.
 *	This base class implementation does nothing.
 */
void DataType::detach( PyObject * pObject )
{
}


/**
 *	This method returns the given object, which was created by us, in the
 *	form of a PropertyOwnerBase, i.e. an object which can own other objects.
 *	If the object cannot own other objects, then NULL should be returned.
 *
 *	This base class implementation always returns NULL.
 */
PropertyOwnerBase * DataType::asOwner( PyObject * pObject )
{
	return NULL;
}

// data_type.cpp
