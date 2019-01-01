/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _FLOOR_H_
#define _FLOOR_H_

#include "moo/forward_declarations.hpp"
#include "moo/moo_math.hpp"
#include "romp/custom_mesh.hpp"
#include "moo/vertex_formats.hpp"

/*#include "moo/render_context.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/effect_material.hpp"
#include "moo/texture_manager.hpp"*/

namespace Moo
{
    class EffectMaterial;
};

/**
 * Floor class.
 *
 * Used by the editors, this class provides the following options:
 *
 * Opacity
 * Floor Texture
 * Texture Tiling options
 */

class Floor
{
public:
	explicit Floor( const std::string& textureName = "" );
	~Floor();

    void			render( Moo::EffectMaterialPtr material = NULL );

    void			setTextureName( const std::string &textureFileName );
    std::string&	getTextureName( void );

    void			visible( bool state )	{ visible_ = state; }
    bool			visible() const			{ return visible_; }

    void			location( const Vector3 & l );
    const Vector3&	location() const;

private:
    void drawSquare( Moo::VertexXYZNDUV& v, float x, float z, float step, float scale );

	//private methods
    void	updateMaterial( void );
    void	updateMesh( void );
    void	cleanupMaterial( void );

    //serializable options
    std::string		textureName_;
    bool			visible_;
    Matrix			transform_;

    //transient variables
    CustomMesh< Moo::VertexXYZNDUV >	mesh_;
    bool								meshDirty_;

    Moo::EffectMaterialPtr	material_;
    bool					materialDirty_;
};

#endif
