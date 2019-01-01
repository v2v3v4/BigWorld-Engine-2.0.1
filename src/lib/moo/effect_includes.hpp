/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __EFFECT_INCLUDES_HPP__
#define __EFFECT_INCLUDES_HPP__

#include "moo_dx.hpp"

namespace Moo
{

/**
*	This class implements the ID3DXInclude interface used for loading effect 
*	files and their includes.
*/
class EffectIncludes : public ID3DXInclude, public ReferenceCount
{
public:
	EffectIncludes()
	{
	}
	~EffectIncludes()
	{
	}

	/*
	*	The overridden open method
	*/
	HRESULT __stdcall Open( D3DXINCLUDE_TYPE IncludeType, 
							LPCSTR	pFileName,
							LPCVOID pParentData, 
							LPCVOID *ppData, 
							UINT	*pBytes );
	HRESULT __stdcall Close( LPCVOID pData );
	void resetDependencies() { includesNames_.clear(); }
	std::list< std::string >& dependencies() { return includesNames_; }
	void currentPath( const std::string& currentPath ) { currentPath_ = currentPath; }
	const std::string& currentPath( ) const { return currentPath_; }

private:
	typedef std::pair<LPCVOID, BinaryPtr> IncludeFile;
	std::vector< IncludeFile > includes_;
	std::list< std::string > includesNames_;
	std::string currentPath_;
};

}

#endif