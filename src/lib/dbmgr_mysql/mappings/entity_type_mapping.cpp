/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_type_mapping.hpp"

#include "blob_mapping.hpp"
#include "string_like_mapping.hpp"
#include "stream_to_query_helper.hpp"
#include "result_to_stream_helper.hpp"
#include "string_mapping.hpp"

#include "../comma_sep_column_names_builder.hpp"
#include "../query.hpp"
#include "../result_set.hpp"
#include "../utils.hpp"

#include "cstdmf/binary_stream.hpp"
#include "network/basictypes.hpp"

#include "entitydef/entity_description.hpp"

namespace
{
const Query getMappedTypeIDQuery(
	"SELECT typeID FROM bigworldEntityTypes WHERE bigworldID= ?" );
}


// -----------------------------------------------------------------------------
// Section: class EntityTypeMappingVisitor
// -----------------------------------------------------------------------------

/**
 *
 */
class EntityTypeMappingVisitor : public IDataDescriptionVisitor
{
public:
	EntityTypeMappingVisitor( const EntityTypeMapping & entityTypeMapping );

	virtual bool visitPropertyMapping(
			const PropertyMapping & propertyMapping ) = 0;

	virtual bool visit( const DataDescription & propDesc );

private:
	const EntityTypeMapping & entityTypeMapping_;
};


/**
 *	Constructor.
 */
EntityTypeMappingVisitor::EntityTypeMappingVisitor(
		const EntityTypeMapping & entityTypeMapping ) :
	entityTypeMapping_( entityTypeMapping )
{
}


/**
 *
 */
bool EntityTypeMappingVisitor::visit( const DataDescription & propDesc )
{
	const PropertyMapping * pPropMapping =
		entityTypeMapping_.getPropMapByName( propDesc.name() );

	MF_ASSERT( pPropMapping );

	return this->visitPropertyMapping( *pPropMapping );
}


// -----------------------------------------------------------------------------
// Section: GetFromDBVisitor
// -----------------------------------------------------------------------------

/**
 *
 */
class GetFromDBVisitor : public EntityTypeMappingVisitor
{
public:
	GetFromDBVisitor( MySql & connection,
			const EntityTypeMapping & entityTypeMapping,
			ResultStream & resultStream,
			BinaryOStream & stream,
			const std::string * pPasswordOverride ) :
		EntityTypeMappingVisitor( entityTypeMapping ),
		helper_( connection ),
		resultStream_( resultStream ),
		stream_( stream ),
		pPasswordOverride_( pPasswordOverride )
	{
		resultStream_ >> dbID_;
		helper_.parentID( dbID_ );
	}

	virtual bool visitPropertyMapping( const PropertyMapping & propertyMapping )
	{
		if ((pPasswordOverride_ != NULL) &&
			(propertyMapping.propName() == "password") &&
			(dynamic_cast< const StringLikeMapping * >( &propertyMapping ) !=
				 NULL))
		{
			std::string password;
			resultStream_ >> password;
			stream_ << *pPasswordOverride_;
		}
		else
		{
			propertyMapping.fromDatabaseToStream( helper_, resultStream_, stream_ );
		}

		return !resultStream_.error();
	}

	DatabaseID databaseID() const	{ return dbID_; }

private:
	ResultToStreamHelper helper_;
	ResultStream resultStream_;
	BinaryOStream & stream_;

	DatabaseID dbID_;

	const std::string * pPasswordOverride_;
};


// -----------------------------------------------------------------------------
// Section: class EntityTypeMapping
// -----------------------------------------------------------------------------

EntityTypeMapping::EntityTypeMapping( MySql & con,
		const EntityDescription & desc,
		PropertyMappings & properties,
		const std::string & identifierProperty ) :
	EntityMapping( desc, properties, TABLE_NAME_PREFIX ),

	getByNameQuery_(),
	getByIDQuery_(),

	getNameFromIDQuery_(
		"SELECT sm_" + identifierProperty +
			" FROM " + this->getTableName() + " WHERE id=?" ),
	getIDFromNameQuery_(
		"SELECT id FROM " + this->getTableName() +
			" WHERE sm_" + identifierProperty + "=?" ),

	idExistsQuery_( "SELECT id FROM " +
						this->getTableName() + " WHERE id=?" ),
	hasNewerQuery_( "SELECT gameTime FROM " +
			this->getTableName() + " WHERE id=? AND gameTime >= ?" ),
	insertNewQuery_(),
	deleteIDQuery_( "DELETE FROM " + this->getTableName() + " WHERE id=?" ),
	propsNameMap_(),
	identifier_( identifierProperty )
{
	const std::string &	tableName = this->getTableName();

	if (properties.size() > 0)
	{
		for (PropertyMappings::iterator prop = properties.begin();
			  prop != properties.end(); ++prop)
		{
			(*prop)->prepareSQL();
		}

		// Create prop name to PropertyMapping map
		for (PropertyMappings::const_iterator iter = properties.begin();
			iter != properties.end(); ++iter)
		{
			PropertyMappingPtr pMapping = *iter;
			propsNameMap_[ pMapping->propName() ] = pMapping.getObject();
		}

		// Cache fixed properties so we don't have to always go look for them.
		fixedCellProps_[ CELL_POSITION_INDEX ] =
										this->getPropMapByName( "position" );
		fixedCellProps_[ CELL_DIRECTION_INDEX ] =
										this->getPropMapByName( "direction" );
		fixedCellProps_[ CELL_SPACE_ID_INDEX ] =
										this->getPropMapByName( "spaceID" );

		fixedMetaProps_[ GAME_TIME_INDEX ] =
			this->getPropMapByName( GAME_TIME_COLUMN_NAME );
		fixedMetaProps_[ TIMESTAMP_INDEX ] =
			this->getPropMapByName( TIMESTAMP_COLUMN_NAME );

		insertNewQuery_.init( createInsertStatement( tableName, properties ) );
		updateQuery_.init( createUpdateStatement( tableName, properties ) );

		getByIDQuery_.init(
				createSelectStatement( tableName, properties, "id=?" ) );

		if (!identifier_.empty())
		{
			getByNameQuery_.init( createSelectStatement( tableName, properties,
				"sm_" + identifier_ + "=?" ) );
		}
	}
	else
	{
		for (int i = 0; i < NUM_FIXED_CELL_PROPS; ++i)
		{
			fixedCellProps_[i] = NULL;
		}

		for (int i = 0; i < NUM_FIXED_META_PROPS; ++i)
		{
			fixedMetaProps_[i] = NULL;
		}
	}

	ResultSet resultSet;
	getMappedTypeIDQuery.execute( con, this->getTypeID(), &resultSet );

	resultSet.getResult( mappedType_);
}


/**
 *	This method checks that the entity with the given DBID exists in the
 *	database.
 *
 *	@return	True if it exists.
 */
bool EntityTypeMapping::checkExists( MySql & connection, DatabaseID dbID ) const
{
	ResultSet resultSet;
	idExistsQuery_.execute( connection, dbID, &resultSet );

	return resultSet.numRows() > 0;
}


/**
 *	This method checks whether the entity with the given id has been saved after
 *	the given time.
 */
bool EntityTypeMapping::hasNewerRecord( MySql & connection,
		DatabaseID dbID, GameTime time ) const
{
	ResultSet resultSet;
	hasNewerQuery_.execute( connection, dbID, time, &resultSet );

	return resultSet.numRows() > 0;
}


/**
 *	This method returns the database ID of the entity given its name.
 *
 *	@param	connection	Connection to use when querying the database.
 *	@param	name		The name of the entity.
 *	@return	The database ID of the entity. Returns 0 if the entity
 *		doesn't exists or if the entity doesn't have a name index.
 */
DatabaseID EntityTypeMapping::getDbID( MySql & connection,
	const std::string & name ) const
{
	DatabaseID dbID = 0;

	if (this->hasIdentifier())
	{
		ResultSet resultSet;
		getIDFromNameQuery_.execute( connection, name, &resultSet );

		if (!resultSet.getResult( dbID ))
		{
			dbID = 0;
		}
	}

	return dbID;
}


/**
 *	This method returns the name of the entity given its database Id.
 *
 *	@param	connection	Connection to use when querying the database.
 *	@param	dbID		The database ID of the entity.
 *	@param	name		Returns the name of the entity here.
 *
 *	@return	True if the entity exists and have a name.
 */
bool EntityTypeMapping::getName( MySql & connection,
									DatabaseID dbID, std::string & name ) const
{
	if (!this->hasIdentifier())
	{
		return false;
	}

	ResultSet resultSet;
	getNameFromIDQuery_.execute( connection, dbID, &resultSet );

	return resultSet.getResult( name );
}




// Retrieving an entity
bool EntityTypeMapping::getStreamByID( MySql & connection,
		DatabaseID dbID, BinaryOStream & outStream,
		const std::string * pPasswordOverride ) const
{
	ResultSet resultSet;
	getByIDQuery_.execute( connection, dbID, &resultSet );

	return this->getStreamFromDBRow( connection,
								resultSet, outStream, pPasswordOverride ) != 0;
}


/**
 *	This method retrieves an entity from the database given its name.
 *
 *	@param connection	A connection to the database.
 *	@param name	The string identifier of the entity.
 *	@param outStream	The result is put into this stream.
 *	@param pPasswordOverride If not NULL, this value is used for the password
 *		field instead of what is stored in the database.
 *
 *	@return The id of the entity or 0 on error.
 */
DatabaseID EntityTypeMapping::getStreamByName( MySql & connection,
		const std::string & name, BinaryOStream & outStream,
		const std::string * pPasswordOverride ) const
{
	ResultSet resultSet;
	getByNameQuery_.execute( connection, name, &resultSet );

	return this->getStreamFromDBRow( connection,
								resultSet, outStream, pPasswordOverride );
}


/**
 *	This method converts a result row from the database to a BinaryOStream.
 *	This call may trigger sub-queries to other tables.
 */
DatabaseID EntityTypeMapping::getStreamFromDBRow( MySql & connection,
		ResultSet & resultSet, BinaryOStream & strm,
		const std::string * pPasswordOverride ) const
{
	ResultRow resultRow;

	if (!resultRow.fetchNextFrom( resultSet ))
	{
		// TODO: Raise exception?
		return 0;
	}

	ResultStream resultStream( resultRow );

	GetFromDBVisitor visitor( connection,
			*this, resultStream, strm, pPasswordOverride );

	if (!this->visit( visitor, /*shouldVisitMetaProps:*/false ))
	{
		return 0;
	}

	return visitor.databaseID();
}


/**
 * 	Deletes an entity by DBID.
 */
bool EntityTypeMapping::deleteWithID( MySql & connection, DatabaseID id ) const
{
	deleteIDQuery_.execute( connection, id, NULL );

	if (connection.affectedRows() == 0)
	{
		return false;
	}

	MF_ASSERT( connection.affectedRows() == 1 );

	// Delete any child table entries
	const PropertyMappings & properties = this->getPropertyMappings();

	PropertyMappings::const_iterator iter = properties.begin();

	while (iter != properties.end())
	{
		(*iter)->deleteChildren( connection, id );

		++iter;
	}

	return true;
}

namespace
{
const Query removeLogOnQuery(
			"DELETE FROM bigworldLogOns WHERE databaseID = ? AND typeID = ?" );
}

void EntityTypeMapping::removeLogOnRecord( MySql & conn, DatabaseID id ) const
{
	removeLogOnQuery.execute( conn, id, this->getDatabaseTypeID(), NULL );
}


// -----------------------------------------------------------------------------
// Section: class UpdateFromStreamVisitor
// -----------------------------------------------------------------------------

/**
 *
 */
class UpdateFromStreamVisitor : public EntityTypeMappingVisitor
{
public:
	UpdateFromStreamVisitor( MySql & connection,
			const EntityTypeMapping & entityTypeMapping,
			DatabaseID dbID, const Query & query, BinaryIStream & stream );

	bool execute();

	bool visitPropertyMapping( const PropertyMapping & propertyMapping );

	DatabaseID databaseID() const		{ return helper_.parentID(); }

	void pushGameTime( GameTime gameTime )
	{
		queryRunner_.pushArg( gameTime );
	}

private:
	StreamToQueryHelper helper_;
	QueryRunner queryRunner_;
	BinaryIStream & stream_;
	DatabaseID databaseID_;
};


/**
 *	Constructor
 */
UpdateFromStreamVisitor::UpdateFromStreamVisitor( MySql & connection,
			const EntityTypeMapping & entityTypeMapping,
			DatabaseID dbID, const Query & query, BinaryIStream & stream ) :
	EntityTypeMappingVisitor( entityTypeMapping ),
	helper_( connection, dbID ),
	queryRunner_( connection, query ),
	stream_( stream ),
	databaseID_( dbID )
{
}


bool UpdateFromStreamVisitor::execute()
{
	if (databaseID_)
	{
		queryRunner_.pushArg( databaseID_ );
	}

	if (stream_.error())
	{
		// TODO: Should this be an exception?
		return false;
	}

	// Note: Can raise exception
	queryRunner_.execute( NULL );

	if (helper_.parentID() == 0)
	{
		helper_.parentID( helper_.connection().insertID() );
	}
	else
	{
		MF_ASSERT( !helper_.hasBufferedQueries() );
	}

	helper_.executeBufferedQueries( helper_.parentID() );

	return true;
}


/**
 *
 */
bool UpdateFromStreamVisitor::visitPropertyMapping(
		const PropertyMapping & propertyMapping )
{
	propertyMapping.fromStreamToDatabase( helper_, stream_, queryRunner_ );

	bool isOkay = !stream_.error();
	if (!isOkay)
	{
		ERROR_MSG( "UpdateFromStreamVisitor::visitPropertyMapping: "
				"Error encountered while de-streaming property '%s'",
					propertyMapping.propName().c_str() );
	}

	return isOkay;
}


/**
 *
 */
bool EntityTypeMapping::visit( EntityTypeMappingVisitor & visitor,
	   bool shouldVisitMetaProps ) const
{
	const EntityDescription & entityDescription = this->getEntityDescription();

	bool isOkay = entityDescription.visit( 
		EntityDescription::BASE_DATA |
				EntityDescription::CELL_DATA |
				EntityDescription::ONLY_PERSISTENT_DATA,
			visitor );

	if (entityDescription.hasCellScript())
	{
		for (int i = 0; (i < NUM_FIXED_CELL_PROPS) && isOkay; ++i)
		{
			isOkay &= visitor.visitPropertyMapping( *fixedCellProps_[i] );
		}
	}

	if (shouldVisitMetaProps)
	{
		for (int i = 0; i < NUM_FIXED_META_PROPS && isOkay; ++i )
		{
			isOkay &= visitor.visitPropertyMapping( *fixedMetaProps_[i] );
		}
	}

	return isOkay;
}


/**
 *	This method updates an existing entity's properties in the database.
 *
 *	@param	connection	Connection to use when updating the database.
 *
 *	@return	Returns true if the entity was updated. False if the entity
 *		doesn't exist.
 */
bool EntityTypeMapping::update( MySql & connection, DatabaseID dbID,
	   BinaryIStream & strm, GameTime * pGameTime ) const
{
	UpdateFromStreamVisitor visitor( connection,
			*this, dbID, updateQuery_, strm );

	const bool isGameTimeOnStream = (pGameTime == NULL);

	this->visit( visitor, /*shouldVisitMetaProps:*/isGameTimeOnStream );

	if (!isGameTimeOnStream)
	{
		visitor.pushGameTime( *pGameTime );
	}

	return visitor.execute();
}


/**
 *	This method inserts a new entity into the database.
 *
 *	@param	connection	Connection to use when updating the database.
 *
 *	@return	The database ID of the newly inserted entity.
 */
DatabaseID EntityTypeMapping::insertNew( MySql & connection,
		BinaryIStream & strm ) const
{
	UpdateFromStreamVisitor visitor( connection,
			*this, 0, insertNewQuery_, strm );

	this->visit( visitor, /*shouldVisitMetaProps:*/true );

	return visitor.execute() ? visitor.databaseID() : 0;
}


namespace
{
const Query addLogOnQuery(
	"INSERT INTO bigworldLogOns "
			"(databaseID, typeID, objectID, ip, port, salt) "
		"VALUES (?,?,?,?,?,?) "
		"ON DUPLICATE KEY "
		"UPDATE "
			"objectID = VALUES(objectID), "
			"ip = VALUES(ip), "
			"port = VALUES(port), "
			"salt = VALUES(salt)" );

const Query updateLogOnAutoLoadQuery(
	"UPDATE bigworldLogOns SET shouldAutoLoad = ? "
		"WHERE databaseID = ? AND typeID = ?" );

const Query getLogOnQuery(
	"SELECT objectID, ip, port, salt FROM bigworldLogOns "
				"WHERE databaseID = ? and typeID = ?" );
}


/**
 *
 */
void EntityTypeMapping::addLogOnRecord( MySql & connection,
			DatabaseID dbID, const EntityMailBoxRef & mailbox ) const
{
	addLogOnQuery.execute( connection,
			dbID, this->getDatabaseTypeID(),
			mailbox.id,
			mailbox.addr.ip, mailbox.addr.port, mailbox.addr.salt,
			NULL );
}


/**
 *	Update the auto-load status of an entity's log on mapping.
 *
 *	@param connection		The database connection.
 *	@param dbID				The entity's database ID.
 *	@param shouldAutoLoad	The value of the auto-load status.
 */
void EntityTypeMapping::updateAutoLoad( MySql & connection,
			DatabaseID dbID, bool shouldAutoLoad ) const
{
	updateLogOnAutoLoadQuery.execute( connection, 
		shouldAutoLoad, dbID, this->getDatabaseTypeID(), NULL );
}


/**
 *
 */
bool EntityTypeMapping::getLogOnRecord( MySql & connection,
		DatabaseID dbID, EntityMailBoxRef & ref ) const
{
	ResultSet resultSet;
	getLogOnQuery.execute( connection,
			dbID, this->getDatabaseTypeID(), &resultSet );

	return resultSet.getResult( ref.id,
				ref.addr.ip, ref.addr.port, ref.addr.salt );
}

// entity_type_mapping.cpp
