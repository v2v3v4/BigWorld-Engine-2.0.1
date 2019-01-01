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
#include "flora_texture.hpp"
#include "geometrics.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

//FloraTexture FloraTexture::s_instance;
static int s_shotID = 0;
static bool s_save = false;
static bool s_useTexture = true;

memoryCounterDeclare( detailObjects );

/**
 *	This class exposes the Flora Texture to the effect file engine. 
 */
class FloraTextureSetter : public Moo::EffectConstantValue
{
public:
	FloraTextureSetter( DX::Texture* pTex )
	{
		value_ = pTex;
	}

	~FloraTextureSetter()
	{
		value_ = NULL;
	}

	bool FloraTextureSetter::operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		if (s_useTexture)
			pEffect->SetTexture(constantHandle, value_);
		else
			pEffect->SetTexture(constantHandle, NULL);
		return true;
	}

	DX::Texture* value_;	
};


/**
 *	Constructor.
 */
FloraTexture::FloraTexture():
	width_( 0 ),
	height_( 0 ),
	widthPerBlock_( 0 ),
	heightPerBlock_( 0 ),
	numBlocksWide_( MAX_ECOTYPES ),
	numBlocksHigh_( 1 ),
	textureMemory_( NULL ),
	largeMap_( false ),
	mediumMap_( true ),
	smallMap_( true ),
	textureSetter_( NULL )
{
	for ( int i=0; i<MAX_ECOTYPES; i++ )
	{
		blocks_[i].ecotype_ = -1;
	}

	//Create the watchers once only
	static bool s_createdWatchers = false;
	if (!s_createdWatchers)
	{
		MF_WATCH( "Client Settings/Flora/Save flora texture",
			s_save,
			Watcher::WT_READ_WRITE,
			"If set to 0, will save the current composite flora texture to disk"
			" at the next available opportunity." );
		MF_WATCH( "Client Settings/Flora/Use flora texture",
			s_useTexture,
			Watcher::WT_READ_WRITE,
			"Enabled use of the flora texture.  Setting to false will display "
			"lit flora triangles only." );
		s_createdWatchers = true;
	}
}


FloraTexture::~FloraTexture()
{
	deleteUnmanagedObjects();
}

/**
 *	This method initialises the flora texture.
 *
 *	@param	pSection	data section describing the flora texture.
 */
bool FloraTexture::init( DataSectionPtr pSection )
{
	if ( !pSection )
		return false;

	//read in our data section.
	widthPerBlock_ = pSection->readInt( "texture_width", 256 );
	heightPerBlock_ = pSection->readInt( "texture_height", 128 );
	numBlocksWide_ = MAX_ECOTYPES;
	numBlocksHigh_ = 1;
	width_ = widthPerBlock_ * numBlocksWide_;
	height_ = heightPerBlock_;

	//Check the texture size capabilities of the graphics device
	int capsMaxWidth = int(Moo::rc().deviceInfo( Moo::rc().deviceIndex() ).caps_.MaxTextureWidth);
	int capsMaxHeight = int(Moo::rc().deviceInfo( Moo::rc().deviceIndex() ).caps_.MaxTextureHeight);

	// use multiple rows of blocks in the flora texture if we need to	
	while( capsMaxWidth < width_ )
	{
		numBlocksWide_ /= 2;
		numBlocksHigh_ *= 2;
		width_ /= 2;
		height_ *= 2;
		if ( height_ > capsMaxHeight )
		{
			CRITICAL_MSG( "FloraTexture::init: "
				"device cannot support large enough textures for the flora\n" );
		}
	}
	//calculate compressed texture metrics for these values.
	largeMap_.dimensions( width_, height_ );
	mediumMap_.dimensions( widthPerBlock_*2, heightPerBlock_*2 );
	smallMap_.dimensions( widthPerBlock_, heightPerBlock_ );

	int i = 0; // index to ecotype
	int j = 0; // index to the current row of blocks
	while ( i < MAX_ECOTYPES )
	{
		blocks_[i].offset_ = Vector2( 
				float(i%numBlocksWide_) / numBlocksWide_, float(j) / numBlocksHigh_);
		blocks_[i].pixelOffsetX_ = int(blocks_[i].offset_.x * width_);
		blocks_[i].pixelOffsetY_ = int(blocks_[i].offset_.y * height_);		
		blocks_[i].texName_ = "";
		i++;
		j = i / numBlocksWide_;
	}
	
	INFO_MSG( "FloraTexture::init - texture(%d,%d), blocks(%d,%d), blocksize(%d,%d)\n",
		width_, height_, numBlocksWide_, numBlocksHigh_, widthPerBlock_, heightPerBlock_);


	return true;
}

/**
 *	Activates the flora texture.
 */
void FloraTexture::activate()
{
	this->isActive_ = true;
	this->createUnmanagedObjects();
}

/**
 *	Deactivates the flora texture.
 */
void FloraTexture::deactivate()
{
	this->deleteUnmanagedObjects();
	this->isActive_ = false;
}

/**
 *	This method overrides Moo::DeviceCallback, and allocates all
 *	GPU memory not managed by D3D.
 */
void FloraTexture::createUnmanagedObjects()
{
	if ( !this->isActive_ || width_ == 0 )
		return;

	//grab texture
	if ( textureMemory_ == 0 )
	{
		// create the main texture with the same number of mipmaps as the textures being paged into it
		// if largeMap_.numMipMaps() is used, the lower level mipmaps are never paged in but still used in rendering
		pTexture_ = Moo::rc().createTexture( width_, height_,  smallMap_.numMipMaps(), 
			0, D3DFMT_DXT3, D3DPOOL_DEFAULT );
		if ( pTexture_ )
		{
			ComObjectWrap<DX::Texture> sysCopy;
			if( sysCopy = Moo::rc().createTexture( width_, height_, smallMap_.numMipMaps(), 0, 
				D3DFMT_DXT3, D3DPOOL_SYSTEMMEM ) )
			{
				uint32 nLevels = sysCopy->GetLevelCount( );
				for (uint32 i = 0; i < nLevels; i++)
				{
					D3DSURFACE_DESC sd;
					sysCopy->GetLevelDesc( i, &sd );
					D3DLOCKED_RECT lr;
					sysCopy->LockRect( i, &lr, NULL, 0 );
					// DXT3 surfaces use one byte per texel, so width x height gives us the surface size
					ZeroMemory( lr.pBits, sd.Width * sd.Height );					
					// This fixes a bug on nvidia cards, they don't like compressed dxt3 textures 
					// to be initialised to all full OR all zero alpha...
					*(uchar*)lr.pBits = 0xff;
					sysCopy->UnlockRect( i );
				}
				Moo::rc().device()->UpdateTexture( sysCopy.pComObject(), pTexture_.pComObject() );
			}
		}
		
		textureSetter_ = new FloraTextureSetter( pTexture_.pComObject() );
		*Moo::EffectConstantValue::get( "FloraTexture" ) = textureSetter_;		
	}

	//recreate texture. it doesn't matter that we are accessing texture manager
	//in the rendering thread, as at this point the game isn't running.
	for ( int i=0; i<MAX_ECOTYPES; i++ )
	{
		if (blocks_[i].texName_ != "")
		{
			Moo::BaseTexturePtr pMooTexture =
					Moo::TextureManager::instance()->getSystemMemoryTexture( blocks_[i].texName_ );			
			this->swapInBlock(i,pMooTexture);
		}
	}
}


/**
 *	This method overrides Moo::DeviceCallback, and releases all
 *	GPU memory not managed by D3D.
 */
void FloraTexture::deleteUnmanagedObjects()
{
	memoryCounterSub( detailObjects );

	//release our reference to the texture setter.  The EffectConstantValue
	//table may still hold onto it.
	textureSetter_ = NULL;

	//release texture header + data
	if ( pTexture_ )
		memoryClaim( pTexture_ );
	pTexture_ = NULL;		

	if ( textureMemory_ )
	{
		textureMemory_ = NULL;
	}
}


//Texture management - these are called when ecotypes go into/out of scope.

/**
 *	This method allocates a new texture block, and copies the given texture
 *	in.
 */
Vector2& FloraTexture::allocate( uint8 id, Moo::BaseTexturePtr pTexture )
{
	int actualID = (int)id;
	//first, see if this ecotype still exists in a deallocated block
	for ( int i = 0; i < MAX_ECOTYPES; i++ )
	{
		if ( blocks_[i].ecotype_ == -(actualID+1) )	//add one so we can negate zero
		{
			//TRACE_MSG( "Ecotype %d (%s) regained texture block %d\n", id, pTexture->resourceID().c_str(), i );
			blocks_[i].ecotype_ = actualID;			
			return blocks_[i].offset_;
		}
	}

	//we have to find an empty block.
	//for now, do a linear search ( only 16 to look at, and it hardly ever happens )
	for ( int i = 0; i < MAX_ECOTYPES; i++ )
	{
		if ( blocks_[i].ecotype_ < 0 )
		{
			//TRACE_MSG( "Ecotype %d (%s) given texture block %d\n", id, pTexture->resourceID().c_str(), i );
			blocks_[i].ecotype_ = actualID;		
			blocks_[i].texName_ = pTexture->resourceID();
			this->swapInBlock( i, pTexture );
			return blocks_[i].offset_;
		}
	}

	ERROR_MSG( "FloraTexture::allocateTextureBlock - ERROR - all blocks used!!!\n" );
	return blocks_[0].offset_;
}


void FloraTexture::deallocate( uint8 id )
{
	int actualID = (int)id;

	//for now, do a linear search ( only 16 to look at, and it hardly ever happens )
	for ( int i = 0; i < MAX_ECOTYPES; i++ )
	{
		if (blocks_[i].ecotype_ == actualID)
		{
			//TRACE_MSG( "Ecotype %d has deallocated texture block %d \n", id, i );
			//negate block's ecotype - this tells us that the block is free,
			//but retains the id of the texture already swapped in.
			blocks_[i].ecotype_ = -(blocks_[i].ecotype_+1);	//add one so we can negate zero
			return;
		}
	}

	//TRACE_MSG( "Ecotype %d could not find its texture block to deallocate\n", id );
}


/**
 *	This method is the PC version of swapInBlock, which swaps in a texture from disk
 *	onto the floraTexture main sheet.
 */
void FloraTexture::swapInBlock( int idx, Moo::BaseTexturePtr pSourceTexture )
{
	if ( !pTexture_ )
	{
		WARNING_MSG( "FloraTexture::swapInBlock: pTexture_ is NULL.\n" );
		return;
	}

	if ( s_save )
	{
		// TODO:UNICODE: Do we need this? Isn't hard-coding this a Bad Thing(tm)?
		wchar_t wbuf[256];
		bw_snwprintf( wbuf, ARRAY_SIZE(wbuf), L"C:\\floraTex%d.dds", s_shotID++ );
		D3DXSaveTextureToFile( wbuf, D3DXIFF_DDS, pTexture_.pComObject(), NULL );
		s_save = false;
	}

	Block& block = blocks_[idx];
	
	if ( !pSourceTexture )
	{
		CRITICAL_MSG( "FloraTexture::swapInBlock - empty texture passed in \n" );
		return;
	}

	DX::BaseTexture* pBaseTex = pSourceTexture->pTexture();
	DX::Texture* pTexture;
	if( pBaseTex->GetType() == D3DRTYPE_TEXTURE )
	{
		pTexture = (DX::Texture*)pBaseTex;
		pTexture->AddRef();
	}
	else
	{
		CRITICAL_MSG( "FloraTexture::swapInBlock - %s is not a texture\n", pSourceTexture->resourceID().c_str() );
		return;
	}

	//Adjust for differences in size between what flora textures should be,
	//and what the artists have created.  Don't load higher-level mip-maps
	//than allowed.
	int mipOffset = 0;
	CompressedMipMapCalculator* smallMap = &smallMap_;
	if ( pSourceTexture->width() != smallMap_.width() )
	{
		ERROR_MSG( "FloraTexture::swapInBlock tried to swap in a flora texture of invalid size %s\n",
			pSourceTexture->resourceID().c_str() );
	}

	int mipLevel[2];
	int numLevels = smallMap->numMipMaps() - mipOffset;
	mipLevel[0] = 0;
	mipLevel[1] = mipOffset;

	RECT srcRect;
	srcRect.left = 0;
	srcRect.right = smallMap_.width();
	srcRect.top = 0;
	srcRect.bottom = smallMap_.height();

	POINT destPt;
	destPt.x = blocks_[idx].pixelOffsetX_;
	destPt.y = blocks_[idx].pixelOffsetY_;

	while ( numLevels )
	{
		DX::Surface* pDestSurface;
		HRESULT hr = pTexture_->GetSurfaceLevel( mipLevel[0], &pDestSurface );
		if ( FAILED( hr ) )
		{
			DEBUG_MSG( "FloraTexture::swapInBlock First GetSurfaceLevel failed for surface %d of texture %s ( error %lx )\n",
				mipLevel[0], pSourceTexture->resourceID().c_str(), hr );
		}

		DX::Surface* pSrcSurface;
		hr = pTexture->GetSurfaceLevel( mipLevel[1], &pSrcSurface );
		if ( FAILED( hr ) )
		{
			DEBUG_MSG( "FloraTexture::swapInBlock Second GetSurfaceLevel failed for surface %d of texture %s ( error %lx )\n",
				mipLevel[0], pSourceTexture->resourceID().c_str(), hr );
		}

		//copy data from small map's mip-map to large map's mip-map
		hr = Moo::rc().device()->UpdateSurface( pSrcSurface, &srcRect, pDestSurface, &destPt );

		//advance to next mip-map
		// we always divide by 2 because if we don't, they we get
		// pixel offsets that are outside the mipmap, and the copyrect fails.
		destPt.x/=2;	
		destPt.y/=2;	

		srcRect.right = max( (srcRect.right/2), 1L );
		srcRect.bottom = max( (srcRect.bottom/2), 1L );

		mipLevel[0]++;
		mipLevel[1]++;
		numLevels--;

		pDestSurface->Release();
		pSrcSurface->Release();
	}

	pTexture->Release();
}


/**
 *	This method displays the flora texture on-screen, and indicates which
 *	blocks are in use, and which are available for re-use.
 */
void FloraTexture::drawDebug()
{
	Moo::Material::setVertexColour();
	Moo::rc().setPixelShader( NULL );
	Moo::rc().push();
	Moo::rc().world( Matrix::identity );

	Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, FALSE );
	Moo::rc().setRenderState( D3DRS_ZENABLE, FALSE );
	Moo::rc().setRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	Moo::rc().setRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	Moo::rc().setRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	Moo::rc().setRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	const float drawHeight = 100.f;
	float drawWidth = Moo::rc().screenWidth();

	Moo::rc().setTexture( 0, this->pTexture_.pComObject() );
	Moo::rc().setTexture( 1, NULL );
	Geometrics::texturedRect(
		Vector2(0,Moo::rc().screenHeight() - drawHeight),
		Vector2(drawWidth,Moo::rc().screenHeight() ),
		0xfeffffff, false );
	Moo::rc().setTexture( 0, NULL );

	float w = drawWidth / numBlocksWide_;
	float h = drawHeight / numBlocksHigh_;

	for (uint32 i=0; i<MAX_ECOTYPES; i++)
	{
		Block& block = blocks_[i];
		POINT destPt;
		destPt.x = block.pixelOffsetX_;
		destPt.y = block.pixelOffsetY_;

		Vector2 topLeft(block.offset_.x * drawWidth,
			Moo::rc().screenHeight() - drawHeight + block.offset_.y * drawHeight);
		Vector2 bottomRight(topLeft.x + w, topLeft.y + h);

		if (block.ecotype_<0)
		{			
			Geometrics::drawRect( topLeft, bottomRight, 0x80000080 );
		}		
	}

	Moo::rc().pop();
}



//------------------------------------------------------------------------
//	Section - CompressedMipMapCalculator
//------------------------------------------------------------------------

/**
 *	Constructor
 */
CompressedMipMapCalculator::CompressedMipMapCalculator( bool includeHeader ):
	includeHeader_( includeHeader ),
	w_( 0 ),
	h_( 0 )
{
}


/**
 *	This method sets the dimensions of the texture, and calculates all relevant
 *	data for all mip-maps thereof.  It assumes that all mip-map levels are used.
 *
 *	@param	width		The new width of the texture
 *	@param	height		The new height of the texture.
 */
void CompressedMipMapCalculator::dimensions( int width, int height )
{
	w_ = width;
	h_ = height;

	//Calculate base data
	mipmaps_.clear();
	data_.numMipMaps_ = 0;
	data_.numBytes_ = includeHeader_ ? 128 : 0;	//128 byte header

	int w=w_;
	int h=h_;


	//compressed texture mip maps cannot have a dimension smaller than 4, 
	// because copyrects must use pixel offsets that are a multiple of 4
	while ( w>=4 && h>=4 )
	{
		MipData md;
		md.offset_ = data_.numBytes_;
		md.numBytesPerRow_ = w * 4;	//each row in mem stores 4 pixel rows at once ( blocks of 4x4 )
		md.numRows_ = h/4;
		md.numBytes_ = w * h;
		mipmaps_.push_back( md );

		data_.numBytes_ += md.numBytes_;
		w = w/2;
		h = h/2;
		data_.numMipMaps_++;
	}
}


/**
 *	This method returns the byte offset and stride for a particular mipmap level.
 *
 *	@param	level			The mip map level to retrieve.
 *	@param	ret 			The returned mip map data.
 */
void CompressedMipMapCalculator::mipMap( int level, MipData& ret )
{
	ret = mipmaps_[level];
}


/**
 *	This method returns the number of bytes for the texture.
 *	This method differs from data size, in that it includes the
 *	optional dds file header of 128 bytes. 
 */
uint32 CompressedMipMapCalculator::ddsSize()
{
	return data_.numBytes_;
}

/**
 *	This method returns the number of bytes for the data segment of the texture.
 */
uint32 CompressedMipMapCalculator::dataSize()
{
	return data_.numBytes_ - (includeHeader_ ? 128 : 0);	//128 byte header;
}

/**
 *	This method returns the number of mip-map levels for the texture.
 */
uint32 CompressedMipMapCalculator::numMipMaps()
{
	return data_.numMipMaps_;
}
