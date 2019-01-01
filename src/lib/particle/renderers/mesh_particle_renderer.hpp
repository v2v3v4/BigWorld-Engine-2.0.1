/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MESH_PARTICLE_RENDERER_HPP
#define MESH_PARTICLE_RENDERER_HPP

#include "base_mesh_particle_renderer.hpp"
#include "mesh_particle_sorter.hpp"
#include "moo/vertex_formats.hpp"
#include "moo/device_callback.hpp"
#include "moo/effect_material.hpp"


/**
 *	This class displays the particle system with each particle sharing a mesh.
 */
class MeshParticleRenderer : public BaseMeshParticleRenderer
{
public:
	/// @name Constructor(s) and Destructor.
	//@{
	MeshParticleRenderer();
	~MeshParticleRenderer();
	//@}
	
	static void prerequisites(
		DataSectionPtr pSection,
		std::set<std::string>& output );


	/// An enumeration of the possible material effects.
	enum MaterialFX
	{
		FX_ADDITIVE,		
		FX_BLENDED,		
		FX_SOLID,		
		FX_MAX	// keep this element at the end
	};


	/// An enumeration of the possible sort types.
	enum SortType
	{
		SORT_NONE,
		SORT_QUICK,
		SORT_ACCURATE
	};


	/// @name Mesh particle renderer methods.
	//@{
	static bool isSuitableVisual( const std::string& v );
#ifdef EDITOR_ENABLED
	static bool quickCheckSuitableVisual( const std::string& v );
#endif
	virtual void visual( const std::string& v );
	virtual const std::string& visual() const	{ return visualName_; }    

	MaterialFX materialFX() const		{ return materialFX_; }
	void materialFX( MaterialFX newMaterialFX );

    SortType sortType() const			{ return sortType_; }
    void sortType( SortType newSortType ) { sortType_ = newSortType; }

	const bool doubleSided() const		{ return doubleSided_; }
	void doubleSided( bool s );
	//@}


	///	@name Renderer Overrides.
	//@{
	virtual void draw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & bb );

	virtual void realDraw( const Matrix & worldTransform,
		Particles::iterator beg,
		Particles::iterator end )	{};

	virtual bool isMeshStyle() const	{ return true; }
	virtual ParticlesPtr createParticleContainer() const
	{
		return new FixedIndexParticles;
	}
	//@}


	// type of renderer
	virtual const std::string & nameID() { return nameID_; }
	static const std::string nameID_;

	virtual size_t sizeInBytes() const { return sizeof(MeshParticleRenderer); }

protected:
	virtual void serialiseInternal(DataSectionPtr pSect, bool load);

private:
	void drawOptimised(
		const Matrix& worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & inbb );

	void drawOptimisedSorted(
		const Matrix& worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & inbb );

	void drawSorted(
		const Matrix& worldTransform,
		Particles::iterator beg,
		Particles::iterator end,
		const BoundingBox & inbb );

	bool beginDraw( const BoundingBox & inbb, const Matrix& worldTransform );
	void draw( const BoundingBox& bb, int primGroupIndex = 0 );
	void endDraw(const BoundingBox & inbb);

    void fixMaterial();

	std::string			visualName_;
	std::string			textureName_;
	MeshParticleSorter	sorter_;	
	SortType    		sortType_;
	MaterialFX			materialFX_;
	bool				doubleSided_;
	bool				materialNeedsUpdate_;

	Moo::VerticesPtr	verts_;
	Moo::PrimitivePtr	prim_;
	Moo::Visual::PrimitiveGroup* primGroup_;
	Moo::EffectMaterialPtr material_;

	void updateMaterial();
};


typedef SmartPointer<MeshParticleRenderer> MeshParticleRendererPtr;


/*~ class Pixie.PyMeshParticleRenderer
 *	MeshParticleRenderer is a ParticleSystemRenderer which renders each particle
 *	as a mesh object.  MeshParticles are exported from 3D Studio Max using
 *	mesh particle option in Visual Exporter.  A MeshParticle .visual file contains
 *	up to 15 unique simple pieces of mesh.  The mesh particle renderer will use each
 *	one of these pieces as a single particle.
 *
 *  From BigWorld 1.8 and on, the MeshParticleRenderer can not render standard
 *	visuals.  You should use VisualParticleRenderer instead.
 */
class PyMeshParticleRenderer : public PyParticleSystemRenderer
{
	Py_Header( PyMeshParticleRenderer, PyParticleSystemRenderer )

public:
	/// @name Constructor(s) and Destructor.
	//@{
	PyMeshParticleRenderer( MeshParticleRendererPtr pR, PyTypePlus *pType = &s_type_ );	
	//@}

	/// An enumeration of the possible material effects.
	enum MaterialFX
	{
		FX_ADDITIVE,		
		FX_BLENDED,		
		FX_SOLID,		
		FX_MAX	// keep this element at the end
	};


	/// An enumeration of the possible sort types.
	enum SortType
	{
		SORT_NONE,
		SORT_QUICK,
		SORT_ACCURATE
	};

	/// @name Mesh particle renderer methods.
	//@{	
	virtual void visual( const std::string& v )	{ pR_->visual(v); }
	virtual const std::string& visual() const	{ return pR_->visual(); }    

	MaterialFX materialFX() const		{ return (MaterialFX)pR_->materialFX(); }
	void materialFX( MaterialFX newMaterialFX );

	SortType sortType() const			{ return (SortType)pR_->sortType(); }
    void sortType( SortType newSortType );

	const bool doubleSided() const		{ return pR_->doubleSided(); }
	void doubleSided( bool s )			{ pR_->doubleSided(s); }
	//@}


	// Python Wrappers for handling Emumerations in Python.
	PY_BEGIN_ENUM_MAP( MaterialFX, FX_ )
		PY_ENUM_VALUE( FX_ADDITIVE )
		PY_ENUM_VALUE( FX_SOLID )		
		PY_ENUM_VALUE( FX_BLENDED )		
	PY_END_ENUM_MAP()

	PY_BEGIN_ENUM_MAP( SortType, SORT_ )
		PY_ENUM_VALUE( SORT_NONE )
		PY_ENUM_VALUE( SORT_QUICK )		
		PY_ENUM_VALUE( SORT_ACCURATE )		
	PY_END_ENUM_MAP()
	//@}


	///	@name Python Interface to the MeshParticleRenderer.
	//@{
	PY_FACTORY_DECLARE()

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( std::string, visual, visual )
	PY_DEFERRED_ATTRIBUTE_DECLARE( materialFX )
    PY_DEFERRED_ATTRIBUTE_DECLARE( sortType )
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, doubleSided, doubleSided )	

	PyObject *pyGetAttribute( const char *attr );
	int pySetAttribute( const char *attr, PyObject *value );
	//@}
private:
	MeshParticleRendererPtr	pR_;
};


PY_ENUM_CONVERTERS_DECLARE( PyMeshParticleRenderer::MaterialFX )
PY_ENUM_CONVERTERS_DECLARE( PyMeshParticleRenderer::SortType )


#endif // MESH_PARTICLE_RENDERER_HPP
