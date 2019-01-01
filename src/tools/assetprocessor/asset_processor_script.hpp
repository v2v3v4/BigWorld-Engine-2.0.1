/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ASSET_PROCESSOR_SCRIPT_HPP
#define ASSET_PROCESSOR_SCRIPT_HPP

#include "moo/visual.hpp"

namespace Moo
{
	class BSPProxy;
	typedef SmartPointer< BSPProxy > BSPProxyPtr;	
};

namespace AssetProcessorScript
{
	void init();
	void fini();

	int populateWorldTriangles( Moo::Visual::Geometry& geometry, 
							RealWTriangleSet & ws,
							const Matrix & m,
							std::vector<std::string>& materialIDs );

	std::string generateBSP2( const std::string& resourceID,
								Moo::BSPProxyPtr& ret,
								std::vector<std::string>& retMaterialIDs,
								uint32& nVisualTris,
								uint32& nDegenerateTris );
	std::string replaceBSPData( const std::string& visualName,
								Moo::BSPProxyPtr pGeneratedBSP,
								std::vector<std::string>& bspMaterialIDs );
};

#endif