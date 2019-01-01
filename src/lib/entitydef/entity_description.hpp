/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_DESCRIPTION_HPP
#define ENTITY_DESCRIPTION_HPP

#include "Python.h"	// Included in data_description.hpp and
					// method_description.hpp but should come before system
					// includes

#include "base_user_data_object_description.hpp"
#include "data_description.hpp"
#include "entity_method_descriptions.hpp"
#include "method_description.hpp"
#include "volatile_info.hpp"

#include "network/basictypes.hpp"
#include "network/compression_type.hpp"
#include "resmgr/datasection.hpp"

#include <vector>

class MD5;

#ifdef MF_SERVER
#include "data_lod_level.hpp"
#endif


class AddToStreamVisitor;


/**
 *	This interface is used by EntityDescription::visit. Derive from this
 *	interface if you want to visit a subset of an EntityDescription's
 *	DataDescriptions.
 */
class IDataDescriptionVisitor
{
public:
	virtual ~IDataDescriptionVisitor() {};

	/**
	 *	This function is called to visit a DataDescription.
	 *
	 *	@param	propDesc	Info about the property.
	 *	@return	Return true when successful.
	 */
	virtual bool visit( const DataDescription& propDesc ) = 0;
};


/**
 *	This class is used to describe a type of entity. It describes all properties
 *	and methods of an entity type, as well as other information related to
 *	object instantiation, level-of-detail etc. It is normally created on startup
 *	when the entities.xml file is parsed.
 *
 * 	@ingroup entity
 */
class EntityDescription: public BaseUserDataObjectDescription

{
public:
	EntityDescription();
	~EntityDescription();

	bool	parse( const std::string & name, bool isFinal = true );
	void	supersede( MethodDescription::Component component );

	enum DataDomain
	{
		BASE_DATA   = 0x1,
		CLIENT_DATA = 0x2,
		CELL_DATA   = 0x4,
		EXACT_MATCH = 0x8,
		ONLY_OTHER_CLIENT_DATA = 0x10,
		ONLY_PERSISTENT_DATA = 0x20
	};

	bool	addSectionToStream( DataSectionPtr pSection,
				BinaryOStream & stream,
				int dataDomains ) const;

	bool	addSectionToDictionary( DataSectionPtr pSection,
				PyObject * pDict,
				int dataDomains ) const;

	bool	addDictionaryToStream( PyObject * pDict,
				BinaryOStream & stream,
				int dataDomains ) const;

	bool	addAttributesToStream( PyObject * pDict,
				BinaryOStream & stream,
				int dataDomains,
				int32 * pDataSizes = NULL,
		   		int numDataSizes = 0 ) const;

	bool	readStreamToDict( BinaryIStream & stream,
				int dataDomains,
				PyObject * pDest ) const;

	bool	readStreamToSection( BinaryIStream & stream,
				int dataDomains,
				DataSectionPtr pSection ) const;

	bool	visit( int dataDomains, IDataDescriptionVisitor & visitor ) const;

	EntityTypeID			index() const;
	void					index( EntityTypeID index );

	EntityTypeID			clientIndex() const;
	void					clientIndex( EntityTypeID index );
	const std::string&		clientName() const;
	void					setParent( const EntityDescription & parent );

	bool hasCellScript() const		{ return hasCellScript_; }
	bool hasBaseScript() const		{ return hasBaseScript_; }
	bool hasClientScript() const	{ return hasClientScript_; }
	// note: the client script is found under 'clientName' not 'name'
	bool isClientOnlyType() const
	{ return !hasCellScript_ && !hasBaseScript_; }

	bool isClientType() const		{ return name_ == clientName_; }

	const VolatileInfo &	volatileInfo() const;

	// Compression used by some large messages associated with this entity type
	// sent over the internal network.
	BWCompressionType		internalNetworkCompressionType() const
								{ return internalNetworkCompressionType_; }

	// Compression used by some large messages associated with this entity type
	// sent to the client.
	BWCompressionType		externalNetworkCompressionType() const
								{ return externalNetworkCompressionType_; }

	unsigned int			clientServerPropertyCount() const;
	DataDescription*		clientServerProperty( unsigned int n ) const;

	unsigned int			clientMethodCount() const;
	MethodDescription*		clientMethod( uint8 n, BinaryIStream & data ) const;

	unsigned int			exposedBaseMethodCount() const
								{ return this->base().exposedSize(); }
	unsigned int			exposedCellMethodCount() const
								{ return this->cell().exposedSize(); }

	INLINE unsigned int		numEventStampedProperties() const;

	MethodDescription*		findClientMethod( const std::string& name ) const;

	bool isTempProperty( const std::string & name ) const;

#ifdef MF_SERVER
	const DataLoDLevels &	lodLevels() const { return lodLevels_; }
#endif

	const DataDescription *	pIdentifier() const;

	const EntityMethodDescriptions &		cell() const	{ return cell_; }
	const EntityMethodDescriptions &		base() const	{ return base_; }
	const EntityMethodDescriptions &		client() const	{ return client_; }

	void					addToMD5( MD5 & md5 ) const;
	void					addPersistentPropertiesToMD5( MD5 & md5 ) const;

	// ---- Error checking ----
	bool checkMethods( const MethodDescriptionList & methods,
		PyObject * pClass, bool warnOnMissing = true ) const;

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif

protected:
	bool				parseProperties( DataSectionPtr pProperties );
	bool				parseInterface( DataSectionPtr pSection,
								const char * interfaceName );

	const std::string	getDefsDir() const;
	const std::string	getClientDir() const;
	const std::string	getCellDir() const;
	const std::string	getBaseDir() const;

private:
//	EntityDescription( const EntityDescription & );
//	EntityDescription &		operator=( const EntityDescription & );

	bool				parseClientMethods( DataSectionPtr pMethods,
							const char * interfaceName );
	bool				parseCellMethods( DataSectionPtr pMethods,
							const char * interfaceName );
	bool				parseBaseMethods( DataSectionPtr pMethods,
							const char * interfaceName );
	bool				parseTempProperties( DataSectionPtr pTempProps,
							const char * interfaceName );

//	bool				parseMethods( DataSectionPtr parseMethods,
//											bool isForServer );
	bool	addToStream( const AddToStreamVisitor & visitor,
				BinaryOStream & stream,
				int dataTypes,
				int32 * pDataSizes = NULL,
		   		int numDataSizes = 0 ) const;

	static bool shouldConsiderData( int pass, const DataDescription * pDD,
		int dataDomains );
	static bool shouldSkipPass( int pass, int dataDomains );

	typedef std::vector< unsigned int >			PropertyIndices;

	EntityTypeID		index_;
	EntityTypeID		clientIndex_;
	std::string			clientName_;
	bool				hasCellScript_;
	bool				hasBaseScript_;
	bool				hasClientScript_;
	VolatileInfo 		volatileInfo_;

	BWCompressionType	internalNetworkCompressionType_;
	BWCompressionType	externalNetworkCompressionType_;

	/// Stores indices of properties sent between the client and the server in
	/// order of their client/server index.
	PropertyIndices		clientServerProperties_;

	// TODO:PM We should probably combine the property and method maps for
	// efficiency. Only one lookup instead of two.

	/// Stores all methods associated with the cell instances of this entity.
	EntityMethodDescriptions	cell_;

	/// Stores all methods associated with the base instances of this entity.
	EntityMethodDescriptions	base_;

	/// Stores all methods associated with the client instances of this entity.
	EntityMethodDescriptions	client_;

	/// Stores the number of properties that may be time-stamped with the last
	/// time that they changed.
	unsigned int		numEventStampedProperties_;
#ifdef MF_SERVER
	DataLoDLevels		lodLevels_;
#endif

	std::set< std::string >	tempProperties_;

#ifdef EDITOR_ENABLED
	std::string			editorModel_;
#endif

};

#ifdef CODE_INLINE
#include "entity_description.ipp"
#endif

#endif // ENTITY_DESCRIPTION_HPP
