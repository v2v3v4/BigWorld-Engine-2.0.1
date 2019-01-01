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

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)

#include "particle_system_renderer.hpp"
#include "amp_particle_renderer.hpp"
#include "blur_particle_renderer.hpp"
#include "mesh_particle_renderer.hpp"
#include "point_sprite_particle_renderer.hpp"
#include "sprite_particle_renderer.hpp"
#include "trail_particle_renderer.hpp"
#include "visual_particle_renderer.hpp"
#include "romp/custom_mesh.hpp"
#include "moo/dynamic_vertex_buffer.hpp"
#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

#ifndef CODE_INLINE
#include "particle_system_renderer.ipp"
#endif

ParticleSystemRenderer * ParticleSystemRenderer::createRendererOfType(const std::string type, DataSectionPtr ds )
{
	BW_GUARD;
	if (type == SpriteParticleRenderer::nameID_)
	{
		return new SpriteParticleRenderer("");
	}
	else if (type == PointSpriteParticleRenderer::nameID_)
	{
		return new PointSpriteParticleRenderer("");
	}
	else if (type == MeshParticleRenderer::nameID_)
	{
		//Note : this switch should be deprecated in BigWorld 1.9.
		//It has been introduced for 1.8 and handles legacy
		//particle systems saved previous to 1.8
		if (ds)
		{
			std::string visualName = ds->readString("visualName_");
			if (MeshParticleRenderer::isSuitableVisual( visualName ))
				return new MeshParticleRenderer();
			else
			{
				INFO_MSG( "Mesh Particle Renderer using visual %s was "
					"converted to using a VisualParticleRenderer. You may "
					"want to check if this visual needs re-exporting as a "
					"mesh particle type.  If not, you should re-save the "
					"particle system file from particle edtior\n",
					visualName.c_str() );
				return new VisualParticleRenderer();
			}
		}
		else
		{
			return new MeshParticleRenderer();
		}
	}
	else if (type == VisualParticleRenderer::nameID_)
	{
		return new VisualParticleRenderer();
	}
	else if (type == AmpParticleRenderer::nameID_)
	{
		return new AmpParticleRenderer();
	}
	else if (type == TrailParticleRenderer::nameID_)
	{
		return new TrailParticleRenderer();
	}
	else if (type == BlurParticleRenderer::nameID_)
	{
		return new BlurParticleRenderer();
	}

	return NULL;
}


PyParticleSystemRendererPtr PyParticleSystemRenderer::createPyRenderer( ParticleSystemRendererPtr pR )
{
	BW_GUARD;
	const std::string& type = pR->nameID();

	if (type == SpriteParticleRenderer::nameID_)
	{
		SpriteParticleRenderer* spr = static_cast<SpriteParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PySpriteParticleRenderer(spr), true);
		return pr;
	}
	else if (type == PointSpriteParticleRenderer::nameID_)
	{
		PointSpriteParticleRenderer* spr = static_cast<PointSpriteParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyPointSpriteParticleRenderer(spr), true);
		return pr;
	}
	else if (type == MeshParticleRenderer::nameID_)
	{
		MeshParticleRenderer* mpr = static_cast<MeshParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyMeshParticleRenderer(mpr), true);
		return pr;
	}
	else if (type == VisualParticleRenderer::nameID_)
	{
		VisualParticleRenderer* mpr = static_cast<VisualParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyVisualParticleRenderer(mpr), true);
		return pr;		
	}
	else if (type == AmpParticleRenderer::nameID_)
	{
		AmpParticleRenderer* apr = static_cast<AmpParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyAmpParticleRenderer(apr), true);
		return pr;		
	}
	else if (type == TrailParticleRenderer::nameID_)
	{
		TrailParticleRenderer* tpr = static_cast<TrailParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyTrailParticleRenderer(tpr), true);
		return pr;
	}
	else if (type == BlurParticleRenderer::nameID_)
	{
		BlurParticleRenderer* bpr = static_cast<BlurParticleRenderer*>(&*pR);
		PyParticleSystemRendererPtr pr(new PyBlurParticleRenderer(bpr), true);
		return pr;		
	}

	return NULL;
}


/**
 *	This static method returns all of the resources that will be required to
 *	create the particle renderer.  It is designed so that the resources
 *	can be loaded in the loading thread before constructing the particle system
 */
/*static*/ void ParticleSystemRenderer::prerequisitesOfType(DataSectionPtr pSect,
												std::set<std::string>& output)
{
	BW_GUARD;
	const std::string& type = pSect->sectionName();

	if (type == SpriteParticleRenderer::nameID_)
	{
		SpriteParticleRenderer::prerequisites(pSect, output);
	}
	else if (type == PointSpriteParticleRenderer::nameID_)
	{
		PointSpriteParticleRenderer::prerequisites(pSect, output);
	}
	else if (type == MeshParticleRenderer::nameID_)
	{
		MeshParticleRenderer::prerequisites(pSect, output);		
	}
	else if (type == VisualParticleRenderer::nameID_)
	{
		VisualParticleRenderer::prerequisites(pSect, output);
	}
	else if (type == AmpParticleRenderer::nameID_)
	{
		AmpParticleRenderer::prerequisites(pSect, output);
	}
	else if (type == TrailParticleRenderer::nameID_)
	{
		TrailParticleRenderer::prerequisites(pSect, output);
	}
	else if (type == BlurParticleRenderer::nameID_)
	{
		BlurParticleRenderer::prerequisites(pSect, output);
	}
}



static bool useUP = true;
static bool firstTime = true;

static DogWatch s_psParticleRenderer( "PSParticles" );
static DogWatch s_drawNormal( "DrawNormal" );
static DogWatch s_drawRotated( "DrawRotated" );
static DogWatch s_build( "build" );
static DogWatch s_draw( "draw" );
	
void ParticleSystemRenderer::serialise(DataSectionPtr pBaseSect, bool load)
{
	BW_GUARD;
	DataSectionPtr pSect;
	if (load)
		pSect = pBaseSect;
	else
		pSect = pBaseSect->newSection( nameID() );

	IF_NOT_MF_ASSERT_DEV(pSect)
	{
		return;
	}

	SERIALISE(pSect, viewDependent_, Bool, load);
	SERIALISE(pSect, local_, Bool, load);

	serialiseInternal(pSect, load);
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to the PyParticleSystemRenderer.
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PyParticleSystemRenderer )

PY_BEGIN_METHODS( PyParticleSystemRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyParticleSystemRenderer )
	/*~ attribute PyParticleSystemRenderer.viewDependent
	 *	If viewDependent is set to true, the coordinates of the particle system
	 *	are in camera space rather than world space (ie, dependant on view).
	 *	The local flag will be false if viewDependant set to true.  If both are
	 *	false, implies particle system will be rendered in world space.
	 *	@type Integer as boolean. 0 or 1.
	 */
	PY_ATTRIBUTE( viewDependent )
	/*~ attribute PyParticleSystemRenderer.local
	 *	If local is set to true, the coordinates of the particle system are in
	 *	local space rather than world space.  The viewDependant flag will be
	 *	false if local set to true.  If both are false, implies particle system
	 *	will be rendered in world space.
	 *	@type Integer as boolean. 0 or 1.
	 */
	PY_ATTRIBUTE( local )
PY_END_ATTRIBUTES()


/**
 *	This is the constructor for PyParticleSystemRenderer.
 *
 *	@param pRenderer		The renderer we're the python interface for.
 *	@param pType			Parameters passed to the parent PyObject class.
 */
PyParticleSystemRenderer::PyParticleSystemRenderer( ParticleSystemRendererPtr pRenderer, PyTypePlus *pType ) :
	PyObjectPlus( pType ),
	pRenderer_( pRenderer )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pRenderer_.getObject() )
	{
		MF_EXIT( "NULL renderer" );
	}
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the readable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 */
PyObject *PyParticleSystemRenderer::pyGetAttribute( const char *attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the writable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 *	@param value	The value to assign to the attribute.
 */
int PyParticleSystemRenderer::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


PY_SCRIPT_CONVERTERS( PyParticleSystemRenderer )


extern int PyMeshParticleRenderer_token;
extern int PyVisualParticleRenderer_token;
extern int PyAmpParticleRenderer_token;
extern int PyTrailParticleRenderer_token;
extern int PyBlurParticleRenderer_token;
extern int PySpriteParticleRenderer_token;
extern int PyPointSpriteParticleRenderer_token;
static int s_tokenList = PyMeshParticleRenderer_token | PyAmpParticleRenderer_token |
						PyTrailParticleRenderer_token | PyBlurParticleRenderer_token |
						PyVisualParticleRenderer_token | PySpriteParticleRenderer_token | 
						PyPointSpriteParticleRenderer_token;
