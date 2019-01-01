/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

// INLINE void VersionInfo::inlineFunction()
// {
// }

INLINE int VersionInfo::OSMajor( void ) const
{
	return osMajor_;
}


INLINE int VersionInfo::OSMinor( void ) const
{
	return osMinor_;
}


INLINE std::string
VersionInfo::OSName( void ) const
{
	return osName_;
}


INLINE std::string
VersionInfo::OSServicePack( void ) const
{
	return osServicePack_;
}


INLINE std::string
VersionInfo::DXName( void ) const
{
	return dxName_;
}


INLINE int VersionInfo::DXMajor( void ) const
{
	return dxMajor_;
}


INLINE int VersionInfo::DXMinor( void ) const
{
	return dxMinor_;
}


INLINE std::string VersionInfo::adapterDriver( void ) const
{
	return adapterDriver_;
}


INLINE std::string VersionInfo::adapterDesc( void ) const
{
	return adapterDesc_;
}


INLINE uint32 VersionInfo::adapterDriverMajorVer( void ) const
{
	return adapterDriverMajorVer_;
}


INLINE uint32 VersionInfo::adapterDriverMinorVer( void ) const
{
	return adapterDriverMinorVer_;
}


/*version_info.ipp*/
