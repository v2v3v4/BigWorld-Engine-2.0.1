/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COM_OBJECT_WRAP_IPP
#define COM_OBJECT_WRAP_IPP

/**
 *	This is the ComObjectWrap constructor.  It sets the underyling COM object
 *	to NULL.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE>::ComObjectWrap():
	comObject_(NULL)
{
}


/**
 *	This is the ComObjectWrap constructor.
 *
 *	@param object			The object being wrapped.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE>::ComObjectWrap(ComObject *object):
	comObject_(NULL)
{
	pComObject(object);
}


/**
 *	This is the ComObjectWrap copy constructor.
 *
 *  @param other			The ComObjectWrap to copy from.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE>::ComObjectWrap(ComObjectWrap const &other):
	comObject_(NULL)
{
	copy(other);
}


/**
 *	This is the ComObjectWrap destructor.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE>::~ComObjectWrap()
{
	release();
}


/**
 *	This resets the ComObjectWrap to NULL.
 *
 *	@param n				This should be NULL.
 *	@returns				*this.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE> &
ComObjectWrap<COMOBJECTTYPE>::operator=(int n)
{
	if(n != 0)
		throw "n must be 0 here for a null pointer";
	release();
	return *this;
}


/**
 *	This is the ComObjectWrap assignment operator.
 *
 *  @param other			The ComObjectWrap to copy from.
 *  @returns				*this.
 */
template<typename COMOBJECTTYPE>
inline
ComObjectWrap<COMOBJECTTYPE>& 
ComObjectWrap<COMOBJECTTYPE>::operator=
(
	ComObjectWrap const &other
)
{
	if (comObject_ != other.comObject_)
		copy(other);
	return *this;
}


/**
 *	This compares the underlying COM objects.
 *
 *  @param other			The other ComObjectWrap to compare against.
 *	@returns				True if the wrappers point to the same
 *							COM object.
 */
template<typename COMOBJECTTYPE>
inline
bool ComObjectWrap<COMOBJECTTYPE>::operator==(ComObjectWrap const &other) const
{
	return comObject_ == other.comObject_;
}


/**
 *	This returns whether the wrapper points to anything.
 *
 *	@param n				Not used.  This should be set to NULL.
 *	@returns				True if the COM object is NULL.
 */
template<typename COMOBJECTTYPE>
inline
bool ComObjectWrap<COMOBJECTTYPE>::operator!=(int n) const
{
	MF_ASSERT(n == 0);
	return comObject_ != NULL;
}


/**
 *	This determines whether the ComObjectWrap points to something.
 *
 *  @returns				True if the ComObjectWrap is not to the
 *							NULL object.
 */
template<typename COMOBJECTTYPE>
inline
bool ComObjectWrap<COMOBJECTTYPE>::hasComObject() const
{
	return comObject_ != NULL;
}

#if ENABLE_RESOURCE_COUNTERS
/**
 *	This class keep tracks of all direct3d resources
 */
class D3DResourceTrack : public IUnknown
{
	ResourceCounters::Handle resCntHandle_;
	DWORD pool_;
	DWORD size_;
	ULONG refCount_;
public:
	D3DResourceTrack( const std::string& allocator, DWORD pool, DWORD size )
		: pool_( pool ), size_( size ), refCount_( 0 )
	{
		resCntHandle_ = RESOURCE_COUNTER_NEWHANDLE(allocator);
		if (ResourceCounters::instance().isValid())
		{
			ResourceCounters::instance().add
			(
				ResourceCounters::DescriptionPool
				(
					ResourceCounters::instance().description(resCntHandle_), 
					pool
				), 
				size
			);
		}
	}
	~D3DResourceTrack()
	{
		if (ResourceCounters::instance().isValid())
		{
			ResourceCounters::instance().subtract
			(
				ResourceCounters::DescriptionPool
				(
					ResourceCounters::instance().description(resCntHandle_), 
					pool_
				), 
				size_
			);
		}
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement( (LONG*)&refCount_ );
	}
	HRESULT STDMETHODCALLTYPE QueryInterface( const IID & riid, void **ppvObj )
	{
		HRESULT hr = E_NOINTERFACE;
		if( riid == __uuidof(IUnknown) )
		{
			AddRef();
			*ppvObj = this;
			hr = S_OK;
		}
		return hr;
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		if( InterlockedDecrement( (LONG*)&refCount_ ) == 0 )
		{
			delete this;
			return 0;
		}
		return refCount_;
	}
};
#endif

/**
 *	This sets the allocator (resource counter) for this COM object and
 *	adds the size of the object to the appropriate pool.
 *
 *  @param desc				The resource counter that the COM object
 *							should be using.
 */
template<typename COMOBJECTTYPE>
inline
void ComObjectWrap<COMOBJECTTYPE>::addAlloc(std::string const &desc)
{
#if ENABLE_RESOURCE_COUNTERS
	IUnknown* unk = new D3DResourceTrack( desc, pool(), size() );
	pComObject()->SetPrivateData( __uuidof(IUnknown), unk, sizeof( unk ), D3DSPD_IUNKNOWN );
#endif
}


/**
 *	This sets a new COM object to wrap.
 *
 *  @param object			The new COM object to wrap.
 */
template<typename COMOBJECTTYPE>
inline
void ComObjectWrap<COMOBJECTTYPE>::pComObject(ComObject *object)
{
	if (comObject_ == object)
		return;

	release();
	comObject_ = object;
	addRef();
}


/**
 *	This gets the COM object that is being wrapped.
 *
 *  @returns				The COM object that is being wrapped.
 */
template<typename COMOBJECTTYPE>
inline
typename ComObjectWrap<COMOBJECTTYPE>::ComObjectPtr 
ComObjectWrap<COMOBJECTTYPE>::pComObject() const
{
	return comObject_;
}


/**
 *	This gets the COM object that is begin wrapped.
 *
 *  @returns				The COM object that is being wrapped.
 */
template<typename COMOBJECTTYPE>
inline
typename ComObjectWrap<COMOBJECTTYPE>::ComObjectPtr 
ComObjectWrap<COMOBJECTTYPE>::operator->() const
{
	return comObject_;
}


/**
 *	This gets a reference to the COM object being wrapped.
 *
 *  @returns				A reference to the object being wrapped.
 */
template<typename COMOBJECTTYPE>
inline
typename ComObjectWrap<COMOBJECTTYPE>::ComObjectRef 
ComObjectWrap<COMOBJECTTYPE>::operator*() const
{
	return *comObject_;
}


/**
 *	This gets a pointer to the COM object being wrapped.
 *
 *  @returns				A pointer to the object being wrapped.
 */
template<typename COMOBJECTTYPE>
inline
typename ComObjectWrap<COMOBJECTTYPE>::ComObjectPtr * 
ComObjectWrap<COMOBJECTTYPE>::operator&()
{
	return &comObject_;
}


/**
 *	This protected function releases the current COM object if there is one and
 *	assigns this object to another.  It is used by the copy constructor and
 *	assignment operators.
 *
 *	@param other			The ComObjectWrap to copy.
 *  @param bAddRef			Boolean indicating whether to add a reference count
 *                          on copy. Default is true.
 */
template<typename COMOBJECTTYPE>
inline
void ComObjectWrap<COMOBJECTTYPE>::copy(ComObjectWrap const &other, bool bAddRef/*=true*/)
{
	release();

	comObject_    = other.comObject_;

	if (bAddRef)
		addRef();
}


/**
 *	This increases the reference count of the COM object by one.
 */
template<typename COMOBJECTTYPE>
inline
void ComObjectWrap<COMOBJECTTYPE>::addRef()
{
	if (comObject_ != NULL)
	{
		comObject_->AddRef();
	}
}

/**
 *	This decreases the reference count of the COM object by one and makes the
 *	wrapper point to NULL.
 */
template<typename COMOBJECTTYPE>
inline
void ComObjectWrap<COMOBJECTTYPE>::release()
{
	BW_GUARD;
	
	if (comObject_ != NULL)
	{
		comObject_->Release();
		comObject_ = NULL;
	}	
}

// Specialisations of pool and size for various DirectX components
#if ENABLE_RESOURCE_COUNTERS


/**
 *	This gets the DirectX pool of the object being wrapped.
 *
 *  @returns				The DirectX pool of the object being 
 *							wrapped.
 */
template<typename COMOBJECTTYPE>
inline uint ComObjectWrap<COMOBJECTTYPE>::pool() const
{
	return ResourceCounters::DEFAULT;
}


/**
 *	This gets the memory size of the object being wrapped.
 *
 *  @returns				The size of the object being wrapped.
 *							This is specialised for many object types,
 *							but needs to be specialised for any
 *							future types.
 */
template<typename COMOBJECTTYPE>
inline uint ComObjectWrap<COMOBJECTTYPE>::size() const
{
	return 0;
}


/**
 *	This is the specialisation of pool for effects.
 */
template<> inline uint ComObjectWrap<ID3DXEffect>::pool() const
{
	return ResourceCounters::DEFAULT;
}


/**
 *	This is the specialisation of size for effects.
 */
template<> inline uint ComObjectWrap<ID3DXEffect>::size() const
{
	// Default size of 1KB
	return 1024;
}


/**
 *	This is the specialisation of pool for vertex buffers.
 */
template<> inline uint ComObjectWrap<DX::VertexBuffer>::pool() const
{
	// Get a vertex buffer description
	D3DVERTEXBUFFER_DESC vbDesc;
	comObject_->GetDesc(&vbDesc);

	return (uint)vbDesc.Pool;
}


/**
 *	This is the specialisation of size for vertex buffers.
 */
template<> inline uint ComObjectWrap<DX::VertexBuffer>::size() const
{
	// Get a vertex buffer description
	D3DVERTEXBUFFER_DESC vbDesc;
	comObject_->GetDesc(&vbDesc);

	return vbDesc.Size;
}


/**
 *	This is the specialisation of pool for index buffers.
 */
template<> inline uint ComObjectWrap<DX::IndexBuffer>::pool() const
{
	// Get a index buffer description
	D3DINDEXBUFFER_DESC ibDesc;
	comObject_->GetDesc(&ibDesc);

	return (uint)ibDesc.Pool;
}


/**
 *	This is the specialisation of size for index buffers.
 */
template<> inline uint ComObjectWrap<DX::IndexBuffer>::size() const
{
	// Get a vertex buffer description
	D3DINDEXBUFFER_DESC ibDesc;
	comObject_->GetDesc(&ibDesc);

	return ibDesc.Size;
}


/**
 *	This is the specialisation of pool for textures.
 */
template<> inline uint ComObjectWrap<DX::Texture>::pool() const
{
	// Get a surface to determine pool
	D3DSURFACE_DESC surfaceDesc;
	comObject_->GetLevelDesc(0, &surfaceDesc);

	return (uint)surfaceDesc.Pool;
}


/**
 *	This is the specialisation of size for textures.
 */
template<> inline uint ComObjectWrap<DX::Texture>::size() const
{
	return DX::textureSize(comObject_);
}

/**
 *	This function gets the pool & format of a texture
 */
inline D3DSURFACE_DESC getSurfaceDesc( DX::BaseTexture* base )
{
	DX::Texture* tex;
	DX::CubeTexture* cubeTex;
	D3DSURFACE_DESC desc = { D3DFMT_UNKNOWN };
	if( SUCCEEDED( base->QueryInterface( __uuidof( DX::Texture ), (LPVOID*)&tex ) ) )
	{
		tex->GetLevelDesc( 0, &desc );
		tex->Release();
	}
	else if( SUCCEEDED( base->QueryInterface( __uuidof( DX::CubeTexture ), (LPVOID*)&cubeTex ) ) )
	{
		cubeTex->GetLevelDesc( 0, &desc );
		cubeTex->Release();
	}
	else
		throw "if the code reaches here, please refine this function";
	return desc;
}

/**
 *	This is the specialisation of pool for textures.
 */
template<> inline uint ComObjectWrap<DX::BaseTexture>::pool() const
{
	// Get a surface to determine pool
	D3DSURFACE_DESC surfaceDesc = getSurfaceDesc( comObject_ );

	return (uint)surfaceDesc.Pool;
}


/**
 *	This is the specialisation of size for textures.
 */
template<> inline uint ComObjectWrap<DX::BaseTexture>::size() const
{
	// Determine the mip-map texture size scaling factor
	double mipmapScaler = ( 4 - pow(0.25, (double)comObject_->GetLevelCount() - 1 ) ) / 3;

	// Get a surface to determine the width, height, and format
	D3DSURFACE_DESC surfaceDesc = getSurfaceDesc( comObject_ );

	// Get the surface size
	uint32 surfaceSize = DX::surfaceSize(surfaceDesc);

	// Track memory usage
	return (uint)(surfaceSize * mipmapScaler);
}

#endif // ENABLE_RESOURCE_COUNTERS


#endif // COM_OBJECT_WRAP_IPP
