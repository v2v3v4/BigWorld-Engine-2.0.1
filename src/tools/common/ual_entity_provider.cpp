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
 *	This file implements all classes needed to implement the asset locator's
 *	entity providers.
 */


#include "pch.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/string_provider.hpp"
#include "ual_entity_provider.hpp"
#include "entitydef/entity_description_map.hpp"
#include "entitydef/user_data_object_description_map.hpp"
#include "entitydef/constants.hpp"
#include <string>

DECLARE_DEBUG_COMPONENT( 0 )

int UalEntityProv_token;
int UalUserDataObjectProv_token;


///////////////////////////////////////////////////////////////////////////////
// Section: UalBaseMap
///////////////////////////////////////////////////////////////////////////////
/**
 *	Helper class to keep one single map for the entity/UserDataObject providers
 */

#define ENTITY_TYPE EntityDescriptionMap
#define USER_DATA_OBJECT_TYPE UserDataObjectDescriptionMap
#define ENTITY_PROVIDER UalUserDefinedProviderMap<ENTITY_TYPE>
#define USER_DATA_OBJECT_PROVIDER UalUserDefinedProviderMap<USER_DATA_OBJECT_TYPE>

template <typename mapType>
class UalUserDefinedProviderMap 
{
public:
	static UalUserDefinedProviderMap<mapType>* instance()
	{
		static UalUserDefinedProviderMap<mapType> instance_;
		return &instance_;
	}
	static void fini()
	{
		instance()->map_.clear();
	}

	static const std::string getDefDirectory();

	static const std::string getAssetName();

	static const std::string getXMLFile();

	static const std::string getFactoryName();

	static int size()
	{
		BW_GUARD;

		return (int)instance()->providers_.size();
	}

	/**
	 *	Fills an AssetInfo structure with the entity at 'index'
	 */
	static AssetInfo assetInfo( int index )
	{
		BW_GUARD;

		if ( index < 0 || index >= (int)size() )
			return AssetInfo();

		// Set the long name to be the .def file in the 'entities/defs' folder.
		// Handling an entity by its def file in WE makes things easier.
		std::string name = instance()->providers_[ index ];
		std::wstring wname = bw_utf8tow( name );
		std::wstring definedProviderFile = bw_utf8tow( BWResource::resolveFilename(
			getDefDirectory() + name + ".def" ) );
		
		// UAL uses back-slashes, as windows
		std::replace( definedProviderFile.begin(), definedProviderFile.end(), L'/', L'\\' );
		return AssetInfo( bw_utf8tow( getAssetName() ), wname, definedProviderFile );
	}

	static const int NAME_NOT_FOUND = -1;

	static int find( const std::string& name )
	{
		BW_GUARD;

		for ( int i = 0; i < (int)instance()->providers_.size(); ++i )
		{
			if ( _stricmp( instance()->providers_[ i ].c_str(), name.c_str() ) == 0 )
				return i;
		}
		return NAME_NOT_FOUND;
	}

private:
	mapType map_;
	std::vector<std::string> providers_;

	static bool s_comparator( const std::string& a, const std::string& b )
	{
		return _stricmp( a.c_str(), b.c_str() ) <= 0;
	}

	UalUserDefinedProviderMap()
	{
		BW_GUARD;

		// Init the map when the instance is created.
		map_.parse( BWResource::openSection( getXMLFile() ) );

		if ( map_.size() > 0 )
		{
			providers_.reserve( map_.size() );
			for (mapType::DescriptionMap::const_iterator i = map_.begin(); i!=map_.end(); i++){
				//TODO: replace with map_.getNames() 
				providers_.push_back( i->first );
			}
			std::sort< std::vector<std::string>::iterator >(
				providers_.begin(), providers_.end(), s_comparator );
		}
	}
};
/* Use Template Specializations here to define
 * the required strings for each map provider 
 */

//UserDataObjects
template<>
static const std::string USER_DATA_OBJECT_PROVIDER::getDefDirectory()
{
	BW_GUARD;

	return EntityDef::Constants::userDataObjectsDefsPath() + std::string("/");
}

template<>
static const std::string USER_DATA_OBJECT_PROVIDER::getAssetName()
{
	return "UserDataObject";
}

template<>
static const std::string USER_DATA_OBJECT_PROVIDER::getXMLFile()
{
	BW_GUARD;

	return EntityDef::Constants::userDataObjectsFile();
}
template<>
static const std::string USER_DATA_OBJECT_PROVIDER::getFactoryName()
{
	return "UserDataObject";
}

//ENTITIES
template<>
static const std::string ENTITY_PROVIDER::getDefDirectory()
{
	BW_GUARD;

	return EntityDef::Constants::entitiesDefsPath() + std::string("/");
}

template<>
static const std::string ENTITY_PROVIDER::getAssetName()
{
	return "ENTITY";
}

template<>
static const std::string ENTITY_PROVIDER::getXMLFile()
{
	BW_GUARD;

	return EntityDef::Constants::entitiesFile();
}
template<>
static const std::string ENTITY_PROVIDER::getFactoryName()
{
	return "Entities";
}

///////////////////////////////////////////////////////////////////////////////
// Section: EntityThumbProv
///////////////////////////////////////////////////////////////////////////////
/**
 *	Entity thumbnail provider. Needed for entities in the history or favourites
 *	lists.
 */
template <typename providerMap>
class EntityThumbProv : public ThumbnailProvider
{
public:
	bool isValid( const ThumbnailManager& manager, const std::wstring& file )
	{
		BW_GUARD;

		std::string nfile;
		bw_wtoutf8( file, nfile );
		if ( BWResource::getExtension( nfile ) != "def" )
			return false;

		std::string entityName =
			BWResource::removeExtension( BWResource::getFilename( nfile ) );
		return providerMap::find( entityName ) != providerMap::NAME_NOT_FOUND;
	}

	bool needsCreate( const ThumbnailManager& manager, const std::wstring& file, std::wstring& thumb, int& size )
	{
		thumb = imageFile_;
		return false;
	}

	bool prepare( const ThumbnailManager& manager, const std::wstring& file )
	{
		// should never get called
		MF_ASSERT( false );
		return false;
	}

	bool render( const ThumbnailManager& manager, const std::wstring& file, Moo::RenderTarget* rt  )
	{
		// should never get called
		MF_ASSERT( false );
		return false;
	}

	static void imageFile( const std::wstring& file ) { imageFile_ = file; }

private:
	static std::wstring imageFile_;

	DECLARE_THUMBNAIL_PROVIDER()

};
IMPLEMENT_THUMBNAIL_PROVIDER( EntityThumbProv< ENTITY_PROVIDER > )
std::wstring EntityThumbProv< ENTITY_PROVIDER >::imageFile_;

IMPLEMENT_THUMBNAIL_PROVIDER( EntityThumbProv< USER_DATA_OBJECT_PROVIDER > )
std::wstring EntityThumbProv< USER_DATA_OBJECT_PROVIDER >::imageFile_;


///////////////////////////////////////////////////////////////////////////////
// Section: EntityVFolderProvider
///////////////////////////////////////////////////////////////////////////////
template <typename providerMap>
EntityVFolderProvider<providerMap>::EntityVFolderProvider( const std::string& thumb ) :
	index_( 0 ),
	thumb_( thumb )
{
	BW_GUARD;

	// Make sure the provider map gets initialised in the main thread. If we
	// don't do this, the first call to providerMap::instance() could come
	// directly from the thumbnail thread, and providerMap uses python.
	providerMap::instance();
}
template <typename providerMap>
EntityVFolderProvider<providerMap>::~EntityVFolderProvider()
{
	providerMap::fini();
}
template <typename providerMap>
bool EntityVFolderProvider<providerMap>::startEnumChildren( const VFolderItemDataPtr parent )
{
	index_ = 0;
	return providerMap::size() > 0;
}
template <typename providerMap>
VFolderItemDataPtr EntityVFolderProvider<providerMap>::getNextChild(
								ThumbnailManager& manager, CImage& img )
{
	BW_GUARD;

	if ( index_ >= (int)providerMap::size() )
		return NULL;

	AssetInfo info = providerMap::assetInfo( index_ );

	if ( info.text().empty() || info.longText().empty() )
		return NULL;

	VFolderItemDataPtr newItem = new VFolderItemData(
		this, info, VFolderProvider::GROUP_ITEM, false );

	getThumbnail( manager, newItem, img );

	index_++;

	return newItem;
}

template <typename providerMap>
void EntityVFolderProvider<providerMap>::getThumbnail(
										ThumbnailManager& manager, 
										VFolderItemDataPtr data, CImage& img )
{
	BW_GUARD;

	if ( !data )
		return;

	if ( img_.IsNull() )
	{
		// The image has not been cached yet, so load it into the img_ cache
		// member. Note the 'loadDirectly' param set to true, to load the image
		// directly. This will be done only once, the first time it's
		// requested.
		manager.create(
			bw_utf8tow( BWResource::getFilePath( bw_wtoutf8( UalManager::instance().getConfigFile() ) ) + thumb_ ),
			img_, 16, 16, NULL, true/*loadDirectly*/ );
	}
	// blit the cached image to the return image.
	img.Create( 16, 16, 32 );
	CDC* pDC = CDC::FromHandle( img.GetDC() );
	img_.BitBlt( pDC->m_hDC, 0, 0 );
	img.ReleaseDC();
}
template <typename providerMap>
const std::wstring EntityVFolderProvider<providerMap>::getDescriptiveText( VFolderItemDataPtr data, int numItems, bool finished )
{
	BW_GUARD;

	if ( !data )
		return L"";

	if ( data->isVFolder() )
	{
		// it's the root entity VFolder, so build the appropriate text.
		return Localise(L"COMMON/UAL_ENTITY_PROVIDER/ENTITIES", providerMap::size());
	}
	else
	{
		// it's an item ( entity ), so return it's editor file.
		return data->assetInfo().longText();
	}
}

template <typename providerMap>
bool EntityVFolderProvider<providerMap>::getListProviderInfo(
	VFolderItemDataPtr data,
	std::wstring& retInitIdString,
	ListProviderPtr& retListProvider,
	bool& retItemClicked )
{
	BW_GUARD;

	if ( !data )
		return false;

	retItemClicked = !data->isVFolder();

	retInitIdString = L"";

	if ( listProvider_ )
	{
		// filter the list provider to force a refresh.
		listProvider_->setFilterHolder( filterHolder_ ); 
		listProvider_->refresh();
	}

	retListProvider = listProvider_;

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// Section: EntityListProvider
///////////////////////////////////////////////////////////////////////////////
template <typename providerMap>
EntityListProvider<providerMap>::EntityListProvider( const std::string& thumb ) :
	thumb_( thumb )
{
}
template <typename providerMap>
void EntityListProvider<providerMap>::refresh()
{
	BW_GUARD;

	filterItems();
}

template <typename providerMap>
bool EntityListProvider<providerMap>::finished()
{
	return true; // it's not asyncronous
}

template <typename providerMap>
int EntityListProvider<providerMap>::getNumItems()
{
	BW_GUARD;

	return (int)searchResults_.size();
}

template <typename providerMap>
const AssetInfo EntityListProvider<providerMap>::getAssetInfo( int index )
{
	BW_GUARD;

	if ( index < 0 || getNumItems() <= index )
		return AssetInfo();

	return searchResults_[ index ];
}

template <typename providerMap>
void EntityListProvider<providerMap>::getThumbnail(
			ThumbnailManager& manager,
			int index, CImage& img, int w, int h, ThumbnailUpdater* updater )
{
	BW_GUARD;

	if ( index < 0 || getNumItems() <= index )
		return;

	if ( img_.IsNull() || img_.GetWidth() != w || img_.GetHeight() != h )
	{
		// The image has not been cached yet, so load it into the img_ cache
		// member. Note the 'loadDirectly' param set to true, to load the image
		// directly. This will be done only once, the first time it's
		// requested.
		manager.create(
			bw_utf8tow( BWResource::getFilePath( bw_wtoutf8( UalManager::instance().getConfigFile() ) ) + thumb_ ),
			img_, w, h, NULL, true/*loadDirectly*/ );
	}
	// blit the cached image to the return image
	if ( !img_.IsNull() )
	{
		img.Create( w, h, 32 );
		CDC* pDC = CDC::FromHandle( img.GetDC() );
		img_.BitBlt( pDC->m_hDC, 0, 0 );
		img.ReleaseDC();
	}
}
template <typename providerMap>
void EntityListProvider<providerMap>::filterItems()
{
	BW_GUARD;

	searchResults_.clear();

	if ( providerMap::size() == 0 )
		return;

	searchResults_.reserve( providerMap::size() );

	// fill the results vector with the filtered entities from the Entity map
	for( int i = 0; i < (int)providerMap::size(); ++i )
	{
		AssetInfo info = providerMap::assetInfo( i );
		if ( filterHolder_ && filterHolder_->filter( info.text(), info.longText() ) )
		{
			searchResults_.push_back( info );
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Section: UalEntityVFolderLoader
///////////////////////////////////////////////////////////////////////////////
/**
 *	Entity loader class 'load' method.
 */
template <typename providerMap>
VFolderPtr UalEntityVFolderLoader<providerMap>::load(
	UalDialog* dlg, DataSectionPtr section, VFolderPtr parent, DataSectionPtr customData,
	bool addToFolderTree )
{
	BW_GUARD;

	if ( !dlg || !section || !test( section->sectionName() ) )
		return NULL;

	beginLoad( dlg, section, customData, 2/*big icon*/ );

	bool showItems = section->readBool( "showItems", false );
	std::string thumb = section->readString( "thumbnail", "" );
	
	// Set the entity thumbnail provider's image file name
	EntityThumbProv<providerMap>::imageFile( bw_utf8tow( 
		BWResource::getFilePath(
			bw_wtoutf8( UalManager::instance().getConfigFile() ) ) +
		thumb ) );

	// create VFolder and List providers, specifying the thumbnail to use
	EntityVFolderProvider<providerMap>* prov = new EntityVFolderProvider<providerMap>( thumb );
	prov->setListProvider( new EntityListProvider<providerMap> ( thumb ) );

	VFolderPtr ret = endLoad( dlg,
		prov,
		parent, showItems, addToFolderTree );
	ret->setSortSubFolders( true );
	return ret;
}
static UalVFolderLoaderFactory entityFactory( new UalEntityVFolderLoader<ENTITY_PROVIDER>() );
static UalVFolderLoaderFactory UserDataObjectFactory( new UalEntityVFolderLoader<USER_DATA_OBJECT_PROVIDER>() );
