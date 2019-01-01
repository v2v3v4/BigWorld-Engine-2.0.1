/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __MOO_UNIT_TEST_SHADER_TEST_HPP__
#define __MOO_UNIT_TEST_SHADER_TEST_HPP__

#include <string>
#include <cstdmf/smartpointer.hpp>

namespace Moo
{
	class EffectMaterial;
	typedef SmartPointer< EffectMaterial > EffectMaterialPtr;
}

class ShaderTest
{
public:
	ShaderTest();

	Moo::EffectMaterialPtr init( const std::string& effectFileName );    
	void fini();

	bool test( uint32 technique, const std::string& refImageFile );

	static void getImageFileNameForEffect( const std::string&	effectFileName,
										   uint32				techniqueIndex,
										   std::string&			refImageFileName );
private:
	Moo::EffectMaterialPtr pEffect_;
};

#endif