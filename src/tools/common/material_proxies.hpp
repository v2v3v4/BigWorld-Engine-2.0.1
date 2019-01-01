/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATERIAL_PROXIES_HPP
#define MATERIAL_PROXIES_HPP


#include "moo/managed_effect.hpp"
#include "gizmo/general_properties.hpp"


class MaterialPropertiesUser;



/**
 *	The editor effect property simply adds a save interface to 
 *	the base class.
 */
class EditorEffectProperty : public Moo::EffectProperty
{
public:
	EditorEffectProperty( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	virtual void save( DataSectionPtr pSection ) = 0;

	bool boolAnnotation( const std::string& annotation, bool& retVal ) const;
	bool intAnnotation( const std::string& annotation, int32& retVal ) const;
	bool stringAnnotation( const std::string& annotation, std::string& retVal ) const;
	bool floatAnnotation( const std::string& annotation, float& retVal ) const;
	const std::string& name() const;
	void setParent( const Moo::EffectProperty* pParent );
protected:

	typedef std::map<std::string, bool> BoolAnnotations;
	typedef std::map<std::string, int32> IntAnnotations;
	typedef std::map<std::string, std::string> StringAnnotations;
	typedef std::map<std::string, float> FloatAnnotations;

	class AnnotationData : public ReferenceCount
	{
	public:
		BoolAnnotations boolAnnotations_;
		IntAnnotations intAnnotations_;
		StringAnnotations stringAnnotations_;
		FloatAnnotations floatAnnotations_;
		std::string name_;

		static std::string s_emptyString_;
	};

	SmartPointer<AnnotationData> pAnnotations_;
};


/**
 *	Base class for all material proxies
 */
class BaseMaterialProxy : public ReferenceCount
{
public:
	BaseMaterialProxy() {}
	virtual ~BaseMaterialProxy() {}
};


/**
 *	Wrapper class for all material proxies
 */
template <class CL, class DT> class MaterialProxy : public BaseMaterialProxy
{
public:
	MaterialProxy( SmartPointer<CL> proxy, MaterialPropertiesUser * callback ) :
		callback_( callback )
	{
		BW_GUARD;

		proxies_.push_back( proxy );
		if (callback_)
		{
			callback_->proxySetCallback();
		}
	}

	void addProperty( SmartPointer<CL> proxy )
	{
		BW_GUARD;

		proxies_.push_back( proxy );
	}

	DT get() const
	{
		BW_GUARD;

		return proxies_[0]->get();
	}

	void set( DT val )
	{
		BW_GUARD;

		for ( std::vector< SmartPointer<CL> >::iterator it = proxies_.begin(); it != proxies_.end(); it++ )
			if (*it)
				(*it)->set( val, true );
		if (callback_)
		{
			callback_->proxySetCallback();
		}
	}

	bool getRange( int& min, int& max )
	{
		BW_GUARD;

		return proxies_[0]->getRange( min, max );
	}

	bool getRange( float& min, float& max, int& digits )
	{
		BW_GUARD;

		return proxies_[0]->getRange( min, max, digits );
	}

private:
	MaterialPropertiesUser * callback_;
	std::vector< SmartPointer<CL> > proxies_;
};


/**
 *	This class is used to avoid multiple-inheritance ambiguity of incRef/decRef
 *	in material proxies, since they inherit from EditorEffectProperty and from
 *	a proxy class, both classes ref-counted.
 */
template<typename Outer, typename Parent> class ProxyHolder
{
public:
	class InnerProxy : public Parent
	{
		Outer& outer_;
	public:
		InnerProxy( Outer& outer ) : outer_( outer )
		{
		}

		void EDCALL set( typename Parent::Data value, bool transient, bool addBarrier = true )
		{
			BW_GUARD;

			outer_.set( value, transient, addBarrier );
		}

		typename Parent::Data EDCALL get() const
		{
			BW_GUARD;

			return outer_.get();
		}

		void EDCALL getMatrix( Matrix & m, bool world = true )
		{
			BW_GUARD;

			outer_.getMatrix( m, world );
		}

		void EDCALL getMatrixContext( Matrix & m )
		{
			BW_GUARD;

			outer_.getMatrix( m );
		}

		void EDCALL getMatrixContextInverse( Matrix & m )
		{
			BW_GUARD;

			outer_.getMatrixContextInverse( m );
		}

		bool EDCALL setMatrix( const Matrix & m )
		{
			BW_GUARD;

			return outer_.setMatrix( m );
		}

		void EDCALL recordState()
		{
			BW_GUARD;

			outer_.recordState();
		}

		bool EDCALL commitState( bool revertToRecord = false, bool addUndoBarrier = true )
		{
			BW_GUARD;

			return outer_.commitState( revertToRecord, addUndoBarrier );
		}

		bool EDCALL hasChanged()
		{
			BW_GUARD;

			return outer_.hasChanged();
		}

        bool getRange( float& min, float& max, int& digits ) const
        {
			BW_GUARD;

            return outer_.getRange( min, max, digits );
        }

        bool getRange( typename Parent::Data& min, typename Parent::Data& max ) const
        {
			BW_GUARD;

            return outer_.getRange( min, max );
        }

		typedef SmartPointer<InnerProxy> SP;
	};

	typename InnerProxy::SP proxy()
	{
		BW_GUARD;

		if( !ptr_ )
			ptr_ = new InnerProxy( *static_cast< Outer* >( this ) );
		return ptr_;
	}

private:
	typename InnerProxy::SP ptr_;
};


/**
 *	Material Bool Proxy
 */
class MaterialBoolProxy : public EditorEffectProperty, public ProxyHolder< MaterialBoolProxy,BoolProxy >
{
public:
	MaterialBoolProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
	  EditorEffectProperty( pEffect, hProperty )
	{
	}
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );

	void EDCALL set( bool value, bool transient, bool addBarrier = true );
	bool EDCALL get() const;

	bool be( const Vector4 & v );
	void asVector4( Vector4 & v ) const;
	bool be( const bool & b );
	bool getBool( bool & b ) const;

	void save( DataSectionPtr pSection );

	Moo::EffectProperty* clone() const;
protected:
	bool value_;
};


/**
 *	Material Int Proxy
 */
class MaterialIntProxy : public EditorEffectProperty, public ProxyHolder< MaterialIntProxy,IntProxy >
{
public:
	MaterialIntProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL );
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	void EDCALL set( int32 value, bool transient,
											bool addBarrier = true  );
	int32 EDCALL get() const { return value_; }
	bool be( const Vector4 & v ) { value_ = int(v.x); return true; }
	void asVector4( Vector4 & v ) const { v.x = float( value_ ); }
	bool be( const int & i ) { value_ = i; return true; }
	bool getInt( int & i ) const { i = value_; return true; }
	void save( DataSectionPtr pSection );
    bool getRange( int32& min, int32& max ) const;
    void setRange( int32 min, int32 max );
	virtual void attach( D3DXHANDLE hProperty, ID3DXEffect* pEffect );
	void setParent( const Moo::EffectProperty* pParent );
	Moo::EffectProperty* clone() const;
protected:
	int32 value_;
    bool ranged_;
    int32 min_;
    int32 max_;
};


/**
 *	Material Float Proxy
 */
class MaterialFloatProxy : public EditorEffectProperty, public ProxyHolder< MaterialFloatProxy,FloatProxy >
{
public:
	MaterialFloatProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
		EditorEffectProperty( pEffect, hProperty ),
		value_(0.f), ranged_(false)
	{
	}

	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		BW_GUARD;

		return SUCCEEDED( pEffect->SetFloat( hProperty, value_ ) );
	}

	float EDCALL get() const { return value_; }

	void EDCALL set( float f, bool transient, bool addBarrier = true ) { value_ = f; }

	bool be( const Vector4 & v ) { value_ = v.x; return true; }
	void asVector4( Vector4 & v ) const { v.x = value_; }
	bool be( const float & f ) { value_ = f; return true; }
	bool getFloat( float & f ) const { f = value_; return true; }

	void save( DataSectionPtr pSection );

    bool getRange( float& min, float& max, int& digits ) const;
    void setRange( float min, float max, int digits );

	virtual void attach( D3DXHANDLE hProperty, ID3DXEffect* pEffect );

	void setParent( const Moo::EffectProperty* pParent );
	Moo::EffectProperty* clone() const;
protected:
	float value_;
    bool ranged_;
    float min_;
    float max_;
    int digits_;
};


/**
 *	Material Vector4 Proxy
 */
class MaterialVector4Proxy : public EditorEffectProperty, public ProxyHolder< MaterialVector4Proxy,Vector4Proxy >
{
public:
	MaterialVector4Proxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) : 
		EditorEffectProperty( pEffect, hProperty ),
		value_(0.f,0.f,0.f,0.f) 
	{}

	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );

	bool be( const Vector4 & v );
	bool getVector( Vector4 & v ) const;
	
	Vector4 EDCALL get() const;
	void EDCALL set( Vector4 f, bool transient, bool addBarrier = true );

	void save( DataSectionPtr pSection );

	Moo::EffectProperty* clone() const;
protected:
	Vector4 value_;
};


/**
 *	Material Matrix Proxy
 */
class MaterialMatrixProxy : public EditorEffectProperty, public ProxyHolder< MaterialMatrixProxy,MatrixProxy >
{
public:
	MaterialMatrixProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
		EditorEffectProperty( pEffect, hProperty )
	{}

	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	bool EDCALL setMatrix( const Matrix & m );
	void EDCALL getMatrix( Matrix& m, bool world = true ) { m = value_; }
    void EDCALL getMatrixContext( Matrix& m )   {};
    void EDCALL getMatrixContextInverse( Matrix& m )   {};
	void save( DataSectionPtr pSection );
    void EDCALL recordState()   {};
	bool EDCALL commitState( bool revertToRecord = false, bool addUndoBarrier = true );
    bool EDCALL hasChanged()    { return true; }
	bool be( const Vector4 & v );
	void asVector4( Vector4 & v ) const;
	bool be( const Matrix & m );
	bool getMatrix( Matrix & m ) const;

	Moo::EffectProperty* clone() const;
protected:
	Matrix value_;
};


/**
 *	Material Colour Proxy
 */
class MaterialColourProxy : public EditorEffectProperty, public ProxyHolder< MaterialColourProxy,ColourProxy >
{
public:
	MaterialColourProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
		EditorEffectProperty( pEffect, hProperty ), 
		value_(0.f,0.f,0.f,0.f)
	{}

	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	
	Moo::Colour EDCALL get() const;
	void EDCALL set( Moo::Colour f, bool transient, bool addBarrier = true );
    
    Vector4 EDCALL getVector4() const;
    void EDCALL setVector4( Vector4 f, bool transient );

	void save( DataSectionPtr pSection );

	Moo::EffectProperty* clone() const;
protected:
	Vector4 value_;
};


/**
 *	Material Texture Proxy
 */
class MaterialTextureProxy : public EditorEffectProperty, public ProxyHolder< MaterialTextureProxy,StringProxy >
{
public:
	MaterialTextureProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
		EditorEffectProperty( pEffect, hProperty )
	{
	}
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	void EDCALL set( std::string value, bool transient,
											bool addBarrier = true  );
	std::string EDCALL get() const { return resourceID_; };
	bool be( const Moo::BaseTexturePtr pTex );
	bool be( const std::string& s );
	void save( DataSectionPtr pSection );
	bool getResourceID( std::string & s ) const;
	Moo::EffectProperty* clone() const;
protected:
	std::string		resourceID_;
	Moo::BaseTexturePtr value_;
};


/**
 *	Material Texture Feed Proxy
 */
class MaterialTextureFeedProxy : public EditorEffectProperty, public ProxyHolder< MaterialTextureFeedProxy, StringProxy >
{
public:
	MaterialTextureFeedProxy( ID3DXEffect* pEffect = NULL, D3DXHANDLE hProperty = NULL ) :
		EditorEffectProperty( pEffect, hProperty )
	{
	}
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty );
	void EDCALL set( std::string value, bool transient,
												bool addBarrier = true );
	std::string EDCALL get() const { return resourceID_; };
	void save( DataSectionPtr pSection );
	void setTextureFeed( std::string value );

	bool be( const Moo::BaseTexturePtr pTex ) { value_ = pTex; return true; }
	bool be( const std::string& s )		 { value_ = Moo::TextureManager::instance()->get(s); return true; }
	bool getResourceID( std::string & s ) const { s = value_ ? value_->resourceID() : ""; return true; }
	Moo::EffectProperty* clone() const;
protected:
	std::string		resourceID_;
	std::string		textureFeed_;
	Moo::BaseTexturePtr value_;
};


#endif // MATERIAL_PROXIES_HPP
