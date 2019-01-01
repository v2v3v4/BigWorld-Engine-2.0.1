/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VERSION_INFO_HPP
#define VERSION_INFO_HPP

#include "cstdmf/stdmf.hpp"

#include "psapi.h"

#include <windows.h>

#include <iostream>

typedef BOOL (WINAPI * VI_GETPROCESSMEMORYINFO)(HANDLE, PROCESS_MEMORY_COUNTERS*, DWORD);
#define VI_GETPROCESSMEMORYINFO_NAME	"GetProcessMemoryInfo"


/**
 *	This class displays driver and operating system info
 */
class VersionInfo
{
public:
	VersionInfo();
	~VersionInfo();

	static VersionInfo&	instance( void );

	///This must be called to initialise all values
	void		queryAll( void );

	///Accessors for individual values
	std::string OSName( void ) const;
	int			OSMajor( void ) const;
	int			OSMinor( void ) const;
	std::string OSServicePack( void ) const;

	std::string DXName( void ) const;
	int			DXMajor( void ) const;
	int			DXMinor( void ) const;


	///Global memory
	int			totalPhysicalMemory( void ) const;
	int			availablePhysicalMemory( void ) const;

	int			totalVirtualMemory( void ) const;
	int			availableVirtualMemory( void ) const;

	int			totalPagingFile( void ) const;
	int			availablePagingFile( void ) const;

	int			memoryLoad( void ) const;

	///Process memory
	int			pageFaults( void ) const;
	int			peakWorkingSet( void ) const;
	int			workingSet( void ) const;
	int			quotaPeakedPagePoolUsage( void ) const;
	int			quotaPagePoolUsage( void ) const;
	int			quotaPeakedNonPagePoolUsage( void ) const;
	int			quotaNonPagePoolUsage( void ) const;
	int			peakPageFileUsage( void ) const;
	int			pageFileUsage( void ) const;

	int			workingSetRefetched( void ) const;

	///Adapter information
	std::string	adapterDriver( void ) const;
	std::string adapterDesc( void ) const;
	uint32		adapterDriverMajorVer( void ) const;
	uint32		adapterDriverMinorVer( void ) const;

		

private:
	void		queryOS( void );
	void		queryDX( void );
	void		queryMemory( void ) const;
	void		queryHardware( void );
	DWORD		queryDXVersion( void );

	VersionInfo(const VersionInfo&);
	VersionInfo& operator=(const VersionInfo&);

	int			osMajor_;
	int			osMinor_;
	std::string osName_;
	std::string osServicePack_;
	int			dxMajor_;
	int			dxMinor_;
	std::string dxName_;

	std::string adapterDriver_;
	std::string adapterDesc_;
	uint32		adapterDriverMajorVer_;
	uint32		adapterDriverMinorVer_;
	
	mutable VI_GETPROCESSMEMORYINFO vi_getProcessMemoryInfo_;
	mutable HINSTANCE loadedDll_;
	mutable MEMORYSTATUS memoryStatus_;
	mutable PROCESS_MEMORY_COUNTERS processMemoryStatus_;
	mutable unsigned long lastMemoryCheck_;

	friend std::ostream& operator<<(std::ostream&, const VersionInfo&);
};

#ifdef CODE_INLINE
#include "version_info.ipp"
#endif




#endif
/*version_info.hpp*/
