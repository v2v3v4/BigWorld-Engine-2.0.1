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
#include "mesh_particle_renderer.hpp"
#include "particle/particle_system_manager.hpp"
#include "moo/visual_channels.hpp"
#include "moo/render_context.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/visual_manager.hpp"
#include "moo/moo_math.hpp"
#include "moo/primitive_file_structs.hpp"
#include "resmgr/multi_file_system.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

const std::string MeshParticleRenderer::nameID_ = "MeshParticleRenderer";

static Moo::VisualHelper s_helper;


// -----------------------------------------------------------------------------
//	Section - MeshParticleRenderer
// -----------------------------------------------------------------------------
/**
 *	Constructor.
 */
MeshParticleRenderer::MeshParticleRenderer():	
	visualName_( "" ),
	textureName_( "" ),
	material_( NULL ),
	doubleSided_( false ),    
	sortType_( SORT_QUICK ),
	materialFX_( FX_SOLID ),
	materialNeedsUpdate_( false )	
{
}


/**
 *	Destructor.
 */
MeshParticleRenderer::~MeshParticleRenderer()
{
}


/**
 *	This method returns true if the visual is suitable to be displayed
 *	by this renderer.  The MeshParticleRenderer requires visuals to be
 *	exported using the "MeshParticle" option in the exporter.
 *
 *	@param	v	name of the visual to check
 *	@return bool true iff the visual is of the required type.
 */
bool MeshParticleRenderer::isSuitableVisual( const std::string& v )
{
	BW_GUARD;
	bool isSuitable = false;

    // Go via the base class to load the visual, since name mangling must
    // be done due to particle visuals and model visuals not being 
    // compatible.
	Moo::VisualPtr spVisual = Moo::VisualManager::instance()->get(v);
	if (!spVisual)
	{
		// Couldn't open visual file. No mesh will be loaded.
		return false;
	}

	Moo::VerticesPtr verts;
	Moo::PrimitivePtr prim;
	Moo::Visual::PrimitiveGroup* primGroup;

	//The vertex format must be xyznuvi, with the visual being exported
	//specifically as mesh particles, so it can be drawn in batches of MAX_MESHES
	if (spVisual->primGroup0(verts,prim,primGroup) && verts->format() == "xyznuvi")
	{		
		if (prim->nPrimGroups() == PARTICLE_MAX_MESHES + 1)
		{
			isSuitable = true;
		}
		else
		{
			INFO_MSG( "Visual %s should be re-exported as Mesh Particles to"
				" enable correct sorting.\n", v.c_str() );			
		}
	}

	spVisual = NULL;
	return isSuitable;
}


#ifdef EDITOR_ENABLED
/**
 *  This class is used to quickly check if a primitive file is of a desired
 *  format, without reloading it.
 */
class PrimitiveFileHeader
{
public:
	PrimitiveFileHeader( const std::string& visualName ) :
		fileExists_( false )
	{

		BW_GUARD;
		std::string filename = BWResource::changeExtension( BWResource::resolveFilename( visualName ), ".primitives" );

		FILE * f = BWResource::instance().fileSystem()->posixFileOpen( filename, "rb" );
		if (f)
		{
			if (fread( &fileHeader_, sizeof( fileHeader_ ), 1, f ) == 1)
			{
				fileExists_ = true;
			}
			fclose( f );

		}
	}

	bool isType( const std::string& fmt )
	{
		return fileExists_ && std::string( fileHeader_.vertHeader_.vertexFormat_ ) == fmt;
	}
	
private:
	bool fileExists_;
	struct 
	{
		int32 ignored32bits_;
		Moo::VertexHeader vertHeader_;
	} fileHeader_;
};


/**
 *	This method quickly checks if vertex format is xyznuvi via opening .primitives file
 *  directly.
 */
bool MeshParticleRenderer::quickCheckSuitableVisual( const std::string& v )
{

	PrimitiveFileHeader header( v );

	return header.isType( "xyznuvi" );

}

#endif

/**
 *	This method sets the visual name for the renderer.
 *	Note : isSuitableVisual should be called first, unless you are sure its ok.
 *
 *	@param	v	name of the visual.
 *	@see	MeshParticleRenderer::isSuitableVisual
 */
void MeshParticleRenderer::visual( const std::string& v )
{
	BW_GUARD;	
#ifdef EDITOR_ENABLED
	if (MeshParticleRenderer::isSuitableVisual(v))
#endif
	{        
		visualName_ = v;
		pVisual_ = Moo::VisualManager::instance()->get(visualName_);

		if (!pVisual_)
		{
			// Mesh Particle Visual file not found.
			return;
		}

		//find texture name in this visual
		Moo::VerticesPtr verts;
		Moo::PrimitivePtr prim;
		Moo::Visual::PrimitiveGroup* primGroup;

		//The vertex format must be xyznuvi, with the visual being exported
		//specifically as mesh particles, so it can be drawn in batches of MAX_MESHES
		textureName_ = "";

		if (pVisual_ && pVisual_->primGroup0(verts,prim,primGroup))
		{
			Moo::EffectPropertyPtr pProp = primGroup->material_->getProperty( "diffuseMap" );
			if (pProp)
			{
				pProp->getResourceID( textureName_ );
			}
		}
		
		materialNeedsUpdate_ = true;
	}
}


/**
 *	This class sets up to PARTICLE_MAX_MESHES world transforms on an effect.
 */
class ParticleWorldPalette : public Moo::EffectConstantValue, public Aligned
{
public:
	ParticleWorldPalette()
	{
		transforms_.resize( PARTICLE_MAX_MESHES );
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetVectorArray( constantHandle, vectorArray_, PARTICLE_MAX_MESHES*3 );
		pEffect->SetArrayRange( constantHandle, 0, PARTICLE_MAX_MESHES*3 );
		return true;
	}

	void transform( int i, const Matrix& orig )
	{
		BW_GUARD;
		IF_NOT_MF_ASSERT_DEV(i<PARTICLE_MAX_MESHES)
		{
			return;
		}
		transforms_[i] = orig;
		Matrix m;
		XPMatrixTranspose( &m, &orig );
		int constant = i*3;
		vectorArray_[constant++] = m.row(0);
		vectorArray_[constant++] = m.row(1);
		vectorArray_[constant++] = m.row(2);
	}

	void nullTransforms(int base, int last = PARTICLE_MAX_MESHES)
	{
		BW_GUARD;
		if (base>=last)
			return;
		char* ptr = (char*)&vectorArray_[base*3];
		memset(ptr, 0, (last-base)*3*sizeof(Vector4));

		ptr = (char*)&transforms_[base];
		memset(ptr, 0, (last-base)*sizeof(Matrix));
	}

	const std::avector<Matrix>& transforms() const	{ return transforms_; }
private:
	Vector4 vectorArray_[PARTICLE_MAX_MESHES*3];
	std::avector<Matrix> transforms_;	//duplicate kept for vertex->getSnapshot
};

static SmartPointer<ParticleWorldPalette> s_particleWorldPalette = new ParticleWorldPalette;


/**
 *	This class sets up to PARTICLE_MAX_MESHES tints on an effect.
 */
class ParticleTintPalette : public Moo::EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetVectorArray( constantHandle, tints_, PARTICLE_MAX_MESHES );
		pEffect->SetArrayRange( constantHandle, 0, PARTICLE_MAX_MESHES );
		return true;
	}

	void tint( int i, const Vector4& t )
	{
		BW_GUARD;
		IF_NOT_MF_ASSERT_DEV(i<PARTICLE_MAX_MESHES)
		{
			return;
		}
		tints_[i] = t;
	}	
private:
	Vector4 tints_[PARTICLE_MAX_MESHES];
};

static ParticleTintPalette* s_particleTintPalette = NULL;


/**
 *	This method views the given particle system using this renderer.
 *
 *	@param	worldTransform		world transform of the particle system.
 *	@param	beg					first particle to draw
 *	@param	end					end of particle vector to draw
 *	@param	inbb				bounding box of particle system.
 */
void MeshParticleRenderer::draw( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	if ( beg == end ) return;
	if ( !pVisual_ ) return;

	if (materialNeedsUpdate_)
		this->updateMaterial();

    // If there are any particles whose colour is not white then there is
    // some tinting, in which case optimisation is not possible:
    bool tinted = false;
    for (Particles::iterator it = beg; it != end; ++it)
    {
        if (it->colour() != 0xffffffff)
        {
            tinted = true;
            break;
        }
    }

	DX::Device* device = Moo::rc().device();	

	bool sorted = true;		
	
	static Moo::EffectConstantValuePtr* pPaletteConstant_ = NULL;
	static Moo::EffectConstantValuePtr* pTintPaletteConstant_ = NULL;
	if ( !pPaletteConstant_ )
	{
		pPaletteConstant_ = Moo::EffectConstantValue::get( "WorldPalette" );
		pTintPaletteConstant_ = Moo::EffectConstantValue::get( "TintPalette" );
	}

	bool current = Moo::EffectVisualContext::instance().overrideConstants();
	Moo::EffectVisualContext::instance().overrideConstants(false);
	Moo::EffectVisualContext::instance().initConstants();
	Moo::EffectVisualContext::instance().overrideConstants(true);
	*pPaletteConstant_ = s_particleWorldPalette;

	if (s_particleTintPalette == NULL)
		s_particleTintPalette = new ParticleTintPalette();

	*pTintPaletteConstant_ = s_particleTintPalette;

	//TODO : adjust inbb for size of particles.  when loading mesh particles, calculate
	//radius of each mesh piece (to account for spinning particles) and use radius
	//to add to bounding box.
	if (this->beginDraw( inbb, worldTransform ))
	{
		if (materialFX_ == FX_SOLID)
		{
			this->drawOptimised(worldTransform,beg,end,inbb);
		}
		else
		{		
			switch (sortType_)
			{
			case SORT_NONE:
				this->drawOptimised(worldTransform,beg,end,inbb);
				break;
			case SORT_QUICK:
				this->drawOptimisedSorted(worldTransform,beg,end,inbb);
				break;
			case SORT_ACCURATE:
				this->drawSorted(worldTransform,beg,end,inbb);			
				break;
			}
		}		
	}

	//note - should always be called even if begin fails.
	this->endDraw(inbb);
	
	Moo::EffectVisualContext::instance().overrideConstants(current);
}


/**
 *	This draw method draws a mesh in optimised fashion, i.e. 15 pieces
 *	at a time, and unsorted.
 *
 *	@see MeshParticleRenderer::draw
 */
void MeshParticleRenderer::drawOptimised( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	int nParticles = end - beg;
	Matrix particleTransform;	

	Particles::iterator it = beg;

	while ( it!=end )
	{
		int idx = 0;		
		BoundingBox bb( BoundingBox::s_insideOut_ );
		
		//loop through PARTICLE_MAX_MESHES particles at a time...
		while ( (it!=end) && (idx<PARTICLE_MAX_MESHES) )
		{			
			//copy particle transforms into constants
			Particle &particle = *it++;

			if (particle.isAlive())
			{				
				particleTransform.setRotate( particle.yaw(), particle.pitch(), 0.f );
				particleTransform.translation( particle.position() );			
				bb.addBounds( particle.position() );

				// Add spin
				float spinSpeed = particle.meshSpinSpeed();
				if (spinSpeed != 0.f )
				{
					Vector3 spinAxis = particle.meshSpinAxis();
					Matrix spin;
					D3DXMatrixRotationAxis
					( 
						&spin, 
						&spinAxis, 
						spinSpeed * PARTICLE_MESH_MAX_SPIN * (particle.age() - particle.meshSpinAge()) 
					);
					particleTransform.preMultiply( spin );
				}

				if (local())
					particleTransform.postMultiply(worldTransform);

				s_particleWorldPalette->transform(idx, particleTransform);
				s_particleTintPalette->tint(idx, Colour::getVector4Normalised(particle.colour()));
			}
			else	//!isAlive
			{
				s_particleWorldPalette->nullTransforms(idx, idx+1);
			}
			idx++;
		}

		//draw this set
		if ( idx>0 && !bb.insideOut() )
		{
			s_particleWorldPalette->nullTransforms(idx);
			this->draw(bb);							
		}
	}	
}


/**
 *	This draw method draws a mesh in sorted optimised fashion, i.e. 15 pieces
 *	at a time, and sorted quickly.
 *
 *	@see MeshParticleRenderer::draw
 */
void MeshParticleRenderer::drawOptimisedSorted( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	int nParticles = end - beg;
	Matrix particleTransform;

	sorter_.sortOptimised( beg, end );

	Particle *particle = sorter_.beginOptimised( &*beg );	

	while ( particle != NULL )
	{		
		int idx = 0;
		BoundingBox bb( BoundingBox::s_insideOut_ );
		
		//loop through PARTICLE_MAX_MESHES particles at a time...
		while ( (particle != NULL) && (idx<PARTICLE_MAX_MESHES) )
		{		
			if (particle->isAlive())
			{				
				//copy particle transforms into constants						
				particleTransform.setRotate( particle->yaw(), particle->pitch(), 0.f );
				particleTransform.translation( particle->position() );			
				bb.addBounds( particle->position() );

				// Add spin
				float spinSpeed = particle->meshSpinSpeed();
				if (spinSpeed != 0.f )
				{
					Vector3 spinAxis = particle->meshSpinAxis();
					Matrix spin;

					D3DXMatrixRotationAxis
					( 
						&spin, 
						&spinAxis, 
						spinSpeed * PARTICLE_MESH_MAX_SPIN * (particle->age() - particle->meshSpinAge()) 
					);

					particleTransform.preMultiply( spin );
				}

				if (local())
					particleTransform.postMultiply(worldTransform);

				s_particleWorldPalette->transform(idx, particleTransform);
				s_particleTintPalette->tint(idx, Colour::getVector4Normalised(particle->colour()));
			}
			else
			{
				s_particleWorldPalette->nullTransforms(idx,idx+1);
			}
			idx++;
			particle = sorter_.nextOptimised( &*beg );
		}

		//draw this set
		if ( idx>0 && !bb.insideOut() )
		{
			s_particleWorldPalette->nullTransforms(idx);			
			this->draw(bb);							
		}
	}
}


/**
 *	This method sorts all of the particles, and draws each particle
 *	one by one.  It is the slowest way to draw, but the sorting will
 *	be as correct as possible.
 *
 *	@see MeshParticleRenderer::draw
 */
void MeshParticleRenderer::drawSorted( const Matrix& worldTransform,
	Particles::iterator beg,
	Particles::iterator end,
	const BoundingBox & inbb )
{
	BW_GUARD;
	Matrix nullMatrix;
	memset( nullMatrix, 0, sizeof( Matrix ) );	

	int nParticles = end - beg;
	int particleType = 0;
	Matrix particleTransform;

	sorter_.sort( beg, end );

	Particle *particle = sorter_.begin( &*beg, particleType );	

	s_particleWorldPalette->nullTransforms(0);

	while ( particle != NULL )
	{
		if (particle->isAlive())
		{
			BoundingBox bb( BoundingBox::s_insideOut_ );
			bb.addBounds( particle->position() );

			// Set particle transform as world matrix
			particleTransform.setRotate( particle->yaw(), particle->pitch(), 0.f );
			particleTransform.translation( particle->position() );
			// Add spin:
			float spinSpeed = particle->meshSpinSpeed();
			if (spinSpeed != 0.f )
			{
				Vector3 spinAxis = particle->meshSpinAxis();
				Matrix spin;

				D3DXMatrixRotationAxis
				( 
					&spin, 
					&spinAxis, 
					spinSpeed * PARTICLE_MESH_MAX_SPIN * (particle->age() - particle->meshSpinAge()) 
				);

				particleTransform.preMultiply( spin );
			}

			if (local())
				particleTransform.postMultiply(worldTransform);

			s_particleWorldPalette->transform(particleType, particleTransform);
			s_particleTintPalette->tint(particleType, Colour::getVector4Normalised(particle->colour()));

			//primitive group in mesh particles are
			//0 : draw all 15 pieces
			//1-15 incl : draw particle piece 1 - 15 individually
			this->draw(bb,particleType+1);
		}

		//s_particleWorldPalette->transform(particleType, nullMatrix);
		particle = sorter_.next( &*beg, particleType );
	}	
}


/**
 *	This method applies common setup for drawing mesh particles, and
 *	must be called once before calling draw many times.
 */
bool MeshParticleRenderer::beginDraw( const BoundingBox & inbb, const Matrix& worldTransform )
{
	BW_GUARD;
	Moo::rc().push();
	Moo::rc().world( local() ? worldTransform : Matrix::identity );

	if (s_helper.shouldDraw(false, inbb))
	{		
		s_helper.start( false, inbb );

		Moo::EffectVisualContext::instance().initConstants();
		Moo::EffectVisualContext::instance().staticLighting( false );

		if (pVisual_->primGroup0(verts_,prim_,primGroup_))
		{			
			Moo::Visual::RenderSet& renderSet = pVisual_->renderSets()[0];
			Moo::EffectVisualContextSetter setter( &renderSet );

			// Set our vertices.
			if (FAILED( verts_->setVertices(Moo::rc().mixedVertexProcessing())))
			{
				ERROR_MSG( "MeshParticleRenderer::drawOptimisedVisual: Couldn't set vertices\n" );
				return false;
			}			

			// Set our indices.
			if (FAILED( prim_->setPrimitives() ))
			{
				ERROR_MSG( "MeshParticleRenderer::drawOptimisedVisual:"
						"Couldn't set primitives for: %s\n",
						prim_->resourceID().c_str() );
				return false;
			}

			return true;
		}
	}
	
	return false;
}


/**
 *	This method draws the visual.
 *	@param ignoreBoundingBox if this value is true, this
 *		method will not cull the visual based on its bounding box.
 *	@param primGroupIndex The primitive group to draw.  For mesh particles,
 *	primGroup 0 refers to the whole mesh of 15 pieces.  pieces 1 - 15 incl.
 *	represent individual particles.
 *	@return S_OK if succeeded
 */
void MeshParticleRenderer::draw( const BoundingBox& bb, int primGroupIndex )
{
	BW_GUARD;
	verts_->clearSoftwareSkinner( );

	if (!material_->channel())
	{
		verts_->setVertices(Moo::rc().mixedVertexProcessing(), false);				

		if (material_->begin())
		{
			for (uint32 i = 0; i < material_->nPasses(); i++)
			{
				material_->beginPass( i );
				prim_->drawPrimitiveGroup( primGroupIndex );
				material_->endPass();
			}
			material_->end();
		}
	}
	else
	{
		Moo::VertexSnapshotPtr pVSS = verts_->getSnapshot
			(s_particleWorldPalette->transforms(),
			material_->skinned(),
			material_->bumpMapped());

		float distance = Moo::rc().view().applyPoint(bb.centre()).z;
		
		material_->channel()->addItem( pVSS, prim_, material_,
			primGroupIndex, distance, NULL );
	}		
}


/**
 *	This method must be called after beginDraw, and draw.
 */
void MeshParticleRenderer::endDraw(const BoundingBox & inbb)
{
	BW_GUARD;
	if (s_helper.shouldDraw(false, inbb))
	{
	    verts_->clearSoftwareSkinner();
	    s_helper.fini();	    
    }
    Moo::rc().pop();
}


/**
 *	This method recreates the material for the visual.
 */
void MeshParticleRenderer::updateMaterial()
{
	BW_GUARD;
	if (!pVisual_)
		return;

	Moo::VerticesPtr	verts;
	Moo::PrimitivePtr	prim;
	Moo::Visual::PrimitiveGroup* primGroup;	

	if (pVisual_->primGroup0(verts,prim,primGroup))
	{
		std::string effectName(
			ParticleSystemManager::instance().meshSolidEffect() );
		switch ( materialFX_ )
		{
		/*case FX_SOLID:
			effectName =
				ParticleSystemManager::instance().meshSolidEffect();
			break;*/
		case FX_ADDITIVE:
			effectName =
				ParticleSystemManager::instance().meshAdditiveEffect();
			break;
		case FX_BLENDED:
			effectName =
				ParticleSystemManager::instance().meshBlendedEffect();
			break;			
		}
		
		
		material_ = NULL;
		material_ = new Moo::EffectMaterial;				
		material_->initFromEffect( effectName, textureName_, doubleSided_ );		
	}

	materialNeedsUpdate_ = false;
}


/**
 *	This method loads or saves the mesh particle renderer to the data
 *	section provided.
 *
 *	@param	pSect		The section to read/write from.
 *	@param	load		Whether we should load or save.
 */
void MeshParticleRenderer::serialiseInternal(DataSectionPtr pSect, bool load)
{
	BW_GUARD;
	SERIALISE(pSect, visualName_ , String, load);
	SERIALISE(pSect, doubleSided_, Bool, load);
	SERIALISE_ENUM(pSect, materialFX_, MaterialFX, load);
    SERIALISE_ENUM(pSect, sortType_, SortType, load);
	if (load)
    {
		this->visual(visualName_);
		materialNeedsUpdate_ = true;
    }
}


/*static*/ void MeshParticleRenderer::prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output )
{
	BW_GUARD;
	const std::string& visualName = pSection->readString( "visualName_" );
	if (visualName.length())
		output.insert(visualName);
}



/**
 *	This is the Set-Accessor for the texture file name of the mesh.
 *
 *	@param m		The new materialFX type.
 */
void MeshParticleRenderer::materialFX( MaterialFX m )
{
	if ( materialFX_ != m )
	{
		materialFX_ = m;
		materialNeedsUpdate_ = true;	
	}
}


/**
 *	This is the Set-Accessor for the double-sidedness of the material.
 *
 *	@param s	The new value for double-sided.
 */
void MeshParticleRenderer::doubleSided( bool s )
{
	if (s != doubleSided_)
	{
		doubleSided_ = s;
		materialNeedsUpdate_ = true;		
	}
}


// -----------------------------------------------------------------------------
// Section: The Python Interface to the PyMeshParticleRenderer.
// -----------------------------------------------------------------------------
#undef PY_ATTR_SCOPE

#define PY_ATTR_SCOPE PyMeshParticleRenderer::

PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyMeshParticleRenderer::MaterialFX, materialFX, materialFX )
PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( PyMeshParticleRenderer::SortType, sortType, sortType )

PY_TYPEOBJECT( PyMeshParticleRenderer )

/*~ function Pixie.MeshParticleRenderer
 *	Factory function to create and return a new PyMeshParticleRenderer object. A
 *	MeshParticleRenderer is a ParticleSystemRenderer which renders each particle
 *	as mesh object.
 *	@return A new PyMeshParticleRenderer object.
 */
PY_FACTORY_NAMED( PyMeshParticleRenderer, "MeshParticleRenderer", Pixie )

PY_BEGIN_METHODS( PyMeshParticleRenderer )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyMeshParticleRenderer )	
	/*~ attribute PyMeshParticleRenderer.visual
	 *	This is the name of the mesh particle type to use for the particles.
	 *	Default value is "".
	 *	@type String.
	 */
	PY_ATTRIBUTE( visual )
	/*~ attribute PyMeshParticleRenderer.materialFX
	 *	Specifies the MaterialFX to use to draw the meshes into the scene.
	 *	Each value specifies either a special effect, or a source and destination
	 *	alpha-blend setting for the material. If no value is specified, materialFX
	 *	defaults to 'SOLID'. The possible values are listed below:
	 *	 
	 *	SOLID: not alpha blended. source = ONE, destination = ZERO
	 *
	 *	ADDITIVE: alpha blended. source = SRC_ALPHA, destination = ONE
	 *
	 *	BLENDED: alpha blended. source = SRC_ALPHA, destination = INV_SRC_ALPHA	 
	 *
	 *	Refer to the DirectX documentation for a description of how material
	 *	settings affect alpha blending.
	 *
	 *	@type String.
	 */
	 PY_ATTRIBUTE( materialFX )
	/*~ attribute PyMeshParticleRenderer.sortType
	 *	Specifies the sort type.  The possible values are listed below:
	 *	 
	 *	NONE: minimal sorting.  Choose this method if speed is paramount and
	 *	the visual artifacts introduced are not noticeable.  The particle
	 *	system as a whole will be rendered in order with respect to other
	 *	sorted objects, and the triangles contained within will be sorted
	 *	back to front, however the particles themselves will render out
	 *	of order.
	 *
	 *	QUICK: sorting is done in a way that allows the renderer to still
	 *	draw in groups of 15, which introduced some sorting inaccuracies but
	 *	maintains most of the speed of unsorted mesh particles.  This method
	 *	is highly recommended if you cannot notice the sorting innacuracies.
	 *
	 *	ACCURATE: sorting of individual objects and triangles is performed,
	 *	which provides the most accurate sorting but breaks the ability of
	 *	the renderer to perform fast 15-at-a-time rendering, thus decreasing
	 *	rendering performance.  If you choose this sorting method, make
	 *	sure you double check the performance hit induced.
	 *
	 *	Refer to the DirectX documentation for a description of how material
	 *	settings affect alpha blending.
	 *
	 *	@type String.
	 */
	 PY_ATTRIBUTE( sortType )	 
	 /*~ attribute PyMeshParticleRenderer.doubleSided
	  *	Specifies whether or not to draw the mesh pieces double sided.
	  * @type Bool.
	  */
	 PY_ATTRIBUTE( doubleSided )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
PyMeshParticleRenderer::PyMeshParticleRenderer( MeshParticleRendererPtr pR, PyTypePlus *pType ):
	PyParticleSystemRenderer(pR,pType),
	pR_(pR)
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pR_.hasObject() )
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
PyObject *PyMeshParticleRenderer::pyGetAttribute( const char *attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	
	return PyParticleSystemRenderer::pyGetAttribute( attr );
}


/**
 *	This is an automatically declared method for any derived classes of
 *	PyObjectPlus. It connects the writable attributes to their corresponding
 *	C++ components and searches the parent class' attributes if not found.
 *
 *	@param attr		The attribute being searched for.
 *	@param value	The value to assign to the attribute.
 */
int PyMeshParticleRenderer::pySetAttribute( const char *attr, PyObject *value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyParticleSystemRenderer::pySetAttribute( attr, value );
}


PY_ENUM_MAP( PyMeshParticleRenderer::MaterialFX )
PY_ENUM_CONVERTERS_CONTIGUOUS( PyMeshParticleRenderer::MaterialFX )

PY_ENUM_MAP( PyMeshParticleRenderer::SortType )
PY_ENUM_CONVERTERS_CONTIGUOUS( PyMeshParticleRenderer::SortType )


/**
 *	This is the Set-Accessor for the texture file name of the mesh.
 *
 *	@param m		The new materialFX type.
 */
void PyMeshParticleRenderer::materialFX( MaterialFX newMaterialFX )
{
	BW_GUARD;
	pR_->materialFX( (MeshParticleRenderer::MaterialFX)newMaterialFX);
}


/**
 *	This is the Set-Accessor for the double-sidedness of the material.
 *
 *	@param s	The new value for double-sided.
 */
void PyMeshParticleRenderer::sortType( SortType newSortType )
{
	BW_GUARD;
	pR_->sortType( (MeshParticleRenderer::SortType)newSortType);
}


/**
 *	Static Python factory method. This is declared through the factory
 *	declaration in the class definition.
 *
 *	@param	args	The list of parameters passed from Python. This should
 *					just be a string (textureName.)
 */
PyObject *PyMeshParticleRenderer::pyNew( PyObject *args )
{
	BW_GUARD;
	char *nameFromArgs = "None";
	if (!PyArg_ParseTuple( args, "|s", &nameFromArgs ) )
	{
		PyErr_SetString( PyExc_TypeError, "MeshParticleRenderer() expects "
			"an optional visual name string" );
		return NULL;
	}
	
	MeshParticleRenderer * mpr = new MeshParticleRenderer();
	if ( _stricmp(nameFromArgs,"None") )
		mpr->visual( std::string( nameFromArgs ) );
	PyMeshParticleRenderer * pmpr = new PyMeshParticleRenderer( mpr );

	return pmpr;
}
