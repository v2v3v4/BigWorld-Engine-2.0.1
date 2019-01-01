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
#if SCALEFORM_SUPPORT
#include "Loader.hpp"
#include "Manager.hpp"
#include "moo/device_callback.hpp"
#include "moo/sys_mem_texture.hpp"
#include "moo/texture_manager.hpp"
#include "resmgr/bwresource.hpp"
#include <GImageInfo.h>
/*#include "MovieDef.hpp"*/

namespace Scaleform
{
	GFile* FileOpener::OpenFile(const char *pfilename, SInt flags, SInt mode)
	{
		BW_GUARD;
		BinaryPtr ptr = BWResource::instance().rootSection()->readBinary( pfilename );
		if( !ptr.hasObject() )
			return NULL;
		
		return new BWGMemoryFile( pfilename, ptr );
	}


	GImageInfoBase* BWToGFxImageLoader::LoadImage(const char *purl)
	{
		BW_GUARD;
		UInt    width, height;
		UByte*  pdata = 0;
		
		std::string resourceID(purl);
		resourceID.replace(0, 6, "");

		// Resolve width, height, pdata based on the url.
		// ...

		Moo::BaseTexturePtr btp = Moo::TextureManager::instance()->getSystemMemoryTexture(resourceID);
		if (!btp)
		{
			ERROR_MSG( "BWToGFxImageLoader::LoadImage: failed to load image url '%s'", purl );
			return NULL;
		}

		Moo::SysMemTexture *BWImage = static_cast<Moo::SysMemTexture*>(btp.getObject());
		
		width = BWImage->width();
		height = BWImage->height();

		GPtr<GImage> pimage = *new GImage(GImage::Image_DXT3, width, height);

		DX::Texture* tex = static_cast<DX::Texture*>(BWImage->pTexture());
		
		if(tex)
		{
			D3DLOCKED_RECT rect;
			if(D3D_OK == tex->LockRect(0, &rect, NULL, D3DLOCK_READONLY))
			{
				pdata = (unsigned char*)rect.pBits;
				memcpy(pimage->pData, pdata, width * height );
				tex->UnlockRect(0);
			}
		}		

		// Return the wrapped image with RefCount of 1.
		return new GImageInfo(pimage); 
	}


	Loader::Loader():
		GFxLoader( *new FileOpener )
	{
		BW_GUARD;
		this->SetImageLoader(GPtr<GFxImageLoader>(*new BWToGFxImageLoader()));

		// For D3D, it is good to override image creator to keep image data,
		// so that it can be restored in case of a lost device.
		GPtr<GFxImageCreator> pimageCreator = *new GFxImageCreator(1);
		this->SetImageCreator(pimageCreator);
	}


	Loader::~Loader()
	{
		BW_GUARD;
	}

}	//namespace Scaleform
#endif //#if SCALEFORM_SUPPORT