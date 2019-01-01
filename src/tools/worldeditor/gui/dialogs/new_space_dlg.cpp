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
#include "worldeditor/gui/dialogs/new_space_dlg.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "appmgr/options.hpp"
#include "resmgr/datasection.hpp"
#include "resmgr/multi_file_system.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "chunk/chunk_format.hpp"
#include "controls/dir_dialog.hpp"
#include "controls/utils.hpp"
#include "chunk/base_chunk_space.hpp"
#include "terrain/terrain_settings.hpp"
#include "terrain/terrain_texture_layer.hpp"
#include "terrain/terrain2/editor_terrain_block2.hpp"
#include "moo/effect_visual_context.hpp"
#include "cstdmf/debug.hpp"
#include "common/file_dialog.hpp"
#include <set>


static AutoConfigString s_skyXML( "environment/skyXML" );
static AutoConfigString s_blankCDataFname( "dummy/cData" );
static AutoConfigString s_blankLegacyCDataFname( "dummy/cDataLegacy" );
static AutoConfigString s_blankTerrainTexture(
	"system/emptyTerrainTextureBmp", "helpers/maps/aid_builder.tga" );


DECLARE_DEBUG_COMPONENT2( "WorldEditor", 0 )


static const std::string LAST_DEFAULT_TEXTURE_TAG = "space/lastDefaultTexture";


static const int NEWSPACEDLG_MAX_CHUNKS = 1000;
static wchar_t* NEWSPACEDLG_KM_FORMAT = L"(%.1f km)";
static wchar_t* NEWSPACEDLG_M_FORMAT = L"(%d m)";


// NewSpace dialog

IMPLEMENT_DYNAMIC(NewSpaceDlg, CDialog)
NewSpaceDlg::NewSpaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NewSpaceDlg::IDD, pParent)
	, defaultSpacePath_( "spaces" )
{
}

NewSpaceDlg::~NewSpaceDlg()
{
}

void NewSpaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPACE, space_);
	DDX_Control(pDX, IDC_WIDTH, width_);
	DDX_Control(pDX, IDC_HEIGHT, height_);
	DDX_Control(pDX, IDC_HEIGHTMAP_SIZE, heightMapSize_);
	DDX_Control(pDX, IDC_NORMALMAP_SIZE, normalMapSize_);
	DDX_Control(pDX, IDC_HOLEMAP_SIZE, holeMapSize_);
	DDX_Control(pDX, IDC_SHADOWMAP_SIZE, shadowMapSize_);
	DDX_Control(pDX, IDC_BLENDMAP_SIZE, blendMapSize_);
	DDX_Control(pDX, IDC_DEFTEX_PATH, defaultTexture_);
	DDX_Control(pDX, IDC_DEFTEX_IMAGE, textureImage_);
	DDX_Control(pDX, IDC_PROGRESS1, progress_ );
	DDX_Control(pDX, IDC_NEWSPACE_WIDTH_KM, widthKms_);
	DDX_Control(pDX, IDC_NEWSPACE_HEIGHT_KM, heightKms_);
	DDX_Control(pDX, IDC_SPACE_PATH, spacePath_);
	DDX_Control(pDX, IDCANCEL, buttonCancel_);
	DDX_Control(pDX, IDOK, buttonCreate_);
}


BEGIN_MESSAGE_MAP(NewSpaceDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_EN_CHANGE(IDC_SPACE, OnEnChangeSpace)
	ON_BN_CLICKED(IDC_NEWSPACE_BROWSE, OnBnClickedNewspaceBrowse)
	ON_EN_CHANGE(IDC_DEFTEX_PATH, OnEnTextureChange)
	ON_BN_CLICKED(IDC_DEFTEX_BROWSE, OnBnTextureBrowse)
	ON_EN_CHANGE(IDC_WIDTH, OnEnChangeWidth)
	ON_EN_CHANGE(IDC_HEIGHT, OnEnChangeHeight)
	ON_BN_CLICKED(IDC_NEWSPC_CREATE_LEGACY, OnBnClickedLegacy)
	ON_EN_CHANGE(IDC_SPACE_PATH, OnEnChangeSpacePath)
END_MESSAGE_MAP()


void NewSpaceDlg::initIntEdit(
	controls::EditNumeric& edit, int minVal, int maxVal, int val )
{
	BW_GUARD;

	edit.SetNumericType( controls::EditNumeric::ENT_INTEGER );
	edit.SetAllowNegative( minVal < 0 );
	edit.SetMinimum( float( minVal ) );
	edit.SetMaximum( float( maxVal ) );
	edit.SetIntegerValue( val );
}


// NewSpace message handlers

BOOL NewSpaceDlg::OnInitDialog()
{
	BW_GUARD;

	CDialog::OnInitDialog();

	INIT_AUTO_TOOLTIP();
	
	initIntEdit( width_, 1, NEWSPACEDLG_MAX_CHUNKS, 10 );
	initIntEdit( height_, 1, NEWSPACEDLG_MAX_CHUNKS, 10 );

	wchar_t buffer[64];
	int size = MIN_MAP_SIZE;
	while ( size <= MAX_MAP_SIZE )
	{
		bw_snwprintf(buffer, ARRAY_SIZE(buffer), L"%d", size );

		heightMapSize_.AddString( buffer );
		normalMapSize_.AddString( buffer );
		shadowMapSize_.AddString( buffer );

		size *= 2;
	}

	_itow( Options::getOptionInt( "terrain2/defaults/heightMapSize", 
		Terrain::TerrainSettings::defaults()->heightMapSize() ), buffer, 10 );
	heightMapSize_.SelectString( -1, buffer );
	_itow( Options::getOptionInt( "terrain2/defaults/normalMapSize", 
		Terrain::TerrainSettings::defaults()->normalMapSize() ), buffer, 10 );
	normalMapSize_.SelectString( -1, buffer );
	initIntEdit( holeMapSize_, MIN_HOLE_MAP_SIZE, MAX_MAP_SIZE,
		Options::getOptionInt( "terrain2/defaults/holeMapSize", 
		Terrain::TerrainSettings::defaults()->holeMapSize() ) );
	_itow( Options::getOptionInt( "terrain2/defaults/shadowMapSize", 
		Terrain::TerrainSettings::defaults()->shadowMapSize() ), buffer, 10 );
	shadowMapSize_.SelectString( -1, buffer );
	initIntEdit( blendMapSize_, MIN_HOLE_MAP_SIZE, MAX_MAP_SIZE,
		Options::getOptionInt( "terrain2/defaults/blendMapSize", 
		Terrain::TerrainSettings::defaults()->blendMapSize() ) );

	space_.SetWindowText( defaultSpace_ );
	spacePath_.SetWindowText( bw_utf8tow( BWResource::resolveFilename( defaultSpacePath_ ) ).c_str() );

	defaultTexture_.SetWindowText( bw_utf8tow( Options::getOptionString( LAST_DEFAULT_TEXTURE_TAG ) ).c_str() );
		
	UpdateData( FALSE );

	width_.setSilent( false );
	height_.setSilent( false );
	holeMapSize_.setSilent( false );
	blendMapSize_.setSilent( false );

	return TRUE;  // return TRUE unless you set the focus to a control
}

class SpaceMaker
{
public:

	// Holder structure for user configurable terrain 2 information
	struct TerrainInfo
	{
		uint32	version_;
		uint32	heightMapSize_;
		uint32	normalMapSize_;
		uint32	holeMapSize_;
		uint32	shadowMapSize_;
		uint32	blendMapSize_;
	};

	SpaceMaker(
		const std::string& spaceName,
		int width, int height, int minHeight, int maxHeight,
		const TerrainInfo& terrainInfo,
		const std::string& defaultTexture,
		bool createLegacy,
		CProgressCtrl& progress )
		: spaceName_( spaceName ),
		  minY_( minHeight ),
		  maxY_( maxHeight ),
		  terrainInfo_( terrainInfo ),
		  defaultTexture_( defaultTexture ),
		  createLegacy_(createLegacy),
		  progress_( progress ),
		  terrainSize_( 0 ),
		  terrainCache_( NULL ),
		  memoryCache_( NULL ),
		  memoryCacheSize_( 128*1024 )
	{
		BW_GUARD;

		calcBounds( width, height );
	}

	~SpaceMaker()
	{
		BW_GUARD;

		if (memoryCache_ != NULL)
		{
			VirtualFree(memoryCache_, 0, MEM_RELEASE);
			memoryCache_ = NULL;
		}
		if (terrainCache_ != NULL)
		{
			VirtualFree(terrainCache_, 0, MEM_RELEASE);
			terrainCache_ = NULL;
		}
	}

	bool create()
	{
		BW_GUARD;

		if( WorldManager::instance().connection().enabled() )
		{
			if( !WorldManager::instance().connection().changeSpace( BWResource::dissolveFilename( spaceName_ ) )
				|| !WorldManager::instance().connection().lock( GridRect( GridCoord( minX_ - 1, minZ_ - 1 ),
					GridCoord( maxX_ + 1, maxZ_ + 1 ) ), "create space" ) )
			{
				errorMessage_ = "Unable to lock space";
				return false;
			}
		}
		if (!createSpaceDirectory())
		{
			errorMessage_ = "Unable to create space directory";
			return false;
		}

		if (!createSpaceSettings())
		{
			errorMessage_ = "Unable to create " + SPACE_SETTING_FILE_NAME;
			return false;
		}

		if (!createTemplateTerrainBlock())
		{
			errorMessage_ = "Unable to create a terrain block for the space";
			return false;
		}

		// True means purge every 'n' chunks to avoid wasting memory
		if (!createChunks( true ))
		{
			errorMessage_ = "Unable to create chunks";
			return false;
		}

		return true;
	}

	std::string errorMessage() { return errorMessage_; }

private:
	bool createLegacy_;

	bool createSpaceDirectory()
	{
		BW_GUARD;

		BWResource::ensurePathExists( spaceName_ + "/" );

		return BWResource::openSection( BWResource::dissolveFilename( spaceName_ ) );
	}

	void calcBounds( int width, int height )
	{
		BW_GUARD;

		/*
		// Standard bounds calc
		*/
		int w = width / 2;
		int h = height / 2;

		minX_ = -w;
		maxX_ = w - 1;
		if (w * 2 != width)
			++maxX_;

		minZ_ = -h;
		maxZ_ = h - 1;
		if (h * 2 != height)
			++maxZ_;
		/*
		// Corner bounds calc
		minX_ = 0;
		maxX_ = width_;
		minZ_ = -height_;
		maxZ_ = -1;
		*/
	}


	bool createSpaceSettings()
	{
		BW_GUARD;

		DataSectionPtr settings = BWResource::openSection(
			BWResource::dissolveFilename( spaceName_ ) )->newSection( SPACE_SETTING_FILE_NAME );

		if (!settings)
			return false;		
		
		std::string skyXML = s_skyXML.value();
		if (skyXML.empty())
			skyXML = "system/data/sky.xml";
		settings->writeString( "timeOfDay", skyXML);
		settings->writeString( "skyGradientDome", skyXML );

		settings->writeInt( "bounds/minX", minX_ );
		settings->writeInt( "bounds/maxX", maxX_ );
		settings->writeInt( "bounds/minY", minZ_ );
		settings->writeInt( "bounds/maxY", maxZ_ );

		// write terrain settings
		DataSectionPtr terrainSettings = settings->openSection( "terrain", true );

		// Create a terrainsettings object with default settings
		pTerrainSettings_ = new Terrain::TerrainSettings;
		pTerrainSettings_->initDefaults();

		pTerrainSettings_->version( terrainInfo_.version_ );
		pTerrainSettings_->heightMapSize( terrainInfo_.heightMapSize_ );
		pTerrainSettings_->normalMapSize( terrainInfo_.normalMapSize_ );
		pTerrainSettings_->holeMapSize( terrainInfo_.holeMapSize_ );
		pTerrainSettings_->shadowMapSize( terrainInfo_.shadowMapSize_ );
		pTerrainSettings_->blendMapSize( terrainInfo_.blendMapSize_ );
		pTerrainSettings_->save( terrainSettings );

		settings->save();

		return true;
	}

	bool createTemplateTerrainBlock()
	{
		BW_GUARD;

		// get relevant space settings
		DataSectionPtr settings = BWResource::openSection(
			BWResource::dissolveFilename( spaceName_ ) + "/" + SPACE_SETTING_FILE_NAME );
		DataSectionPtr terrainSettingsDS = NULL;
		if ( settings != NULL )
			terrainSettingsDS = settings->openSection( "terrain" );

		// init the terrain settings so the terrain is set to the appropriate
		// version when creating/loading the block.
		Terrain::TerrainSettingsPtr pTerrainSettings = new Terrain::TerrainSettings();
		pTerrainSettings->init(terrainSettingsDS);

		uint32 blendSize = pTerrainSettings->blendMapSize();

		// setup the template cdata file
		std::string templateTname;
		bool deleteTemplate = false;
		if ( createLegacy_ )
		{
			templateTname = BWResource::resolveFilename( s_blankLegacyCDataFname );
		}
		else
		{
			// create a new terrain2 block
			templateTname = BWResource::resolveFilename( s_blankCDataFname );
			SmartPointer<Terrain::EditorTerrainBlock2> block = 
				new Terrain::EditorTerrainBlock2(pTerrainSettings);
			std::string error;
			if ( !block->create( terrainSettingsDS, &error ) )
			{
				if ( !error.empty() )
					ERROR_MSG( "Failed to create new space: %s\n", error.c_str() );
				return false;
			}

			// add a default texture layer
			int32 idx = block->insertTextureLayer( blendSize, blendSize );
			if ( idx == -1 )
			{
				ERROR_MSG( "Couldn't create default texture for new space.\n" );
				return false;
			}
			else
			{
				Terrain::TerrainTextureLayer& layer = block->textureLayer( idx );
				layer.textureName( s_blankTerrainTexture.value() );
				{
					// set the blends to full white
					Terrain::TerrainTextureLayerHolder holder( &layer, false );
					layer.image().fill(
						std::numeric_limits<Terrain::TerrainTextureLayer::PixelType>::max() );
				}
				// update and save
				block->rebuildCombinedLayers();
				if (!block->rebuildLodTexture( Matrix::identity ))
				{
					return false;
				}
			}			

			// save it
			std::remove( templateTname.c_str() );
			
			DataSectionPtr cdata = new BinSection( s_blankCDataFname, NULL );
			cdata=cdata->convertToZip(s_blankCDataFname);
			if ( cdata == NULL )
				return false;

			DataSectionPtr pTerrain = cdata->openSection( block->dataSectionName(), true );
			pTerrain->setParent(cdata);
			pTerrain = pTerrain->convertToZip();
			bool savedOK = (pTerrain && block->save( pTerrain ));
			pTerrain->setParent( NULL );
			if (!savedOK)
			{
				return false;
			}
			// flags are lighting, thumbnail and shadow. Only shadow is clean at the moment.
			EditorChunkCache::UpdateFlags flags( cdata );
			flags.shadow_ = 1;
			flags.save();

			if ( !cdata->save( s_blankCDataFname ) )
			{
				return false;
			}

			deleteTemplate = true;
		}

		if ( !defaultTexture_.empty() )
		{
			// Create temporary blank terrain file with the default texture
			std::string fname = templateTname;
			if ( createLegacy_ )
				fname += ".temp"; // need a temp file for old spaces
			std::string terrainName = fname + "/terrain";
			if ( createLegacy_ )
				CopyFile( bw_utf8tow(templateTname).c_str(), bw_utf8tow(fname).c_str(), FALSE ); // need a temp file for old spaces
			
			// load the blank block, insert a texture layer with the default,
			// texture, and save it.
			Terrain::EditorBaseTerrainBlockPtr block =
				static_cast<Terrain::EditorBaseTerrainBlock*>(
					Terrain::EditorBaseTerrainBlock::loadBlock(
						terrainName,
						Matrix::identity,
						Vector3( 0, 0, 0 ),
						pTerrainSettings,
						NULL ).getObject() );
			if ( block )
			{
				// remove all layers
				size_t numLayers = block->numberTextureLayers();
		        for (size_t i = 0; i < numLayers; ++i)
					block->removeTextureLayer( 0 );

				// insert a clean layer with the default texture
				int32 idx = block->insertTextureLayer( blendSize, blendSize );
				if ( idx == -1 )
				{
					ERROR_MSG( "Couldn't create default texture for new space.\n" );
				}
				else
				{
					Terrain::TerrainTextureLayer& layer = block->textureLayer( idx );
					layer.textureName( defaultTexture_ );
					{
						// set the blends to full white
						Terrain::TerrainTextureLayerHolder holder( &layer, false );
						layer.image().fill(
							std::numeric_limits<Terrain::TerrainTextureLayer::PixelType>::max() );
					}
					// update and save
					block->rebuildCombinedLayers();
					if (!block->rebuildLodTexture( Matrix::identity ))
					{
						return false;
					}
					DataSectionPtr cDataDS = BWResource::openSection( fname );
					if ( cDataDS )
					{
						DataSectionPtr blockDS = cDataDS->openSection( block->dataSectionName() );
						if ( blockDS )
						{
							if ( block->save( blockDS ) )
							{
								cDataDS->save();
								// set 'templateTname' to the temp file instead of the blank
								// 's_blankCDataFname' file.
								templateTname = fname;
								deleteTemplate = true;
							}
						}
					}
				}
			}
		}

		//Copy the cData
		HANDLE file = CreateFile( bw_utf8tow( templateTname ).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, 0, NULL );
		if( file != INVALID_HANDLE_VALUE )
		{
			DWORD size = GetFileSize( file, NULL );
			terrainSize_ = size;
			wchar_t diskRoot[] = { templateTname[0], L':', L'\0' };
			DWORD dummy, bytesPerSector;
			GetDiskFreeSpace( diskRoot, &dummy, &bytesPerSector, &dummy, &dummy );
			size = ( size + bytesPerSector - 1 ) / bytesPerSector * bytesPerSector;
			if (terrainCache_ != NULL)
			{
				VirtualFree(terrainCache_, 0, MEM_RELEASE);
				terrainCache_ = NULL;
			}
			terrainCache_ = VirtualAlloc( NULL, size, MEM_COMMIT, PAGE_READWRITE );
			ReadFile( file, terrainCache_, size, &dummy, NULL );
			CloseHandle( file );
		}
		if (memoryCache_ != NULL)
		{
			VirtualFree(memoryCache_, 0, MEM_RELEASE);
			memoryCache_ = NULL;
		}
		memoryCache_ = (char*)VirtualAlloc( NULL, memoryCacheSize_ /*big enough*/, MEM_COMMIT, PAGE_READWRITE );
		
		// delete temp file
		if ( deleteTemplate )
			std::remove( templateTname.c_str() );
		return true;
	}

	std::string chunkName( int x, int z ) const
	{
		BW_GUARD;

		return ChunkFormat::outsideChunkIdentifier( x,z );
	}

	void createTerrain( int x, int z, char*& buffer )
	{
		BW_GUARD;

		std::string name = chunkName( x, z );
		std::string cDataName = name.substr( 0, name.size() - 1 )
			+ "o.cdata";
		std::string terrainName = cDataName + "/terrain";

		std::string chunkFile;
		chunkFile =  "<root>\r\n";
		chunkFile +=	"\t<terrain>\r\n";
		chunkFile +=		"\t\t<resource>\t"+terrainName+"\t</resource>\r\n";
		chunkFile +=	"\t</terrain>\r\n";
		chunkFile += "</root>\r\n";

		buffer += bw_snprintf( buffer, memoryCacheSize_, chunkFile.c_str() );

		if ( createLegacy_ )
		{
			MF_ASSERT(!s_blankLegacyCDataFname.value().empty());
		}
		else
		{
			MF_ASSERT(!s_blankCDataFname.value().empty());
		}

		//Copy the cData
		std::string fname = spaceName_ + "/" + cDataName;
		HANDLE file = CreateFile( bw_utf8tow( fname ).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL );
		if( file != INVALID_HANDLE_VALUE )
		{
			DWORD dummy;
			static DWORD bytesPerSector = 0;
			if( bytesPerSector == 0 )
			{
				wchar_t diskRoot[] = { fname[0], L':', L'\0' };
				GetDiskFreeSpace( diskRoot, &dummy, &bytesPerSector, &dummy, &dummy );
			}
			DWORD size = ( terrainSize_ + bytesPerSector - 1 ) / bytesPerSector * bytesPerSector;
			WriteFile( file, terrainCache_, size, &dummy, NULL );
			CloseHandle( file );
			file = CreateFile( bw_utf8tow( fname ).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, 0, NULL );
			SetFilePointer( file, terrainSize_, NULL, FILE_BEGIN );
			SetEndOfFile( file );
			CloseHandle( file );
		}

		if ( !createLegacy_ )
		{
			// terrain2 needs the texture lod to be calculated here.
			Moo::EffectVisualContext::instance().initConstants();
			std::string terrainName = fname + "/terrain";
			SmartPointer<Terrain::EditorTerrainBlock2> block =
				static_cast<Terrain::EditorTerrainBlock2*>(
					Terrain::EditorBaseTerrainBlock::loadBlock(
						terrainName,
						Matrix::identity,
						Vector3( 0, 0, 0 ),
						pTerrainSettings_,
						NULL ).getObject() );
			if ( block )
			{
				Matrix m( Matrix::identity );
				m.translation( Vector3( GRID_RESOLUTION * x, 0.0f, GRID_RESOLUTION * z ) );
				if ( block->rebuildLodTexture( m ) )
				{
					DataSectionPtr cdataDS = BWResource::openSection( fname );
					if ( cdataDS )
					{
						DataSectionPtr terrainDS = cdataDS->openSection( block->dataSectionName() );
						if ( terrainDS && block->saveLodTexture( terrainDS ) )
						{
							EditorChunkCache::UpdateFlags flags( cdataDS );

							flags.terrainLOD_ = 1;
							flags.save();
							cdataDS->save();
						}
						else
						{
							ERROR_MSG( "Couldn't save terrain for %s\n", fname.c_str() );
						}
					}
					else
					{
						ERROR_MSG( "Couldn't open terrain block %s .cdata file\n", fname.c_str() );
					}
				}
				else
				{
					ERROR_MSG( "Couldn't create terrain lod texture for %s\n", fname.c_str() );
				}
			}
			else
			{
				ERROR_MSG( "Couldn't load terrain block %s to create its terrain lod texture\n", fname.c_str() );
			}
		}
	}

	bool createChunk( int x, int z )
	{
		BW_GUARD;

		if( !terrainCache_ )
		{
			errorMessage_ = "Cannot find template cdata file.";
			return false;
		}

		std::string name = chunkName( x, z ) + ".chunk";

		if( name.rfind( '/' ) != name.npos  )
		{
			std::string path = name.substr( 0, name.rfind( '/' ) + 1 );
			if( subDir_.find( path ) == subDir_.end() )
			{
				BWResource::ensurePathExists( spaceName_ + "/" + path );
				subDir_.insert( path );
			}
		}

		char* buffer = memoryCache_;
		createTerrain( x, z, buffer );

		std::string fname = spaceName_ + "/" + name;
		HANDLE file = CreateFile( bw_utf8tow( fname ).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL );
		if( file != INVALID_HANDLE_VALUE )
		{
			DWORD dummy;
			static DWORD bytesPerSector = 0;
			if( bytesPerSector == 0 )
			{
				wchar_t diskRoot[] = { fname[0], L':', L'\0' };
				GetDiskFreeSpace( diskRoot, &dummy, &bytesPerSector, &dummy, &dummy );
			}
			DWORD size = ( buffer - memoryCache_ + bytesPerSector - 1 ) / bytesPerSector * bytesPerSector;
			WriteFile( file, memoryCache_, size, &dummy, NULL );
			CloseHandle( file );
			file = CreateFile( bw_utf8tow( fname ).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING, 0, NULL );
			SetFilePointer( file, buffer - memoryCache_ , NULL, FILE_BEGIN );
			SetEndOfFile( file );
			CloseHandle( file );
		}

		return true;
	}

	bool createChunks( bool withPurge )
	{
		BW_GUARD;

		// Create a copy of the default texture if possible.  This keeps the
		// texture in the TextureManager's cache throughout the chunk
		// creation. 
		Moo::BaseTexturePtr pDefaultTexture = 
			Moo::TextureManager::instance()->get( defaultTexture_ );

		subDir_.clear();
		bool good = true;

		progress_.SetRange32( 0, (maxX_-minX_+1) * (maxZ_-minZ_+1) );
		progress_.SetPos( 0 );
		progress_.SetStep( 1 );

		int i = 0;
		
		int start = GetTickCount();
		
		for (int x = minX_; x <= maxX_; ++x)
		{
			int loop = GetTickCount();
			
			for (int z = minZ_; z <= maxZ_; ++z)
			{
				good &= createChunk( x, z );
				progress_.StepIt();
				if ((withPurge) && (++i > 150))
				{
					BWResource::instance().purgeAll();
					i = 0;
				}
			}
			
			int last = GetTickCount();

			float average = float( (last - start) / (x - minX_ + 1) );

			DEBUG_MSG( "output row %d of %d , %d , %2.0f\n", x-minX_+1, maxX_-minX_+1, last - loop,  average);
		}

		return good;
	}

	std::string spaceName_;
	std::string errorMessage_;
	int minX_, maxX_, minZ_, maxZ_, minY_, maxY_;
	TerrainInfo terrainInfo_;
	Terrain::TerrainSettingsPtr pTerrainSettings_;
	std::string defaultTexture_;
	CProgressCtrl& progress_;
	LPVOID terrainCache_;
	DWORD terrainSize_;
	char* memoryCache_;
	const uint32 memoryCacheSize_;
	std::set<std::string> subDir_;
};

void NewSpaceDlg::OnBnClickedOk()
{
	BW_GUARD;

	CWaitCursor wait;

	if ( !validateDefaultTexture() )
	{
		return;
	}

	if (GetFocus() != &buttonCreate_)
	{
		if (!holeMapSize_.isValidValue())
		{
			holeMapSize_.setSilent( true );
			holeMapSize_.SetFocus();
			holeMapSize_.setSilent( false );
			return;
		}

		if (!blendMapSize_.isValidValue())
		{
			blendMapSize_.setSilent( true );
			blendMapSize_.SetFocus();
			blendMapSize_.setSilent( false );
			return;
		}
	}
	// Create the space!
	CString s, p, w, h, ny, my, defaultTexture;
	space_.GetWindowText( s );
	spacePath_.GetWindowText( p );
	width_.GetWindowText( w );
	height_.GetWindowText( h );
	defaultTexture_.GetWindowText( defaultTexture );
	ny.Format( L"%d", MIN_CHUNK_HEIGHT );
	my.Format( L"%d", MAX_CHUNK_HEIGHT );

	if ( !p.IsEmpty() && p.Right( 1 ) != "/" && p.Right( 1 ) != "\\" )
		p = p + L"/";

	std::wstring spaceName = ( p + s ).GetString();

	if ( s.IsEmpty() )
	{
		std::wstring msg = L"There is no space name specified.\n\nFill in the space name, e.g. \"space001\".";
		MessageBox( msg.c_str(), L"No space name given", MB_ICONERROR );
		space_.SetSel( 0, -1 ); // Select the space name field
		space_.SetFocus();
		return;
	}
	
	if ( p.IsEmpty() || BWResource::dissolveFilename( bw_wtoutf8( p.GetString( ) ) ) == "" )
	{
		std::wstring msg = L"The space cannot be located in the root directory."
			L"\n\nWould you like to create it in \""
			+ bw_utf8tow( BWResource::resolveFilename( defaultSpacePath_ ) ) + L"/" + s.GetString() + L"\"?";
		if ( MessageBox( msg.c_str(), L"No space folder given", MB_ICONINFORMATION | MB_OKCANCEL ) == IDCANCEL )
		{
			spacePath_.SetSel( 0, -1 ); // Select the space path field
			spacePath_.SetFocus();
			return;
		}
		spaceName = bw_utf8tow( BWResource::resolveFilename( defaultSpacePath_ ) ) + L"/" + s.GetString();
	}
	else if ( BWResource::fileExists( bw_wtoutf8( spaceName ) + "/" + SPACE_SETTING_FILE_NAME ) )
	{
		std::wstring msg = std::wstring( L"The folder \"" )
			+ spaceName + L"\" already contains a space."
			L"\nPlease use a different space name or select a different folder.";
		MessageBox( msg.c_str(), L"Folder already contains a space", MB_ICONERROR );
		spacePath_.SetSel( 0, -1 ); // Select the space path field
		spacePath_.SetFocus();
		return;
	}
	else if ( bw_MW_strcmp( BWResource::dissolveFilename( bw_wtoutf8( spaceName ) ).c_str(), spaceName.c_str() ) == 0 )
	{
		std::wstring msg = std::wstring( L"The folder \"" )
			+ p.GetString() +
			L"\" is not in a BigWorld path."
			L"\nPlease correct the path, or select a the desired folder using the '...' button";
		MessageBox( msg.c_str(), L"Folder not inside a BigWorld path.", MB_ICONERROR );
		spacePath_.SetSel( 0, -1 ); // Select the space path field
		spacePath_.SetFocus();
		return;
	}

	int width = _wtoi( w.GetString() );
	int height = _wtoi( h.GetString() );
	int minHeight = _wtoi( ny.GetString() );
	int maxHeight = _wtoi( my.GetString() );

	if ( minHeight > maxHeight )
	{
		int h = maxHeight;
		maxHeight = minHeight;
		minHeight = h;
	}

	bool createLegacy = controls::isButtonChecked( *this, IDC_NEWSPC_CREATE_LEGACY );

	SpaceMaker::TerrainInfo terrainInfo;
	if ( createLegacy )
	{
		terrainInfo.version_		= 100;
		// terrain1 sizes
		terrainInfo.heightMapSize_	= 25;
		terrainInfo.normalMapSize_	= 25;
		terrainInfo.holeMapSize_	= 25;
		terrainInfo.shadowMapSize_	= 25;
		terrainInfo.blendMapSize_	= 25;
	}
	else
	{
		CString buffer;
		terrainInfo.version_		= 200;
		heightMapSize_.GetLBText( heightMapSize_.GetCurSel(), buffer );
		terrainInfo.heightMapSize_	= _wtoi( buffer.GetString() );
		normalMapSize_.GetLBText( normalMapSize_.GetCurSel(), buffer );
		terrainInfo.normalMapSize_	= _wtoi( buffer.GetString() );
		terrainInfo.holeMapSize_	= holeMapSize_.GetIntegerValue();
		shadowMapSize_.GetLBText( shadowMapSize_.GetCurSel(), buffer );
		terrainInfo.shadowMapSize_	= _wtoi( buffer.GetString() );
		terrainInfo.blendMapSize_	= blendMapSize_.GetIntegerValue();

		// store the defaults for next time
		Options::setOptionInt( "terrain2/defaults/heightMapSize", terrainInfo.heightMapSize_ );
		Options::setOptionInt( "terrain2/defaults/normalMapSize", terrainInfo.normalMapSize_ );
		Options::setOptionInt( "terrain2/defaults/holeMapSize", terrainInfo.holeMapSize_ );
		Options::setOptionInt( "terrain2/defaults/shadowMapSize", terrainInfo.shadowMapSize_ );
		Options::setOptionInt( "terrain2/defaults/blendMapSize", terrainInfo.blendMapSize_ );
	}

	SpaceMaker maker( bw_wtoutf8( spaceName ),
		width, height, minHeight, maxHeight,
		terrainInfo,
		bw_wtoutf8( defaultTexture.GetString() ),
		createLegacy,
		progress_ );
	buttonCreate_.EnableWindow( FALSE );
	buttonCancel_.EnableWindow( FALSE );
	UpdateWindow();
	bool result = maker.create();
	buttonCreate_.EnableWindow( TRUE );
	buttonCancel_.EnableWindow( TRUE );
	if ( !result )
	{
		MessageBox( bw_utf8tow( maker.errorMessage()).c_str(), L"Unable to create space", MB_ICONERROR );
	}
	else
	{
		createdSpace_ = bw_utf8tow( BWResource::dissolveFilename( bw_wtoutf8( spaceName ) ) ).c_str();

		if ( !textureImage_.image().isEmpty() )
		{
			// save last used default texture
			CString dt;
			defaultTexture_.GetWindowText( dt );
			std::string dissolvedFileName = BWResource::dissolveFilename( bw_wtoutf8( dt.GetString( )) );
			Options::setOptionString( LAST_DEFAULT_TEXTURE_TAG, dissolvedFileName );
		}
		OnOK();
	}
}

void NewSpaceDlg::validateFile( CEdit& ctrl, bool isPath )
{
	BW_GUARD;

	static CString invalidFileChar = L"\\/:*?\"<>| ";
	static CString invalidDirChar = L"*?\"<>|";
	CString s;
	ctrl.GetWindowText( s );

	bool changed = false;
	for (int i = 0; i < s.GetLength(); ++i)
	{
		if (isupper(s[i]))
		{
			changed = true;
			s.SetAt(i, tolower(s[i]));
		}
		else if ( !isPath && ( s[i] == ':' || s[i] == '/' ) )
		{
			changed = true;
			s.SetAt(i, '_');
		}
		else if ( isPath && s[i] == '\\' )
		{
			changed = true;
			s.SetAt(i, '/');
		}
		else if ( isPath && invalidDirChar.Find( s[i] ) != -1 )
		{
			changed = true;
			s.SetAt(i, '_');
		}
		else if ( !isPath && invalidFileChar.Find( s[i] ) != -1 )
		{
			changed = true;
			s.SetAt(i, '_');
		}
	}

	if ( changed || s.GetLength() > MAX_PATH )
	{
		int start, end;
		ctrl.GetSel( start, end );
		s = s.Left( MAX_PATH );
		ctrl.SetWindowText( s );
		ctrl.SetSel( start, end );
	}
}

bool NewSpaceDlg::validateDefaultTexture()
{
	BW_GUARD;

	if ( defaultTexture_.GetWindowTextLength() > 0 && textureImage_.image().isEmpty() )
	{
		CString dt;
		defaultTexture_.GetWindowText( dt );
		std::wstring resolvedFName = bw_utf8tow( BWResource::resolveFilename( bw_wtoutf8( dt.GetString( ) ) ) );
		std::wstring msg = L"The Default Terrain Texture '" + resolvedFName +
			L"' cannot be used because it's missing or it's not in a valid resource path.\n" +
			L"To create the space, please enter a valid texture file name, or clear the Default Terrain Texture field.";
		MessageBox( msg.c_str(), L"Invalid Default Terrain Texture", MB_ICONERROR );
		return false;
	}

	return true;
}

void NewSpaceDlg::formatChunkToKms( CString& str )
{
	BW_GUARD;

	int val = _wtoi( str.GetString() );

	if ( val * GRID_RESOLUTION >= 1000 )
		str.Format( NEWSPACEDLG_KM_FORMAT, val * GRID_RESOLUTION / 1000.0f );
	else
		str.Format( NEWSPACEDLG_M_FORMAT, (int) ( val * GRID_RESOLUTION ) );
}

void NewSpaceDlg::OnEnChangeSpace()
{
	BW_GUARD;

	validateFile( space_, false );
}

void NewSpaceDlg::OnEnChangeSpacePath()
{
	BW_GUARD;

	validateFile( spacePath_, true );
}

void NewSpaceDlg::OnBnClickedNewspaceBrowse()
{
	BW_GUARD;

	DirDialog dlg;

	dlg.windowTitle_ = _T("New Space Folder");
	dlg.promptText_  = _T("Choose a folder for the new space...");
	dlg.fakeRootDirectory_ = dlg.basePath();

	CString curpath;
	spacePath_.GetWindowText( curpath );

	// Set the start directory, check if exists:
	if ( BWResource::instance().fileSystem()->getFileType( bw_wtoutf8( curpath.GetString( ) ) ) == IFileSystem::FT_DIRECTORY )
		dlg.startDirectory_ = BWResource::resolveFilename( bw_wtoutf8( curpath.GetString( ) ) ).c_str();
	else
		dlg.startDirectory_ = dlg.basePath();

	if (dlg.doBrowse(AfxGetApp()->m_pMainWnd)) 
	{
		spacePath_.SetWindowText( dlg.userSelectedDirectory_ );
	}
}

void NewSpaceDlg::OnEnTextureChange()
{
	BW_GUARD;

	validateFile( defaultTexture_, true );
	CString dt;
	defaultTexture_.GetWindowText( dt );

	textureImage_.image().load( BWResource::resolveFilename( bw_wtoutf8( dt.GetString() ) ) );
	textureImage_.Invalidate();
	textureImage_.UpdateWindow();
}

void NewSpaceDlg::OnBnTextureBrowse()
{
	BW_GUARD;

	static wchar_t szFilter[] = 
        L"Texture Files (*.tga;*.bmp;*.dds)|*.tga;*.bmp;*.dds|"
        L"All Files (*.*)|*.*||";

	BWFileDialog fileDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, GetParent() );

	CString dt;
	defaultTexture_.GetWindowText( dt );
	// lastDefTex must be declared here to keep the string alive during DoModal
	std::wstring lastDefTex;
	if ( !dt.IsEmpty() )
	{
		lastDefTex = bw_utf8tow( 
			BWResource::getFilePath(
				BWResource::resolveFilename( bw_wtoutf8( dt.GetString() ) ) ) );
		std::replace( lastDefTex.begin(), lastDefTex.end(), L'/', L'\\' );
		fileDialog.GetOFN().lpstrInitialDir = lastDefTex.c_str();
	}

	if ( fileDialog.DoModal() == IDOK )
	{
		bool failed = true;
		std::string dissolvedFileName = BWResource::dissolveFilename( bw_wtoutf8( fileDialog.GetPathName().GetString() ) );
		if ( dissolvedFileName[1] != ':' && dissolvedFileName.size() )
		{
			defaultTexture_.SetWindowText( bw_utf8tow( dissolvedFileName ).c_str() );
			OnEnTextureChange();
			failed = false;
		}
		
		if ( textureImage_.image().isEmpty() || failed )
		{
			std::string msg = "The file '" + dissolvedFileName +
				"' cannot be used as a texture because it's missing or is not in the resource paths.";
			MessageBox( bw_utf8tow( msg ).c_str(), L"Browse for Default Terrain Texture", MB_ICONERROR );
		}
	}
}

void NewSpaceDlg::OnEnChangeWidth()
{
	BW_GUARD;

	CString str;

	width_.GetWindowText( str );

	formatChunkToKms( str );

	widthKms_.SetWindowText( str );
}

void NewSpaceDlg::OnEnChangeHeight()
{
	BW_GUARD;

	CString str;

	height_.GetWindowText( str );

	formatChunkToKms( str );

	heightKms_.SetWindowText( str );
}

void NewSpaceDlg::OnBnClickedLegacy()
{
	BW_GUARD;

	BOOL enable = controls::isButtonChecked( *this, IDC_NEWSPC_CREATE_LEGACY )
		? FALSE : TRUE;

	heightMapSize_.EnableWindow( enable );
	normalMapSize_.EnableWindow( enable );
	holeMapSize_.EnableWindow( enable );
	shadowMapSize_.EnableWindow( enable );
	blendMapSize_.EnableWindow( enable );

	wchar_t buffer[64];
	if ( enable )
	{
		// set the terrain2 values stored in the options.xml, or set the defaults
		heightMapSize_.DeleteString( heightMapSize_.FindStringExact( -1, L"25" ) );
		_itow( Options::getOptionInt( "terrain2/defaults/heightMapSize", 
			Terrain::TerrainSettings::defaults()->heightMapSize() ), buffer, 10 );
		heightMapSize_.SelectString( 0, buffer );
		normalMapSize_.DeleteString( normalMapSize_.FindStringExact( -1, L"25" ) );
		_itow( Options::getOptionInt( "terrain2/defaults/normalMapSize", 
			Terrain::TerrainSettings::defaults()->normalMapSize() ), buffer, 10 );
		normalMapSize_.SelectString( 0, buffer );
		holeMapSize_.SetIntegerValue(
			Options::getOptionInt( "terrain2/defaults/holeMapSize", 
			Terrain::TerrainSettings::defaults()->holeMapSize() ) );
		shadowMapSize_.DeleteString( shadowMapSize_.FindStringExact( -1, L"25" ) );
		_itow( Options::getOptionInt( "terrain2/defaults/shadowMapSize", 
			Terrain::TerrainSettings::defaults()->shadowMapSize() ), buffer, 10 );
		shadowMapSize_.SelectString( 0, buffer );
		blendMapSize_.SetIntegerValue(
			Options::getOptionInt( "terrain2/defaults/blendMapSize", 
			Terrain::TerrainSettings::defaults()->blendMapSize() ) );
	}
	else
	{
		// set default terrain1 values
		heightMapSize_.InsertString( 0, L"25" );
		heightMapSize_.SelectString( -1, L"25" );
		normalMapSize_.InsertString( 0, L"25" );
		normalMapSize_.SelectString( -1, L"25" );
		holeMapSize_.SetIntegerValue( 25 );
		shadowMapSize_.InsertString( 0, L"25" );
		shadowMapSize_.SelectString( -1, L"25" );
		blendMapSize_.SetIntegerValue( 25 );
	}
}
