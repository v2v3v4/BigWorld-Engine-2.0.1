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
#include "item_functor.hpp"
#include "general_properties.hpp"

#include "tool_manager.hpp"
#include "appmgr/options.hpp"
#include "undoredo.hpp"
#include "snap_provider.hpp"
#include "coord_mode_provider.hpp"
#include "pyscript/py_data_section.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/debug.hpp"
#include "current_general_properties.hpp"

DECLARE_DEBUG_COMPONENT2( "Editor", 0 )



// -----------------------------------------------------------------------------
// Section: MatrixMover
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( MatrixMover )

PY_BEGIN_METHODS( MatrixMover )
	PY_METHOD( setUndoName )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MatrixMover )
PY_END_ATTRIBUTES()

PY_FACTORY( MatrixMover, Functor )


int MatrixMover::moving_ = 0;
/**
 *	Constructor.
 */
MatrixMover::MatrixMover( MatrixProxyPtr pMatrix, PyTypePlus * pType ) :
	ToolFunctor( pType ),
	lastLocatorPos_( Vector3::zero() ),
	totalLocatorOffset_( Vector3::zero() ),
	gotInitialLocatorPos_( false ),
	snap_( true ),
	rotate_( false ),
	snapMode_( SnapProvider::instance()->snapMode() )
{
	BW_GUARD;

	++moving_;
	std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();
	for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
	{
		(*i)->pMatrix()->recordState();
	}
}

MatrixMover::MatrixMover( MatrixProxyPtr pMatrix, bool snap, bool rotate, PyTypePlus * pType ) :
	ToolFunctor( pType ),
	lastLocatorPos_( Vector3::zero() ),
	totalLocatorOffset_( Vector3::zero() ),
	gotInitialLocatorPos_( false ),
	snap_( snap ),
	rotate_( rotate ),
	snapMode_( SnapProvider::instance()->snapMode() )
{
	BW_GUARD;

	++moving_;
	std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();
	for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
	{
		(*i)->pMatrix()->recordState();
	}
}

MatrixMover::~MatrixMover()
{
	--moving_;
}

PyObject* MatrixMover::py_setUndoName( PyObject* args )
{
	BW_GUARD;

	// parse arguments
	char* str;
	if (!PyArg_ParseTuple( args, "s", &str ))	{
		PyErr_SetString( PyExc_TypeError, "setUndoName() "
			"expects a string argument" );
		return NULL;
	}

	undoName_ = str;

	Py_Return;
}


/**
 *	Update method
 */
void MatrixMover::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ) ||
		snapMode_ != SnapProvider::instance()->snapMode() )
	{
		std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();

		bool addUndoBarrier = (props.size() > 1) || (undoName_ != "");

		bool success = true;
		for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
		{
			MatrixProxyPtr pMatrix = (*i)->pMatrix();
			if (pMatrix->hasChanged())
			{
				// set its transform permanently
				if ( !pMatrix->commitState( false, !addUndoBarrier ) )
				{
					success = false;
				}
			}
			else
			{
				pMatrix->commitState( true );
			}
		}

		if (addUndoBarrier)
		{
			if (undoName_ != "")
				UndoRedo::instance().barrier( undoName_, false );
			else
				UndoRedo::instance().barrier( "Move group", true );

			if ( !success )
			{
				UndoRedo::instance().undo();
			}
		}

		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	// figure out movement
	if (tool.locator())
	{
		std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();

		Vector3 locatorPos = tool.locator()->transform().applyToOrigin();

		if (!gotInitialLocatorPos_)
		{
			lastLocatorPos_ = tool.locator()->transform().applyToOrigin();
			gotInitialLocatorPos_ = true;

			if (props.size() == 1)
			{
				Vector3 objPos = props[0]->pMatrix()->get().applyToOrigin();
				Vector3 clipPos = Moo::rc().viewProjection().applyPoint( objPos );
				clipPos.x = ( clipPos.x + 1 ) / 2 * Moo::rc().screenWidth();
				clipPos.y = ( 1 - clipPos.y ) / 2 * Moo::rc().screenHeight();

				POINT pt;
				pt.x = LONG( clipPos.x );
				pt.y = LONG( clipPos.y );
				::ClientToScreen( Moo::rc().windowHandle(), &pt );
				::SetCursorPos( pt.x, pt.y );

				if (Moo::rc().device()) 
				{
					Moo::rc().device()->SetCursorPosition( pt.x, pt.y, 0 );
				}

				lastLocatorPos_ = locatorPos = objPos;
			}
		}

		totalLocatorOffset_ += locatorPos - lastLocatorPos_;
		lastLocatorPos_ = locatorPos;


		for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
		{
			MatrixProxyPtr pMatrix = (*i)->pMatrix();

			Matrix oldMatrix;
			pMatrix->getMatrix( oldMatrix );

			// reset the last change we made
			pMatrix->commitState( true );

			Matrix m;
			pMatrix->getMatrix( m );

			Vector3 delta = totalLocatorOffset_;

			if( snap_ )
				SnapProvider::instance()->snapPositionDelta( delta );

			Vector3 newPos = m.applyToOrigin() + delta;

            bool snapPosOK = true;
			if( snap_ )
				snapPosOK = SnapProvider::instance()->snapPosition( newPos );

            if( rotate_ && snapPosOK )
			{
				Vector3 normalOfSnap = SnapProvider::instance()->snapNormal( newPos );
				Vector3 yAxis( 0, 1, 0 );
				yAxis = m.applyVector( yAxis );

				Vector3 binormal = yAxis.crossProduct( normalOfSnap );

				normalOfSnap.normalise();
				yAxis.normalise();
				binormal.normalise();

				float angle = acosf( Math::clamp(-1.0f, yAxis.dotProduct( normalOfSnap ), +1.0f) );

				Quaternion q( binormal.x * sinf( angle / 2.f ),
					binormal.y * sinf( angle / 2.f ),
					binormal.z * sinf( angle / 2.f ),
					cosf( angle / 2.f ) );

				q.normalise();

				Matrix rotation;
				rotation.setRotate( q );

				m.postMultiply( rotation );
			}

            if (snapPosOK)
            {
			    m.translation( newPos );

			    Matrix worldToLocal;
			    pMatrix->getMatrixContextInverse( worldToLocal );

			    m.postMultiply( worldToLocal );

			    pMatrix->setMatrix( m );
            }
            else
            {
                // snapping the position failed, revert back to the previous
                // good matrix:
			    Matrix worldToLocal;
			    pMatrix->getMatrixContextInverse( worldToLocal );
			    oldMatrix.postMultiply( worldToLocal );
                pMatrix->setMatrix( oldMatrix );
            }
		}
	}
}


/**
 *	Key event method
 */
bool MatrixMover::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// Set the items back to their original states
		std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();
		for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
		{
			(*i)->pMatrix()->commitState( true );
		}

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


/**
 *	Factory method
 */
PyObject * MatrixMover::pyNew( PyObject * args )
{
	BW_GUARD;

	if (CurrentPositionProperties::properties().empty())
	{
		PyErr_Format( PyExc_ValueError, "MatrixMover() "
			"No current editor" );
		return NULL;
	}

	return new MatrixMover( NULL );
}



// -----------------------------------------------------------------------------
// Section: MatrixScaler
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( MatrixScaler )

PY_BEGIN_METHODS( MatrixScaler )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MatrixScaler )
PY_END_ATTRIBUTES()

//PY_FACTORY( MatrixScaler, Functor )


/**
 *	Constructor.
 */
MatrixScaler::MatrixScaler( MatrixProxyPtr pMatrix, float scaleSpeedFactor,
		   FloatProxyPtr scaleX, FloatProxyPtr scaleY, FloatProxyPtr scaleZ,
		   PyTypePlus * pType ) :
	ToolFunctor( pType ),
	pMatrix_( pMatrix ),
	scaleSpeedFactor_(scaleSpeedFactor),
	grabOffset_( 0, 0, 0 ),
	grabOffsetSet_( false ),
	scaleX_( scaleX ),
	scaleY_( scaleY ),
	scaleZ_( scaleZ )
{
	BW_GUARD;

	pMatrix_->recordState();
	pMatrix_->getMatrix( initialMatrix_, false );

	initialScale_.set( initialMatrix_[0].length(),
		initialMatrix_[1].length(),
		initialMatrix_[2].length() );

	initialMatrix_[0] /= initialScale_.x;
	initialMatrix_[1] /= initialScale_.y;
	initialMatrix_[2] /= initialScale_.z;

	invInitialMatrix_.invert( initialMatrix_ );
}



/**
 *	Update method
 */
void MatrixScaler::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ))
	{
		// set its transform permanently
		pMatrix_->commitState();

		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	// figure out movement
	if (tool.locator())
	{
		Matrix localPosn;
		pMatrix_->getMatrixContextInverse( localPosn );
		localPosn.preMultiply( tool.locator()->transform() );

		if (!grabOffsetSet_)
		{
			grabOffsetSet_ = true;
			grabOffset_ = localPosn.applyToOrigin();
		}

		Vector3 scale = localPosn.applyToOrigin() - grabOffset_;
		scale *= scaleSpeedFactor_;

		scale = invInitialMatrix_.applyVector( scale );

		Vector3 direction = invInitialMatrix_.applyVector( tool.locator()->direction() );
		direction.normalise();

		scale = scale + initialScale_;

		const float scaleEpsilon = 0.01f;
		scale.x = max( scale.x, scaleEpsilon );
		scale.y = max( scale.y, scaleEpsilon );
		scale.z = max( scale.z, scaleEpsilon );

		if (scaleX_)
		{
			scaleX_->set( scale.x, false );
		}
		if (scaleY_)
		{
			scaleY_->set( scale.y, false );
		}
		if (scaleZ_)
		{
			scaleZ_->set( scale.z, false );
		}

		Matrix curPose;
		curPose.setScale( scale );
		curPose.postMultiply( initialMatrix_ );

		pMatrix_->setMatrix( curPose );
	}
}


/**
 *	Key event method
 */
bool MatrixScaler::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// set the item back to it's original pose
		pMatrix_->commitState( true );

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: PropertyScaler
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( PropertyScaler )

PY_BEGIN_METHODS( PropertyScaler )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PropertyScaler )
PY_END_ATTRIBUTES()


/**
 *	Constructor.
 */
PropertyScaler::PropertyScaler( float scaleSpeedFactor,
			FloatProxyPtr scaleX, FloatProxyPtr scaleY, FloatProxyPtr scaleZ,
			PyTypePlus * pType ) :
	ToolFunctor( pType ),
	scaleX_( scaleX ),
	scaleY_( scaleY ),
	scaleZ_( scaleZ ),
	scaleSpeedFactor_(scaleSpeedFactor),
	grabOffsetSet_( false ),
	invGrabOffset_( Matrix::identity )
{
}


/**
 *	This method updates the scale
 *	@param dTime - unused
 *	@param tool the tool used to manipulate the scale
 */
void PropertyScaler::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ))
	{
		
		scalerHelper_.fini( true );

		UndoRedo::instance().barrier( "Scale", true );

		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	if (tool.locator())
	{
		// If this is our first time in update, grab our frame of reference etc
		if (!grabOffsetSet_)
		{
			grabOffsetSet_ = true;

			// Get the initial picked position on the tool
			Matrix grabOffset = tool.locator()->transform();

			// Normalise the tool transform
			grabOffset[0].normalise();
			grabOffset[1].normalise();
			grabOffset[2].normalise();

			// Store the inverse of the tool transform
			invGrabOffset_.invert( grabOffset );

			// init the scaler helper
			scalerHelper_.init( grabOffset );
		}

		// Calculate the scale value, the scale value is the position of the 
		// currently picked point relative to the first picked point
		// in gizmo space
		Vector3 scale = invGrabOffset_.applyPoint( 
			tool.locator()->transform().applyToOrigin() ) * scaleSpeedFactor_;

		// Fix up the scale values for all the axis, a negative scale value means 
		// shrink the object, positive means That the object grows
		
		if (scale.x < 0.0)
		{
			scale.x = - 1 / ( scale.x - 1 ) - 1;
		}

		if (scale.y < 0.0)
		{
			scale.y = - 1 / ( scale.y - 1 ) - 1;
		}

		if (scale.z < 0.0)
		{
			scale.z = - 1 / ( scale.z - 1 ) - 1;
		}

		scale += Vector3( 1, 1, 1 );

		// Set the scale values on the proxies
		// This is used for the feedback display of the scale
		if (scaleX_)
		{
			scaleX_->set( scale.x, true );
		}

		if (scaleY_)
		{
			scaleY_->set( scale.y, true );
		}

		if (scaleZ_)
		{
			scaleZ_->set( scale.z, true );
		}

		scalerHelper_.updateScale( scale );
	}
}


/**
 *	Key event method
 */
bool PropertyScaler::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// Cancel the scale operation
		scalerHelper_.fini( false );

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: MatrixRotator
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( MatrixRotator )

PY_BEGIN_METHODS( MatrixRotator )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MatrixRotator )
PY_END_ATTRIBUTES()


MatrixRotator::MatrixRotator( MatrixProxyPtr pMatrix,  
			PyTypePlus * pType ) :
	ToolFunctor( pType ),
	grabOffsetSet_( false ),
	invGrabOffset_( Matrix::identity ),
	initialToolLocation_( Vector3::zero() )
{
}

/**
 *	This method updates the rotation
 *	@param dTime - unused
 *	@param tool the tool used to manipulate the rotation
 */
void MatrixRotator::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ))
	{
		// Tell our rotation helper that we are finished
		rotaterHelper_.fini( true );

		UndoRedo::instance().barrier( "Rotation", true );

		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	if (tool.locator())
	{
		// If this is our first time in update, grab our frame of reference etc
		if (!grabOffsetSet_)
		{
			grabOffsetSet_ = true;

			// Get the initial picked position on the tool
			Matrix grabOffset = tool.locator()->transform();

			// Normalise the tool transform
			grabOffset[0].normalise();
			grabOffset[1].normalise();
			grabOffset[2].normalise();

			Vector3 toolLocation = grabOffset.applyToOrigin();

			// Make the graboffset the actual gizmo location
			grabOffset.translation( CurrentRotationProperties::averageOrigin() );

			// Store the inverse of the tool transform
			invGrabOffset_.invert( grabOffset );

			// Get the initial tool location in gizmo space
			initialToolLocation_ = invGrabOffset_.applyPoint( toolLocation );
			initialToolLocation_.normalise();

			// init the rotater helper
			rotaterHelper_.init( grabOffset );
		}

		// Get the current tool location
		Vector3 currentToolLocation = invGrabOffset_.applyPoint( 
			tool.locator()->transform().applyToOrigin() );

		currentToolLocation.normalise();

		// If the angle between the current tool location and the initial
		// tool location is less than 0.5 degrees we don't do anything,
		// this is because we use the cross product between the tool
		// locations to work out the rotation axis, and if the two directions
		// are very similar we can't get the axis
		if (fabsf(currentToolLocation.dotProduct( initialToolLocation_ ) )
			< cos( DEG_TO_RAD( 0.5f ) ))
		{
			// Get the axis we are rotating about
			Vector3 axis = initialToolLocation_.crossProduct(
				currentToolLocation );
			axis.normalise();

			// Calculate the angle to rotate
			float angle = acosf( Math::clamp( -1.f, 
				currentToolLocation.dotProduct( initialToolLocation_ ), 1.f ));

			// Do angle snapping
			float snapAmount = SnapProvider::instance()->angleSnapAmount() / 180 * MATH_PI;

			if( snapAmount != 0.0f )
			{
				float snapAngle = int( angle / snapAmount ) * snapAmount;
				if( angle - snapAngle >= snapAmount / 2 )
					angle = snapAngle + snapAmount;
				else
					angle = snapAngle;
			}

			// Update rotation
			rotaterHelper_.updateRotation( angle, axis );
		}
	}
}


/**
 *	Key event method
 */
bool MatrixRotator::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// Cancel the rotate operation
		rotaterHelper_.fini( false );

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: DynamicFloatDevice
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( DynamicFloatDevice )

PY_BEGIN_METHODS( DynamicFloatDevice )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( DynamicFloatDevice )
PY_END_ATTRIBUTES()

/**
 *	Constructor.
 */
DynamicFloatDevice::DynamicFloatDevice( MatrixProxyPtr pCenter,
				   FloatProxyPtr pFloat,
				   float adjFactor,
				   PyTypePlus * pType ) :
	ToolFunctor( pType ),
	pCenter_( pCenter ),
	pFloat_( pFloat ),
	grabOffset_( 0, 0, 0 ),
	grabOffsetSet_( false ),
	adjFactor_( adjFactor )
{
	BW_GUARD;

	initialFloat_ = pFloat->get();
	pCenter->getMatrix( initialCenter_, true );
}


/**
 *	Update method
 */
void DynamicFloatDevice::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ))
	{
		// set its value permanently
		float finalValue = pFloat_->get();

		pFloat_->set( initialFloat_, true );
		pFloat_->set( finalValue, false );

		if (UndoRedo::instance().barrierNeeded())
			UndoRedo::instance().barrier( "Scale", false );

		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	// figure out radius
	if (tool.locator())
	{
		// Raymond, for bug 4734, the scale function has been changed
		// Now it will be smooth near 1:1 and quick when far away from 1:1
		// Also it meets 0 in the middle of the RadiusGizmo
		if (!grabOffsetSet_)
		{
			grabOffsetSet_ = true;
			grabOffset_ = tool.locator()->transform().applyToOrigin() - initialCenter_.applyToOrigin();

			Vector4 v4Locator( tool.locator()->transform().applyToOrigin(), 1. );

			Moo::rc().viewProjection().applyPoint( v4Locator, v4Locator );
			grabOffset_[0] = v4Locator[0] / v4Locator[3];
			grabOffset_[1] = v4Locator[1] / v4Locator[3];
			grabOffset_[2] = v4Locator[2] / v4Locator[3];

			Vector4 v4InitialCenter( initialCenter_.applyToOrigin(), 1 );

			Moo::rc().viewProjection().applyPoint( v4InitialCenter, v4InitialCenter );
			grabOffset_[0] -= v4InitialCenter[0] / v4InitialCenter[3];
			grabOffset_[1] -= v4InitialCenter[1] / v4InitialCenter[3];
			grabOffset_[2] -= v4InitialCenter[2] / v4InitialCenter[3];
			grabOffset_[2] = 0;
		}

		Vector3 offset = (tool.locator()->transform().applyToOrigin() - initialCenter_.applyToOrigin());

		Vector4 v4Locator( tool.locator()->transform().applyToOrigin(), 1. );

		Moo::rc().viewProjection().applyPoint( v4Locator, v4Locator );
		offset[0] = v4Locator[0] / v4Locator[3];
		offset[1] = v4Locator[1] / v4Locator[3];
		offset[2] = v4Locator[2] / v4Locator[3];

		Vector4 v4InitialCenter( initialCenter_.applyToOrigin(), 1 );

		Moo::rc().viewProjection().applyPoint( v4InitialCenter, v4InitialCenter );
		offset[0] -= v4InitialCenter[0] / v4InitialCenter[3];
		offset[1] -= v4InitialCenter[1] / v4InitialCenter[3];
		offset[2] -= v4InitialCenter[2] / v4InitialCenter[3];
		offset[2] = 0;

		float ratio = offset.length() / grabOffset_.length();
		if (initialFloat_ == 0.0f) // Bug 5153 fix: There was a problem if the initial radius was 0.0.
		{
			pFloat_->set( initialFloat_ + ((offset.length() - grabOffset_.length()) * adjFactor_), true );
		}
		else if( ratio < 1.0f )
		{
			pFloat_->set( initialFloat_ * ( 1 - ( ratio - 1 ) * ( ratio - 1 ) ), true );
		}
		else
		{
			pFloat_->set( initialFloat_ * ratio * ratio, true );
		}
	}
}


/**
 *	Key event method
 */
bool DynamicFloatDevice::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// set the item back to it's original pose
		pFloat_->set( initialFloat_, true );

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
// Section: WheelRotator
// -----------------------------------------------------------------------------


PY_TYPEOBJECT( WheelRotator )

PY_BEGIN_METHODS( WheelRotator )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( WheelRotator )
PY_END_ATTRIBUTES()

PY_FACTORY( WheelRotator, Functor )

WheelRotator::WheelRotator( PyTypePlus * pType ) :
	timeSinceInput_(0.0f),
	rotAmount_(0.0f),
	needsInit_(true)
{
	BW_GUARD;

	curProperties_ = CurrentRotationProperties::properties();
}


bool WheelRotator::arePropertiesValid() const
{
	BW_GUARD;

	return curProperties_ == CurrentRotationProperties::properties();
}


void WheelRotator::update( float dTime, Tool& tool )
{
	BW_GUARD;

	if (!arePropertiesValid())
	{
		ToolManager::instance().popTool();
		return;
	}

	if (needsInit_)
	{
		needsInit_ = false;

		// Get the initial picked position on the tool
		Matrix grabOffset = tool.locator()->transform();
		rotaterHelper_.init( grabOffset );
	}

	// The wheelrotator always rotates around the y-axis
	Vector3 axis(0, 1, 0);

	float angle = DEG_TO_RAD( rotAmount_ );

	// Snap the angle
	float snapAmount = SnapProvider::instance()->angleSnapAmount() / 180 * MATH_PI;

	if (snapAmount != 0.0f)
	{
		float snapAngle = int( angle / snapAmount ) * snapAmount;
		if( angle - snapAngle >= snapAmount / 2 )
			angle = snapAngle + snapAmount;
		else
			angle = snapAngle;
	}

	// Update the rotation
	rotaterHelper_.updateRotation( angle, axis );

	// Automatically commit after 750ms of no input
	if (timeSinceInput_ > 0.75f)
	{
		commitChanges();

		ToolManager::instance().popTool();
	}
	
	// update the time since input value
	timeSinceInput_ += dTime;
}

bool WheelRotator::handleMouseEvent( const MouseEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!arePropertiesValid())
		return false;


	if (event.dz() != 0)
	{
		timeSinceInput_ = 0.0f;

		// Get the direction only, we don't want the magnitude
		float amnt = (event.dz() > 0) ? -1.0f : 1.0f;

		// Move at 1deg/click with the button down, 15 degs/click otherwise
		if (!InputDevices::instance().isKeyDown( KeyCode::KEY_MIDDLEMOUSE ))
			amnt *= 15.0f;

		rotAmount_ += amnt;

		return true;
	}
	else
	{
		// commit the rotation now, so we don't have to wait for the timeout
		commitChanges();

		ToolManager::instance().popTool();
	}

	return false;
}

bool WheelRotator::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!arePropertiesValid())
		return false;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		rotaterHelper_.fini( false );
		
		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	if (event.key() == KeyCode::KEY_LEFTMOUSE || event.key() == KeyCode::KEY_RIGHTMOUSE)
	{
		commitChanges();

		ToolManager::instance().popTool();
	}

	return false;
}

/**
 *	This helper method commits the changes that have been done 
 *	to the rotation and sets up the undo barrier
 */
void WheelRotator::commitChanges()
{
	BW_GUARD;

	rotaterHelper_.fini(true);

	UndoRedo::instance().barrier( "Rotation", false );

	needsInit_ = true;
	rotAmount_ = 0;
}

PyObject * WheelRotator::pyNew( PyObject * args )
{
	BW_GUARD;

	return new WheelRotator();
}



// -----------------------------------------------------------------------------
// Section: MatrixShaker
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( MatrixPositioner )

PY_BEGIN_METHODS( MatrixPositioner )
	PY_METHOD( setUndoName )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( MatrixPositioner )
PY_END_ATTRIBUTES()

PY_FACTORY( MatrixPositioner, Functor )



/**
 *	Constructor.
 */
MatrixPositioner::MatrixPositioner( MatrixProxyPtr pMatrix, PyTypePlus * pType ) :
	ToolFunctor( pType ),
	lastLocatorPos_( Vector3::zero() ),
	totalLocatorOffset_( Vector3::zero() ),
	gotInitialLocatorPos_( false )
{
	BW_GUARD;

	std::vector<GenPositionProperty*> props = CurrentPositionProperties::properties();
	for (std::vector<GenPositionProperty*>::iterator i = props.begin(); i != props.end(); ++i)
	{
		(*i)->pMatrix()->recordState();
	}

	matrix_ = pMatrix;
}



PyObject* MatrixPositioner::py_setUndoName( PyObject* args )
{
	BW_GUARD;

	// parse arguments
	char* str;
	if (!PyArg_ParseTuple( args, "s", &str ))	{
		PyErr_SetString( PyExc_TypeError, "setUndoName() "
			"expects a string argument" );
		return NULL;
	}

	undoName_ = str;

	Py_Return;
}


/**
 *	Update method
 */
void MatrixPositioner::update( float dTime, Tool& tool )
{
	BW_GUARD;

	// see if we want to commit this action
	if (!InputDevices::isKeyDown( KeyCode::KEY_LEFTMOUSE ))
	{
		if (matrix_->hasChanged())
		{
			// set its transform permanently
			matrix_->commitState( false );
		}
		else
		{
			matrix_->commitState( true );
		}


		// and this tool's job is over
		ToolManager::instance().popTool();
		return;
	}

	// figure out movement
	if (tool.locator())
	{
		if (!gotInitialLocatorPos_)
		{
			lastLocatorPos_ = tool.locator()->transform().applyToOrigin();
			gotInitialLocatorPos_ = true;
		}

		totalLocatorOffset_ += tool.locator()->transform().applyToOrigin() - lastLocatorPos_;
		lastLocatorPos_ = tool.locator()->transform().applyToOrigin();


		// reset the last change we made
		matrix_->commitState( true );


		Matrix m;
		matrix_->getMatrix( m );

		Vector3 newPos = m.applyToOrigin() + totalLocatorOffset_;

		SnapProvider::instance()->snapPosition( newPos );

		m.translation( newPos );

		Matrix worldToLocal;
		matrix_->getMatrixContextInverse( worldToLocal );

		m.postMultiply( worldToLocal );


		matrix_->setMatrix( m );
	}
}


/**
 *	Key event method
 */
bool MatrixPositioner::handleKeyEvent( const KeyEvent & event, Tool& tool )
{
	BW_GUARD;

	if (!event.isKeyDown()) return false;

	if (event.key() == KeyCode::KEY_ESCAPE)
	{
		// Set the items back to their original states
		matrix_->commitState( true );

		// and we're that's it from us
		ToolManager::instance().popTool();
		return true;
	}

	return false;
}


/**
 *	Factory method
 */
PyObject * MatrixPositioner::pyNew( PyObject * args )
{
	BW_GUARD;

	if (CurrentPositionProperties::properties().empty())
	{
		PyErr_Format( PyExc_ValueError, "MatrixPositioner()  No current editor" );
		return NULL;
	}

	return new MatrixPositioner( NULL );
}

// item_functor.cpp
