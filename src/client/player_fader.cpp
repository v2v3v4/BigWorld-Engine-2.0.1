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
#include "player_fader.hpp"

#include "entity_manager.hpp"
#include "player.hpp"
#include "camera/base_camera.hpp"
#include "moo/visual_channels.hpp"
#include "romp/geometrics.hpp"
#include "post_processing/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "App", 0 )

namespace PostProcessing
{
// Python statics
PY_TYPEOBJECT( PlayerFader )

PY_BEGIN_METHODS( PlayerFader )	
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PlayerFader )	
	PY_ATTRIBUTE( renderTarget )
	PY_ATTRIBUTE( clearRenderTarget )
	PY_ATTRIBUTE( name )
	PY_ATTRIBUTE( opacity )
PY_END_ATTRIBUTES()

/*~ function PostProcessing.Phase
 *	Factory function to create and return a PostProcessing PyPhase object.
 *	@return A new PostProcessing PyPhase object.
 */
PY_FACTORY_NAMED( PlayerFader, "PlayerFader", _PostProcessing )

IMPLEMENT_PHASE( PlayerFader, PlayerFader )

/**
 *	This helper function is used by PlayerFader.
 *
 *	This function returns the rectangle that is the intersection of the
 *	near-plane with the view frustum.
 *
 *	@param rc			The render context (used to calculate the near-plane).
 *	@param corner		Is set to the position of the bottom-left corner of the
 *							rectangle.
 *	@param xAxis		Is set to the length of the horizontal edges.
 *	@param yAxis		Is set to the length of the vertical edges.
 *
 *	@note	The invView matrix must be correct before this method is called. You
 *			may need to call updateViewTransforms.
 */
void getNearPlaneRect( const Moo::RenderContext & rc, Vector3 & corner,
		Vector3 & xAxis, Vector3 & yAxis )
{
	BW_GUARD;
	// TODO: May want to make this function available to others.

	const Matrix & matrix = rc.invView();
	const Moo::Camera & camera = rc.camera();

	// zAxis is the vector from the camera position to the centre of the
	// near plane of the camera.
	Vector3 zAxis = matrix.applyToUnitAxisVector( Z_AXIS );
	zAxis.normalise();

	// xAxis is the vector from the centre of the near plane to its right edge.
	xAxis = matrix.applyToUnitAxisVector( X_AXIS );
	xAxis.normalise();

	// yAxis is the vector from the centre of the near plane to its top edge.
	yAxis = matrix.applyToUnitAxisVector( Y_AXIS );
	yAxis.normalise();

	const float fov = camera.fov();
	const float nearPlane = camera.nearPlane();
	const float aspectRatio = camera.aspectRatio();

	const float yLength = nearPlane * tanf( fov / 2.0f );
	const float xLength = yLength * aspectRatio;

	xAxis *= xLength;
	yAxis *= yLength;
	zAxis *= nearPlane;

	Vector3 nearPlaneCentre( matrix.applyToOrigin() + zAxis );
	corner = nearPlaneCentre - xAxis - yAxis;
	xAxis *= 2.f;
	yAxis *= 2.f;
}

/**
 *	Constructor. In the constructor, this object checks whether the near-plane
 *	clips the player. If so, it adjusts the visibility of the player's model.
 */
PlayerFader::PlayerFader(PyTypePlus *pType) :	
	Phase( pType ),
	transparency_( 0.f ),
	ptp_( 2.f ),
	maxPt_( 0.85f ),
	wasVisible_( true ),
	opacity_( new Vector4Basic, true )
{
	BW_GUARD;
	pOpacity_ = opacity_;
}


/**
 *	Destructor. In the destructor, the visibility of the player's model is
 *	restored.
 */
PlayerFader::~PlayerFader()
{
	BW_GUARD;
}


void PlayerFader::updateOpacity()
{
	float amt = 1.f - transparency_;
	opacity_->set( Vector4(amt, amt, amt, amt) );
}


void PlayerFader::tick( float dTime )
{
	BW_GUARD;

	transparency_ = 1.f;
	wasVisible_ = false;

	this->updateOpacity();	

	Entity * pPlayer = Player::entity();
	if (pPlayer == NULL)
		return;	

	PyModel * pModel = pPlayer->pPrimaryModel();
	if (pModel == NULL)
		return;

	if (!BaseCamera::checkCameraTooClose())
		return;

	BaseCamera::checkCameraTooClose( false );	

	//allow the player to fade out smoothly before completely disappearing
	//the transparency_ member is set with a value between 0 and 1.
	//For any value > 0, the player is fading or faded out.
	//
	//For a smooth transition, the near plane check no longer uses the
	//bounding box because it has sharp corners (and would thus create
	//discontinuities in the transparency value when the camera circles
	//around the player).
	//Instead, an ellipsoid is fitted around the bounding box and the distance
	//from the camera to it is calculated.
	if (pPlayer->pPrimaryModel()->visible())
	{
		wasVisible_ = true;

		//The distance is calculated by transforming the camera position into
		//unit-bounding-box space.  A sphere is fitted around the unit bounding
		//box and the distance is calculated.
		BoundingBox bb;
		pPlayer->pPrimaryModel()->localBoundingBox( bb, true );
		//1.414 is so the sphere fits around the unit cube, instead of inside.
		const Vector3 s = (bb.maxBounds() - bb.minBounds())*1.414f;
		Matrix m( pPlayer->fallbackTransform() );
		m.invert();
		Vector3 origin( Moo::rc().invView().applyToOrigin() );
		m.applyPoint( origin, origin );
		origin -= bb.centre();
		origin = origin * Vector3(1.f/s.x,1.f/s.y,1.f/s.z);

		//1 is subtracted so if camera is inside unit sphere then we are fully
		//faded out.
		float d = origin.length() - 1.f;
		transparency_ = 1.f - min( max( d,0.f ),1.f );

		//Check the player itself for transparency	
		transparency_ = max( pPlayer->transparency(), transparency_ );

		//And adjust the values for final output.
		transparency_ = powf(transparency_,ptp_);
		transparency_ = min( transparency_, maxPt_ );

		this->updateOpacity();

		if ( !almostEqual( transparency_, 0.f) )
		{
			Entity * pPlayer = Player::entity();
			pPlayer->pPrimaryModel()->visible( false );
		}
	}
}


void PlayerFader::draw( Debug* debug, RECT* debugRect )
{
	BW_GUARD;

	if ( !wasVisible_ )
		return;

	Entity * pPlayer = Player::entity();
	if (pPlayer == NULL)
		return;	

	PyModel * pModel = pPlayer->pPrimaryModel();
	if (pModel == NULL)
		return;

	pPlayer->pPrimaryModel()->visible( true );

	bool drawPlayer = ( 0.f < transparency_ && transparency_ < 1.f );

	if (pRenderTarget_.hasObject() && pRenderTarget_->pRenderTarget()->push() )
	{		

		if (clearRenderTarget_)
		{
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1, 0 );
		}
		else
		{
			//Just clear the z-buffer
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0x00000000, 1, 0 );
		}

		if (drawPlayer)
		{
			Player::instance().drawPlayer(true);
		}

		pRenderTarget_->pRenderTarget()->pop();
	}

	if ( debug )
	{
		if (pRenderTarget_.hasObject())
			debug->copyPhaseOutput( pRenderTarget_->pRenderTarget() );
		else
			debug->copyPhaseOutput( NULL );
	}
}


bool PlayerFader::load( DataSectionPtr pSect )
{
	name_ = pSect->readString( "name", name_ );
	std::string rtName = pSect->readString( "renderTarget" );
	if (!rtName.empty())
	{
		Moo::BaseTexturePtr pTex = Moo::TextureManager::instance()->get( rtName );
		if (pTex.hasObject())
		{
			Moo::RenderTarget* pRT = static_cast<Moo::RenderTarget*>( pTex.getObject() );
			pRenderTarget_ = new PyRenderTarget(pRT);
		}
	}
	return true;
}


bool PlayerFader::save( DataSectionPtr pDS )
{
	DataSectionPtr pSect = pDS->newSection( "PlayerFader" );
	pSect->writeString( "name", name_ );
	if (pRenderTarget_.hasObject())
	{
		pSect->writeString( "renderTarget", pRenderTarget_->pRenderTarget()->resourceID() );
	}
	return true;
}


PyObject * PlayerFader::pyGetAttribute( const char *attr )
{
	PY_GETATTR_STD();
	return Phase::pyGetAttribute(attr);
}


int PlayerFader::pySetAttribute( const char *attr, PyObject *value )
{
	PY_SETATTR_STD();
	return Phase::pySetAttribute(attr,value);
}


PyObject* PlayerFader::pyNew(PyObject* args)
{
	return new PlayerFader;
}

}	//namespace PostProcessing

// player_fader.cpp
