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
#include "histogram_provider.hpp"

HistogramProvider::HistogramProvider() : usageCount_( 0 )
{
}

void HistogramProvider::addUser()
{
	++usageCount_;
}

void HistogramProvider::removeUser()
{
	--usageCount_;
}

bool HistogramProvider::inUse() const
{
	return usageCount_ != 0;
}

HistogramProvider::Histogram HistogramProvider::get( HistogramProvider::Type type ) const
{
	return histogram_[ type ];
}

void HistogramProvider::update()
{
	if( !inUse() )
		return;
	// 1. clear the values
	ZeroMemory( histogram_, sizeof( histogram_ ) );

	// 2. get resources
	ComObjectWrap<DX::Surface> inmemSurface;
	ComObjectWrap<DX::Texture> inmemTexture;
	ComObjectWrap<DX::Surface> backSurface;
	D3DSURFACE_DESC desc;
	D3DLOCKED_RECT lockRect;

	if( SUCCEEDED( Moo::rc().device()->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backSurface ) ) &&
		SUCCEEDED( backSurface->GetDesc( &desc ) ) &&
		!!( inmemTexture = Moo::rc().createTexture( desc.Width, desc.Height, 1, 0, desc.Format, D3DPOOL_SYSTEMMEM,
			"texture/histogram" ) ) &&
		SUCCEEDED( inmemTexture->GetSurfaceLevel( 0, &inmemSurface ) ) &&
		SUCCEEDED( Moo::rc().device()->GetRenderTargetData( backSurface.pComObject(), inmemSurface.pComObject() ) ) &&
		SUCCEEDED( inmemSurface->LockRect( &lockRect, NULL, 0 ) ) )
	{
		for( DWORD i = 0; i < desc.Height ; ++i )
		{
			unsigned char* inColours = (unsigned char*)lockRect.pBits + lockRect.Pitch * i;
			unsigned char* inColoursEnd = inColours + 4 * desc.Width;
			while( inColours != inColoursEnd )
			{
				++histogram_[ HT_LUMINANCE ].value_[ ( inColours[ 0 ] * 1 + inColours[ 1 ] * 6 + inColours[ 2 ] * 3 ) / 10 ];
				++histogram_[ HT_B ].value_[ inColours[ 0 ] ];
				++histogram_[ HT_G ].value_[ inColours[ 1 ] ];
				++histogram_[ HT_R ].value_[ inColours[ 2 ] ];
				inColours += 4;
			}
		}
		inmemSurface->UnlockRect();
	}
}

HistogramProvider& HistogramProvider::instance()
{
	static HistogramProvider provider;
	return provider;
}
