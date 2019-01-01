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
#include "effect_visual_context.hpp"
#include "render_context_callback.hpp"


DECLARE_DEBUG_COMPONENT2( "Moo", 0 )


BW_SINGLETON_STORAGE( Moo::EffectVisualContext );


namespace Moo
{

#define MAX_POINT_LIGHTS 		4U
#define MAX_SPEC_POINT_LIGHTS 	2U

#define MAX_DIRECTIONAL_LIGHTS 	2U

#define MAX_SPOT_LIGHTS 		2U


//-----------------------------------------------------------------------------
// Section: FixedListAllocator
//-----------------------------------------------------------------------------

/**
 *	A template class for allocation of objects maintained in a static
 *	list per object type. The objects in the list are recycled each frame.
 */
template<typename ObjectType>
class FixedListAllocator : public RenderContextCallback
{
public:
	FixedListAllocator() :
	  offset_( 0 )
	{
	}

	~FixedListAllocator()
	{
		BW_GUARD;
		// Make sure fini was called and no further allocations were made.
		MF_ASSERT_DEV( container_.empty() );
	}

	typedef SmartPointer<ObjectType> ObjectPtr;
	typedef std::vector<ObjectPtr> Container;

	// TODO: Find a way to remove this singleton/static
	static FixedListAllocator& instance()
	{
		static FixedListAllocator s_instance;
		return s_instance;
	}

	ObjectType* alloc()
	{
		BW_GUARD;
		static uint32 timestamp = -1;
		if (!rc().frameDrawn(timestamp))
			offset_ = 0;
		if (offset_ >= container_.size())
		{
			container_.push_back( new ObjectType );
		}
		return container_[offset_++].getObject();
	}

	/*virtual*/ void renderContextFini()
	{
		container_.clear();
	}

private:
	Container	container_;
	uint32		offset_;
};


//-----------------------------------------------------------------------------
// Section: EffectVisualContext
//-----------------------------------------------------------------------------

/**
 * This method sets up the constant mappings to prepare for rendering a visual.
 */
void EffectVisualContext::initConstants()
{
	BW_GUARD;
	if (!overrideConstants_)
	{
		ConstantMappings::iterator it = constantMappings_.begin();
		ConstantMappings::iterator end = constantMappings_.end();

		while (it != end)
		{
			*(it->first) = it->second;
			it++;
		}
	}
}


/**
 * Recorded Vector4 constant array.
 */
class RecordedVector4Array : public RecordedEffectConstant
{
public:
	RecordedVector4Array(){}
	void init(ID3DXEffect* pEffect, D3DXHANDLE constantHandle, uint32 nConstants)
	{
		BW_GUARD;
		pEffect_ = pEffect;
		constantHandle_ = constantHandle;
		nConstants_ = nConstants;
		if (constants_.size() < nConstants_)
		{
			constants_.resize( nConstants_ );
		}
	}
	void apply()
	{
			BW_GUARD;
			pEffect_->SetVectorArray( constantHandle_, &constants_.front(), nConstants_ );
			pEffect_->SetArrayRange( constantHandle_, 0, nConstants_ );
	}
	static RecordedVector4Array* get( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, uint32 nConstants )
	{
		BW_GUARD;
		RecordedVector4Array* pRet = FixedListAllocator<RecordedVector4Array>::instance().alloc();
		pRet->init( pEffect, constantHandle, nConstants );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	D3DXHANDLE constantHandle_;
	uint32 nConstants_;
	std::vector<Vector4> constants_;
};

/**
 *	World transformation palette. Used for skinning.
 */
class WorldPaletteConstant : public EffectConstantValue
{
public:

	void captureConstants( Vector4* pTransforms, uint32& nConstants )
	{
		BW_GUARD;
		nConstants = 0;
		NodePtrVector::iterator it = EffectVisualContext::instance().pRenderSet()->transformNodes_.begin();
		NodePtrVector::iterator end = EffectVisualContext::instance().pRenderSet()->transformNodes_.end();
		Matrix m;
		while (it != end)
		{
			XPMatrixTranspose( &m, &(*it)->worldTransform() );
			pTransforms[nConstants++] = m.row(0);
			pTransforms[nConstants++] = m.row(1);
			pTransforms[nConstants++] = m.row(2);
			it++;
		}
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet())
		{		
			static Vector4 transforms[256*3];
			uint32 nConstants = 0;
			captureConstants( transforms, nConstants );
			pEffect->SetVectorArray( constantHandle, transforms, nConstants );
			pEffect->SetArrayRange( constantHandle, 0, nConstants );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{ 
		BW_GUARD;
		RecordedVector4Array* pRecorder = NULL;
		if (EffectVisualContext::instance().pRenderSet())
		{
			uint32 nConstants = EffectVisualContext::instance().pRenderSet()->transformNodes_.size() * 3;
			pRecorder = RecordedVector4Array::get( pEffect, constantHandle, nConstants );
			captureConstants( &pRecorder->constants_.front(), nConstants );
		}
		return pRecorder;
	}
};


/**
 *	A generic recorded matrix effect constant.
 */
class RecordedMatrixConstant : public RecordedEffectConstant, public Aligned
{
public:
	RecordedMatrixConstant(){}
	void init( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, const Matrix& transform )
	{
		BW_GUARD;
		pEffect_ = pEffect;
		constantHandle_ = constantHandle;
		transform_ = transform;
	}
	void apply()
	{
		BW_GUARD;
		pEffect_->SetMatrix( constantHandle_, &transform_ );
	}
	static RecordedMatrixConstant* get( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, const Matrix& transform )
	{
		BW_GUARD;
		RecordedMatrixConstant* pRet = FixedListAllocator<RecordedMatrixConstant>::instance().alloc();
		pRet->init( pEffect, constantHandle, transform );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	D3DXHANDLE constantHandle_;
	Matrix transform_;
};

/**
 *	The ViewProjection transformation matrix constant.
 */
class ViewProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetMatrix( constantHandle, &Moo::rc().viewProjection() );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		return RecordedMatrixConstant::get( pEffect, constantHandle, Moo::rc().viewProjection() );
	}
};


/**
 *	The InvViewProjection transformation matrix constant.
 */
class InvViewProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Matrix m(Moo::rc().viewProjection());
		m.invert();
		pEffect->SetMatrix( constantHandle, &m );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Matrix m(Moo::rc().viewProjection());
		m.invert();
		return RecordedMatrixConstant::get( pEffect, constantHandle, m );
	}
};



/**
 *	The ViewProjection transformation matrix constant.
 */
class LastViewProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetMatrix( constantHandle, &Moo::rc().lastViewProjection() );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		return RecordedMatrixConstant::get( pEffect, constantHandle, Moo::rc().lastViewProjection() );
	}
};

/**
 *	The World transformation matrix constant.
 */
class WorldConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			pEffect->SetMatrix( constantHandle, &EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform() );
		}
		else
		{
			pEffect->SetMatrix( constantHandle, &Matrix::identity );
		}

		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			return RecordedMatrixConstant::get( pEffect, constantHandle, 
				EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform() );
		}
		return RecordedMatrixConstant::get( pEffect, constantHandle, Matrix::identity );
	}
};

/**
 *	The InverseWorld transformation matrix constant.
 */
class InverseWorldConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Matrix m;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			m = EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform();
		}
		else
		{
			m = Matrix::identity;
		}

		m.invert();
		pEffect->SetMatrix( constantHandle, &m );

		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Matrix m;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			m = EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform();
			m.invert();
		}
		else
		{
			m = Matrix::identity;
		}
		return RecordedMatrixConstant::get( pEffect, constantHandle, m );
	}
};

/**
 *	The View transformation matrix constant.
 */
class ViewConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetMatrix( constantHandle, &Moo::rc().view() );
		return true;
	}
};

/**
 *	The InvView transformation matrix constant.
 */
class InvViewConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetMatrix( constantHandle, &Moo::rc().invView() );
		return true;
	}
};

/**
 *	The Projection transformation matrix constant.
 */
class ProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetMatrix( constantHandle, &Moo::rc().projection() );
		return true;
	}
};

/**
 *	Screen dimensions stored in a vector4 constant.
 */
class ScreenConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Vector4 v( rc().screenWidth(),
			rc().screenHeight(),
			rc().halfScreenWidth(),
			rc().halfScreenHeight() );
		pEffect->SetVector( constantHandle, &v );
		return true;
	}
};

/**
 *	The WorldView transformation matrix constant.
 */
class WorldViewConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wv;
			wv.multiply( EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform(), 
				Moo::rc().view() );
			pEffect->SetMatrix( constantHandle, &wv );
		}
		else
		{
			pEffect->SetMatrix( constantHandle, &rc().view() );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wv;
			wv.multiply( EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform(), 
				Moo::rc().view() );
			return RecordedMatrixConstant::get( pEffect, constantHandle, wv );
		}
		return RecordedMatrixConstant::get( pEffect, constantHandle, rc().view() );
	}
};

/**
 *	The WorldViewProjection transformation matrix constant.
 */
class WorldViewProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wvp;
			wvp.multiply( EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform(), 
				Moo::rc().viewProjection() );
			pEffect->SetMatrix( constantHandle, &wvp );
		}
		else
		{
			pEffect->SetMatrix( constantHandle, &rc().viewProjection() );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wvp;
			wvp.multiply( EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->worldTransform(), 
				Moo::rc().viewProjection() );
			return RecordedMatrixConstant::get( pEffect, constantHandle, wvp );
		}
		return RecordedMatrixConstant::get( pEffect, constantHandle, rc().viewProjection() );
	}
};

/**
 *	The WorldViewProjection transformation matrix constant.
 */
/*class LastWorldViewProjectionConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wvp;
			wvp.multiply(
				EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->lastWorldTransform(), 
				Moo::rc().lastViewProjection() );
			pEffect->SetMatrix( constantHandle, &wvp );
		}
		else
		{
			pEffect->SetMatrix( constantHandle, &rc().lastViewProjection() );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (EffectVisualContext::instance().pRenderSet() &&
			!EffectVisualContext::instance().pRenderSet()->treatAsWorldSpaceObject_)
		{
			Matrix wvp;
			wvp.multiply( EffectVisualContext::instance().pRenderSet()->transformNodes_.front()->lastWorldTransform(), 
				Moo::rc().lastViewProjection() );
			return RecordedMatrixConstant::get( pEffect, constantHandle, wvp );
		}
		return RecordedMatrixConstant::get( pEffect, constantHandle, rc().lastViewProjection() );
	}
};*/

/**
 *	The EnvironmentTransform transformation matrix constant. 
 *	(ViewProjection matrix with the View translation removed)
 */
class EnvironmentTransformConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		BW_GUARD;
		Matrix tr( Moo::rc().view() );
		tr.translation(Vector3(0,0,0));
		tr.postMultiply( Moo::rc().projection() );

		pEffect->SetMatrix( constantHandle, &tr );
		return true;
	}	
};


/**
 *	The EnvironmentShadowTransformConstant makes a matrix that
 *	take a position from world coordinates to clip coordinates.
 */
class EnvironmentShadowTransformConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{			
		BW_GUARD;
		float farPlane = Moo::rc().camera().farPlane();

		Matrix m;
		m.row(0, Vector4(  1.f / farPlane,	0,					0,		0 ) );
		m.row(1, Vector4(  0,				0,					0,		0 ) );
		m.row(2, Vector4(  0,				1.f / farPlane,		0,		0 ) );		
		m.row(3, Vector4(  0,				0,					0,		1 ) );		

		pEffect->SetMatrix( constantHandle, &m );
		return true;
	}	
};


/**
 *	The FarPlane constant (fp, 1.f/fp, skyfp, 1.f/skyfp)
 *	The sky has a different, 'virtual' far plane to allow
 *	it to appear as though it's much bigger than the world.
 */
class FarPlaneConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{	
		BW_GUARD;
		float skyFarPlane = 1500.f;

		Vector4 fp;
		fp.x = Moo::rc().camera().farPlane();
		fp.y = 1.f / fp.x;
		fp.z = skyFarPlane;
		fp.w = 1.f / fp.z;		
		pEffect->SetVector( constantHandle, &fp );
		return true;
	}	
};


/**
 *	A generic recorded Vector4 effect constant.
 */
class RecordedVector4Constant : public RecordedEffectConstant
{
public:
	RecordedVector4Constant(){}
	void init( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, const Vector4& value )
	{
		BW_GUARD;
		pEffect_ = pEffect;
		constantHandle_ = constantHandle;
		value_ = value;
	}
	void apply()
	{
		BW_GUARD;
		pEffect_->SetVector( constantHandle_, &value_ );
	}
	static RecordedVector4Constant* get( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, const Vector4& value )
	{
		BW_GUARD;
		RecordedVector4Constant* pRet = FixedListAllocator<RecordedVector4Constant>::instance().alloc();
		pRet->init( pEffect, constantHandle, value );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	D3DXHANDLE constantHandle_;
	Vector4 value_;
};

/**
 *	Ambient light colour constant.
 */
class AmbientConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		if (Moo::rc().lightContainer())
			pEffect->SetVector( constantHandle, (Vector4*)(&Moo::rc().lightContainer()->ambientColour())  );
		else
			pEffect->SetVector( constantHandle, &Vector4( 1.f, 1.f, 1.f, 1.f ));
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{	
		BW_GUARD;
		if (Moo::rc().lightContainer())
			return RecordedVector4Constant::get( pEffect, constantHandle, 
				(const Vector4&)Moo::rc().lightContainer()->ambientColour() );
		
		return RecordedVector4Constant::get( pEffect, constantHandle, 
			Vector4( 1.f, 1.f, 1.f, 1.f ) );
	}
};


/**
 *	A Recorded directional light effect constant.
 */
class RecordedDirectionalConstant : public RecordedEffectConstant
{
public:
	/**
	 * TODO: to be documented.
	 */
	struct ShadDirLight
	{
		Vector3 direction_;
		Colour colour_;
	};
	RecordedDirectionalConstant()
	{
	}
	void init( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
		pEffect_ = pEffect;
		nConstants_ = nConstants;
		baseHandle_ = baseHandle;
	}
	void apply()
	{
		BW_GUARD;
		for (uint32 i = 0; i < nConstants_; i++)
		{
			pEffect_->SetValue( constants_[i].first, 
				&constants_[i].second, sizeof( ShadDirLight ) );
		}
		pEffect_->SetArrayRange( baseHandle_, 0, nConstants_ );
	}
	static RecordedDirectionalConstant* get( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
		RecordedDirectionalConstant* pRet = FixedListAllocator<RecordedDirectionalConstant>::instance().alloc();
		pRet->init( pEffect, nConstants, baseHandle );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	typedef std::pair<D3DXHANDLE, ShadDirLight> DirConstant;
	DirConstant constants_[2];
	uint32 nConstants_;
	D3DXHANDLE baseHandle_;

};

/**
 *	A directional light effect constant.
 */
class DirectionalConstant : public EffectConstantValue
{
public:
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		RecordedDirectionalConstant* pConstant = NULL;
		const Matrix& invWorld = EffectVisualContext::instance().invWorld();

		LightContainer * pContainer;
		if (diffuse_)
			pContainer = Moo::rc().lightContainer().get();
		else
			pContainer = Moo::rc().specularLightContainer().get();

		if (pContainer != NULL)
		{
			DirectionalLightVector& dLights = pContainer->directionals();
			if (dLights.size() > 0)
			{
				uint32 nLights =  MAX_DIRECTIONAL_LIGHTS < dLights.size() ?
									MAX_DIRECTIONAL_LIGHTS : dLights.size();
				pConstant = RecordedDirectionalConstant::get(pEffect, nLights, constantHandle);
				for (uint32 i = 0; i < nLights; i++)
				{
					RecordedDirectionalConstant::ShadDirLight& constant = pConstant->constants_[i].second;
					if (objectSpace_)
					{
						invWorld.applyVector( constant.direction_, dLights[i]->worldDirection() );
						constant.direction_.normalise();
					}
					else
					{
						constant.direction_ = dLights[i]->worldDirection();
					}
					constant.colour_ = dLights[i]->colour() * dLights[i]->multiplier();
					pConstant->constants_[i].first = pEffect->GetParameterElement( constantHandle, i );
				}
			}
		}
		return pConstant;
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		struct ShadDirLight
		{
			Vector3 direction_;
			Colour colour_;
		};
		static ShadDirLight constant;

		const Matrix& invWorld = EffectVisualContext::instance().invWorld();

		LightContainer * pContainer;
		if (diffuse_)
		{
			pContainer = Moo::rc().lightContainer().get();
		}
		else
		{
			pContainer = Moo::rc().specularLightContainer().get();
		}

		if (pContainer != NULL)
		{

			DirectionalLightVector& dLights = pContainer->directionals();
			uint32 nLights =  MAX_DIRECTIONAL_LIGHTS < dLights.size() ?
								MAX_DIRECTIONAL_LIGHTS : dLights.size();
			for (uint32 i = 0; i < nLights; i++)
			{
				D3DXHANDLE elemHandle = pEffect->GetParameterElement( constantHandle, i );
				if (elemHandle)
				{
					if (objectSpace_)
					{
						invWorld.applyVector( constant.direction_, dLights[i]->worldDirection() );
						constant.direction_.normalise();
					}
					else
					{
						constant.direction_ = dLights[i]->worldDirection();
					}
					constant.colour_ = dLights[i]->colour() * dLights[i]->multiplier();
					pEffect->SetValue( elemHandle, &constant, sizeof( constant ) );
				}
			}
			pEffect->SetArrayRange( constantHandle, 0, nLights );
		}
		return true;
	}
	DirectionalConstant( bool diffuse = true, bool objectSpace = false ) : 
		objectSpace_( objectSpace ),
		diffuse_( diffuse )
	{

	}
private:
	bool objectSpace_;
	bool diffuse_;
};


/**
 *	A recorded omni directional light source constant.
 */
class RecordedOmniConstant : public RecordedEffectConstant
{
public:
	/**
	 * The omni light parameters
	 */
	struct ShadPointLight
	{
		Vector3	position_;
		Colour	colour_;
		Vector2	attenuation_;
	};
	RecordedOmniConstant(){}
	void init( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
			pEffect_ = pEffect;
		nConstants_ = nConstants;
		baseHandle_ = baseHandle;
	}

	void apply()
	{
		BW_GUARD;
		for (uint32 i = 0; i < nConstants_; i++)
		{
			pEffect_->SetValue( constants_[i].first, 
				&constants_[i].second, sizeof( ShadPointLight ) );
		}
		pEffect_->SetArrayRange( baseHandle_, 0, nConstants_ );
	}
	static RecordedOmniConstant* get( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
		RecordedOmniConstant* pRet = FixedListAllocator<RecordedOmniConstant>::instance().alloc();
		pRet->init( pEffect, nConstants, baseHandle );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	typedef std::pair<D3DXHANDLE, ShadPointLight> PointConstant;
	PointConstant constants_[4];
	uint32 nConstants_;
	D3DXHANDLE baseHandle_;
};

/**
 *	A omni directional light source constant.
 */
class OmniConstant : public EffectConstantValue
{
public:
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		const Matrix& invWorld = EffectVisualContext::instance().invWorld();
		float scale = EffectVisualContext::instance().invWorldScale();

		LightContainer * pContainer;
		uint32 maxLights;
		if (diffuse_)
		{
			maxLights = MAX_POINT_LIGHTS;
			pContainer = Moo::rc().lightContainer().get();
		}
		else
		{
			maxLights = MAX_SPEC_POINT_LIGHTS;
			pContainer = Moo::rc().specularLightContainer().get();
		}

		RecordedOmniConstant* pConstant = NULL;
		if (pContainer != NULL)
		{
			OmniLightVector& oLights = pContainer->omnis();
			uint32 nLights = maxLights < oLights.size() ? maxLights : oLights.size();
			if (nLights > 0)
			{
				pConstant = RecordedOmniConstant::get( pEffect, nLights, constantHandle );
				for (uint32 i = 0; i < nLights; i++)
				{
					D3DXHANDLE elemHandle = pEffect->GetParameterElement( constantHandle, i );
					pConstant->constants_[i].first = elemHandle;
					RecordedOmniConstant::ShadPointLight& constant = pConstant->constants_[i].second;
					if (elemHandle)
					{
						float innerRadius = oLights[i]->worldInnerRadius();
						float outerRadius = oLights[i]->worldOuterRadius();
						if (objectSpace_)
						{
							innerRadius *= scale;
							outerRadius *= scale;
							invWorld.applyPoint( constant.position_, oLights[i]->worldPosition() );
						}
						else
						{
							constant.position_ = oLights[i]->worldPosition();
						}

						constant.colour_ = oLights[i]->colour() * oLights[i]->multiplier();
						constant.attenuation_.set( outerRadius, 1.f / ( outerRadius - innerRadius ) );
					}
				}
			}
		}
		return pConstant;
	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		struct ShadPointLight
		{
			Vector3 position_;
			Colour colour_;
			Vector2 attenuation_;
		};
		static ShadPointLight constant;

		const Matrix& invWorld = EffectVisualContext::instance().invWorld();
		float scale = EffectVisualContext::instance().invWorldScale();

		LightContainer * pContainer;
		uint32 maxLights;
		if (diffuse_)
		{
			maxLights = MAX_POINT_LIGHTS;
			pContainer = Moo::rc().lightContainer().get();
		}
		else
		{
			maxLights = MAX_SPEC_POINT_LIGHTS;
			pContainer = Moo::rc().specularLightContainer().get();
		}
		if (pContainer != NULL)
		{
			OmniLightVector& oLights = pContainer->omnis();
			uint32 nLights = maxLights < oLights.size() ? maxLights : oLights.size();
			for (uint32 i = 0; i < nLights; i++)
			{
				D3DXHANDLE elemHandle = pEffect->GetParameterElement( constantHandle, i );
				if (elemHandle)
				{
					float innerRadius = oLights[i]->worldInnerRadius();
					float outerRadius = oLights[i]->worldOuterRadius();
					if (objectSpace_)
					{
						innerRadius *= scale;
						outerRadius *= scale;
						invWorld.applyPoint( constant.position_, oLights[i]->worldPosition() );
					}
					else
					{
						constant.position_ = oLights[i]->worldPosition();
					}

					constant.colour_ = oLights[i]->colour() * oLights[i]->multiplier();
					constant.attenuation_.set( outerRadius, 1.f / ( outerRadius - innerRadius ) );
					pEffect->SetValue( elemHandle, &constant, sizeof( constant ) );
				}
			}
			pEffect->SetArrayRange( constantHandle, 0, nLights );
		}
		return true;
	}
	OmniConstant( bool diffuse = true, bool objectSpace = false ):
		objectSpace_( objectSpace ),
		diffuse_( diffuse )
	{

	}
private:
	bool objectSpace_;
	bool diffuse_;
};


/**
 *	A recorded spot light source constant.
 */
class RecordedSpotConstant : public RecordedEffectConstant
{
public:
	/**
	 *	The spot light parameters.
	 */
	struct ShadSpotLight
	{
		Vector3 position;
		Colour colour;
		Vector3 attenuation;
		Vector3 direction;
	};
	RecordedSpotConstant() {}

	void init( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
		pEffect_ = pEffect;
		nConstants_ = nConstants;
		baseHandle_ = baseHandle;
	}
	void apply()
	{
		BW_GUARD;
		for (uint32 i = 0; i < nConstants_; i++)
		{
			pEffect_->SetValue( constants_[i].first, 
				&constants_[i].second, sizeof( ShadSpotLight ) );
		}
		pEffect_->SetArrayRange( baseHandle_, 0, nConstants_ );
	}
	static RecordedSpotConstant* get( ID3DXEffect* pEffect, uint32 nConstants, D3DXHANDLE baseHandle )
	{
		BW_GUARD;
		RecordedSpotConstant* pRet = FixedListAllocator<RecordedSpotConstant>::instance().alloc();
		pRet->init( pEffect, nConstants, baseHandle );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	typedef std::pair<D3DXHANDLE, ShadSpotLight> SpotConstant;
	SpotConstant constants_[2];
	uint32 nConstants_;
	D3DXHANDLE baseHandle_;
};

/**
 *	A spot light source constant.
 */
class SpotDiffuseConstant : public EffectConstantValue
{
public:
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		const Matrix& invWorld = EffectVisualContext::instance().invWorld();
		float scale = EffectVisualContext::instance().invWorldScale();

		RecordedSpotConstant* pConstant = NULL;

		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
		{

			SpotLightVector& sLights = pContainer->spots();
			uint32 nLights = MAX_SPOT_LIGHTS < sLights.size() ? 
								MAX_SPOT_LIGHTS : sLights.size();
			if (nLights > 0)
			{
				pConstant = RecordedSpotConstant::get( pEffect, nLights, constantHandle );
				for (uint32 i = 0; i < 2 && i < sLights.size(); i++)
				{
					D3DXHANDLE elemHandle = pEffect->GetParameterElement( constantHandle, i );
					pConstant->constants_[i].first = elemHandle;
					RecordedSpotConstant::ShadSpotLight& constant = pConstant->constants_[i].second;
					if (elemHandle)
					{
						float innerRadius = sLights[i]->worldInnerRadius();
						float outerRadius = sLights[i]->worldOuterRadius();
						if (!objectSpace_)
						{
							constant.position = sLights[i]->worldPosition();
							constant.direction = sLights[i]->worldDirection();
						}
						else
						{
							innerRadius *= scale;
							outerRadius *= scale;
							constant.position = invWorld.applyPoint( sLights[i]->worldPosition() );
							constant.direction = invWorld.applyVector( sLights[i]->worldDirection() );
						}

						constant.colour = sLights[i]->colour() * sLights[i]->multiplier();
						constant.attenuation.set( outerRadius, 1.f / ( outerRadius - innerRadius ), sLights[i]->cosConeAngle() );
					}
				}
			}
		}
		return pConstant;
	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		struct ShadSpotLight
		{
			Vector3 position;
			Colour colour;
			Vector3 attenuation;
			Vector3 direction;
		};

		static ShadSpotLight constant;

		const Matrix& invWorld = EffectVisualContext::instance().invWorld();
		float scale = EffectVisualContext::instance().invWorldScale();

		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
		{
			SpotLightVector& sLights = pContainer->spots();
			uint32 nLights = MAX_SPOT_LIGHTS < sLights.size() ? 
								MAX_SPOT_LIGHTS : sLights.size();
			for (uint32 i = 0; i < 2 && i < sLights.size(); i++)
			{
				D3DXHANDLE elemHandle = pEffect->GetParameterElement( constantHandle, i );
				if (elemHandle)
				{
					float innerRadius = sLights[i]->worldInnerRadius();
					float outerRadius = sLights[i]->worldOuterRadius();
					if (!objectSpace_)
					{
						constant.position = sLights[i]->worldPosition();
						constant.direction = sLights[i]->worldDirection();
					}
					else
					{
						innerRadius *= scale;
						outerRadius *= scale;
						constant.position = invWorld.applyPoint( sLights[i]->worldPosition() );
						constant.direction = invWorld.applyVector( sLights[i]->worldDirection() );
					}

					constant.colour = sLights[i]->colour() * sLights[i]->multiplier();
					constant.attenuation.set( outerRadius, 1.f / ( outerRadius - innerRadius ), sLights[i]->cosConeAngle() );
					pEffect->SetValue( elemHandle, &constant, sizeof( constant ) );
				}
			}
			pEffect->SetArrayRange( constantHandle, 0, nLights );
		}
		return true;
	}
	SpotDiffuseConstant( bool objectSpace = false )
	: objectSpace_( objectSpace )
	{

	}
private:
	bool objectSpace_;
};


/**
 *	A generic recorded Integer constant.
 */
class RecordedIntConstant : public RecordedEffectConstant
{
public:
	RecordedIntConstant()
	{
	}
	void init( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, int value )
	{
	  BW_GUARD;
	  pEffect_ = pEffect;
	  constantHandle_ = constantHandle;
	  value_ = value;
	}
	void apply()
	{
		BW_GUARD;
		pEffect_->SetInt( constantHandle_, value_ );
	}
	static RecordedIntConstant* get( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, int value )
	{
		BW_GUARD;
		RecordedIntConstant* pRet = FixedListAllocator<RecordedIntConstant>::instance().alloc();
		pRet->init( pEffect, constantHandle, value );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	D3DXHANDLE constantHandle_;
	int value_;
};

/**
 *	A generic recorded BOOL constant.
 */
class RecordedBoolConstant : public RecordedEffectConstant
{
public:
	RecordedBoolConstant()
	{
	}
	void init( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, BOOL value )
	{
	  BW_GUARD;
	  pEffect_ = pEffect;
	  constantHandle_ = constantHandle;
	  value_ = value;
	}
	void apply()
	{
		BW_GUARD;
		pEffect_->SetBool( constantHandle_, value_ );
	}
	static RecordedBoolConstant* get( ID3DXEffect* pEffect, D3DXHANDLE constantHandle, BOOL value )
	{
		BW_GUARD;
		RecordedBoolConstant* pRet = FixedListAllocator<RecordedBoolConstant>::instance().alloc();
		pRet->init( pEffect, constantHandle, value );
		return pRet;
	}
	ID3DXEffect* pEffect_;
	D3DXHANDLE constantHandle_;
	BOOL value_;
};

/**
 *	A constant integer containing the number of directional light sources.
 */
class NDirectionalsConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
		{
			pEffect->SetInt( constantHandle,
				min(pContainer->directionals().size(),
					 MAX_DIRECTIONAL_LIGHTS) );
		}
		else
		{
			pEffect->SetInt( constantHandle, 0 );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
		{
			return RecordedIntConstant::get( pEffect,
				constantHandle,
				min(pContainer->directionals().size(),
					MAX_DIRECTIONAL_LIGHTS) );
		}
		return  RecordedIntConstant::get( pEffect, constantHandle, 0 );
	}
};

/**
 *	A constant integer containing the number of omni light sources.
 */
class NOmnisConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
		{
			pEffect->SetInt( constantHandle, 
				min( pContainer->omnis().size(),
					MAX_POINT_LIGHTS )  );
		}
		else
		{
			pEffect->SetInt( constantHandle, 0 );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
			return RecordedIntConstant::get( pEffect,
					constantHandle, 
					min(pContainer->omnis().size(),
						MAX_POINT_LIGHTS));
		return RecordedIntConstant::get( pEffect, constantHandle, 0 );
	}
};

/**
 *	A constant integer containing the number of spot light sources.
 */
class NSpotsConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
			pEffect->SetInt( constantHandle,
				min(pContainer->spots().size(),MAX_SPOT_LIGHTS) );
		else
			pEffect->SetInt( constantHandle, 0 );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().lightContainer().get();
		if (pContainer != NULL)
			return RecordedIntConstant::get( pEffect,
				constantHandle,
				min(pContainer->spots().size(),MAX_SPOT_LIGHTS) );
		return RecordedIntConstant::get( pEffect, constantHandle, 0 );
	}
};

/**
 *	A constant integer containing the number of specular directional light 
 *	sources.
 */
class NSpecularDirectionalsConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().specularLightContainer().get();
		if (pContainer != NULL)
		{
			pEffect->SetInt( constantHandle,
				min(pContainer->directionals().size(),
					MAX_DIRECTIONAL_LIGHTS) );
		}
		else
		{
			pEffect->SetInt( constantHandle, 0 );
		}
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().specularLightContainer().get();
		if (pContainer != NULL)
			return RecordedIntConstant::get( pEffect,
					constantHandle, 
					min(pContainer->directionals().size(),
						MAX_DIRECTIONAL_LIGHTS) );
		return RecordedIntConstant::get( pEffect, constantHandle, 0 );
	}
};

/**
 *	A constant integer containing the number of specular omni light 
 *	sources.
 */
class NSpecularOmnisConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().specularLightContainer().get();
		if (pContainer != NULL)
			pEffect->SetInt( constantHandle,
				min(pContainer->omnis().size(),MAX_SPEC_POINT_LIGHTS));
		else
			pEffect->SetInt( constantHandle, 0 );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		LightContainer * pContainer = Moo::rc().specularLightContainer().get();
		if (pContainer != NULL)
			return RecordedIntConstant::get( pEffect,
						constantHandle,
						min(pContainer->omnis().size(),
							MAX_SPEC_POINT_LIGHTS) );
		return RecordedIntConstant::get( pEffect, constantHandle, 0 );
	}
};

/**
 *	A boolean value constant indicating static lighting should be used.
 */
class StaticLightingConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetBool( constantHandle, EffectVisualContext::instance().staticLighting() );
		return true;
	}
	RecordedEffectConstant* record(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		return RecordedBoolConstant::get( pEffect,
					constantHandle, 
					EffectVisualContext::instance().staticLighting() );
	}
};

/**
 *	The camera position constant.
 */
class CameraPosConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		Vector4 camPos;
		if (objectSpace_)
		{
			reinterpret_cast<Vector3&>(camPos) = EffectVisualContext::instance().invWorld().applyPoint( rc().invView()[ 3 ] );
		}
		else
		{
			camPos = rc().invView().row(3);
		}
		pEffect->SetVector( constantHandle, &camPos );
		return true;
	}
	CameraPosConstant( bool objectSpace = false )
	: objectSpace_( objectSpace )
	{

	}
private:
	bool objectSpace_;
};

/**
 *	The automatically generated normalisation cube map constant.
 */
class NormalisationMapConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetTexture( constantHandle, EffectVisualContext::instance().pNormalisationMap() );
		return true;
	}
	NormalisationMapConstant()
	{

	}
private:
};


/**
 *	Time.
 */
class TimeConstant : public EffectConstantValue
{
public:
	static void tick( float dTime )
	{
		time_ += dTime;
	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetFloat( constantHandle, time_ );

		return true;
	}
private:
	static float time_;

};

/**
 *	Fog range parameters. A float constant used for both start and end fog 
 *	values (specified in the constructor).
 */
class FogConstant : public EffectConstantValue
{
public:
	FogConstant( bool fogStart )
	: fogStart_( fogStart )
	{

	}
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetFloat( constantHandle, fogStart_ ? rc().fogNear() : rc().fogFar() );

		return true;
	}
private:
	bool fogStart_;
};

/**
 *	Fog colour constant.
 */
class FogColourConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetVector( constantHandle, (D3DXVECTOR4*)&Colour(Moo::rc().fogColour()) );

		return true;
	}
};

/**
 *	Fog enabled flag.
 */
class FogEnabledConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetBool( constantHandle, (BOOL)Moo::rc().fogEnabled() );		

		return true;
	}
private:
};

/**
 *	Near plane
 */
class NearPlaneConstant : public EffectConstantValue
{
public:
	NearPlaneConstant( )
	{
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetFloat( constantHandle, rc().camera().nearPlane() );

		return true;
	}
};


/**
 *	Object ID
 */
class ObjectIDConstant : public EffectConstantValue
{
public:
	ObjectIDConstant( )
	{
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetFloat( constantHandle, rc().currentObjectID() );

		return true;
	}
};

/**
 *	Sets the "MipFilter" constant. Shaders can read this constant to 
 *	allow users to define the mip mapping filter to be used via the 
 *	TEXTURE_FILTER graphics setting.
 */
class MipFilterConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetInt( constantHandle, (INT)TextureManager::instance()->configurableMipFilter() );

		return true;
	}
private:
};

/**
 *	Sets the "MinMagFilter" constant. Shaders can read this constant to 
 *	allow users to define the minification/magnification mapping filters
 *	to be used via the TEXTURE_FILTER graphics setting.
 */
class MinMagFilterConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetInt( constantHandle, (INT)TextureManager::instance()->configurableMinMagFilter() );

		return true;
	}
private:
};

/**
 *	Sets the "MaxAnisotropy" constant. Shaders can read this constant 
 *	to allow users to define the anisotropy value to be used via the 
 *	TEXTURE_FILTER graphics setting.
 */
class MaxAnisotropyConstant : public EffectConstantValue
{
public:
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		BW_GUARD;
		pEffect->SetInt( constantHandle, (INT)TextureManager::instance()->configurableMaxAnisotropy() );

		return true;
	}
private:
};


float TimeConstant::time_ = 0;

/**
 *	Constructor.
 *	This is where he common effect constants are all bound to their
 *	corresponding semantics.
 */
EffectVisualContext::EffectVisualContext() : 
	pRenderSet_( NULL ),
	invWorldScale_( 1.f ),
	staticLighting_( false ),
	isOutside_( true ),
	overrideConstants_( false ),
	inited_( true )
{
	BW_GUARD;
	invWorld_.setIdentity();
	constantMappings_[ EffectConstantValue::get( "WorldPalette" ) ] = new WorldPaletteConstant;
	constantMappings_[ EffectConstantValue::get( "World" ) ] = new WorldConstant;
	constantMappings_[ EffectConstantValue::get( "WorldIT" ) ] = new InverseWorldConstant;	//for rt/shader
	constantMappings_[ EffectConstantValue::get( "ViewProjection" ) ] = new ViewProjectionConstant;
	constantMappings_[ EffectConstantValue::get( "InvViewProjection" ) ] = new InvViewProjectionConstant;
	constantMappings_[ EffectConstantValue::get( "LastViewProjection" ) ] = new LastViewProjectionConstant;
	constantMappings_[ EffectConstantValue::get( "WorldViewProjection" ) ] = new WorldViewProjectionConstant;
	//constantMappings_[ EffectConstantValue::get( "LastWorldViewProjection" ) ] = new LastWorldViewProjectionConstant;
	constantMappings_[ EffectConstantValue::get( "View" ) ] = new ViewConstant;
	constantMappings_[ EffectConstantValue::get( "InvView" ) ] = new InvViewConstant;
	constantMappings_[ EffectConstantValue::get( "WorldView" ) ] = new WorldViewConstant;
	constantMappings_[ EffectConstantValue::get( "Projection" ) ] = new ProjectionConstant;
	constantMappings_[ EffectConstantValue::get( "Screen" ) ] = new ScreenConstant;
	constantMappings_[ EffectConstantValue::get( "EnvironmentTransform" ) ] = new EnvironmentTransformConstant;
	constantMappings_[ EffectConstantValue::get( "EnvironmentShadowTransform" ) ] = new EnvironmentShadowTransformConstant;
	constantMappings_[ EffectConstantValue::get( "FarPlane" ) ] = new FarPlaneConstant;
	constantMappings_[ EffectConstantValue::get( "Ambient" ) ] = new AmbientConstant;
	constantMappings_[ EffectConstantValue::get( "DirectionalLights" ) ] = new DirectionalConstant;
	constantMappings_[ EffectConstantValue::get( "PointLights" ) ] = new OmniConstant;
	constantMappings_[ EffectConstantValue::get( "SpotLights" ) ] = new SpotDiffuseConstant;
	constantMappings_[ EffectConstantValue::get( "SpecularDirectionalLights" ) ] = new DirectionalConstant(false);
	constantMappings_[ EffectConstantValue::get( "SpecularPointLights" ) ] = new OmniConstant(false);
	constantMappings_[ EffectConstantValue::get( "CameraPos" ) ] = new CameraPosConstant;
	constantMappings_[ EffectConstantValue::get( "WorldEyePosition" ) ] = new CameraPosConstant; //for rt/shader
	constantMappings_[ EffectConstantValue::get( "DirectionalLightsObjectSpace" ) ] = new DirectionalConstant(true, true);
	constantMappings_[ EffectConstantValue::get( "PointLightsObjectSpace" ) ] = new OmniConstant(true, true);
	constantMappings_[ EffectConstantValue::get( "SpotLightsObjectSpace" ) ] = new SpotDiffuseConstant(true);
	constantMappings_[ EffectConstantValue::get( "SpecularDirectionalLightsObjectSpace" ) ] = new DirectionalConstant(false, true);
	constantMappings_[ EffectConstantValue::get( "SpecularPointLightsObjectSpace" ) ] = new OmniConstant(false, true);
	constantMappings_[ EffectConstantValue::get( "CameraPosObjectSpace" ) ] = new CameraPosConstant(true);
	constantMappings_[ EffectConstantValue::get( "DirectionalLightCount" ) ] = new NDirectionalsConstant;
	constantMappings_[ EffectConstantValue::get( "PointLightCount" ) ] = new NOmnisConstant;
	constantMappings_[ EffectConstantValue::get( "SpotLightCount" ) ] = new NSpotsConstant;
	constantMappings_[ EffectConstantValue::get( "SpecularDirectionalLightCount" ) ] = new NSpecularDirectionalsConstant;
	constantMappings_[ EffectConstantValue::get( "SpecularPointLightCount" ) ] = new NSpecularOmnisConstant;
	constantMappings_[ EffectConstantValue::get( "Time" ) ] = new TimeConstant;
	constantMappings_[ EffectConstantValue::get( "StaticLighting" ) ] = new StaticLightingConstant;	
	constantMappings_[ EffectConstantValue::get( "FogStart" ) ] = new FogConstant(true);
	constantMappings_[ EffectConstantValue::get( "FogEnd" ) ] = new FogConstant(false);
	constantMappings_[ EffectConstantValue::get( "FogColour" ) ] = new FogColourConstant;
	constantMappings_[ EffectConstantValue::get( "FogEnabled" ) ] = new FogEnabledConstant;
	constantMappings_[ EffectConstantValue::get( "MipFilter" ) ] = new MipFilterConstant;
	constantMappings_[ EffectConstantValue::get( "MinMagFilter" ) ] = new MinMagFilterConstant;
	constantMappings_[ EffectConstantValue::get( "MaxAnisotropy" ) ] = new MaxAnisotropyConstant;

	constantMappings_[ EffectConstantValue::get( "ObjectID" ) ] = new ObjectIDConstant;	

	constantMappings_[ EffectConstantValue::get( "NearPlane" ) ] = new NearPlaneConstant();

	createNormalisationMap();
	constantMappings_[ EffectConstantValue::get( "NormalisationMap" ) ] = new NormalisationMapConstant;

	initConstants();
}

/**
 *	Destructor.
 */
EffectVisualContext::~EffectVisualContext()
{
	BW_GUARD;
	if (!inited_)
		return;
	
	inited_ = false;
	initConstants();

	if (pRenderSet_)
	{
		delete pRenderSet_;
	}

	if (pNormalisationMap_.hasComObject())
	{
		pNormalisationMap_ = NULL;
	}

	constantMappings_.clear();

	EffectConstantValue::fini();	
}

/**
 *	Setup the render set.
 *
 *	@param pRenderSet		The new render set.
 */
void EffectVisualContext::pRenderSet( Visual::RenderSet* pRenderSet )
{ 
	BW_GUARD;
	pRenderSet_ = pRenderSet; 
	if (pRenderSet_ && !pRenderSet_->treatAsWorldSpaceObject_)
	{
		invWorld_.invert( pRenderSet_->transformNodes_.front()->worldTransform() );
		invWorldScale_ = XPVec3Length( &Vector3Base( invWorld_ ) );
	}
	else
	{
		invWorld_ = Matrix::identity;
		invWorldScale_ = 1.f;
	}
}

/**
 *	Tick the constants that need it.
 *
 *	@param dTime		Elapsed time since last frame.
 */
void EffectVisualContext::tick( float dTime )
{
	BW_GUARD;
	TimeConstant::tick( dTime );	
}


const uint32 CUBEMAP_SIZE = 128;
const uint32 CUBEMAP_SHIFT = 7;

/**
 * This method helps with creating the normalisation cubemap.
 *
 * @param buffer pointer to the side of the cubemap we are filling in.
 * @param xMask the direction of the x-axis on this cubemap side.
 * @param yMask the direction of the y-axis on this cubemap side.
 * @param zMask the direction of the z-axis on this cubemap side.
 */
static void fillNormMapSurface( uint32* buffer, Vector3 xMask, Vector3 yMask, Vector3 zMask )
{
	BW_GUARD;
	for (int i = 0; i < int(CUBEMAP_SIZE*CUBEMAP_SIZE); i++)
	{
		int xx = i & (CUBEMAP_SIZE - 1);
		int yy = i >> CUBEMAP_SHIFT;
		float x = (( float(xx) / float(CUBEMAP_SIZE - 1) ) * 2.f ) - 1.f;
		float y = (( float(yy) / float(CUBEMAP_SIZE - 1) ) * 2.f ) - 1.f;
		Vector3 out = ( xMask * x ) + (yMask * y) + zMask;
		out.normalise();
		out += Vector3(1, 1, 1);
		out *= 255.5f * 0.5f;
		*(buffer++) = 0xff000000 |
			(uint32(out.x)<<16) |
			(uint32(out.y)<<8) |
			uint32(out.z);
	}
}

/**
 *	This method creates the normalisation cube map.
 */
void EffectVisualContext::createNormalisationMap()
{
	BW_GUARD;
	DX::CubeTexture* pNormalisationMap;
	HRESULT hr = rc().device()->CreateCubeTexture( CUBEMAP_SIZE, 1, 0, 
		D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pNormalisationMap, NULL );

	if ( FAILED( hr ) )
	{
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't create cube texture.  error code %lx\n", hr );
	}

	// fill in all six cubemap sides.

	//positive x
	D3DLOCKED_RECT lockedRect;
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_POSITIVE_X, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3( 0, 0, -1 ), Vector3( 0, -1, 0), Vector3(  1, 0, 0 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_POSITIVE_X, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	//positive x
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_NEGATIVE_X, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3( 0, 0, 1 ), Vector3( 0, -1, 0), Vector3( -1, 0, 0 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_NEGATIVE_X, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	//positive x
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_POSITIVE_Y, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3( 1, 0, 0 ), Vector3( 0, 0,  1), Vector3( 0,  1, 0 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_POSITIVE_Y, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	//positive x
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_NEGATIVE_Y, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3( 1, 0, 0 ), Vector3( 0, 0, -1), Vector3( 0, -1, 0 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_NEGATIVE_Y, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	//positive x
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_POSITIVE_Z, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3(  1, 0, 0 ), Vector3( 0, -1, 0), Vector3( 0, 0,  1 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_POSITIVE_Z, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	//positive x
	hr = pNormalisationMap->LockRect( D3DCUBEMAP_FACE_NEGATIVE_Z, 0, &lockedRect, NULL, 0 );
	if ( SUCCEEDED( hr ) )
	{
		fillNormMapSurface( (uint32*)lockedRect.pBits, Vector3( -1, 0, 0 ), Vector3( 0, -1, 0), Vector3( 0, 0, -1 )  );
		pNormalisationMap->UnlockRect( D3DCUBEMAP_FACE_NEGATIVE_Z, 0 );
	}
	else
		ERROR_MSG( "EffectVisualContext::createNormalisationMap - couldn't get cube surface.  error code %lx\n", hr );

	pNormalisationMap_ = pNormalisationMap;
	pNormalisationMap->Release();
}



}	// namespace Moo

// effect_visual_context.cpp
