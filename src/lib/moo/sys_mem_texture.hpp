/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SYS_MEM_TEXTURE_HPP
#define SYS_MEM_TEXTURE_HPP

#include "base_texture.hpp"

namespace Moo
{

/**
 *	This class implements a system memory texture.
 */
class SysMemTexture : public BaseTexture, public DeviceCallback
{
public:
	SysMemTexture( const std::string& resourceID, D3DFORMAT format = D3DFMT_UNKNOWN );
	~SysMemTexture() {};

	DX::BaseTexture*	pTexture( ) { return pTexture_.pComObject(); }
	uint32				width( ) const { return width_; }
	uint32				height( ) const { return height_; }
	D3DFORMAT			format( ) const { return format_; }
	uint32				textureMemoryUsed( ) { return 0; }
	const std::string&	resourceID( ) const { return resourceID_; }
	HRESULT				load();
	HRESULT				release() { pTexture_ = NULL; height_ = width_ = 0; return S_OK; }
	HRESULT				reload( ) { release(); load(); return S_OK; }

	void				createUnmanagedObjects() { this->load(); }
	void				deleteUnmanagedObjects() { this->release(); }

	uint32				maxMipLevel() const { return 0; }
	void				maxMipLevel( uint32 level ) {}

	bool				failedToLoad() { return failedToLoad_; }

private:
	ComObjectWrap< DX::Texture >	pTexture_;
	uint32							width_;
	uint32							height_;
	D3DFORMAT						format_;
	std::string						resourceID_;
	std::string						qualifiedResourceID_;
	bool							failedToLoad_;

	SysMemTexture( const SysMemTexture& );
	SysMemTexture& operator=( const SysMemTexture& );
};

}


#endif // SYS_MEM_TEXTURE_HPP
