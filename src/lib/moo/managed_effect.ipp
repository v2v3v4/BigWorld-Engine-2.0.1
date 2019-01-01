/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// managed_effect.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


namespace Moo
{

/**
 * This method returns the handle of the currently used technique.
 */
INLINE D3DXHANDLE ManagedEffect::getCurrentTechnique() const
{
	return hCurrentTechnique_;
}

/**
 * This method finds the technique with the given handle, returns NULL if not
 * found.
 */
INLINE TechniqueInfo* 
	ManagedEffect::findTechniqueInfo( D3DXHANDLE handle )
{	
	BW_GUARD_PROFILER( ME_findTechniqueInfo );
	
	// look up technique using "magic" handle which could be a string or an int
	for (	TechniqueInfoCache::iterator it = techniques_.begin();
			it != techniques_.end(); 
			it++ )
	{
		if ( handle == it->handle_ )
			return &(*it);
	}
	
	// its a string, do it the slow way
	D3DXTECHNIQUE_DESC techniqueDesc;
	if ( SUCCEEDED( pEffect_->GetTechniqueDesc( handle , &techniqueDesc ) ) )
	{	
		for (	TechniqueInfoCache::iterator it = techniques_.begin();
				it != techniques_.end(); 
				it++ )
		{
			if ( bw_stricmp( techniqueDesc.Name, it->name_.c_str() ) == 0 )
			{
				return &(*it);
			}
		}
	}
	
	return NULL;
}

} // namespace Moo

// managed_effect.ipp
