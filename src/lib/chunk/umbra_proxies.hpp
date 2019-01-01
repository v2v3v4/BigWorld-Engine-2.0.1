/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UMBRA_PROXIES_HPP
#define UMBRA_PROXIES_HPP

#include "umbra_config.hpp"

#if UMBRA_ENABLE

namespace Umbra
{
	class Model;
	class Object;
};


/**
 *	This class implements the umbra model proxy.
 *	The umbra model proxy keeps a reference counted reference to a
 *	umbra model. The umbra model proxy registers itself in a list
 *	so that the umbra model can be destroyed before umbra is destroyed.
 */
class UmbraModelProxy : public SafeReferenceCount
{
public:
	~UmbraModelProxy();

	void invalidate();

	Umbra::Model*	model() const { return pModel_; }
	void			model(Umbra::Model* pModel){ pModel_ = pModel; }

	static UmbraModelProxy* getObbModel( const Matrix& obb );
	static UmbraModelProxy* getObbModel( const Vector3* pVertices, uint32 nVertices );
	static UmbraModelProxy* getObbModel( const Vector3& min, const Vector3& max );
	static UmbraModelProxy* getMeshModel( const Vector3* pVertices, const uint32* pIndices, 
		uint32 nVertices, uint32 nTriangles, bool clockwise = true );
	static UmbraModelProxy* getSphereModel( const Vector3& center, float radius );
	static UmbraModelProxy* getSphereModel( const Vector3* pVertices, uint32 nVertices );

	static void invalidateAll();
private:
	static UmbraModelProxy* get( Umbra::Model* pModel );
	static void del( UmbraModelProxy* pProxy );
	UmbraModelProxy();
	Umbra::Model* pModel_;

	typedef std::vector<UmbraModelProxy*> Proxies;
	static Proxies proxies_;

};

typedef SmartPointer<UmbraModelProxy> UmbraModelProxyPtr;

/**
 *	This class implements the umbra object proxy.
 *	The umbra object proxy keeps a reference counted reference to a
 *	umbra object. The umbra object proxy registers itself in a list
 *	so that the umbra object can be destroyed before umbra is destroyed.
 */
class UmbraObjectProxy : public SafeReferenceCount
{
public:
	~UmbraObjectProxy();
	void invalidate();
	void				pModelProxy( UmbraModelProxyPtr pModel );
	UmbraModelProxyPtr	pModelProxy() const { return pModel_; }
	Umbra::Object*		object() const { return pObject_; }
	void				object(Umbra::Object* pObject){ pObject_ = pObject; }

	static UmbraObjectProxy* get( UmbraModelProxyPtr pModel, UmbraModelProxyPtr pWriteModel = NULL,
		const std::string& identifier = "" );
	static UmbraObjectProxy* getCopy( const std::string& identifier );
	static UmbraObjectProxy* get( Umbra::Object* pObject );
	
	static void invalidateAll();
private:
	static void del( UmbraObjectProxy* pProxy );
	UmbraObjectProxy(UmbraModelProxyPtr pModel = NULL,
		UmbraModelProxyPtr pWriteModel = NULL);
	Umbra::Object* pObject_;

	UmbraModelProxyPtr pModel_;
	UmbraModelProxyPtr pWriteModel_;
	typedef std::vector<UmbraObjectProxy*> Proxies;
	static Proxies proxies_;
	typedef std::map<std::string, UmbraObjectProxy*> NamedProxies;
	static NamedProxies namedProxies_;
};

typedef SmartPointer<UmbraObjectProxy> UmbraObjectProxyPtr;

#endif // UMBRA_ENABLE

#endif // UMBRA_PROXIES_HPP
/*umbra_proxies.hpp*/
