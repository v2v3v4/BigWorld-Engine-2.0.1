/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATIC_LIGHT_VALUES_HPP
#define STATIC_LIGHT_VALUES_HPP

#include "cstdmf/stdmf.hpp"
#include "moo/com_object_wrap.hpp"
#include "moo/device_callback.hpp"
#include "moo/visual.hpp"
#include "moo/vertex_buffer.hpp"
#include "resmgr/forward_declarations.hpp"

#include <d3d9.h>
#include <vector>

class BinaryBlock;
typedef SmartPointer<BinaryBlock> BinaryPtr;
typedef std::vector< D3DCOLOR > ColourValueVector;

class StaticLightValueCache : public SafeReferenceCount
{
	static const D3DCOLOR DEFAULT_LIGHT_VALUE = 0x00aaaaaa;
public:
	void set( DWORD offset, DWORD size );
	bool readyToRender() const	{	return vb_.valid();	}
	uint32 sizeInBytes() {	return vb_.valid() ? vb_.size() : 0;	}
	void invalidateData();
	bool load( DataSectionPtr ds );

private:
	void createVB( const D3DCOLOR *colours, size_t size );

	Moo::VertexBuffer   vb_;

#ifdef EDITOR_ENABLED
public:
	StaticLightValueCache() { invalid_ = false; }
	void startCalculating();
	void endCalculating();
	void addDefault( size_t size );
	D3DCOLOR* allocate( size_t size, D3DCOLOR ambient );
	size_t size() const;
	bool isDataValid() const { return readyToRender() && !invalid_; }
	bool save( DataSectionPtr ds );

private:
	ColourValueVector	colours_;
	bool invalid_;
#endif//EDITOR_ENABLED
};

typedef SmartPointer<StaticLightValueCache> StaticLightValueCachePtr;

/**
 * This class is a container for static light values,
 * which also loads and saves, and takes care of the
 * vertexbuffer for the static light values.
 */
class StaticLightValues : public Moo::StaticVertexColours
{
public:
	StaticLightValues( StaticLightValueCachePtr cache,
		DataSectionPtr pData, uint32 vertexNum = 0 );

	virtual bool readyToRender() const {	return valid() && cache_.exists() && cache_->readyToRender();	}
	virtual void unset();
	virtual void set();

	bool valid() const {	return !!size_;	}

#ifdef EDITOR_ENABLED
	bool save( DataSectionPtr ds );
	StaticLightValueCachePtr cache() { return cache_; }
#endif//EDITOR_ENABLED

private:
	StaticLightValueCachePtr cache_;

	UINT offset_;
	UINT size_;

	StaticLightValues( const StaticLightValues& );
	StaticLightValues& operator=( const StaticLightValues& );
};

typedef SmartPointer<StaticLightValues> StaticLightValuesPtr;

#endif // STATIC_LIGHT_VALUES_HPP
