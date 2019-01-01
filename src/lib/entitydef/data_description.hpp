/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_DESCRIPTION_HPP
#define DATA_DESCRIPTION_HPP

#include <Python.h>

#include "data_type.hpp"
#include "meta_data_type.hpp"

#include "entitydef/entity_member_stats.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "resmgr/datasection.hpp"

#include <set>

class BinaryOStream;
class BinaryIStream;
class MD5;
class Watcher;
typedef SmartPointer<Watcher> WatcherPtr;


/**
 *	This enumeration is used for flags to indicate properties of data associated
 *	with an entity type.
 *
 *	@ingroup entity
 */
enum EntityDataFlags
{
	DATA_GHOSTED		= 0x01,		///< Synchronised to ghost entities.
	DATA_OTHER_CLIENT	= 0x02,		///< Sent to other clients.
	DATA_OWN_CLIENT		= 0x04,		///< Sent to own client.
	DATA_BASE			= 0x08,		///< Sent to the base.
	DATA_CLIENT_ONLY	= 0x10,		///< Static client-side data only.
	DATA_PERSISTENT		= 0x20,		///< Saved to the database.
	DATA_EDITOR_ONLY	= 0x40,		///< Only read and written by editor.
	DATA_ID				= 0X80		///< Is an indexed column in the database.
};

#define DATA_DISTRIBUTION_FLAGS (DATA_GHOSTED | DATA_OTHER_CLIENT | \
								DATA_OWN_CLIENT | DATA_BASE | 		\
								DATA_CLIENT_ONLY | DATA_EDITOR_ONLY)

#define DEFAULT_DATABASE_LENGTH 65535

class MetaDataType;

#ifdef EDITOR_ENABLED
class GeneralProperty;
class EditorChunkEntity;
class ChunkItem;
#endif



class DataType;
typedef SmartPointer<DataType> DataTypePtr;


/**
 *	This class is used to describe a type of data associated with an entity
 *	class.
 *
 *	@ingroup entity
 */
class DataDescription
{
public:
	DataDescription();
	DataDescription( const DataDescription& description );
	~DataDescription();

	enum PARSE_OPTIONS
	{
		PARSE_DEFAULT,			// Parses all known sections.
		PARSE_IGNORE_FLAGS = 1	// Ignores the 'Flags' section.
	};

	bool parse( DataSectionPtr pSection, const std::string & parentName,
		PARSE_OPTIONS options = PARSE_DEFAULT );

//	DataDescription & operator=( const DataDescription & description );

	bool isCorrectType( PyObject * pNewValue );

	bool addToStream( PyObject * pNewValue, BinaryOStream & stream,
		bool isPersistentOnly ) const;
	PyObjectPtr createFromStream( BinaryIStream & stream,
		bool isPersistentOnly ) const;

	bool addToSection( PyObject * pNewValue, DataSectionPtr pSection );
	PyObjectPtr createFromSection( DataSectionPtr pSection ) const;

	bool fromStreamToSection( BinaryIStream & stream, DataSectionPtr pSection,
			bool isPersistentOnly ) const;
	bool fromSectionToStream( DataSectionPtr pSection,
			BinaryOStream & stream, bool isPersistentOnly ) const;

	void addToMD5( MD5 & md5 ) const;

	/// @name Accessors
	//@{
	const std::string& name() const;
	PyObjectPtr pInitialValue() const;

	INLINE bool isGhostedData() const;
	INLINE bool isOtherClientData() const;
	INLINE bool isOwnClientData() const;
	INLINE bool isCellData() const;
	INLINE bool isBaseData() const;
	INLINE bool isClientServerData() const;
	INLINE bool isPersistent() const;
	INLINE bool isIdentifier() const;

	DataSectionPtr pDefaultSection() const;

	bool isEditorOnly() const;

	bool isOfType( EntityDataFlags flags );
	const char* getDataFlagsAsStr() const;

	int index() const;
	void index( int index );

	int localIndex() const					{ return localIndex_; }
	void localIndex( int i )				{ localIndex_ = i; }

	int eventStampIndex() const;
	void eventStampIndex( int index );

	int clientServerFullIndex() const		{ return clientServerFullIndex_; }
	void clientServerFullIndex( int i )		{ clientServerFullIndex_ = i; }

	int detailLevel() const;
	void detailLevel( int level );

	int databaseLength() const;

#ifdef EDITOR_ENABLED
	bool editable() const;
	void editable( bool v );

	void widget( DataSectionPtr pSection );
	DataSectionPtr widget();
#endif

	DataType *		dataType() { return pDataType_.get(); }
	const DataType* dataType() const { return pDataType_.get(); }
	//@}

#ifdef ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif

	EntityMemberStats & stats() const
	{ return stats_; }

private:
	std::string	name_;
	DataTypePtr	pDataType_;
	int			dataFlags_;
	PyObjectPtr	pInitialValue_;
	DataSectionPtr	pDefaultSection_;

	int			index_;
	int			localIndex_;		// Index into local prop value vector.
	int			eventStampIndex_;	// Index into time-stamp vector.
	int			clientServerFullIndex_;

	int			detailLevel_;

	int			databaseLength_;

#ifdef EDITOR_ENABLED
	bool		editable_;
	DataSectionPtr pWidgetSection_;
#endif

	mutable EntityMemberStats stats_;

	// NOTE: If adding data, check the assignment operator.
};


#ifdef CODE_INLINE
#include "data_description.ipp"
#endif

#endif // DATA_DESCRIPTION_HPP

// data_description.hpp
