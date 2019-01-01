/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _RESOURCE_FILE_DIALOG__
#define _RESOURCE_FILE_DIALOG__

#include "resmgr/bwresource.hpp"
#include "resmgr/string_provider.hpp"

class ResourceFileDialog : public CFileDialog
{
public:
	ResourceFileDialog( BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL )
		: CFileDialog( bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags | OFN_EXPLORER | OFN_NOCHANGEDIR,
		lpszFilter,pParentWnd )
	{}
	virtual BOOL OnFileNameOK()
	{
		BW_GUARD;

		std::string filename = BWResource::dissolveFilename( (LPCTSTR)GetPathName() );
		if( filename[ 1 ] == ':' ) // absolute folder
		{
			std::string msg = L("CONTROLS/RESOURCE_FILE_DIALOG/MUST_BE_IN_RESOURCE_DIR");
			for( int i = 0; i < BWResource::getPathNum(); ++i )
			{
				msg += '\t';
				msg += BWResource::getPath( i );
				if( i != BWResource::getPathNum() - 1 )
					msg += L("CONTROLS/RESOURCE_FILE_DIALOG/OR");
			}
			std::replace( msg.begin(), msg.end(), '/', '\\' );
			MessageBox( msg.c_str() );
			return 1;
		}
		return 0;
	}
};

#endif//_RESOURCE_FILE_DIALOG__
