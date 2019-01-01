/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DYNAMIC_INDEX_BUFFER_HPP
#define DYNAMIC_INDEX_BUFFER_HPP

#include <iostream>

#include "cstdmf/stdmf.hpp"
#include "moo_dx.hpp"
#include "com_object_wrap.hpp"
#include "device_callback.hpp"
#include "index_buffer.hpp"

namespace Moo
{


/**
 * Base class for all dynamic index buffer functionality.
 */
class DynamicIndexBufferBase : public DeviceCallback
{
public:
	virtual ~DynamicIndexBufferBase();

	IndicesReference			lock( uint32 nLockIndices );
	IndicesReference			lock2( uint32 nLockIndices );

	/**
	 *	@return the index of the first index locked in the last lock call.
	 */
	uint32 lockIndex() const
	{
		return lockIndex_;
	}

	HRESULT						unlock();

	IndexBuffer					indexBuffer()
	{
		return indexBuffer_;
	}

	void						release();
	void						resetLock();

private:
	void						deleteUnmanagedObjects( );

	IndexBuffer					indexBuffer_;

	uint32						lockIndex_;
	bool						reset_;
	bool						locked_;
	int							lockBase_;
	uint						maxIndices_;
	DWORD						usage_;
	D3DFORMAT					format_;

protected:
	DynamicIndexBufferBase( DWORD usage, D3DFORMAT format );
	DynamicIndexBufferBase(const DynamicIndexBufferBase&);
	DynamicIndexBufferBase& operator=(const DynamicIndexBufferBase&);
};

/**
 *	Most commonly used 16 bit index buffer. Hardware based (video memory resident)
 */
class DynamicIndexBuffer16 : public DynamicIndexBufferBase
{
public:
	DynamicIndexBuffer16() : DynamicIndexBufferBase( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16 )
	{}
};

/**
 *	Most commonly used 32 bit index buffer. Hardware based (video memory resident)
 */
class DynamicIndexBuffer32 : public DynamicIndexBufferBase
{
public:
	DynamicIndexBuffer32() : DynamicIndexBufferBase( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX32 )
	{}
};

/**
 *	Software based 16 bit index buffer (system memory resident).
 */
class DynamicSoftwareIndexBuffer16 : public DynamicIndexBufferBase
{
public:
	DynamicSoftwareIndexBuffer16() : DynamicIndexBufferBase( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING, D3DFMT_INDEX16 )
	{}
};

/**
 *	Software based 32 bit index buffer (system memory resident).
 */
class DynamicSoftwareIndexBuffer32 : public DynamicIndexBufferBase
{
public:
	DynamicSoftwareIndexBuffer32() : DynamicIndexBufferBase( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING, D3DFMT_INDEX32 )
	{}
};


/**
 *  Inteface for using all types of dynamic index buffers.
 */
class DynamicIndexBufferInterface
{
public:
	DynamicIndexBufferBase&	get( D3DFORMAT format );

	void resetLocks();

private:
	DynamicIndexBuffer16 hwBuffer16_;
	DynamicIndexBuffer32 hwBuffer32_;

	DynamicSoftwareIndexBuffer16 swBuffer16_;
	DynamicSoftwareIndexBuffer32 swBuffer32_;
};


}

#endif
/*dynamic_index_buffer.hpp*/
