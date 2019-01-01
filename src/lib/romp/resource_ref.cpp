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
#include "resource_ref.hpp"

#include "model/super_model.hpp"
#include "romp/font.hpp"
#include "romp/font_manager.hpp"
#include "romp/lens_effect.hpp"
#include "romp/py_texture_provider.hpp"
#include "romp/py_material.hpp"
#include "moo/texture_manager.hpp"
#include "moo/visual_manager.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/data_section_census.hpp"
#include "resmgr/xml_section.hpp"
#include "pyscript/py_data_section.hpp"
#include "cstdmf/debug.hpp"
#include "particle/meta_particle_system.hpp"
#include "particle/py_meta_particle_system.hpp"
#include "duplo/pymodel.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


/**
 *	Returns a Python object that wraps the resource reference.
 *	Note the new python object returned will hold another reference
 *	to the resource, thus you must release the python reference
 *	and the resource reference to free the object.
 *
 *	@param		rr			The loaded resource.
 *
 *	@return		Python version of the ResourceRef.
 */
PyObject * pyInstance_dsect( ResourceRef& rr )
{
	//Data sections are already stored in the cache.	
	if (rr.data() != NULL)
	{		
		DataSection* pDS = (DataSection*)(rr.data());
		return new PyDataSection(pDS);
	}
	else
	{
		return NULL;
	}
}


/**
 *	Returns a ResourceRef object only if the 
 *	requested resource have previously been loaded.
 *
 *	@param	id	path to resource
 *
 *	@return		ResourceRef object referencing the requested resource if it
 *				have previously been loaded. NULL ResourceRef otherwise.
 */
static ResourceRef getIfLoaded_dsect( const std::string & id )
{
	DataSectionPtr pDS = DataSectionCensus::find( id );
	return ResourceRef( &*pDS, ResourceRef::stdModRef<DataSection>, id, pyInstance_dsect );
}


/**
 *	Returns a ResourceRef object to the requested 
 *	resource, loading it if necessary.
 *
 *	@param	id	path to resource
 *
 *	@return		ResourceRef object referencing the requested resource 
 *				if found. NULL ResourceRef otherwise.
 */
static ResourceRef getOrLoad_dsect( const std::string & id )
{
	DataSectionPtr pDS = BWResource::openSection( id );
	return ResourceRef( &*pDS, ResourceRef::stdModRef<DataSection>, id, pyInstance_dsect );
}


static ResourceRef::ResourceFns s_RFns_dsect =
	{ &getIfLoaded_dsect, &getOrLoad_dsect };


/**
 *	Returns a Python object that wraps the resource reference.
 *	Note the new python object returned will hold another reference
 *	to the resource, thus you must release the python reference
 *	and the resource reference to free the object.
 *
 *	@param		rr			The loaded resource.
 *
 *	@return		Python tuple containing the provided ResourceRef.
 */
PyObject * pyInstance_model( ResourceRef& rr )
{
	if (rr.data()!=NULL)
	{		
		PyObject * pTuple = PyTuple_New(1);
		PyTuple_SetItem( pTuple, 0, PyString_FromString(rr.id().c_str()) );
		PyObject * ret = PyModel::pyNew(pTuple);
		Py_DECREF( pTuple );
		return ret;
	}
	else
	{
		return NULL;
	}
}


/**
 *	getIfLoaded/getOrLoad functions for models
 */
static ResourceRef getIfLoaded_model( const std::string & id )
{
	ModelPtr pModel = Model::get( id, false );
	return ResourceRef( &*pModel, ResourceRef::stdModRef<Model>, id, pyInstance_model );
}


static ResourceRef getOrLoad_model( const std::string & id )
{
	ModelPtr pModel = Model::get( id );
	return ResourceRef( &*pModel, ResourceRef::stdModRef<Model>, id, pyInstance_model );
}


static ResourceRef::ResourceFns s_RFns_model =
	{ &getIfLoaded_model, &getOrLoad_model };


/**
 *	getIfLoaded/getOrLoad functions for visuals
 */
static ResourceRef getIfLoaded_visual( const std::string & id )
{
	Moo::VisualPtr pVisual =
		Moo::VisualManager::instance()->get( id, false );
	return ResourceRef( &*pVisual, ResourceRef::stdModRef<Moo::Visual>, id, NULL );
}


static ResourceRef getOrLoad_visual( const std::string & id )
{
	Moo::VisualPtr pVisual =
		Moo::VisualManager::instance()->get( id );
	return ResourceRef( &*pVisual, ResourceRef::stdModRef<Moo::Visual>, id, NULL );
}


static ResourceRef::ResourceFns s_RFns_visual =
	{ &getIfLoaded_visual, &getOrLoad_visual };


/**
 *	Returns a Python object that wraps the resource reference.
 *	Note the new python object returned will hold another reference
 *	to the resource, thus you must release the python reference
 *	and the resource reference to free the object.
 *
 *	@param		ResourceRef	the loaded resource.
 *
 *	@return		PyObject*	python version of the ResourceRef.
 */
PyObject * pyInstance_texture( ResourceRef& rr )
{
	void * data = rr.data();
	if (data)
	{
		Moo::BaseTexture* pTexture = (Moo::BaseTexture*)data;
		return new PyTextureProvider( Py_None, pTexture );
	}
	else
	{
		return NULL;
	}	
}


/**
 *	getIfLoaded/getOrLoad functions for textures
 */
static ResourceRef getIfLoaded_texture( const std::string & id )
{
	Moo::BaseTexturePtr pTex =	// allowAnim, !mustExist, !loadIfMissing
		Moo::TextureManager::instance()->get( id, true, false, false );
	return ResourceRef( &*pTex, ResourceRef::stdModRef<Moo::BaseTexture>, id, &pyInstance_texture );
}


static ResourceRef getOrLoad_texture( const std::string & id )
{
	Moo::BaseTexturePtr pTex =
		Moo::TextureManager::instance()->get( id );
	return ResourceRef( &*pTex, ResourceRef::stdModRef<Moo::BaseTexture>, id, &pyInstance_texture );
}


static ResourceRef::ResourceFns s_RFns_texture =
	{ &getIfLoaded_texture, &getOrLoad_texture };



/**
 *	getIfLoaded/getOrLoad functions for fonts
 */
static ResourceRef getIfLoaded_font( const std::string & id )
{
	CRITICAL_MSG( "Bad\n" );
	FontPtr pFont = FontManager::instance().get( id );
	return ResourceRef( &*pFont, ResourceRef::stdModRef<Font>, id, NULL );
}


static ResourceRef getOrLoad_font( const std::string & id )
{
	CRITICAL_MSG( "Bad\n" );
	FontPtr pFont = FontManager::instance().get( id );
	return ResourceRef( &*pFont, ResourceRef::stdModRef<Font>, id, NULL );
}


static ResourceRef::ResourceFns s_RFns_font = 
	{ &getIfLoaded_font, &getOrLoad_font };


/**
 *	Returns a Python object that wraps the resource reference.
 *	Note the new python object returned will hold another reference
 *	to the resource, thus you must release the python reference
 *	and the resource reference to free the object.
 *
 *	@param		rr			The loaded resource.
 *
 *	@return		The provided ResourceRef as a PyMetaParticleSystem.
 */
PyObject * pyInstance_particle( ResourceRef& rr )
{
	void * data = rr.data();
	if (data)
	{
		MetaParticleSystem* pSystem = (MetaParticleSystem*)data;
		return new PyMetaParticleSystem(pSystem);
	}
	else
	{
		return NULL;
	}		
}


/**
 *	getIfLoaded/getOrLoad functions for particles
 */
static ResourceRef getIfLoaded_particle( const std::string & id )
{
	MetaParticleSystemPtr pSystem = NULL;	
	return ResourceRef( &*pSystem, ResourceRef::stdModRef<MetaParticleSystem>, id, &pyInstance_particle );	
}


static ResourceRef getOrLoad_particle( const std::string & id )
{
	MetaParticleSystemPtr pSystem = new MetaParticleSystem();
	pSystem->load( id, "" );
	return ResourceRef( &*pSystem, ResourceRef::stdModRef<MetaParticleSystem>, id, pyInstance_particle );
}


static ResourceRef::ResourceFns s_RFns_particle =
	{ &getIfLoaded_particle, &getOrLoad_particle };


/**
 *	getIfLoaded/getOrLoad functions for Lens Flares
 */
static ResourceRef getIfLoaded_lensEffect( const std::string & id )
{
	LensEffectPtr le = NULL;	
	return ResourceRef( &*le, ResourceRef::stdModRef<LensEffect>, id, NULL );
}


static ResourceRef getOrLoad_lensEffect( const std::string & id )
{
	LensEffectPtr le = NULL;

	DataSectionPtr pSect = BWResource::openSection( id );
	if (pSect)
	{
		le = new LensEffect;
		le->load(pSect);
	}
	
	return ResourceRef( &*le, ResourceRef::stdModRef<LensEffect>, id, NULL );
}


static ResourceRef::ResourceFns s_RFns_lensEffect =
	{ &getIfLoaded_lensEffect, &getOrLoad_lensEffect };


/**
 *	Returns a Python object that wraps the resource reference.
 *	Note the new python object returned will hold another reference
 *	to the resource, thus you must release the python reference
 *	and the resource reference to free the object.
 *
 *	@param		rr			The loaded resource.
 *
 *	@return		Python version of the ResourceRef.
 */
PyObject * pyInstance_fx( ResourceRef& rr )
{
	//Data sections are already stored in the cache.	
	if (rr.data() != NULL)
	{		
		Moo::EffectMaterial* pMat = (Moo::EffectMaterial*)(rr.data());
		return new PyMaterial(pMat);
	}
	else
	{
		return NULL;
	}
}


/**
 *	getIfLoaded/getOrLoad functions for .fx files
 */
static ResourceRef getIfLoaded_fx( const std::string & id )
{
	Moo::EffectMaterial* pMat = NULL;	
	return ResourceRef( NULL, ResourceRef::stdModRef<Moo::EffectMaterial>, id, &pyInstance_fx );	
}


static ResourceRef getOrLoad_fx( const std::string & id )
{
	Moo::EffectMaterialPtr pMat = new Moo::EffectMaterial();
	pMat->initFromEffect( id );
	return ResourceRef( pMat.getObject(), ResourceRef::stdModRef<Moo::EffectMaterial>, id, pyInstance_fx );
}


static ResourceRef::ResourceFns s_RFns_fx =
	{ &getIfLoaded_fx, &getOrLoad_fx };


/**
 *	The static method gets the named resource if it is loaded.
 *	Otherwise it returns an empty ResourceRef.
 */
const ResourceRef::ResourceFns & ResourceRef::ResourceFns::getForString(
	const std::string & id )
{
	uint period = id.find_last_of( '.' );
	if (period >= id.size())
		return s_RFns_dsect;
	std::string ext = id.substr( period+1 );

	if (ext == "model")
	{
		return s_RFns_model;
	}

	if (ext == "visual")
	{
		return s_RFns_visual;
	}
	
	const char** textureExts = Moo::TextureManager::validTextureExtensions();
	for (size_t i = 0; textureExts[i] != NULL; ++i)
	{
		if (ext == textureExts[i])
			return s_RFns_texture;
	}

	if (ext == "font")
	{
		return s_RFns_font;
	}

	if (ext == "gui")
	{
		return s_RFns_dsect;
	}

	if (ext == "fx")
	{
		return s_RFns_fx;
	}

	if ( MetaParticleSystem::isParticleSystem( id ) )
	{
		return s_RFns_particle;
	}

	if ( LensEffect::isLensEffect( id ) )
	{
		return s_RFns_lensEffect;
	}

	// assume it is a datasection then
	return s_RFns_dsect;
}


/**
 *	The static method gets the named resource if it is loaded.
 *	Otherwise it returns an empty ResourceRef.
 */
ResourceRef ResourceRef::getIfLoaded( const std::string & id )
{
	const ResourceFns & rf = ResourceFns::getForString( id );
	return (*rf.getIfLoaded_)( id );
}


/**
 *	The static method gets or loads the named resource.
 *	If loading fails it returns an empty ResourceRef.
 */
ResourceRef ResourceRef::getOrLoad( const std::string & id )
{
	const ResourceFns & rf = ResourceFns::getForString( id );
	return (*rf.getOrLoad_)( id );
}

// resource_ref.cpp
