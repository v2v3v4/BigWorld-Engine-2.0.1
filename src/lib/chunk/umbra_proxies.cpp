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

#include "umbra_proxies.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/guard.hpp"


#if UMBRA_ENABLE

#include <umbramodel.hpp>
#include <umbraobject.hpp>

PROFILER_DECLARE( UmbraModelProxy_del, "UmbraModelProxy del" );
PROFILER_DECLARE( UmbraObjectProxy_del, "UmbraObjectProxy del" );

namespace
{
	SimpleMutex modelProxyMutex;
	SimpleMutex objectProxyMutex;
}

// -----------------------------------------------------------------------------
// Section: UmbraModelProxy
// -----------------------------------------------------------------------------

UmbraModelProxy::UmbraModelProxy()
	: pModel_( NULL )
{

}

UmbraModelProxy::~UmbraModelProxy()
{
	if (pModel_)
		pModel_->release();

	UmbraModelProxy::del( this );
}


/**
 *	This static method creates a umbra model from a oriented bounding box.
 *	@param obb the oritented bounding box in Matrix form
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getObbModel( const Matrix& obb )
{
	BW_GUARD;
	Umbra::Model* pModel = Umbra::OBBModel::create( (Umbra::Matrix4x4&)obb );
	return get( pModel );
}

/**
 *	This method creates a oriented bounding box umbra model from a group of vertices.
 *	@param pVertices pointer to the vertex list
 *	@param nVertices the vertex count
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getObbModel( const Vector3* pVertices, uint32 nVertices )
{
	BW_GUARD;
	Umbra::Model* pModel = Umbra::OBBModel::create( (const Umbra::Vector3*)pVertices, nVertices );
	return get( pModel );
}

/**
 *	This method creates a oriented bounding box model from min and max points.
 *	@param min the minimum values of the bounding box
 *	@param max the maximum values of the bounding box
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getObbModel( const Vector3& min, const Vector3& max )
{
	BW_GUARD;
	Umbra::Model* pModel = Umbra::OBBModel::create( (const Umbra::Vector3&)min, (const Umbra::Vector3&)max );
	return get( pModel );
}

/**
 *	This method creates a mesh model from vertices and triangles
 *	@param pVertices pointer to the vertices
 *	@param pIndices pointer to the indices
 *	@param nVertices the number of vertices
 *	@param nTriangles the number of triangles
 *	@param clockwise the winding of the triangles, clockwise if true
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getMeshModel( const Vector3* pVertices, const uint32* pIndices, 
	uint32 nVertices, uint32 nTriangles, bool clockwise )
{
	BW_GUARD;
	Umbra::Model* pModel = Umbra::MeshModel::create( (const Umbra::Vector3*)pVertices, (const Umbra::Vector3i*)pIndices,
		nVertices, nTriangles, clockwise );
	return get( pModel );
}

/**
 *	This mehtod creates a sphere model from center and radius values
 *	@param center the center of the sphere
 *	@param radius the radius of the model
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getSphereModel( const Vector3& center, float radius )
{
	BW_GUARD;
	Umbra::Model* pModel = Umbra::SphereModel::create( (const Umbra::Vector3&)center, radius );
	return get( pModel );
}

/**
 *	This mehtod creates a sphere model from a vertex list
 *	@return the the newly created modelproxy with umbra model
 */
UmbraModelProxy* UmbraModelProxy::getSphereModel( const Vector3* pVertices, uint32 nVertices )
{
	BW_GUARD;
	Umbra::Model* pmodel = Umbra::SphereModel::create( (const Umbra::Vector3*) pVertices, nVertices );
	return get( pmodel );
}

/**
 *	This mehtod creates a model proxy from a umbra model
 *	@param pModel the umbra model to create a proxy for
 *	@return the the newly created modelproxy with umbra model
 *	@note it is the callers responsibility to manage the lifetime of the object
 */
UmbraModelProxy* UmbraModelProxy::get( Umbra::Model* pModel )
{
	BW_GUARD;
	SimpleMutexHolder smh( modelProxyMutex );
	proxies_.push_back( new UmbraModelProxy );
	proxies_.back()->model( pModel );
	return proxies_.back();
}

/**
 *	This method deletes the model proxy from the list of model proxies
 *	@param pProxy the model proxy to delete from the list
 */
void UmbraModelProxy::del( UmbraModelProxy* pProxy )
{
	BW_GUARD_PROFILER( UmbraModelProxy_del );
	SimpleMutexHolder smh( modelProxyMutex );
	Proxies::iterator it = std::find( proxies_.begin(), proxies_.end(), pProxy );
	if (it != proxies_.end())
	{
		proxies_.erase( it );
	}
}

/**
 *	This method invalidates all umbra models handled by the proxies.
 */
void UmbraModelProxy::invalidateAll()
{
	BW_GUARD;
	SimpleMutexHolder smh( modelProxyMutex );
	Proxies::iterator it = proxies_.begin();
	Proxies::iterator end = proxies_.end();
	while (it != end)
	{
		(*it)->invalidate();
		++it;
	}
}

/**
 *	This method invalidates the umbra model for this proxy.
 */
void UmbraModelProxy::invalidate()
{
	BW_GUARD;
	if (pModel_)
	{
		pModel_->release();
		pModel_ = NULL;
	}
}

UmbraModelProxy::Proxies UmbraModelProxy::proxies_;


// -----------------------------------------------------------------------------
// Section: UmbraObjectProxy
// -----------------------------------------------------------------------------

UmbraObjectProxy::UmbraObjectProxy(UmbraModelProxyPtr pModel, UmbraModelProxyPtr pWriteModel ) :
	pObject_( NULL ),
	pModel_(pModel),
	pWriteModel_(pWriteModel)
{

}

UmbraObjectProxy::~UmbraObjectProxy()
{
	if (pObject_)
		invalidate();

	UmbraObjectProxy::del( this );
}


void UmbraObjectProxy::pModelProxy( UmbraModelProxyPtr pModel )
{
	pObject_->setTestModel( pModel->model() );
	pModel_ = pModel;
}


/**
 *	This method creates a umbra object and proxy
 *	@param pModel the test model used by the umbra object
 *	@param pWriteModel the write model used by the umbra object
 *	@param identifier the identifier of this proxy or 
 *		empty string if the object is unnamed
 *	@return the umbra object proxy
 *	@note it is the callers responsibility to manage the lifetime of the object
 */
UmbraObjectProxy* UmbraObjectProxy::get( UmbraModelProxyPtr pModel, 
	UmbraModelProxyPtr pWriteModel, const std::string& identifier )
{
	BW_GUARD;
	SimpleMutexHolder smh( objectProxyMutex );
	Umbra::Model* pMod = pModel.hasObject() ? pModel->model() : NULL;

	Umbra::Object* pObject = Umbra::Object::create( pMod );
	
	if (pWriteModel.hasObject())
	{
		pObject->setWriteModel( pWriteModel->model() );
	}
	
	UmbraObjectProxy* pProxy = new UmbraObjectProxy(pModel, pWriteModel);
	pProxy->object( pObject );

	if (identifier.empty())
	{
		proxies_.push_back( pProxy );
	}
	else
	{
		namedProxies_.insert( std::make_pair(identifier, pProxy ) );
	}
	return pProxy;
}
/**
 *	This method creates a umbra object proxy for the supplied object
 *	@param pObject the umbra object to create a proxy for
 *	@return the newly created object proxy
 *	@note it is the callers responsibility to manage the lifetime of the object
 */
UmbraObjectProxy* UmbraObjectProxy::get( Umbra::Object* pObject )
{
	BW_GUARD;
	UmbraObjectProxy* pProxy = new UmbraObjectProxy(NULL, NULL);
	pProxy->object( pObject );

	proxies_.push_back( pProxy );

	return pProxy;
}

/**
 *	This method creates a copy of a named object proxy, it reuses
 *	the umbra models from the original object proxy.
 *	@return the new object proxy or NULL if source proxy is not found
 *	@note it is the callers responsibility to manage the lifetime of the object
 */
UmbraObjectProxy* UmbraObjectProxy::getCopy( const std::string& identifier )
{
	BW_GUARD;
	NamedProxies::iterator it = namedProxies_.find( identifier );
	if (it != namedProxies_.end())
	{
		return get( it->second->pModel_, it->second->pWriteModel_ );
	}
	return NULL;
}

/*
 *	This class is a helper class for finding the key value for a
 *	specific umbra object proxy
 */
class NamedObjectProxyFinder
{
public:
	NamedObjectProxyFinder( UmbraObjectProxy* pProxy ) :
	  pProxy_(pProxy) {}
	bool operator () (const std::pair<std::string, UmbraObjectProxy*>& item) const
	{
		return item.second == pProxy_;
	}
	UmbraObjectProxy* pProxy_;
};


/**
 *	This method deletes the object proxy from the list of object proxies
 *	@param pProxy the object proxy to delete from the list
 */
void UmbraObjectProxy::del( UmbraObjectProxy* pProxy )
{
	BW_GUARD_PROFILER( UmbraObjectProxy_del );
	SimpleMutexHolder smh( objectProxyMutex );
	Proxies::iterator it = std::find( proxies_.begin(), proxies_.end(), 
		pProxy );
	if (it != proxies_.end())
	{
		proxies_.erase( it );
	}
	else
	{
		NamedProxies::iterator it = std::find_if(namedProxies_.begin(), 
			namedProxies_.end(), NamedObjectProxyFinder(pProxy));
		if (it != namedProxies_.end())
			namedProxies_.erase( it );
	}
}

/**
 *	This method invalidates all umbra objects handled by the proxies.
 */
void UmbraObjectProxy::invalidateAll()
{
	BW_GUARD;
	SimpleMutexHolder smh( objectProxyMutex );
	Proxies::iterator it = proxies_.begin();
	Proxies::iterator end = proxies_.end();
	while (it != end)
	{
		(*it)->invalidate();
		++it;
	}
	NamedProxies::iterator nit = namedProxies_.begin();
	NamedProxies::iterator nend = namedProxies_.end();
	while (nit != nend)
	{
		nit->second->invalidate();
		++nit;
	}
}

/**
 *	This method invalidates the umbra object for this proxy.
 */
void UmbraObjectProxy::invalidate()
{
	BW_GUARD;
	if (pObject_)
	{
		pObject_->setCell( NULL );
		pObject_->release();
		pObject_ = NULL;
	}
	
	pModel_ = NULL;
	pWriteModel_ = NULL;
}

UmbraObjectProxy::Proxies UmbraObjectProxy::proxies_;
UmbraObjectProxy::NamedProxies UmbraObjectProxy::namedProxies_;
#endif