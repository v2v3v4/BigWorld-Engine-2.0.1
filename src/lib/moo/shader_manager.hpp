/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SHADER_MANAGER_HPP
#define SHADER_MANAGER_HPP

#include <iostream>
#include <map>
#include "cstdmf/stdmf.hpp"
#include "cstdmf/stringmap.hpp"
#include "cstdmf/init_singleton.hpp"
#include "resmgr/datasection.hpp"
#include "shader_set.hpp"
#include "vertex_declaration.hpp"

/**
 * TODO: should ShaderManager be namespaced in Moo?
 */
namespace Moo { class VertexDeclaration; }


/**
 * TODO: to be documented.
 */
class ShaderManager : public InitSingleton< ShaderManager >
{
public:

	ShaderManager();
	~ShaderManager();

	/// Get a smartpointer to a shaderset.
	ShaderSetPtr		shaderSet( const std::string& vertexFormat, const std::string& shaderType, 
		const Moo::VertexDeclaration* pDecl = NULL );

private:

	ShaderSetPtr		loadSet( const std::string& vertexFormat, const std::string& shaderType, 
		const Moo::VertexDeclaration* pDecl = NULL );

	DataSectionPtr		shaderRoot_;

	typedef StringMap< ShaderSetPtr > ShaderSetMap;
	typedef StringMap< ShaderSetMap > ShaderFormatMap;

	ShaderFormatMap		sets_;

	ShaderManager(const ShaderManager&);
	ShaderManager& operator=(const ShaderManager&);

	friend std::ostream& operator<<(std::ostream&, const ShaderManager&);
};


#endif
/*shader_manager.hpp*/
