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

#include "entity_description_map.hpp"
// #include "cstdmf/md5.hpp"

#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )

/**
 *	This class is used to clear entity description state before PyFinalize is
 *	called.
 */
class EntityDefFiniTimeJob : public Script::FiniTimeJob
{
protected:
	void fini()
	{
		DataType::clearAllScript();
	}
};

static EntityDefFiniTimeJob s_entityDefFiniTimeJob;


/**
 *	Constructor.
 */
EntityDescriptionMap::EntityDescriptionMap()
{
}

/**
 *	Destructor.
 */
EntityDescriptionMap::~EntityDescriptionMap()
{
}

/**
 *	This method parses the entity description map from a datasection.
 *
 *	@param pSection	Datasection containing the entity descriptions.
 *
 *	@return true if successful, false otherwise.
 */
bool EntityDescriptionMap::parse( DataSectionPtr pSection )
{
	if (!pSection)
	{
		ERROR_MSG( "EntityDescriptionMap::parse: pSection is NULL\n" );
		return false;
	}

	bool isOkay = true;
	int size = pSection->countChildren();
	vector_.resize( size );

	EntityTypeID clientIndex = 0;

	for (int i = 0; i < size; i++)
	{
		DataSectionPtr pSubSection = pSection->openChild( i );
		EntityDescription & desc = vector_[i];

		std::string typeName = pSubSection->sectionName();

		if (!desc.parse( typeName ))
		{
			ERROR_MSG( "EntityDescriptionMap: "
				"Failed to parse def for entity type %s\n",
				typeName.c_str() );

			isOkay = false;
			continue;
		}

		desc.index( i );

#ifdef DEBUG_DEFS
		INFO_MSG( "%-14s has %3d props "
					"and %2d client, %2d base, %2d cell methods\n",
				desc.name().c_str(), desc.propertyCount(),
				desc.clientMethodCount(),
				desc.base().size(),
				desc.server().size() );
		for (int index = 0; index < desc.propertyCount(); index++)
		{
			DataDescription * pDataDesc = desc.property( index );
			INFO_MSG( "%s %s %d\n", pDataDesc->name().c_str(),
					pDataDesc->description().c_str(),
					pDataDesc->isClientServerData() );
		}
#endif // DEBUG_DEFS

		map_[ desc.name() ] = desc.index();

		if (desc.isClientType())
		{
			desc.clientIndex( clientIndex );
			++clientIndex;
		}
	}

	DescriptionVector::iterator iter = vector_.begin();

	while (iter != vector_.end())
	{
		// if name_ != clientName_
		if (iter->hasClientScript() && !iter->isClientType())
		{
			EntityTypeID id;

			if (this->nameToIndex( iter->clientName(), id ))
			{
				const EntityDescription & alias = this->entityDescription( id );

				if (alias.clientIndex() != INVALID_ENTITY_TYPE_ID)
				{
					if ((alias.clientServerPropertyCount() ==
							iter->clientServerPropertyCount()) &&
						(alias.clientMethodCount() ==
						 	iter->clientMethodCount()))
					{
						iter->clientIndex( alias.clientIndex() );
						INFO_MSG( "%s is aliased as %s\n",
								iter->name().c_str(),
								alias.name().c_str() );
					}
					else
					{
						ERROR_MSG( "EntityDescriptionMap::parse: "
									"%s has mismatched ClientName %s\n",
								iter->name().c_str(),
								iter->clientName().c_str() );
						ERROR_MSG( "EntityDescriptionMap::parse: "
								"There are %d methods and %d props instead of "
								"%d and %d\n",
							alias.clientMethodCount(),
							alias.clientServerPropertyCount(),
						 	iter->clientMethodCount(),
							iter->clientServerPropertyCount() );
						isOkay = false;
					}
				}
				else
				{
					ERROR_MSG( "EntityDescriptionMap::parse: "
								"%s has server-only ClientName %s\n",
							iter->name().c_str(),
							iter->clientName().c_str() );
					isOkay = false;
				}
			}
			else
			{
				ERROR_MSG( "EntityDescriptionMap::parse: "
							"%s has invalid ClientName %s\n",
						iter->name().c_str(),
						iter->clientName().c_str() );
				isOkay = false;
			}
		}

		++iter;
	}

	INFO_MSG( "Highest exposed counts:\n" );

	isOkay &= this->checkCount( "client top-level property",
						&EntityDescription::clientServerPropertyCount, 61, 256);
	isOkay &= this->checkCount( "client method",
						&EntityDescription::clientMethodCount, 62, 62*256 );
	isOkay &= this->checkCount( "base method",
						&EntityDescription::exposedBaseMethodCount, 62, 62*256);
	isOkay &= this->checkCount( "cell method",
						&EntityDescription::exposedCellMethodCount, 62, 62*256);
	INFO_MSG( "\n" );

	return isOkay;
}


/**
 *	This method is used to check whether we have exceeded any property or
 *	method limits.
 */
bool EntityDescriptionMap::checkCount( const char * description,
		unsigned int (EntityDescription::*fn)() const,
		int maxEfficient, int maxAllowed ) const
{
	if (!vector_.empty())
	{
		const EntityDescription * pBest = &vector_[0];
		unsigned int maxCount = 0;

		for (EntityTypeID typeID = 0; typeID < this->size(); ++typeID)
		{
			const EntityDescription & eDesc = this->entityDescription( typeID );

			if ((eDesc.*fn)() > maxCount)
			{
				pBest = &eDesc;
				maxCount = (eDesc.*fn)();
			}
		}

		if (maxCount <= (unsigned int)maxAllowed)
		{
			INFO_MSG( "\t%s: %s count = %d. Efficient to %d (limit is %d)\n",
					pBest->name().c_str(), description, maxCount,
					maxEfficient, maxAllowed );
		}
		else
		{
			ERROR_MSG( "EntityDescriptionMap::checkCount: "
					"%s exposed %s count of %d is more than allowed (%d)\n",
				pBest->name().c_str(), description, maxCount, maxAllowed );
			return false;
		}
	}

	return true;
}


/**
 *	This method returns the number of entities.
 *
 *	@return Number of entities.
 */
int EntityDescriptionMap::size() const
{
	return vector_.size();
}


/**
 *	This method returns the entity description with the given index.
 */
const EntityDescription& EntityDescriptionMap::entityDescription(
		EntityTypeID index ) const
{
	IF_NOT_MF_ASSERT_DEV( index < EntityTypeID(vector_.size()) )
	{
		MF_EXIT( "invalid entity type id index" );
	}

	return vector_[index];
}


/**
 *	This method maps an entity class name to an index.
 *
 *	@param name		Name of the entity class.
 *	@param index	The index is returned here.
 *
 *	@return true if found, false otherwise.
 */
bool EntityDescriptionMap::nameToIndex(const std::string& name,
	   	EntityTypeID& index) const
{
	DescriptionMap::const_iterator it = map_.find(name);

	if(it != map_.end())
	{
		index = it->second;
		return true;
	}

	return false;
}


/**
 *	This method adds this object to the input MD5 object.
 */
void EntityDescriptionMap::addToMD5( MD5 & md5 ) const
{
	DescriptionVector::const_iterator iter = vector_.begin();

	while (iter != vector_.end())
	{
		// Ignore the server side only ones.
		if (iter->isClientType())
		{
			iter->addToMD5( md5 );
		}

		iter++;
	}
}


/**
 *	This method adds this object's persistent properties to the input MD5
 *	object.	
 */
void EntityDescriptionMap::addPersistentPropertiesToMD5( MD5 & md5 ) const
{
	DescriptionVector::const_iterator iter = vector_.begin();
	
	// Assumes typeID is its order in the vector.
	while (iter != vector_.end())
	{
		iter->addPersistentPropertiesToMD5( md5 );
		iter++;
	}
}


/**
 *	This method clears all the entity descriptions stored in this object.
 */
void EntityDescriptionMap::clear()
{
	vector_.clear();
	map_.clear();
}

bool EntityDescriptionMap::isEntity( const std::string& name ) const
{
	return map_.find( name ) != map_.end();
}
/* entity_description_map.cpp */
