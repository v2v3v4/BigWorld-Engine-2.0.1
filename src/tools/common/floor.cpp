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

#include "cstdmf/debug.hpp"
#include "moo/render_context.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/effect_material.hpp"
#include "moo/texture_manager.hpp"

#include "resmgr/auto_config.hpp"

#include "../common/material_utility.hpp"
#include "../common/material_properties.hpp"

#include "appmgr/options.hpp"

DECLARE_DEBUG_COMPONENT2( "Floor", 0 );

static AutoConfigString s_lightonly_fx( "system/lightOnlyEffect" );
static AutoConfigString s_floor_texture( "system/defaultFloorTexture" );

#include "floor.hpp"

Floor::Floor( const std::string& textureName /* = "" */ ) :
    textureName_        ( textureName ),
    meshDirty_          ( true ),
    material_           ( NULL ),
    materialDirty_      ( true ),
    visible_			( true )
{
	BW_GUARD;

	transform_.setIdentity();

    if (textureName == "")
	{
		textureName_ = s_floor_texture;
	}
}

Floor::~Floor()
{
	BW_GUARD;

    if (material_)
        cleanupMaterial( );
}

void
Floor::render( Moo::EffectMaterialPtr material /* = NULL */ )
{
	BW_GUARD;

    uint32 preMem = 0;

    if (material == NULL)
    {
        if ( materialDirty_ )
		{
			updateMaterial( );
		}
		material = material_; // Use the floor's effect
    }
    
    if ( meshDirty_ )
    {
        updateMesh( );
    }

    if ( visible_ )
    {
    	Moo::rc().push();
    	Moo::rc().preMultiply( transform_ );

        Moo::EffectVisualContext::instance().initConstants();
		Moo::EffectVisualContext::instance().staticLighting(false);

        if (material->begin())
        {
          for (uint32 i = 0; i < material_->nPasses(); i++)
          {
                  material->beginPass( i );
                  mesh_.drawEffect();
                  material->endPass();
          }
          material->end();
        }
        else
        {
                ERROR_MSG( "Drawing of floor failed.\n" );
        }


	    Moo::rc().pop( );
    }
}

void
Floor::setTextureName( const std::string &textureFileName )
{
	BW_GUARD;

    if (textureFileName != textureName_)
    {
        textureName_ = textureFileName;
        materialDirty_ = true;
    }
}

std::string&
Floor::getTextureName( void )
{
    return textureName_;
}

void
Floor::updateMaterial( void )
{
	BW_GUARD;

	//Out with the old...
	material_ = NULL;
   
	//In with the new...
	material_ = new Moo::EffectMaterial();
	material_->initFromEffect( s_lightonly_fx, textureName_ );

    materialDirty_ = false;
}

void
Floor::drawSquare( Moo::VertexXYZNDUV& v, float x, float z, float step, float scale )
{
	BW_GUARD;

	v.pos_.set( x, 0, z );
    v.uv_.set( (x)/scale, -(z)/scale );
    mesh_.push_back( v );

    v.pos_.set( x, 0,  z + step );
    v.uv_.set( (x)/scale,  -(z + step)/scale );
    mesh_.push_back( v );

    v.pos_.set(  x + step, 0, z + step );
    v.uv_.set(  (x + step)/scale, -(z + step)/scale );
    mesh_.push_back( v );

    v.pos_.set( x, 0, z );
    v.uv_.set( (x)/scale, -(z)/scale );
    mesh_.push_back( v );

    v.pos_.set( x + step, 0, z + step );
    v.uv_.set( (x + step)/scale, -(z + step)/scale );
    mesh_.push_back( v );

    v.pos_.set( x + step, 0, z );
    v.uv_.set( (x + step)/scale, -(z)/scale );
    mesh_.push_back( v );
}

void
Floor::updateMesh( void )
{
	BW_GUARD;

    mesh_.clear();

    //These are horribly tweaked magic numbers;
	//Don't mess with them unless you know exactly what you are doing...

	float gridSize = Options::getOptionFloat( "settings/floorGrid", 1.f );
		
	const float lowSpan = 64.f;
	const float lowStep = 4.f;

	const float midSpan = 32.f;
    const float midStep = 2.f;

	const float highSpan = 12.f;
    const float highStep = 0.5f;

/* This code is useful for debugging the tri counts
int high = 2*(highSpan*highSpan)/(highStep*highStep);
int mid = 2*((midSpan*midSpan)-(highSpan*highSpan))/(midStep*midStep);
int low = 2*((lowSpan*lowSpan)-(midSpan*midSpan))/(lowStep*lowStep);

char buf[256];
bw_snprintf( buf, sizeof(buf), "hi =%d\nmid=%d\nlow=%d\ntot=%d",high, mid, low, high+mid+low);
MessageBox( NULL, buf, "Floor Triangle Counts", MB_OK );
//*/

    Moo::VertexXYZNDUV v;
    v.normal_ = Vector3( 0, 1, 0 );
	v.colour_ = 0xffffffff;

    for ( float z = -highSpan; z < highSpan; z += highStep )
    {
        for ( float x = -highSpan; x < highSpan; x += highStep )
        {
			drawSquare( v, x, z, highStep, gridSize );
        }
    }

	for ( float x = -midSpan; x < midSpan; x += midStep )
    {
		for ( float z = -midSpan; z < -highSpan; z += midStep )
		{
			drawSquare( v, x, z, midStep, gridSize );
		}
		for ( float z = highSpan; z < midSpan; z += midStep )
		{
			drawSquare( v, x, z, midStep, gridSize );
		}
	}

	for ( float z = -highSpan; z < highSpan; z += midStep )
    {
		for ( float x = -midSpan; x < -highSpan; x += midStep )
		{
			drawSquare( v, x, z, midStep, gridSize );
		}
		for ( float x = highSpan; x < midSpan; x += midStep )
		{
			drawSquare( v, x, z, midStep, gridSize );
		}
	}
	
	for ( float x = -lowSpan; x < lowSpan; x += lowStep )
    {
		for ( float z = -lowSpan; z < -midSpan; z += lowStep )
		{
			drawSquare( v, x, z, lowStep, gridSize );
		}
		for ( float z = midSpan; z < lowSpan; z += lowStep )
		{
			drawSquare( v, x, z, lowStep, gridSize );
		}
	}

	for ( float z = -midSpan; z < midSpan; z += lowStep )
    {
		for ( float x = -lowSpan; x < -midSpan; x += lowStep )
		{
			drawSquare( v, x, z, lowStep, gridSize );
		}
		for ( float x = midSpan; x < lowSpan; x += lowStep )
		{
			drawSquare( v, x, z, lowStep, gridSize );
		}
	}
    
    meshDirty_ = false;
}

void
Floor::cleanupMaterial( )
{
	BW_GUARD;

    material_ = NULL;
}

void Floor::location( const Vector3 & l )
{
	BW_GUARD;

	transform_.setTranslate( l );
}


const Vector3& Floor::location() const
{
	return transform_.applyToOrigin();
}
