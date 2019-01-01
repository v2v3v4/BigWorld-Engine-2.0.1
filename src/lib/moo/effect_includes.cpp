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
#include "effect_includes.hpp"

#include "effect_manager.hpp"

namespace Moo
{

/*
*	The overridden open method
*/
HRESULT __stdcall EffectIncludes::Open( D3DXINCLUDE_TYPE IncludeType, 
										LPCSTR pFileName,
										LPCVOID pParentData, 
										LPCVOID *ppData, 
										UINT *pBytes )
{
	BW_GUARD;
	// Make sure our return values are initialised
	*ppData = NULL;
	*pBytes = 0;
	BinaryPtr pData;

	//  Commented the 'dissolveFilename' line below as it seems it's not needed
	//		std::string relativeFilename = BWResource::dissolveFilename( pFileName );
	DataSectionPtr pFile = BWResource::openSection( pFileName );
	if (pFile)
	{
		//			DEBUG_MSG( "EffectIncludes::Open - opening %s\n", pFileName.c_str() );
		pData = pFile->asBinary();
	}
	else
	{
		std::string name = BWResource::getFilename( pFileName );
		std::string pathname = currentPath_ + name;
		pFile = BWResource::openSection( pathname );
		if (pFile)
		{
			pData = pFile->asBinary();
		}
		else
		{

			const EffectManager::IncludePaths& paths = EffectManager::instance().includePaths();
			EffectManager::IncludePaths::const_iterator it = paths.begin();
			EffectManager::IncludePaths::const_iterator end = paths.end();
			while (it != end)
			{
				pathname = *(it++) + name;
				pFile = BWResource::openSection( pathname );
				if (pFile)
				{
					pData = pFile->asBinary();
					it = end;
				}
			}
		}
	}

	if (pData)
	{
		*ppData = pData->data();
		*pBytes = (UINT)pData->len();
		includes_.push_back( std::make_pair( *ppData, pData ) );

		includesNames_.push_back( pFileName );
	}
	else
	{
		ERROR_MSG( "EffectIncludes::Open - Failed to load %s\n", pFileName );
		return E_FAIL;
	}

	return S_OK;
}

HRESULT __stdcall EffectIncludes::Close( LPCVOID pData )
{
	BW_GUARD;
	std::vector< IncludeFile >::iterator it = includes_.begin();
	std::vector< IncludeFile >::iterator end = includes_.end();
	while (it != end)
	{
		if ((*it).first == pData)
		{
			includes_.erase( it );
			break;
		}
		it++;
	}
	return S_OK;
}

}