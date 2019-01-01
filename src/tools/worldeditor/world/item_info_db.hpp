/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_INFO_DB_HPP
#define ITEM_INFO_DB_HPP


#include "cstdmf/smartpointer.hpp"
#include "cstdmf/singleton.hpp"
#include "gizmo/general_editor.hpp"
#include "gizmo/value_type.hpp"


class ChunkItem;


/**
 *	This class keeps very lean info about chunk items without dealing with them
 *	directly.
 */
class ItemInfoDB : public Singleton< ItemInfoDB >
{
public:
	// Forward Declarations for Item class
	class Item;
	typedef SmartPointer< Item > ItemPtr;


	/**
	 *	This is the base class that implements the actual comparison to be used
	 *	in the Comparer class.
	 */
	class ComparerHelper : public ReferenceCount
	{
	public:
		virtual ~ComparerHelper() {}

		virtual int compare( const ItemPtr & a, const ItemPtr & b )
																	const = 0;
	};
	typedef SmartPointer< ComparerHelper > ComparerHelperPtr;


	/**
	 *	This is an interface to a STL-friendly item property compare class.
	 */
	class Comparer : public ReferenceCount
	{
	public:
		Comparer( bool ascending, ComparerHelperPtr pHelper ) :
			ascending_( ascending ),
			pHelper_( pHelper )
		{}
		
		bool operator() ( const ItemPtr & a, const ItemPtr & b ) const
		{
			int cmp = (ascending_ ? pHelper_->compare( a, b ) :
									pHelper_->compare( b, a ) );
			if (cmp == 0)
			{
				// To keep the list stable, sort by chunkItem pointer if
				// everything else fails.
				cmp = (ascending_ ?
							int( a->chunkItem() ) - int( b->chunkItem() ) :
							int( b->chunkItem() ) - int( a->chunkItem() ) );
			}
			return cmp < 0;
		}

	protected:
		bool ascending_;
		ComparerHelperPtr pHelper_;
	};
	typedef SmartPointer< Comparer > ComparerPtr;


	/**
	 *	This class helps in making decisions about how to handle a chunk item's
	 *	property. For example, the user of the ItemInfoDB might want to
	 *	handle bools in a different way than the rest (display a check box
	 *	instead of a 'True'/'False' string).
	 */
	class Type
	{
	public:

		// IDs for built-in types.
		enum BuiltinTypeId
		{
			TYPEID_ASSETNAME,
			TYPEID_CHUNKID,
			TYPEID_NUMTRIS,
			TYPEID_NUMPRIMS,
			TYPEID_ASSETTYPE,
			TYPEID_FILEPATH,
			TYPEID_HIDDEN,
			TYPEID_FROZEN
		};

		Type() {}
		Type( const std::string & name, const ValueType & valueType );

		bool operator==( const Type & other ) const
		{
			return this->name_ == other.name_ &&
										this->valueType_ == other.valueType_;
		}

		bool operator!=( const Type & other ) const
		{
			return !this->operator==( other );
		}

		bool operator<( const Type & other ) const
		{
			return this->name_ < other.name_;
		}

		const std::string & name() const { return name_; }
		const ValueType & valueType() const { return valueType_; }

		ComparerPtr comparer( bool ascending ) const;

		static const Type & builtinType( BuiltinTypeId typeId );

		bool fromDataSection( DataSectionPtr pDS );
		bool toDataSection( DataSectionPtr pDS ) const;

	private:
		std::string name_;
		ValueType valueType_;
	};

	typedef std::pair< Type, std::string > ItemPropertiesPair;
	typedef std::vector< ItemPropertiesPair * > ItemProperties;
	typedef std::pair< Type, int > TypeUsagePair;
	typedef std::map< Type, int > TypeUsage;
	typedef std::pair< Type, std::string > TypeOwnerPair;
	typedef std::map< Type, std::string > TypeOwner;


	/**
	 *	This class keeps lean info about one chunk item.
	 */
	class Item : public SafeReferenceCount
	{
	public:
		typedef std::pair< Type, std::string * > PropertyPair;

		Item( const std::string & chunkId, ChunkItem * pChunkItem,
			int numTris, int numPrimitives, const std::string & assetName, 
			const std::string & assetType, const std::string & filePath,
			const ItemProperties & properties );
			
		Item( const Item & other );

		virtual ~Item();

		virtual const std::string & propertyAsString(
													const Type & type ) const;

		virtual const std::wstring & propertyAsStringW(
													const Type & type ) const;

		void startProperties() const
		{
			propertyCacheIter_ = propertyCache_.begin();
		}

		bool isPropertiesEnd() const
		{
			return propertyCacheIter_ == propertyCache_.end();
		}

		const PropertyPair nextProperty() const
		{
			MF_ASSERT_DEBUG( !isPropertiesEnd() );
			return *propertyCacheIter_++;
		}

		const ItemProperties & properties() const { return properties_; }

		const std::string & assetName() const { return assetName_; }
		const std::wstring & assetNameW() const { return assetNameW_; }
		const std::string & filePath() const { return filePath_; }
		const std::wstring & filePathW() const { return filePathW_; }
		const std::string & chunkId() const { return chunkId_; }
		ChunkItem * chunkItem() const { return pChunkItem_; }
		const std::string & assetType() const { return assetType_; }
		int numTris() const { return numTris_; }
		int numPrimitives() const { return numPrimitives_; }
		bool isHidden() const { return hidden_; }
		bool isFrozen() const { return frozen_; }

		const Item & operator=( const Item & other );

	protected:
		typedef std::map< Type, std::string * > PropertyCacheMap;
		typedef std::map< Type, std::wstring > PropertyCacheMapW;

		// Basic properties
		std::string			chunkId_;
		ChunkItem *			pChunkItem_;
		int					numTris_;
		int					numPrimitives_;
		std::string			numTrisStr_;
		std::string			numPrimitivesStr_;
		std::string			assetName_;
		std::wstring		assetNameW_;
		std::string			assetType_;
		std::string			filePath_;
		std::wstring		filePathW_;
		bool				hidden_;
		std::string			hiddenStr_;
		bool				frozen_;
		std::string			frozenStr_;

		// Generic Properties
		ItemProperties		properties_;

		// Other
		PropertyCacheMap	propertyCache_;
		mutable PropertyCacheMapW	propertyCacheW_;
		mutable PropertyCacheMap::const_iterator propertyCacheIter_;

		// Private methods
		void clearItemProps( ItemProperties & props );
		void updateCache();
	};


	// ItemInfoDB specific declarations.

	typedef std::list< ItemPtr > ItemList;
	typedef std::list< ChunkItem * > ChunkItemList;
	typedef std::map< ChunkItem *, ChunkItemList::iterator > PendingChunkItemItersMap;
	typedef std::map< ChunkItem *, ItemList::iterator > ChunkItemItersMap;
	typedef std::map< ChunkItem *, ItemPtr > ChunkItemsMap;


	ItemInfoDB();
	~ItemInfoDB();

	void itemModified( EditorChunkItem * pItem );
	void itemDeleted( EditorChunkItem * pItem );

	void clearChanged() { hasChanged_ = false; typesChanged_ = false; }
	bool hasChanged() const { return hasChanged_; }
	bool typesChanged() const { return typesChanged_; }
	bool needsTick() const { return numNeedsTick_ > 0; }

	void tick( int maxMillis = 15 );

	void toss( ChunkItem * pChunkItem, bool tossingIn,
					bool ignoreExistence = true, bool highPriority = false );

	void knownTypes( TypeUsage & retKnownTypes ) const;

	const std::string typeOnwer( const Type & type ) const;

	void items( ItemList & retItems ) const;

	void chunkItemsMap( ChunkItemsMap & retChunkItems ) const;

	uint32 numPending() const { return numPending_; }

	void lock() const;
	void unlock() const;
	const ItemList & itemsLocked() const;
	const ChunkItemsMap & chunkItemsMapLocked() const;

private:
	ItemList items_;
	ChunkItemItersMap chunkItemIters_;
	ChunkItemsMap chunkItems_;
	TypeOwner knownTypeOwners_;
	TypeUsage knownTypes_;

	ChunkItemList pendingChunkItems_;
	PendingChunkItemItersMap pendingChunkItemsIters_;
	mutable SimpleMutex mutex_;
	mutable int lockCount_;

	bool hasChanged_;
	bool typesChanged_;
	int numNeedsTick_;

	uint32 numPending_;

	BWFunctor1< ItemInfoDB, EditorChunkItem * > * onModifyCallback_;
	BWFunctor1< ItemInfoDB, EditorChunkItem * > * onDeleteCallback_;

	void updateItemType( ItemPtr pItem, int usageAdd );
	void updateType( const Type & type, int usageAdd,
												const std::string & owner  );
};


#endif // ITEM_INFO_DB_HPP