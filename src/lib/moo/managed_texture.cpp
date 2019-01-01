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

#include "managed_texture.hpp"
#include "resmgr/datasection.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/timestamp.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/auto_config.hpp"
#include "cstdmf/diary.hpp"

#include "render_context.hpp"

#include "resource_load_context.hpp"

#ifndef CODE_INLINE
#include "managed_texture.ipp"
#endif

#include "cstdmf/memory_counter.hpp"

#include "dds.h"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

extern SIZE_T textureMemSum;
memoryCounterDeclare( texture );

static AutoConfigString s_notFoundBmp( "system/notFoundBmp" );

namespace Moo
{

uint32 ManagedTexture::frameTimestamp_ = -1;
uint32 ManagedTexture::totalFrameTexmem_ = 0;

/*static*/ bool					ManagedTexture::s_accErrs = false;
/*static*/ std::string			ManagedTexture::s_accErrStr = "";
/*static*/ void					ManagedTexture::accErrs( bool acc ) { s_accErrs = acc; s_accErrStr = ""; }
/*static*/ const std::string&	ManagedTexture::accErrStr() { return s_accErrStr; } 


ManagedTexture::ManagedTexture( 
	const std::string& resourceID, bool mustExist, int mipSkip, 
	bool noResize, bool noFilter, const std::string& allocator )
: resourceID_( canonicalTextureName(resourceID) ),
  qualifiedResourceID_( resourceID ),
  valid_( false ),
  width_( 0 ),
  height_( 0 ),
  format_( D3DFMT_UNKNOWN ),
  mipSkip_( mipSkip ),
  textureMemoryUsed_( 0 ),
  localFrameTimestamp_( 0 ),
  failedToLoad_( false ),
  noResize_( noResize ),
  noFilter_( noFilter ),
  texture_( NULL ),
  loadedFromDisk_( true )
#if ENABLE_RESOURCE_COUNTERS
  , BaseTexture(allocator)
#endif
#if MANAGED_CUBEMAPS
  ,cubemap_( false )
#endif
{
	BW_GUARD;
	load( mustExist );

	memoryCounterAdd( texture );
	memoryClaim( resourceID_ );
}


ManagedTexture::ManagedTexture( 
		const std::string& resourceID, uint32 w, uint32 h, int nLevels,
		DWORD usage, D3DFORMAT fmt, const std::string& allocator ):
	width_( w ),
	height_( h ),
	format_( fmt ),
	mipSkip_( false ),
	textureMemoryUsed_( 0 ),
	originalTextureMemoryUsed_( 0 ),
#if ENABLE_RESOURCE_COUNTERS
	BaseTexture(allocator),
#endif
	resourceID_( canonicalTextureName(resourceID) ),
	qualifiedResourceID_( resourceID ),
	valid_( false ),
	failedToLoad_( false ),
	noResize_( false ),
	noFilter_( true ),
	loadedFromDisk_( false ),
#if MANAGED_CUBEMAPS
	cubemap_( false ),
#endif
	localFrameTimestamp_( 0 )
{
	tex_ = Moo::rc().createTexture( w, h, nLevels, usage, fmt, D3DPOOL_MANAGED, resourceID.c_str() );
	texture_ = tex_.pComObject();
	valid_ = true;
	this->BaseTexture::addToManager();
}


ManagedTexture::ManagedTexture
( 
	DataSectionPtr		data, 
	std::string			const& resourceID, 
	bool				mustExist, 
	int					mipSkip /*= 0*/,
	bool				noResize,
	bool				noFilter, 
	const std::string&	allocator
):
	width_( 0 ),
	height_( 0 ),
	format_( D3DFMT_UNKNOWN ),
	mipSkip_( mipSkip ),
	textureMemoryUsed_( 0 ),
	originalTextureMemoryUsed_( 0 ),
#if ENABLE_RESOURCE_COUNTERS
	BaseTexture(allocator),
#endif
	resourceID_( canonicalTextureName(resourceID) ),
	qualifiedResourceID_( resourceID ),
	valid_( false ),
	failedToLoad_( false ),
	noResize_( noResize ),
	noFilter_( noFilter ),
	loadedFromDisk_( true ),
#if MANAGED_CUBEMAPS
	cubemap_( false ),
#endif
	localFrameTimestamp_( 0 )
{
	BW_GUARD;
	if (data != NULL && Moo::rc().device() != NULL)
	{
		BinaryPtr binData = data->asBinary();
		if (binData != NULL && binData->len() >= 4)
		{
			DWORD *rawData = (DWORD *)binData->data();
			bool ddsHeader = (*rawData == DDS_MAGIC_VALUE);
			bool result = loadBin(binData, mustExist, !ddsHeader);
			MF_ASSERT_DEV(result);
		}
	}
}

ManagedTexture::~ManagedTexture()
{
	BW_GUARD;
	memoryCounterSub( texture );
	memoryClaim( resourceID_ );

	// let the texture manager know we're gone
	this->delFromManager();
}


void ManagedTexture::resize( uint32 newWidth, uint32 newHeight, int nLevels, DWORD usage, D3DFORMAT fmt )
{
	this->release();
	tex_ = Moo::rc().createTexture( newWidth, newHeight, nLevels, usage, fmt, D3DPOOL_MANAGED, resourceID_.c_str() );
	width_ = newWidth;
	height_ = newHeight;
	texture_ = tex_.pComObject();
	valid_ = true;
}


HRESULT ManagedTexture::reload( )
{
	BW_GUARD;
	if ( !loadedFromDisk_ )
	{
		return S_OK;
	}
	release();
	failedToLoad_ = false;
	return load( );
}

HRESULT ManagedTexture::reload(const std::string & resourceID)
{
	BW_GUARD;
	if ( !loadedFromDisk_ )
	{
		return S_OK;
	}
	release();
	failedToLoad_ = false;
	resourceID_   = canonicalTextureName(resourceID);
	qualifiedResourceID_ = resourceID;
	return load();
}

bool ManagedTexture::load( bool mustExist )
{
	BW_GUARD;
	DiaryScribe dsLoad( Diary::instance(), std::string("ManagedTexture load ") + resourceID_ );

	if (failedToLoad_)
		return false;

	// Has the texture already been initialised?
	if( valid_ )
		return true;
	
	// Have we created the device to load the texture into?
	if( Moo::rc().device() == (void*)NULL )
		return false;

	// Debug: time the texture load.
	// uint64 totalTime = timestamp();

	// Open the texture file resource.
	BinaryPtr texture;
	if (qualifiedResourceID_.substr( 0, 4 ) != "////")
	{	
		texture = BWResource::instance().rootSection()->readBinary( qualifiedResourceID_ );
	}
	else
	{	
		texture = 
			new BinaryBlock
			(
				qualifiedResourceID_.data() + 4, 
				qualifiedResourceID_.length() - 4,
				"BinaryBlock/ManagedTexture"
			);
	}
	if( !texture.hasObject() && mustExist )
	{
		if (!ManagedTexture::s_accErrs)
		{
			WARNING_MSG( "Moo::ManagedTexture::load: "
				"Can't read texture file '%s'%s.\n", qualifiedResourceID_.c_str(),
				ResourceLoadContext::formattedRequesterString().c_str() );
		}
		else
		{
			if (s_accErrStr.find( qualifiedResourceID_ ) == std::string::npos)
			{
				s_accErrStr = s_accErrStr + "\n * " + qualifiedResourceID_ + " (invalid format?)";
			}
		}
		qualifiedResourceID_ = s_notFoundBmp;
		if (qualifiedResourceID_.substr( 0, 4 ) != "////")
		{
			texture = BWResource::instance().rootSection()->readBinary( qualifiedResourceID_ );
		}
		else
		{
			texture = 
				new BinaryBlock
				( 
					qualifiedResourceID_.data() + 4, 
					qualifiedResourceID_.length() - 4,
					"BinaryBlock/ManagedTexture"
				);
		}
	}
	return loadBin(texture, mustExist);
}


bool ManagedTexture::loadBin(BinaryPtr texture, bool mustExist, 
	bool skipHeader/* = false*/)
{
	BW_GUARD;
	if( texture.hasObject() )
	{
		void   *data  = (uint8 *)texture->data() + (skipHeader ? 4 : 0);
		size_t length = texture->len() - (skipHeader ? 4 : 0);

		static PALETTEENTRY palette[256];

#if MANAGED_CUBEMAPS
		DWORD* head = (DWORD*)data;		
		if ( head[0] == DDS_MAGIC_VALUE && head[1] == 124 )
		{			
			// we have a DDS file....
			if ( ((DDS_HEADER*)&head[1])->dwCubemapFlags & DDS_CUBEMAP )
			{
				// and we have a cube map...
				cubemap_ = true;
			}
		}
#endif // MANAGED_CUBEMAPS

		// Cubemaps do not use the mip skip functionality - it was causing problems.
		DWORD boxFilter = noFilter_ ? D3DX_FILTER_POINT : D3DX_FILTER_BOX;
		
		DWORD mipFilter = BWResource::getExtension(qualifiedResourceID_) == "dds"
			? D3DX_SKIP_DDS_MIP_LEVELS(cubemap_ ? 0 : this->mipSkip(), boxFilter|D3DX_FILTER_MIRROR)
			: boxFilter|D3DX_FILTER_MIRROR;

#if MANAGED_CUBEMAPS
		//LPDIRECT3DCUBETEXTURE9
		if ( cubemap_ )
		{
			cubeTex_ = Moo::rc().createCubeTextureFromFileInMemoryEx( 
				data, length,
				D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
				D3DPOOL_MANAGED, D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 
				mipFilter, 0, NULL, palette
#if ENABLE_RESOURCE_COUNTERS
				, allocator_.c_str()
#endif
				);

			if( cubeTex_ )
			{
				DX::Surface* surface;
				if( SUCCEEDED( cubeTex_->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X,  0, &surface ) ) )
				{
					D3DSURFACE_DESC desc;
					surface->GetDesc( &desc );

					width_ = desc.Width;
					height_ = desc.Height;

					surface->Release();
					surface = NULL;
				}

				// Assign the texture.
				texture_ = cubeTex_.pComObject();

				valid_ = true;

				// Note : this must be called after the pTexture() will return
				// a pointer ( valid_ and texture_ must be set )
				//textureMemoryUsed_ = BaseTexture::textureMemoryUsed();
				textureMemoryUsed_ = length >> (this->mipSkip() * 3);
				originalTextureMemoryUsed_ = textureMemoryUsed_;

			}
			else
			{
				failedToLoad_ = true;
				if (mustExist)
				{
					if (!ManagedTexture::s_accErrs)
					{
						WARNING_MSG( "Moo::ManagedTexture::load: "
							"Failed to load texture resource '%s'%s - Invalid format?\n",
							qualifiedResourceID_.c_str(),
							ResourceLoadContext::formattedRequesterString().c_str() );
					}
					else
					{
						if (s_accErrStr.find( qualifiedResourceID_ ) == std::string::npos)
						{
							s_accErrStr = s_accErrStr + "\n * " + qualifiedResourceID_ + " (invalid format?)";
						}
					}
				}
			}
		}
		else
#endif //MANAGED_CUBEMAPS
		{
		uint32 sizeFlag = noResize_ ? D3DX_DEFAULT_NONPOW2 : D3DX_DEFAULT;
		uint32 mipLevels = noResize_ ? 1 : D3DX_DEFAULT;
		// Create the texture and the mipmaps.
		tex_ = Moo::rc().createTextureFromFileInMemoryEx( 
			data, length,
			sizeFlag, sizeFlag, mipLevels , 0, D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED, D3DX_FILTER_BOX|D3DX_FILTER_MIRROR, 
			mipFilter, 0, NULL, palette
#if ENABLE_RESOURCE_COUNTERS
			, allocator_.c_str()
#endif
			); 

		if( tex_ )
		{
			DX::Surface* surface;
			if( SUCCEEDED( tex_->GetSurfaceLevel( 0, &surface ) ) )
			{
				D3DSURFACE_DESC desc;
				surface->GetDesc( &desc );

				width_ = desc.Width;
				height_ = desc.Height;
				format_ = desc.Format;

				surface->Release();
				surface = NULL;
			}

			// Assign the texture.
			texture_ = tex_.pComObject();

			valid_ = true;

			// Note : this must be called after the pTexture() will return
			// a pointer ( valid_ and texture_ must be set )
			//textureMemoryUsed_ = BaseTexture::textureMemoryUsed();
			textureMemoryUsed_ = length >> (this->mipSkip() * 2);
			originalTextureMemoryUsed_ = textureMemoryUsed_;

			// Debug: output texture load time.
			//totalTime = timestamp() - totalTime;
			//totalTime = (totalTime * 1000) / stampsPerSecond();
/*			DEBUG_MSG( "Moo : ManagedTexture : Loaded texture %s, Width: %d, Height: %d, Mem: %d, it took %d milliseconds\n", 
				qualifiedResourceID_.c_str(), width_, height_, textureMemoryUsed_, (uint32)totalTime );*/		
		}
		else
		{
			failedToLoad_ = true;
			if (mustExist)
			{
				if (!ManagedTexture::s_accErrs)
				{
					WARNING_MSG( "Moo::ManagedTexture::load: "
						"Failed to load texture resource '%s'%s - Invalid format?\n",
						qualifiedResourceID_.c_str(),
						ResourceLoadContext::formattedRequesterString().c_str() );
				}
				else
				{
					if (s_accErrStr.find( qualifiedResourceID_ ) == std::string::npos)
					{
						s_accErrStr = s_accErrStr + "\n * " + qualifiedResourceID_ + " (invalid format?)";
					}
				}
			}
		}
		}

	}
	else
	{
		failedToLoad_ = true;
		if (mustExist)
		{
			if (!ManagedTexture::s_accErrs)
			{
				WARNING_MSG( "Moo::ManagedTexture::load: "
					"Failed to read binary resource '%s'%s\n", qualifiedResourceID_.c_str(),
					ResourceLoadContext::formattedRequesterString().c_str() );
			}
			else
			{
				if (s_accErrStr.find( qualifiedResourceID_ ) == std::string::npos)
				{
					s_accErrStr = s_accErrStr + "\n * " + qualifiedResourceID_ + " (failed to read binary resource)";
				}
			}
		}
	}

	if (texture_)
	{
		// Add the texture to the preload list so that it can get uploaded
		// to video memory
		rc().addPreloadResource( texture_ );
	}

	return !!texture_;
}

HRESULT ManagedTexture::release( )
{
	BW_GUARD;
	// release our reference to the texture
	texture_ = NULL;
	
	tex_.pComObject( NULL );
	cubeTex_.pComObject( NULL );

	valid_ = false;
	failedToLoad_ = false;

	textureMemoryUsed_ = 0;
	width_ = 0;
	height_ = 0;

	return S_OK;
}

DX::BaseTexture* ManagedTexture::pTexture()
{
	BW_GUARD;
	if( !valid_ )
		load( true );

	if (!rc().frameDrawn(frameTimestamp_))
	{
		totalFrameTexmem_ = 0;
	}

	if (frameTimestamp_ != localFrameTimestamp_)
	{
		localFrameTimestamp_ = frameTimestamp_;
		totalFrameTexmem_ += textureMemoryUsed_;
	}
	return texture_;
}

uint32 ManagedTexture::width( ) const
{
	return width_;
}

uint32 ManagedTexture::height( ) const
{
	return height_;
}

D3DFORMAT ManagedTexture::format( ) const
{
	return format_;
}

uint32 BaseTexture::textureMemoryUsed( )
{
	BW_GUARD;
	return BaseTexture::textureMemoryUsed(
			static_cast<DX::Texture*>(pTexture()) );
}

const std::string& ManagedTexture::resourceID( ) const
{
	return resourceID_;
}

std::ostream& operator<<(std::ostream& o, const ManagedTexture& t)
{
	o << "ManagedTexture\n";
	return o;
}

}
