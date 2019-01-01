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
#include "gizmo/scale_gizmo.hpp"
#include "gizmo/tool.hpp"
#include "gizmo/tool_manager.hpp"
#include "gizmo/general_properties.hpp"
#include "gizmo/current_general_properties.hpp"
#include "gizmo/item_functor.hpp"
#include "moo/dynamic_index_buffer.hpp"
#include "moo/dynamic_vertex_buffer.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/visual_channels.hpp"
#include "input/input.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/resource_cache.hpp"
#include "moo/visual_manager.hpp"

extern Moo::Colour g_unlit;
static AutoConfigString s_gizmoVisualXYZ( "editor/scaleGizmo" );
static AutoConfigString s_gizmoVisualUV ( "editor/scaleGizmoUV" );
extern bool g_showHitRegion;


class ScaleFloatProxy : public FloatProxy
{
	float data_;
public:
	virtual float EDCALL get() const
	{
		return data_;
	}
	virtual void EDCALL set( float f, bool transient, bool addBarrier = true )
	{
		data_ = f;
	}
	ScaleFloatProxy()
	{
	}
};


class ScaleMatrixProxy : public MatrixProxy
{
	Matrix data_;
public:
	virtual void EDCALL getMatrix( Matrix & m, bool world = true )
	{
		m = data_;
	}
	virtual void EDCALL getMatrixContext( Matrix & m )
	{
		m = data_;
	}
	virtual void EDCALL getMatrixContextInverse( Matrix & m )
	{
		m.invert( data_ );
	}
	virtual bool EDCALL setMatrix( const Matrix & m )
	{
		data_ = m;
		return true;
	}
	virtual void EDCALL recordState(){}
	virtual bool EDCALL commitState( bool revertToRecord = false, bool addUndoBarrier = true )
	{
		return true;
	}

	virtual bool EDCALL hasChanged()
	{
		return true;
	}
	ScaleMatrixProxy()
	{
	}
};


class ScaleShapePart : public ShapePart
{
public:
	ScaleShapePart( const Moo::Colour& col, int axis, bool isPlane ) 
	:	colour_( col ),
		isPlane_( isPlane ),
		axis_( axis )
	{
	}
	
	const Moo::Colour& colour() const { return colour_; }

	bool isPlane() const {return isPlane_;};
	int  axis() const {return axis_;}
private:
	bool		isPlane_;
	Moo::Colour	colour_;
	int			axis_;
};


// -----------------------------------------------------------------------------
// Section: ScaleGizmo
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ScaleGizmo::ScaleGizmo
( 
	MatrixProxyPtr		pMatrix, 
	int					enablerModifier, 
	float				scaleSpeedFactor,
	Type				type
) 
:	active_( false ),
	inited_( false ),
	currentPart_( NULL ),
	pMatrix_( pMatrix ),
	drawMesh_( NULL ),
	lightColour_( 0, 0, 0, 0 ),
	enablerModifier_( enablerModifier ),
	scaleSpeedFactor_( scaleSpeedFactor ),
	scaleXFloatProxy_( new ScaleFloatProxy() ),
	scaleYFloatProxy_( new ScaleFloatProxy() ),
	scaleZFloatProxy_( new ScaleFloatProxy() ),
	scaleMatrixProxy_( new ScaleMatrixProxy() ),
	type_(type)
{
}


void ScaleGizmo::init()
{
	BW_GUARD;

	if (!inited_)
	{
		switch (type_)
		{
		case SCALE_XYZ:
			if ( !s_gizmoVisualXYZ.value().empty() )
				drawMesh_ = Moo::VisualManager::instance()->get( s_gizmoVisualXYZ );
			break;
		case SCALE_UV:
			if ( !s_gizmoVisualUV.value().empty() )
				drawMesh_ = Moo::VisualManager::instance()->get( s_gizmoVisualUV );
			break;
		}
		ResourceCache::instance().addResource( drawMesh_ );

		Matrix m;
		m.setIdentity();
		selectionMesh_.transform( m );

		float radius = 0.2f;
		float boxRadius = 0.4f;
		float length = 3.f;

		Vector3 min( -boxRadius, -boxRadius, length );
		Vector3 max( boxRadius, boxRadius, length + boxRadius*2.f );

		selectionMesh_.addSphere( Vector3( 0, 0, 0 ), radius, 0xffffffff );

		selectionMesh_.addCylinder( Vector3( 0, 0, length ), radius, -length, false, true,0xffff0000, new ScaleShapePart( Moo::Colour( 1, 0, 0, 0 ), 2, false ) );
		selectionMesh_.addBox( min, max, 0xffff0000, new ScaleShapePart( Moo::Colour( 1, 0, 0, 0 ), 2, false ) );

		m.setRotateY( DEG_TO_RAD( 90 ) );
		selectionMesh_.transform( m );
		selectionMesh_.addCylinder( Vector3( 0, 0, length ), radius, -length, false, true,0xff00ff00, new ScaleShapePart( Moo::Colour( 0, 1, 0, 0 ), 0, false ) );
		selectionMesh_.addBox( min, max, 0xff00ff00, new ScaleShapePart( Moo::Colour( 0, 1, 0, 0 ), 0, false ) );

		if (type_ == SCALE_XYZ)
		{
			m.setRotateX( DEG_TO_RAD( -90 ) );
			selectionMesh_.transform( m );
			selectionMesh_.addCylinder( Vector3( 0, 0, length ), radius, -length, false, true,0xff0000ff, new ScaleShapePart( Moo::Colour( 0, 0, 1, 0 ), 1, false ) );
			selectionMesh_.addBox( min, max, 0xff0000ff, new ScaleShapePart( Moo::Colour( 0, 0, 1, 0 ), 1, false ) );
		}

		float offset = length / 6.f;
		float boxSize = length / 12.f;
		float boxHeight = length / 12.f;
		if (drawMesh_ != NULL)
		{
			static float s_boxSize = 0.6f;
			static float s_boxHeight = 0.01f;
			static float s_offset = 0.f;
			static float s_length = 0.f;

			//increase hit region to cover mesh.
			boxHeight = s_boxHeight;
			boxSize = s_boxSize;
			offset = s_offset;
			length = s_length;
		}
		float pos1 = length + offset;
		float pos2 = length + offset + boxSize;
		float pos3 = length + offset + boxSize + boxSize;
		Vector3 min1( pos1, -boxHeight/2.f, pos2 );
		Vector3 max1( pos3, boxHeight/2.f, pos3 );

		Vector3 min2( pos2, -boxHeight/2.f, pos1 );
		Vector3 max2( pos3, boxHeight/2.f, pos2 );

		selectionMesh_.transform( Matrix::identity );
		selectionMesh_.addBox( min1, max1, 0xffffff00, new ScaleShapePart( Moo::Colour( 1, 1, 0, 0 ), 1, true ) );
		selectionMesh_.addBox( min2, max2, 0xffffff00, new ScaleShapePart( Moo::Colour( 1, 1, 0, 0 ), 1, true ) );

		m.setRotateZ( DEG_TO_RAD( 90 ) );
		selectionMesh_.transform( m );
		selectionMesh_.addBox( min1, max1, 0xffff00ff, new ScaleShapePart( Moo::Colour( 1, 0, 1, 0 ), 0, true ) );
		selectionMesh_.addBox( min2, max2, 0xffff00ff, new ScaleShapePart( Moo::Colour( 1, 0, 1, 0 ), 0, true ) );

		if (type_ == SCALE_XYZ)
		{
			m.setRotateX( DEG_TO_RAD( -90 ) );
			selectionMesh_.transform( m );
			selectionMesh_.addBox( min1, max1, 0xff00ffff, new ScaleShapePart( Moo::Colour( 0, 1, 1, 0 ), 2, true ) );
			selectionMesh_.addBox( min2, max2, 0xff00ffff, new ScaleShapePart( Moo::Colour( 0, 1, 1, 0 ), 2, true ) );
		}
		inited_ = true;
	}
}


/**
 *	Destructor.
 */
ScaleGizmo::~ScaleGizmo()
{

}


bool ScaleGizmo::draw( bool force )
{
	BW_GUARD;

	active_ = false;
	if 
	( 
		!force 
		&&
		enablerModifier_ != ALWAYS_ENABLED
		&& 
		(InputDevices::modifiers() & enablerModifier_) == 0 
	)
	{
		return false;
	}
	active_ = true;

	init();

	Moo::RenderContext& rc = Moo::rc();
	DX::Device* pDev = rc.device();

	if (drawMesh_)
	{
		rc.fogEnabled( false );
		Moo::LightContainerPtr pOldLC = rc.lightContainer();
		Moo::LightContainerPtr pLC = new Moo::LightContainer;
		pLC->ambientColour( lightColour_ );
		rc.lightContainer( pLC );
		rc.setPixelShader( NULL );		

		rc.push();
		rc.world( gizmoTransform() );
		drawMesh_->draw();
		rc.pop();

		rc.lightContainer( pOldLC );
		Moo::SortedChannel::draw();
	}
	
	if (!drawMesh_ || g_showHitRegion)
	{
		rc.setRenderState( D3DRS_NORMALIZENORMALS, TRUE );
		Moo::Material::setVertexColour();
		rc.fogEnabled( false );
		rc.setRenderState( D3DRS_LIGHTING, FALSE );
		rc.setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		rc.setTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		rc.setTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		rc.setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
		rc.setTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

		uint32 tfactor = lightColour_;
		rc.setRenderState( D3DRS_TEXTUREFACTOR, tfactor );

		pDev->SetTransform( D3DTS_WORLD, &gizmoTransform() );
		pDev->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
		pDev->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
		Moo::rc().setPixelShader( NULL );
		Moo::rc().setVertexShader( NULL );
		Moo::rc().setFVF( Moo::VertexXYZND::fvf() );

		Moo::DynamicIndexBufferBase& dib = Moo::rc().dynamicIndexBufferInterface().get( D3DFMT_INDEX16 );
		Moo::IndicesReference ind = dib.lock( selectionMesh_.indices().size() );

		if (ind.size())
		{
			ind.fill( &selectionMesh_.indices().front(), selectionMesh_.indices().size() );
			uint32 ibLockIndex = 0;
			dib.unlock();
			ibLockIndex = dib.lockIndex();

			Moo::DynamicVertexBufferBase2< Moo::VertexXYZND >& dvb = Moo::DynamicVertexBufferBase2< Moo::VertexXYZND >::instance();

			uint32 vbLockIndex = 0;
			if ( dvb.lockAndLoad( &selectionMesh_.verts().front(), selectionMesh_.verts().size(), vbLockIndex ) &&
				 SUCCEEDED(dvb.set()) &&
				 SUCCEEDED(dib.indexBuffer().set()) )
			{
				rc.drawIndexedPrimitive( D3DPT_TRIANGLELIST, vbLockIndex, 0, (uint32)(selectionMesh_.verts().size()),
					ibLockIndex, (uint32)(selectionMesh_.indices().size()) / 3 );
			}
		}
	}

	return true;
}


bool ScaleGizmo::intersects( const Vector3& origin, const Vector3& direction,
														float& t, bool force )
{
	BW_GUARD;

	if ( !active_ )
	{
		currentPart_ = NULL;
		return false;
	}

	if 
	(
		!force
		&&
		enablerModifier_ != ALWAYS_ENABLED
		&&
		(InputDevices::modifiers() & enablerModifier_) == 0
	)
	{
		return false;
	}

	init();

	lightColour_ = g_unlit;
	Matrix m = gizmoTransform();
	m.invert();
	
	Vector3 lo = m.applyPoint( origin );
	Vector3 ld = m.applyVector( direction );
	float l = ld.length();
	t *= l;
	ld /= l;;

	currentPart_ = (ScaleShapePart*)selectionMesh_.intersects(
		m.applyPoint( origin ),
		m.applyVector( direction ),
		&t );

	t /= l;

	return currentPart_ != NULL;
}


void ScaleGizmo::click( const Vector3& origin, const Vector3& direction )
{
	BW_GUARD;

	if (currentPart_ != NULL) 
	{
		ToolFunctorPtr func =
			ToolFunctorPtr
			( 
				new PropertyScaler
				( 
					scaleSpeedFactor_, 
					scaleXFloatProxy_.getObject(),
					scaleYFloatProxy_.getObject(),
					scaleZFloatProxy_.getObject()
				), 
				true 
			);

		ToolLocatorPtr locator;
		if (currentPart_->isPlane())
		{
			PlaneEq	peq( this->gizmoTransform().applyToOrigin(),
				this->gizmoTransform()[ currentPart_->axis() ] );
			locator = ToolLocatorPtr( new PlaneToolLocator( &peq ), true );
		}
		else
		{
			Vector3 dir = this->gizmoTransform()[ currentPart_->axis() ];
			locator = ToolLocatorPtr( new LineToolLocator( this->objectTransform().applyToOrigin(), dir), true );
		}
		locator->transform( this->gizmoTransform() );

		ToolViewPtr scaleView;

		if (currentPart_->isPlane())
		{
			FloatProxyPtr axis1;
			FloatProxyPtr axis2;
			std::string name;
			if (type_ == SCALE_UV)
			{
				axis1 = scaleXFloatProxy_.getObject();
				axis2 = scaleZFloatProxy_.getObject();
				name = "U,V scale";
			}
			else
			{
				if (currentPart_->axis() == 0)
				{
					axis1 = scaleYFloatProxy_.getObject();
					axis2 = scaleZFloatProxy_.getObject();
					name = "Y,Z scale";
				}
				else if (currentPart_->axis() == 1)
				{
					axis1 = scaleXFloatProxy_.getObject();
					axis2 = scaleZFloatProxy_.getObject();
					name = "X,Z scale";
				}
				else
				{
					axis1 = scaleXFloatProxy_.getObject();
					axis2 = scaleYFloatProxy_.getObject();
					name = "X,Y scale";
				}
			}
			scaleView = ToolViewPtr(
				new Vector2Visualiser( 
					scaleMatrixProxy_.getObject(), 
					axis1.getObject(),
					axis2.getObject(),
					Moo::Colour( 1, 1, 1, 1 ), 
					name,
					SimpleFormatter::s_def ),
				true );
		}
		else
		{
			FloatProxyPtr axis;
			std::string name;
			if (currentPart_->axis() == 0)
			{
				axis = scaleXFloatProxy_.getObject();
				name = type_ == SCALE_UV ? "U scale" : "X scale";
			}
			else if (currentPart_->axis() == 1)
			{
				axis = scaleYFloatProxy_.getObject();
				name = "Y scale";
			}
			else if (currentPart_->axis() == 2)
			{
				axis = scaleZFloatProxy_.getObject();
				name = type_ == SCALE_UV ? "V scale" : "Z scale";
			}
			scaleView = ToolViewPtr(
				new FloatVisualiser( 
					scaleMatrixProxy_.getObject(), 
					axis,
					Moo::Colour( 1, 1, 1, 1 ), 
					name,
					SimpleFormatter::s_def ),
				true );
		}
		ToolPtr moveTool( new Tool( locator, scaleView, func ), true );

		ToolManager::instance().pushTool( moveTool );
	}
}


void ScaleGizmo::rollOver( const Vector3& origin, const Vector3& direction )
{
	BW_GUARD;

	// roll it over.
	if (currentPart_ != NULL)
	{
		lightColour_ = currentPart_->colour();
	}
	else
	{
		lightColour_ = g_unlit;
	}
}


Matrix ScaleGizmo::getCoordModifier() const
{
	BW_GUARD;

	Matrix coord;
	if( CoordModeProvider::ins()->getCoordMode() == CoordModeProvider::COORDMODE_OBJECT )
	{
		if (CurrentScaleProperties::properties().size() == 1)
		{
			CurrentScaleProperties::properties()[0]->pMatrix()->getMatrix(coord);
		}
		else
		{
			coord.setIdentity();
		}
	}
	else if( CoordModeProvider::ins()->getCoordMode() == CoordModeProvider::COORDMODE_VIEW )
	{
		coord = Moo::rc().invView();
	}
	else
	{
		coord.setIdentity();
	}
	return coord;
}


ScaleGizmo::Type ScaleGizmo::type() const
{
	return type_;
}


Matrix ScaleGizmo::objectTransform() const
{
	BW_GUARD;

	Matrix m;

	if (pMatrix_)
	{
		Matrix xform;
		pMatrix_->getMatrix(xform);
		m.setTranslate(xform.applyToOrigin());
	}
	else
	{
		m.setTranslate( CurrentScaleProperties::averageOrigin() );
	}

	return m;
}


Matrix ScaleGizmo::gizmoTransform()
{
	BW_GUARD;

	Vector3 pos = this->objectTransform()[3];

	Matrix m = getCoordModifier();

	m[3].setZero();
	m[0].normalise();
	m[1].normalise();
	m[2].normalise();

	float scale = ( Moo::rc().invView()[2].dotProduct( pos ) -
		Moo::rc().invView()[2].dotProduct( Moo::rc().invView()[3] ) );
	if (scale > 0.05)
		scale /= 25.f;
	else
		scale = 0.05f / 25.f;

	Matrix scaleMat;
	scaleMat.setScale( scale, scale, scale );

	m.postMultiply( scaleMat );

	Matrix m2 = objectTransform();
	m2[0].normalise();
	m2[1].normalise();
	m2[2].normalise();

	m2.preMultiply( m );

	scaleMatrixProxy_->setMatrix( m2 );
	return m2;
}


Matrix  ScaleGizmo::objectCoord() const
{
	BW_GUARD;

	Matrix coord;
	if (CurrentScaleProperties::properties().size() > 0)
		CurrentScaleProperties::properties()[0]->pMatrix()->getMatrix(coord);
	else
		coord = Matrix::identity;
	return coord;
}
